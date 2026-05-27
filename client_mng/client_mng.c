#include "client_mng.h"

struct Client
{
    ClientState state;

    int server_socket_fd;

    char username[UNAME_MAX_LEN + 1];
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
    uint8_t buffer[BUFFER_SIZE];

    if (_client == NULL)
    {
        return CM_PROTOCOL_ERROR;
    }

    if (_client->state == CLIENT_DISCONNECTED)
    {
        if (ConnectToServer(_client) != CN_SUCCESS)
        {
            return CM_CONNECTION_FAILED;
        }

        _client->state = CLIENT_CONNECTED;
    }

    if (ProtocolBuildAuthReq(buffer, MSG_REG_REQ, _username, _password) != PROTOCOL_SUCCESS)
    {
        return CM_PROTOCOL_ERROR;
    }

    /*
        send
        recv
        parse response
    */

    return CM_SUCCESS;
}