#ifndef __CLIENT_NET_H__
#define __CLIENT_NET_H__

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>



typedef enum{
    CN_SUCCESS,
    CN_INVALID_ARGUMENTS,
    CN_SOCKET_INIT_FAILURE,
    CN_CONNECTION_TO_SERVER_FAILURE,
    CN_SEND_FAIL,
    CN_RECV_FAIL
} ClientNetworkMessage;


#define BUFFER_SIZE 200

ClientNetworkMessage ClientNetSend(int _sock_fd, uint8_t* _buffer, int msg_size);

ClientNetworkMessage ClientNetRecv(int _sock_fd, uint8_t* _buffer);

ClientNetworkMessage ClientConnectToServer(int* _sock_fd, const char* _address, uint16_t _port);

void ClientDisconnectFromServer(int* _sock_fd);

#endif