#ifndef __CLIENT_MNG_H__
#define __CLIENT_MNG_H__

#include "protocol.h"
#include "client_net.h"

typedef struct Client Client;

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

Client* ClientCreate();

void ClientDestroy(Client* _client);


/*===========================================================================*/
/*============================== AUTH ACTIONS ===============================*/
/*===========================================================================*/

ClientMngStatus ClientMngRegister(const char* _username, const char* _password);

ClientMngStatus ClientMngLogin(const char* _username, const char* _password);

ClientMngStatus ClientMngLogout();


#endif /* __CLIENT_MNG_H__ */