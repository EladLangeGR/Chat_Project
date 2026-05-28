#include "client_net.h"
#include "status_defs.h"
#include <stdio.h>


ClientNetworkMessage ClientConnectToServer(int* _sock_fd, const char* _address, uint16_t _port)
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
    sock_addr.sin_addr.s_addr = inet_pton(AF_INET, _address, &sock_addr.sin_addr);
    sock_addr.sin_port = htons(_port);

    printf("Connecting to %s:%d\n", _address, _port);

    // Connection
    if (connect(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr))<0)
    {
        perror("connect");
        return CN_CONNECTION_TO_SERVER_FAILURE;
    }

    *_sock_fd = sock;

    return CN_SUCCESS;
}

void ClientDisconnectFromServer(int* _sock_fd)
{
    close(*_sock_fd); 
} 

ClientNetworkMessage ClientNetSend(int _sock_fd, uint8_t* _buffer, int msg_size)
{
    ssize_t sent_bytes;

    if ((sent_bytes = send(_sock_fd, _buffer, msg_size, 0)) != msg_size)
        return CN_SEND_FAIL;
    
    return CN_SUCCESS;
}

ClientNetworkMessage ClientNetRecv(int _sock_fd, uint8_t* _buffer)
{
    ssize_t recv_bytes;
    uint8_t payload_size;

    if ((recv_bytes = recv(_sock_fd ,_buffer ,HEADER_SIZE, MSG_WAITALL)) < 0)
        return CN_RECV_FAIL;

    payload_size = _buffer[1];

    if ((recv_bytes = recv(_sock_fd ,&_buffer[HEADER_SIZE] ,payload_size, MSG_WAITALL)) < 0)
        return CN_RECV_FAIL;

    return CN_SUCCESS;
}