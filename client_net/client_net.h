// #include "../protocol/protocol.h"
#include "../client_mng/client_mng.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>


typedef struct Client Client;

typedef enum{
    CONNECTED,
    DISCONNECTED,
} ClientState;

typedef enum{
    CN_SUCCESS,
    CN_INVALID_ARGUMENTS,
    CN_SOCKET_INIT_FAILURE,
    CN_CONNECTION_TO_SERVER_FAILURE,
    CN_SEND_FAIL,
    CN_RECV_FAIL
} ClientNetworkMessage;

#define PORT 2222
#define ADDRESS ("127.0.0.1")
#define BUFFER_SIZE 200

ClientNetworkMessage ClientNetSend(int _sock_fd, uint8_t* _buffer);

ClientNetworkMessage ClientNetRecv(int _sock_fd, uint8_t* _buffer);