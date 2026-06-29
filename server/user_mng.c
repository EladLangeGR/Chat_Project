#include <stdlib.h>
#include <string.h>
#include "user_mng.h"
#include "gen_hash.h"

#define USER_HASH_SIZE 100

struct UserMng {
    HashMap* m_users; /* key: user->name (char*) -> value: User* */
};

/*===========================================================================*/
/*                         STATIC HELPER FUNCTIONS                           */
/*===========================================================================*/

static size_t StringHash(void* _key)
{
    unsigned char* str = (unsigned char*)_key;
    size_t hash = 5381;
    while (*str)
        hash = hash * 33 + *str++;
    return hash;
}

static int StringEqual(void* _a, void* _b)
{
    return strcmp((const char*)_a, (const char*)_b) == 0;
}

/*===========================================================================*/
/*                              PUBLIC API                                   */
/*===========================================================================*/

UserMng* UserMng_Create(void)
{
    UserMng* mng = (UserMng*)malloc(sizeof(UserMng));
    if (!mng) return NULL;

    mng->m_users = HashMap_Create(USER_HASH_SIZE, StringHash, StringEqual);
    if (!mng->m_users) {
        free(mng);
        return NULL;
    }
    return mng;
}

void UserMng_Destroy(UserMng** _mng)
{
    if (!_mng || !*_mng) return;
    /* free=NULL for keys (they live inside User->name), free User structs as values */
    HashMap_Destroy(&(*_mng)->m_users, NULL, free);
    free(*_mng);
    *_mng = NULL;
}

RegRespStatus UserMng_Register(UserMng* _mng, const char* _name, const char* _password)
{
    User* user;
    void* found;

    if (!_mng || !_name || !_password)
        return REG_INVALID_ARGUMENTS;

    /* Check for duplicate */
    if (HashMap_Find(_mng->m_users, (void*)_name, &found) == MAP_SUCCESS)
        return REG_USER_EXISTS;

    user = (User*)malloc(sizeof(User));
    if (!user) return REG_INVALID_ARGUMENTS;

    strncpy(user->name,     _name,     UNAME_MAX_LEN);
    user->name[UNAME_MAX_LEN] = '\0';
    strncpy(user->password, _password, PSWD_MAX_LEN);
    user->password[PSWD_MAX_LEN] = '\0';
    user->sockfd    = -1;
    user->is_active =  0;

    if (HashMap_Insert(_mng->m_users, user->name, user) != MAP_SUCCESS) {
        free(user);
        return REG_INVALID_ARGUMENTS;
    }
    return REG_SUCCESS;
}

LoginRespStatus UserMng_Login(UserMng* _mng, const char* _name, const char* _password, int _sockfd)
{
    User* user;

    if (!_mng || !_name || !_password)
        return LOGIN_INVALID_ARGUMENTS;

    if (HashMap_Find(_mng->m_users, (void*)_name, (void**)&user) != MAP_SUCCESS)
        return LOGIN_NO_SUCH_USER;

    if (strcmp(user->password, _password) != 0)
        return LOGIN_WRONG_PASSWORD;

    if (user->is_active)
        return LOGIN_USER_ALREADY_ACTIVE;

    user->sockfd    = _sockfd;
    user->is_active = 1;
    return LOGIN_SUCCESS;
}

LogoutRespStatus UserMng_Logout(UserMng* _mng, const char* _name)
{
    User* user;

    if (!_mng || !_name)
        return LOGOUT_INVALID_ARGUMENTS;

    if (HashMap_Find(_mng->m_users, (void*)_name, (void**)&user) != MAP_SUCCESS)
        return LOGOUT_USER_NOT_ACTIVE;

    if (!user->is_active)
        return LOGOUT_USER_NOT_ACTIVE;

    user->sockfd    = -1;
    user->is_active =  0;
    return LOGOUT_SUCCESS;
}

User* UserMng_GetUser(UserMng* _mng, const char* _name)
{
    User* user;
    if (!_mng || !_name) return NULL;
    if (HashMap_Find(_mng->m_users, (void*)_name, (void**)&user) != MAP_SUCCESS)
        return NULL;
    return user;
}
