#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "server_mng.h"
#include "user_mng.h"
#include "group_mng.h"
#include "protocol.h"

#define MAX_FD 1024

struct ServerMng {
    UserMng*        m_userMng;
    GroupMng*       m_groupMng;
    char*           m_fdToUser[MAX_FD]; /* sockfd -> username (heap string) */
    pthread_mutex_t m_userLock;
    pthread_mutex_t m_groupLock;
    pthread_mutex_t m_fdLock;
};

/*===========================================================================*/
/*                     INTERNAL fd <-> username HELPERS                     */
/*===========================================================================*/

static char* GetUsernameByFd(ServerMng* _mng, int _sockfd)
{
    char* name = NULL;
    pthread_mutex_lock(&_mng->m_fdLock);
    if (_sockfd >= 0 && _sockfd < MAX_FD && _mng->m_fdToUser[_sockfd])
        name = strdup(_mng->m_fdToUser[_sockfd]);
    pthread_mutex_unlock(&_mng->m_fdLock);
    return name;
}

static void SetUsernameByFd(ServerMng* _mng, int _sockfd, const char* _username)
{
    pthread_mutex_lock(&_mng->m_fdLock);
    if (_sockfd >= 0 && _sockfd < MAX_FD) {
        free(_mng->m_fdToUser[_sockfd]);
        _mng->m_fdToUser[_sockfd] = _username ? strdup(_username) : NULL;
    }
    pthread_mutex_unlock(&_mng->m_fdLock);
}

/*===========================================================================*/
/*                          INTERNAL LOGOUT HELPER                          */
/*===========================================================================*/

static void LogoutSocket(ServerMng* _mng, int _sockfd)
{
    char* username = GetUsernameByFd(_mng, _sockfd);
    if (!username) return;

    pthread_mutex_lock(&_mng->m_groupLock);
    GroupMng_RemoveUserFromAll(_mng->m_groupMng, username);
    pthread_mutex_unlock(&_mng->m_groupLock);

    pthread_mutex_lock(&_mng->m_userLock);
    UserMng_Logout(_mng->m_userMng, username);
    pthread_mutex_unlock(&_mng->m_userLock);

    SetUsernameByFd(_mng, _sockfd, NULL);
    free(username);
}

/*===========================================================================*/
/*                          MESSAGE HANDLERS                                 */
/*===========================================================================*/

static void HandleRegister(ServerMng* _mng, int _sockfd, uint8_t* _buffer)
{
    char          username[UNAME_MAX_LEN + 1] = {0};
    char          password[PSWD_MAX_LEN  + 1] = {0};
    RegRespStatus status;
    uint8_t       resp[MAX_BUFFER_SIZE];
    int           msg_size;

    if (ProtocolParseAuthReq(_buffer, username, password) != PROTOCOL_SUCCESS)
        status = REG_INVALID_ARGUMENTS;
    else {
        pthread_mutex_lock(&_mng->m_userLock);
        status = UserMng_Register(_mng->m_userMng, username, password);
        pthread_mutex_unlock(&_mng->m_userLock);
    }

    msg_size = ProtocolBuildAuthResp(resp, MSG_REG_RESP, (uint8_t)status);
    ServerNet_SendMsg(_sockfd, resp, msg_size);
}

static void HandleLogin(ServerMng* _mng, int _sockfd, uint8_t* _buffer)
{
    char            username[UNAME_MAX_LEN + 1] = {0};
    char            password[PSWD_MAX_LEN  + 1] = {0};
    LoginRespStatus status;
    uint8_t         resp[MAX_BUFFER_SIZE];
    int             msg_size;

    if (ProtocolParseAuthReq(_buffer, username, password) != PROTOCOL_SUCCESS)
        status = LOGIN_INVALID_ARGUMENTS;
    else {
        pthread_mutex_lock(&_mng->m_userLock);
        status = UserMng_Login(_mng->m_userMng, username, password, _sockfd);
        pthread_mutex_unlock(&_mng->m_userLock);

        if (status == LOGIN_SUCCESS)
            SetUsernameByFd(_mng, _sockfd, username);
    }

    msg_size = ProtocolBuildAuthResp(resp, MSG_LOGIN_RESP, (uint8_t)status);
    ServerNet_SendMsg(_sockfd, resp, msg_size);
}

