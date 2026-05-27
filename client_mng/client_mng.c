#include "client_mng.h"

struct Client
{
    ClientState state;

    int server_socket_fd;

    char username[UNAME_MAX_LEN + 1];

    uint8_t send_buffer[MAX_BUFFER_SIZE];

    uint8_t recv_buffer[MAX_BUFFER_SIZE];
};


Client* ClientCreate()
{
    Client* new_client;

    new_client = malloc(sizeof(Client));

    if (new_client == NULL)
    {
        return NULL;
    }

    new_client->state = CLIENT_DISCONNECTED;

    new_client->server_socket_fd = -1;

    return new_client;
}


void ClientDestroy(Client* _client)
{
    if (_client == NULL)
    {
        return;
    }

    free(_client);
}


ClientMngStatus ClientMngRegister(Client* _client, const char* _username, const char* _password)
{

    ClientMngStatus ret_status;
    RegRespStatus auth_resp;

    if (_client == NULL)
    {
        return CM_UNINITIALIZED_ERROR;
    }

    if (_client->state == CLIENT_DISCONNECTED)
    {
        if (ConnectToServer(&(_client->server_socket_fd)) != CN_SUCCESS)
            return CM_CONNECTION_FAILED;

        _client->state = CLIENT_CONNECTED;
    }

    if (ProtocolBuildAuthReq(_client->send_buffer, MSG_REG_REQ, _username, _password) != PROTOCOL_SUCCESS)
        return CM_PROTOCOL_ERROR;


    if (ClientNetSend(_client->server_socket_fd, _client->send_buffer) != CN_SUCCESS)
        return CM_SEND_FAILED;


    if (ClientNetRecv(_client->server_socket_fd, _client->recv_buffer) != CN_SUCCESS)
        return CM_RECV_FAILED;

    if (ProtocolParseAuthResp(_client->recv_buffer, &auth_resp) != PROTOCOL_SUCCESS)
        return CM_PROTOCOL_ERROR;

    return CM_SUCCESS;
}