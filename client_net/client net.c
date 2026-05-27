#include "client_net.h"

struct Client
{
    ClientState state;
    int server_socket_fd;
};


Client* InitClient()
{
    Client* client;

    client = (Client*)malloc(sizeof(Client));
    client->state = DISCONNECTED;

    return client;
}


ClientNetwokMessage ClientConnectToServer(Client* _client)
{
    int sock;
    struct sockaddr_in sock_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock<0)
    {
        return CN_SOCKET_INIT_FAILURE;
    }

    memset(&sock_addr, 0, sizeof(sock_addr));

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr(ADDRESS);
    sock_addr.sin_port = htons(PORT);

    // Connection
    if (connect(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr))<0)
    {
        return CN_CONNECTION_TO_SERVER_FAILURE;
    }

    _client->server_socket_fd = sock;
    _client->state = CONNECTED;

    return CN_SUCCESS;
}

void ClientDisconnectFromServer(Client* _client)
{
    close(_client->server_socket_fd);
    _client->state = DISCONNECTED;   
} 