static void HandleLogout(ServerMng* _mng, int _sockfd, uint8_t* _buffer)
{
    LogoutRespStatus status;
    uint8_t          resp[MAX_BUFFER_SIZE];
    int              msg_size;
    char*            username;

    (void)_buffer;

    username = GetUsernameByFd(_mng, _sockfd);
    if (!username) {
        status = LOGOUT_USER_NOT_ACTIVE;
    } else {
        free(username);
        LogoutSocket(_mng, _sockfd);
        status = LOGOUT_SUCCESS;
    }

    msg_size = ProtocolBuildLogoutResp(resp, status);
    ServerNet_SendMsg(_sockfd, resp, msg_size);
}

static void HandleCreateGroup(ServerMng* _mng, int _sockfd, uint8_t* _buffer)
{
    char                  group_name[GROUP_NAME_MAX_LEN + 1] = {0};
    char                  mc_ip[16]                          = {0};
    CreateGroupRespStatus status;
    uint8_t               resp[MAX_BUFFER_SIZE];
    int                   msg_size;
    char*                 username;
    User*                 user;

    if (ProtocolParseGroupReq(_buffer, group_name) != PROTOCOL_SUCCESS) {
        msg_size = ProtocolBuildGroupResp(resp, MSG_CREATE_GROUP_RESP,
                                          CREATE_GROUP_INVALID_ARGUMENTS, "");
        ServerNet_SendMsg(_sockfd, resp, msg_size);
        return;
    }

    username = GetUsernameByFd(_mng, _sockfd);
    if (!username) {
        msg_size = ProtocolBuildGroupResp(resp, MSG_CREATE_GROUP_RESP,
                                          CREATE_GROUP_INVALID_ARGUMENTS, "");
        ServerNet_SendMsg(_sockfd, resp, msg_size);
        return;
    }

    pthread_mutex_lock(&_mng->m_userLock);
    user = UserMng_GetUser(_mng->m_userMng, username);
    pthread_mutex_unlock(&_mng->m_userLock);

    pthread_mutex_lock(&_mng->m_groupLock);
    status = GroupMng_CreateGroup(_mng->m_groupMng, group_name, username, user, mc_ip);
    pthread_mutex_unlock(&_mng->m_groupLock);

    free(username);

    msg_size = ProtocolBuildGroupResp(resp, MSG_CREATE_GROUP_RESP, (uint8_t)status,
                                      (status == CREATE_GROUP_SUCCESS) ? mc_ip : "");
    ServerNet_SendMsg(_sockfd, resp, msg_size);
}

static void HandleJoinGroup(ServerMng* _mng, int _sockfd, uint8_t* _buffer)
{
    char                group_name[GROUP_NAME_MAX_LEN + 1] = {0};
    char                mc_ip[16]                          = {0};
    JoinGroupRespStatus status;
    uint8_t             resp[MAX_BUFFER_SIZE];
    int                 msg_size;
    char*               username;
    User*               user;

    if (ProtocolParseGroupReq(_buffer, group_name) != PROTOCOL_SUCCESS) {
        msg_size = ProtocolBuildGroupResp(resp, MSG_JOIN_GROUP_RESP,
                                          JOIN_GROUP_INVALID_ARGUMENTS, "");
        ServerNet_SendMsg(_sockfd, resp, msg_size);
        return;
    }

    username = GetUsernameByFd(_mng, _sockfd);
    if (!username) {
        msg_size = ProtocolBuildGroupResp(resp, MSG_JOIN_GROUP_RESP,
                                          JOIN_GROUP_INVALID_ARGUMENTS, "");
        ServerNet_SendMsg(_sockfd, resp, msg_size);
        return;
    }

    pthread_mutex_lock(&_mng->m_userLock);
    user = UserMng_GetUser(_mng->m_userMng, username);
    pthread_mutex_unlock(&_mng->m_userLock);

    pthread_mutex_lock(&_mng->m_groupLock);
    status = GroupMng_JoinGroup(_mng->m_groupMng, group_name, username, user, mc_ip);
    pthread_mutex_unlock(&_mng->m_groupLock);

    free(username);

    msg_size = ProtocolBuildGroupResp(resp, MSG_JOIN_GROUP_RESP, (uint8_t)status,
                                      (status == JOIN_GROUP_SUCCESS) ? mc_ip : "");
    ServerNet_SendMsg(_sockfd, resp, msg_size);
}

