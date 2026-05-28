#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server_net.h"

#define LISTEN_BACKLOG  10
#define HEADER_SIZE      2   /* [type 1B][payload_size 1B] */
#define MAX_PAYLOAD    256

struct ServerNet {
    int           m_listenfd;
    int           m_port;
    volatile int  m_flag;      /* 1 = running */
    OnNewClient   m_onNew;
    OnGotMsg      m_onMsg;
    OnCloseClient m_onClose;
    OnFail        m_onFail;
    void*         m_context;
};

typedef struct {
    ServerNet* net;
    int        sockfd;
} ClientArgs;

/*===========================================================================*/
/*                          CLIENT THREAD                                    */
/*===========================================================================*/

static void* ClientThread(void* _arg)
{
    ClientArgs* args   = (ClientArgs*)_arg;
    int         sockfd = args->sockfd;
    ServerNet*  net    = args->net;
    uint8_t     buffer[HEADER_SIZE + MAX_PAYLOAD];
    ssize_t     bytes;

    free(args);

    if (net->m_onNew)
        net->m_onNew(sockfd, net->m_context);

    while (net->m_flag) {
        /* Read the 2-byte header first */
        bytes = recv(sockfd, buffer, HEADER_SIZE, MSG_WAITALL);
        if (bytes <= 0) break;

        uint8_t payload_size = buffer[1];

        /* Read the payload */
        if (payload_size > 0) {
            bytes = recv(sockfd, buffer + HEADER_SIZE, payload_size, MSG_WAITALL);
            if (bytes <= 0) break;
        }

        if (net->m_onMsg)
            net->m_onMsg(sockfd, buffer, net->m_context);
    }

    if (net->m_onClose)
        net->m_onClose(sockfd, net->m_context);

    close(sockfd);
    return NULL;
}

/*===========================================================================*/
/*                              PUBLIC API                                   */
/*===========================================================================*/

ServerNet* ServerNet_Create(int           _port,
                             OnNewClient   _onNew,
                             OnGotMsg      _onMsg,
                             OnCloseClient _onClose,
                             OnFail        _onFail,
                             void*         _context)
{
    ServerNet*         net;
    struct sockaddr_in addr;
    int                opt = 1;

    net = (ServerNet*)malloc(sizeof(ServerNet));
    if (!net) return NULL;

    net->m_port    = _port;
    net->m_flag    = 1;
    net->m_onNew   = _onNew;
    net->m_onMsg   = _onMsg;
    net->m_onClose = _onClose;
    net->m_onFail  = _onFail;
    net->m_context = _context;

    net->m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (net->m_listenfd < 0) { free(net); return NULL; }

    setsockopt(net->m_listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(_port);

    if (bind(net->m_listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(net->m_listenfd);
        free(net);
        return NULL;
    }

    if (listen(net->m_listenfd, LISTEN_BACKLOG) < 0) {
        perror("listen");
        close(net->m_listenfd);
        free(net);
        return NULL;
    }

    printf("[ServerNet] Listening on port %d\n", _port);
    return net;
}

void ServerNet_Run(ServerNet* _net)
{
    struct sockaddr_in client_addr;
    socklen_t          addr_len = sizeof(client_addr);
    int                clientfd;
    pthread_t          tid;
    ClientArgs*        args;

    if (!_net) return;

    while (_net->m_flag) {
        clientfd = accept(_net->m_listenfd,
                          (struct sockaddr*)&client_addr, &addr_len);
        if (clientfd < 0) {
            if (_net->m_onFail) _net->m_onFail(_net->m_context);
            continue;
        }

        args = (ClientArgs*)malloc(sizeof(ClientArgs));
        if (!args) { close(clientfd); continue; }

        args->net    = _net;
        args->sockfd = clientfd;

        if (pthread_create(&tid, NULL, ClientThread, args) != 0) {
            free(args);
            close(clientfd);
        } else {
            pthread_detach(tid); /* thread cleans up itself */
        }
    }
}

void ServerNet_SendMsg(int _sockfd, uint8_t* _buffer, uint8_t _size)
{
    ssize_t sent;
    int     total = 0;

    if (_sockfd < 0 || !_buffer || _size == 0) return;

    while (total < _size) {
        sent = send(_sockfd, _buffer + total, _size - total, 0);
        if (sent <= 0) break;
        total += (int)sent;
    }
}

void ServerNet_StopRun(ServerNet* _net)
{
    if (!_net) return;
    _net->m_flag = 0;
    close(_net->m_listenfd);
}

void ServerNet_Destroy(ServerNet** _net)
{
    if (!_net || !*_net) return;
    close((*_net)->m_listenfd);
    free(*_net);
    *_net = NULL;
}
