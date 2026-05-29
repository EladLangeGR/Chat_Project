#ifndef __CLIENT_MNG_H__
#define __CLIENT_MNG_H__

#include "protocol.h"
#include "client_net.h"
#include <signal.h>
#include <sys/types.h>

typedef struct Client Client;

typedef struct Group Group;

typedef enum
{
    CLIENT_DISCONNECTED,
    CLIENT_CONNECTED,
    CLIENT_LOGGED_IN

} ClientState;

typedef enum
{
    CM_SUCCESS,

    CM_CONNECTION_FAILED,

    CM_SEND_FAILED,

    CM_RECV_FAILED,

    CM_PROTOCOL_ERROR,

    CM_REG_FAILED,

    CM_LOGIN_FAILED,
    
    CM_ALLOCATION_ERROR

} ClientMngStatus;


/*===========================================================================*/
/*============================== CLIENT LIFECYCLE ===========================*/
/*===========================================================================*/

ClientMngStatus ClientMngInit();

void ClientMngDestroy();


/*===========================================================================*/
/*============================== AUTH ACTIONS ===============================*/
/*===========================================================================*/

RegRespStatus ClientMngRegister(const char* _username, const char* _password);

LoginRespStatus ClientMngLogin(const char* _username, const char* _password);

LogoutRespStatus ClientMngLogout();

/*===========================================================================*/
/*============================== GROUP ACTIONS ==============================*/
/*===========================================================================*/

CreateGroupRespStatus ClientMngCreateGroup(const char* _group_name);

JoinGroupRespStatus ClientMngJoinGroup(const char* _group_name);

ExitGroupRespStatus ClientMngExitGroup(const char* _group_name);

#endif /* __CLIENT_MNG_H__ */