#ifndef __CLIENT_MNG_H__
#define __CLIENT_MNG_H__

#include <stdint.h>
#include <signal.h>
#include <sys/types.h>

#include "client_net.h"
#include "protocol.h"
#include "gen_dlist.h"


typedef struct Client Client;

typedef struct Group Group;

typedef enum
{
    CLIENT_DISCONNECTED,
    CLIENT_CONNECTED,
    CLIENT_LOGGED_IN

} ClientState;

typedef enum
{
    CM_SUCCESS,

    CM_CONNECTION_FAILED,

    CM_SEND_FAILED,

    CM_RECV_FAILED,

    CM_PROTOCOL_ERROR,

    CM_REG_FAILED,

    CM_LOGIN_FAILED,
    
    CM_ALLOCATION_ERROR

} ClientMngStatus;

/**
 * @brief Initializes the client manager module.
 *
 * Allocates and initializes the internal Client object.
 *
 * @return Status code.
 */
ClientMngStatus ClientMngInit(void);


/**
 * @brief Releases all resources owned by the client manager.
 *
 * Closes active groups, destroys internal data structures,
 * and frees allocated memory.
 */
void ClientMngDestroy(void);


/**
 * @brief Registers a new user.
 *
 * Establishes a connection to the server if needed and sends
 * a registration request.
 *
 * @param[in] _username Requested username.
 * @param[in] _password Requested password.
 *
 * @return Registration response status.
 */
RegRespStatus ClientMngRegister(const char* _username, const char* _password);


/**
 * @brief Logs an existing user into the system.
 *
 * Establishes a connection to the server if needed and sends
 * a login request.
 *
 * @param[in] _username User name.
 * @param[in] _password User password.
 *
 * @return Login response status.
 */
LoginRespStatus ClientMngLogin(const char* _username, const char* _password);


/**
 * @brief Logs the current user out.
 *
 * Sends a logout request to the server and closes the
 * server connection on success.
 *
 * @return Logout response status.
 */
LogoutRespStatus ClientMngLogout(void);


/**
 * @brief Creates a new chat group.
 *
 * Sends a create-group request to the server. Upon success,
 * the group is added locally and the chat processes are launched.
 *
 * @param[in] _group_name Requested group name.
 *
 * @return Create-group response status.
 */
CreateGroupRespStatus ClientMngCreateGroup(const char* _group_name);


/**
 * @brief Joins an existing chat group.
 *
 * Sends a join-group request to the server. Upon success,
 * the group is added locally and the chat processes are launched.
 *
 * @param[in] _group_name Group name.
 *
 * @return Join-group response status.
 */
JoinGroupRespStatus ClientMngJoinGroup(const char* _group_name);


/**
 * @brief Exits a previously joined group.
 *
 * Sends an exit-group request to the server. Upon success,
 * associated chat processes are terminated and the group is
 * removed from the local list.
 *
 * @param[in] _group_name Group name.
 *
 * @return Exit-group response status.
 */
ExitGroupRespStatus ClientMngExitGroup(const char* _group_name);


#endif /* __CLIENT_MNG_H__ */
