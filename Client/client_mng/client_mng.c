#include "client_mng.h"

struct Client
{
    ClientState state;

    int server_socket_fd;

    char username[UNAME_MAX_LEN + 1];

    char* address;

    uint16_t port;

    uint8_t send_buffer[MAX_BUFFER_SIZE];

    uint8_t recv_buffer[MAX_BUFFER_SIZE];
};

static Client* client;


ClientMngStatus ClientMngInit()
{
    client = malloc(sizeof(Client));

    if (client == NULL)
    {
        return CM_ALLOCATION_ERROR;
    }

    client->address = malloc(sizeof(strlen(ADDRESS)+1));

    if (client->address == NULL)
    {
        free(client);
        return CM_ALLOCATION_ERROR;
    }

    client->state = CLIENT_DISCONNECTED;

    client->server_socket_fd = -1;

    strcpy(client->address, SERVER_IP);

    client->port = SERVER_PORT;

    return CM_SUCCESS;
}


void ClientMngDestroy()
{
    if (client == NULL)
    {
        return;
    }

    free(client->address);

    free(client);
}


RegRespStatus ClientMngRegister(const char* _username, const char* _password)
{

    RegRespStatus auth_resp;
    int req_msg_size;

    if (client->state == CLIENT_DISCONNECTED)
    {
        if (ClientConnectToServer(&(client->server_socket_fd), client->address, client->port) != CN_SUCCESS)
            return REG_SYSTEM_ERROR;

        client->state = CLIENT_CONNECTED;
    }

    if ((req_msg_size = ProtocolBuildAuthReq(client->send_buffer, MSG_REG_REQ, _username, _password)) < 0)
        return REG_SYSTEM_ERROR;


    if (ClientNetSend(client->server_socket_fd, client->send_buffer, req_msg_size) != CN_SUCCESS)
        return REG_SYSTEM_ERROR;


    if (ClientNetRecv(client->server_socket_fd, client->recv_buffer) != CN_SUCCESS)
        return REG_SYSTEM_ERROR;

    if (ProtocolParseAuthResp(client->recv_buffer, (uint8_t*)&auth_resp) != PROTOCOL_SUCCESS)
        return REG_SYSTEM_ERROR;

    if (auth_resp == REG_SUCCESS)
    {
        client->state = CLIENT_LOGGED_IN;
        strcpy(client->username, _username);
    }
        
    return auth_resp;
}

LoginRespStatus ClientMngLogin(const char* _username, const char* _password)
{
    LoginRespStatus auth_resp;
    int req_msg_size;

    if (client->state == CLIENT_DISCONNECTED)
    {
        if (ClientConnectToServer(&(client->server_socket_fd), client->address, client->port) != CN_SUCCESS)
            return LOGIN_SYSTEM_ERROR;

        client->state = CLIENT_CONNECTED;
    }

    if ((req_msg_size = ProtocolBuildAuthReq(client->send_buffer, MSG_LOGIN_REQ, _username, _password)) < 0)
        return LOGIN_SYSTEM_ERROR;


    if (ClientNetSend(client->server_socket_fd, client->send_buffer, req_msg_size) != CN_SUCCESS)
        return LOGIN_SYSTEM_ERROR;


    if (ClientNetRecv(client->server_socket_fd, client->recv_buffer) != CN_SUCCESS)
        return LOGIN_SYSTEM_ERROR;

    if (ProtocolParseAuthResp(client->recv_buffer, (uint8_t*)&auth_resp) != PROTOCOL_SUCCESS)
        return LOGIN_SYSTEM_ERROR;

    if (auth_resp == LOGIN_SUCCESS)
        {
            client->state = CLIENT_LOGGED_IN;
            strcpy(client->username, _username);
        }
    return auth_resp;
}

LogoutRespStatus ClientMngLogout()
{
    LogoutRespStatus logout_resp;
    int req_msg_size;

    if (client->state == CLIENT_CONNECTED)
        return LOGOUT_SUCCESS;
    
    if ((req_msg_size = ProtocolBuildLogoutReq(client->send_buffer)) < 0)
        return LOGOUT_SYSTEM_ERROR;

    if (ClientNetSend(client->server_socket_fd, client->send_buffer, req_msg_size) != CN_SUCCESS)
        return LOGOUT_SYSTEM_ERROR;

    if (ClientNetRecv(client->server_socket_fd, client->recv_buffer) != CN_SUCCESS)
        return LOGOUT_SYSTEM_ERROR;

    if (ProtocolParseLogoutResp(client->recv_buffer, &logout_resp) != PROTOCOL_SUCCESS)
        return LOGOUT_SYSTEM_ERROR;

    if (logout_resp == LOGOUT_SUCCESS)
    {
        ClientDisconnectFromServer(&(client->server_socket_fd));
        client->state = CLIENT_DISCONNECTED;
        memset(client->username, 0, sizeof(client->username));
    }

    return logout_resp;
}