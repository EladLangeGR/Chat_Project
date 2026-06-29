#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
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

/*===========================================================================*/
/*                          INTERNAL HELPERS                                 */
/*===========================================================================*/

/*
 * Read one complete protocol message from a ready socket.
 * select() already told us the socket has data, so the blocking recv
 * returns immediately for these small LAN messages.
 * Returns 0 on success, -1 if the client closed or on error.
 */
static int ReadOneMessage(int _sockfd, uint8_t* _buffer)
{
    ssize_t bytes;
    uint8_t payload_size;

    bytes = recv(_sockfd, _buffer, HEADER_SIZE, MSG_WAITALL);
    if (bytes <= 0) return -1;

    payload_size = _buffer[1];
    if (payload_size > 0) {
        bytes = recv(_sockfd, _buffer + HEADER_SIZE, payload_size, MSG_WAITALL);
        if (bytes <= 0) return -1;
    }
    return 0;
}

/* Accept a new client and register it in the master fd set */
static void AcceptClient(ServerNet* _net, fd_set* _master, int* _maxfd)
{
    struct sockaddr_in client_addr;
    socklen_t          addr_len = sizeof(client_addr);
    int                clientfd;

    clientfd = accept(_net->m_listenfd,
                      (struct sockaddr*)&client_addr, &addr_len);
    if (clientfd < 0) {
        if (_net->m_onFail) _net->m_onFail(_net->m_context);
        return;
    }

    FD_SET(clientfd, _master);
    if (clientfd > *_maxfd)
        *_maxfd = clientfd;

    if (_net->m_onNew)
        _net->m_onNew(clientfd, _net->m_context);
}

/* Handle readable data (or disconnect) on an existing client socket */
static void HandleClient(ServerNet* _net, int _sockfd, fd_set* _master)
{
    uint8_t buffer[HEADER_SIZE + MAX_PAYLOAD];

    if (ReadOneMessage(_sockfd, buffer) < 0) {
        /* client disconnected */
        if (_net->m_onClose) _net->m_onClose(_sockfd, _net->m_context);
        close(_sockfd);
        FD_CLR(_sockfd, _master);
        return;
    }

    if (_net->m_onMsg)
        _net->m_onMsg(_sockfd, buffer, _net->m_context);
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
    fd_set master;    /* all sockets we care about */
    fd_set readfds;   /* the working copy select() modifies each round */
    int    maxfd;
    int    fd;

    if (!_net) return;

    FD_ZERO(&master);
    FD_SET(_net->m_listenfd, &master);
    maxfd = _net->m_listenfd;

    while (_net->m_flag) {
        readfds = master; /* select() overwrites the set, so work on a copy */

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            if (_net->m_onFail) _net->m_onFail(_net->m_context);
            continue;
        }

        for (fd = 0; fd <= maxfd; fd++) {
            if (!FD_ISSET(fd, &readfds))
                continue;

            if (fd == _net->m_listenfd)
                AcceptClient(_net, &master, &maxfd);
            else
                HandleClient(_net, fd, &master);
        }
    }

    /* Loop stopped: close every client socket still open */
    for (fd = 0; fd <= maxfd; fd++) {
        if (fd != _net->m_listenfd && FD_ISSET(fd, &master))
            close(fd);
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
