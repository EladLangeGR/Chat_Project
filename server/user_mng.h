#ifndef __USER_MNG_H__
#define __USER_MNG_H__

#include "protocol.h"
#include "user.h"

typedef struct UserMng UserMng;

/**
 * @brief Create user manager
 * @return UserMng* on success, NULL on failure
 */
UserMng* UserMng_Create(void);

/**
 * @brief Destroy user manager and free all memory
 */
void UserMng_Destroy(UserMng** _mng);

/**
 * @brief Register a new user
 * @return RegRespStatus result code
 */
RegRespStatus UserMng_Register(UserMng* _mng, const char* _name, const char* _password);

/**
 * @brief Login an existing user
 * @param _sockfd  TCP socket of the connecting client
 * @return LoginRespStatus result code
 */
LoginRespStatus UserMng_Login(UserMng* _mng, const char* _name, const char* _password, int _sockfd);

/**
 * @brief Logout a user (sets inactive, clears socket)
 * @return LogoutRespStatus result code
 */
LogoutRespStatus UserMng_Logout(UserMng* _mng, const char* _name);

/**
 * @brief Retrieve a User pointer by name
 * @return User* or NULL if not found
 */
User* UserMng_GetUser(UserMng* _mng, const char* _name);

#endif /* __USER_MNG_H__ */
