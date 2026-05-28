#ifndef __SERVER_MNG_H__
#define __SERVER_MNG_H__

#include <stdint.h>
#include "server_net.h"

typedef struct ServerMng ServerMng;

/**
 * @brief Create server manager (creates UserMng + GroupMng internally)
 * @return ServerMng* on success, NULL on failure
 */
ServerMng* ServerMng_Create(void);

/**
 * @brief Destroy server manager and free all resources
 */
void ServerMng_Destroy(ServerMng** _mng);

/*
 * The four callbacks registered with ServerNet.
 * Pass these (and the ServerMng* as _context) to ServerNet_Create.
 */
void ServerMng_OnNewClient  (int _sockfd, void* _context);
void ServerMng_OnGotMsg     (int _sockfd, uint8_t* _buffer, void* _context);
void ServerMng_OnCloseClient(int _sockfd, void* _context);
void ServerMng_OnFail       (void* _context);

#endif /* __SERVER_MNG_H__ */