static void HandleExitGroup(ServerMng* _mng, int _sockfd, uint8_t* _buffer)
{
    char                group_name[GROUP_NAME_MAX_LEN + 1] = {0};
    ExitGroupRespStatus status;
    uint8_t             resp[MAX_BUFFER_SIZE];
    int                 msg_size;
    char*               username;

    if (ProtocolParseGroupReq(_buffer, group_name) != PROTOCOL_SUCCESS) {
        msg_size = ProtocolBuildExitGroupResp(resp, EXIT_GROUP_INVALID_ARGUMENTS);
        ServerNet_SendMsg(_sockfd, resp, msg_size);
        return;
    }

    username = GetUsernameByFd(_mng, _sockfd);
    if (!username) {
        msg_size = ProtocolBuildExitGroupResp(resp, EXIT_GROUP_INVALID_ARGUMENTS);
        ServerNet_SendMsg(_sockfd, resp, msg_size);
        return;
    }

    pthread_mutex_lock(&_mng->m_groupLock);
    status = GroupMng_ExitGroup(_mng->m_groupMng, group_name, username);
    pthread_mutex_unlock(&_mng->m_groupLock);

    free(username);

    msg_size = ProtocolBuildExitGroupResp(resp, status);
    ServerNet_SendMsg(_sockfd, resp, msg_size);
}

/*===========================================================================*/
/*                              PUBLIC API                                   */
/*===========================================================================*/

ServerMng* ServerMng_Create(void)
{
    ServerMng* mng = (ServerMng*)calloc(1, sizeof(ServerMng));
    if (!mng) return NULL;

    mng->m_userMng = UserMng_Create();
    if (!mng->m_userMng) { free(mng); return NULL; }

    mng->m_groupMng = GroupMng_Create();
    if (!mng->m_groupMng) {
        UserMng_Destroy(&mng->m_userMng);
        free(mng);
        return NULL;
    }

    pthread_mutex_init(&mng->m_userLock,  NULL);
    pthread_mutex_init(&mng->m_groupLock, NULL);
    pthread_mutex_init(&mng->m_fdLock,    NULL);

    return mng;
}

void ServerMng_Destroy(ServerMng** _mng)
{
    int i;
    if (!_mng || !*_mng) return;

    UserMng_Destroy (&(*_mng)->m_userMng);
    GroupMng_Destroy(&(*_mng)->m_groupMng);

    for (i = 0; i < MAX_FD; i++)
        free((*_mng)->m_fdToUser[i]);

    pthread_mutex_destroy(&(*_mng)->m_userLock);
    pthread_mutex_destroy(&(*_mng)->m_groupLock);
    pthread_mutex_destroy(&(*_mng)->m_fdLock);

    free(*_mng);
    *_mng = NULL;
}

void ServerMng_OnNewClient(int _sockfd, void* _context)
{
    (void)_sockfd;
    (void)_context;
}

void ServerMng_OnGotMsg(int _sockfd, uint8_t* _buffer, void* _context)
{
    ServerMng*  mng  = (ServerMng*)_context;
    MessageType type = (MessageType)_buffer[0];

    switch (type) {
        case MSG_REG_REQ:          HandleRegister   (mng, _sockfd, _buffer); break;
        case MSG_LOGIN_REQ:        HandleLogin      (mng, _sockfd, _buffer); break;
        case MSG_LOGOUT_REQ:       HandleLogout     (mng, _sockfd, _buffer); break;
        case MSG_CREATE_GROUP_REQ: HandleCreateGroup(mng, _sockfd, _buffer); break;
        case MSG_JOIN_GROUP_REQ:   HandleJoinGroup  (mng, _sockfd, _buffer); break;
        case MSG_EXIT_GROUP_REQ:   HandleExitGroup  (mng, _sockfd, _buffer); break;
        default: break;
    }
}

void ServerMng_OnCloseClient(int _sockfd, void* _context)
{
    ServerMng* mng = (ServerMng*)_context;
    LogoutSocket(mng, _sockfd);
}

void ServerMng_OnFail(void* _context)
{
    (void)_context;
}
