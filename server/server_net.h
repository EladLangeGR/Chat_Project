#ifndef __SERVER_NET_H__
#define __SERVER_NET_H__

#include <stdint.h>

/*
 * Callbacks registered by Server Mng.
 * _context is the pointer passed to ServerNet_Create (Server Mng itself).
 */
typedef void (*OnNewClient)  (int _sockfd, void* _context);
typedef void (*OnGotMsg)     (int _sockfd, uint8_t* _buffer, void* _context);
typedef void (*OnCloseClient)(int _sockfd, void* _context);
typedef void (*OnFail)       (void* _context);

typedef struct ServerNet ServerNet;

/**
 * @brief Create the server: opens TCP socket, binds, listens.
 * @return ServerNet* on success, NULL on failure
 */
ServerNet* ServerNet_Create(int           _port,
                             OnNewClient   _onNew,
                             OnGotMsg      _onMsg,
                             OnCloseClient _onClose,
                             OnFail        _onFail,
                             void*         _context);

/**
 * @brief Blocking accept-loop. Spawns a thread per client.
 *        Returns only after ServerNet_StopRun is called.
 */
void ServerNet_Run(ServerNet* _net);

/**
 * @brief Send a message to a connected client.
 * @param _size  Total bytes to send (header + payload)
 */
void ServerNet_SendMsg(int _sockfd, uint8_t* _buffer, uint8_t _size);

/**
 * @brief Stop the accept-loop (closes listening socket).
 */
void ServerNet_StopRun(ServerNet* _net);

/**
 * @brief Free all resources.
 */
void ServerNet_Destroy(ServerNet** _net);

#endif /* __SERVER_NET_H__ */
