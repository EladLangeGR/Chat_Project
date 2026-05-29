#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>
#include <string.h>
#include "status_defs.h"


/*===========================================================================*/
/*============================= PROTOCOL STATUS =============================*/
/*===========================================================================*/

typedef enum
{
    PROTOCOL_SUCCESS,

    PROTOCOL_SERIALIZATION_ERR,

    PROTOCOL_INVALID_ARGUMENTS

} ProtocolStatus;




/*===========================================================================*/
/*============================= AUTH REQUESTS ===============================*/
/*===========================================================================*/

/**
 * @brief Builds authentication request message.
 *
 * Supported message types:
 * - MSG_REG_REQ
 * - MSG_LOGIN_REQ
 *
 * Message format:
 * [type][size][uname_len][username][pass_len][password]
 *
 * @param[out] _buffer Destination serialization buffer.
 * @param[in] _msg_type Authentication request type.
 * @param[in] _username User name string.
 * @param[in] _password Password string.
 *
 * @return ProtocolStatus result.
 */
int ProtocolBuildAuthReq(uint8_t* _buffer, MessageType _msg_type, const char* _username, const char* _password);


/**
 * @brief Parses authentication request message.
 *
 * Supported message types:
 * - MSG_REG_REQ
 * - MSG_LOGIN_REQ
 *
 * @param[in] _buffer Serialized message buffer.
 * @param[out] _username Parsed username.
 * @param[out] _password Parsed password.
 *
 * @return ProtocolStatus result.
 */
ProtocolStatus ProtocolParseAuthReq(uint8_t* _buffer, char* _username, char* _password);


/*===========================================================================*/
/*============================= AUTH RESPONSES ==============================*/
/*===========================================================================*/

/**
 * @brief Builds authentication response message.
 *
 * Supported message types:
 * - MSG_REG_RESP
 * - MSG_LOGIN_RESP
 *
 * Message format:
 * [type][size][status]
 *
 * @param[out] _buffer Destination serialization buffer.
 * @param[in] _msg_type Authentication response type.
 * @param[in] _status Status code.
 *
 * @return ProtocolStatus result.
 */
int ProtocolBuildAuthResp(uint8_t* _buffer, MessageType _msg_type, uint8_t _status);


/**
 * @brief Parses authentication response message.
 *
 * Supported message types:
 * - MSG_REG_RESP
 * - MSG_LOGIN_RESP
 *
 * @param[in] _buffer Serialized message buffer.
 * @param[out] _status Parsed status code.
 *
 * @return ProtocolStatus result.
 */
ProtocolStatus ProtocolParseAuthResp(uint8_t* _buffer, uint8_t* _status);


/*===========================================================================*/
/*============================= LOGOUT REQUEST ==============================*/
/*===========================================================================*/

/**
 * @brief Builds logout request message.
 *
 * Message format:
 * [type][size]
 *
 * @param[out] _buffer Destination serialization buffer.
 *
 * @return ProtocolStatus result.
 */
int ProtocolBuildLogoutReq(uint8_t* _buffer);


/**
 * @brief Parses logout request message.
 *
 * @param[in] _buffer Serialized message buffer.
 *
 * @return ProtocolStatus result.
 */
ProtocolStatus ProtocolParseLogoutReq(uint8_t* _buffer);


/*===========================================================================*/
/*============================= LOGOUT RESPONSE =============================*/
/*===========================================================================*/

/**
 * @brief Builds logout response message.
 *
 * Message format:
 * [type][size][status_len][status]
 *
 * @param[out] _buffer Destination serialization buffer.
 * @param[in] _status Logout status.
 *
 * @return ProtocolStatus result.
 */
int ProtocolBuildLogoutResp(uint8_t* _buffer, LogoutRespStatus _status);


/**
 * @brief Parses logout response message.
 *
 * @param[in] _buffer Serialized message buffer.
 * @param[out] _status Parsed logout status.
 *
 * @return ProtocolStatus result.
 */
ProtocolStatus ProtocolParseLogoutResp(uint8_t* _buffer, LogoutRespStatus* _status);


/*===========================================================================*/
/*============================= GROUP REQUESTS ==============================*/
/*===========================================================================*/

/**
 * @brief Builds group request message.
 *
 * Supported message types:
 * - MSG_CREATE_GROUP_REQ
 * - MSG_JOIN_GROUP_REQ
 * - MSG_EXIT_GROUP_REQ
 *
 * Message format:
 * [type][size][group_len][group_name]
 *
 * @param[out] _buffer Destination serialization buffer.
 * @param[in] _msg_type Group request type.
 * @param[in] _group_name Group name string.
 *
 * @return ProtocolStatus result.
 */
int ProtocolBuildGroupReq(uint8_t* _buffer, MessageType _msg_type, const char* _group_name);


/**
 * @brief Parses group request message.
 *
 * Supported message types:
 * - MSG_CREATE_GROUP_REQ
 * - MSG_JOIN_GROUP_REQ
 * - MSG_EXIT_GROUP_REQ
 *
 * @param[in] _buffer Serialized message buffer.
 * @param[out] _group_name Parsed group name.
 *
 * @return ProtocolStatus result.
 */
ProtocolStatus ProtocolParseGroupReq(uint8_t* _buffer, char* _group_name);


/*===========================================================================*/
/*============================= GROUP RESPONSES =============================*/
/*===========================================================================*/

/**
 * @brief Builds group response message.
 *
 * Supported message types:
 * - MSG_CREATE_GROUP_RESP
 * - MSG_JOIN_GROUP_RESP
 *
 * Message format:
 * [type][size][status_len][status][ip_len][multicast_ip][port_len][port]
 *
 * @param[out] _buffer Destination serialization buffer.
 * @param[in] _msg_type Group response type.
 * @param[in] _status Response status.
 * @param[in] _mc_ip Multicast IP string.
 * @param[in] _mc_port Multicast port value.
 *
 * @return ProtocolStatus result.
 */
int ProtocolBuildGroupResp(uint8_t* _buffer, MessageType _msg_type, uint8_t _status, const char* _mc_ip, uint16_t* _mc_port);


/**
 * @brief Parses group response message.
 *
 * Supported message types:
 * - MSG_CREATE_GROUP_RESP
 * - MSG_JOIN_GROUP_RESP
 *
 * @param[in] _buffer Serialized message buffer.
 * @param[out] _status Parsed response status.
 * @param[out] _mc_ip Parsed multicast IP string.
 * @param[out] _mc_port parsed multicast port value.
 *
 * @return ProtocolStatus result.
 */
ProtocolStatus ProtocolParseGroupResp(uint8_t* _buffer, uint8_t* _status, char* _mc_ip, uint16_t* _mc_port);


/*===========================================================================*/
/*=========================== EXIT GROUP RESPONSE ===========================*/
/*===========================================================================*/

/**
 * @brief Builds exit-group response message.
 *
 * Message format:
 * [type][size][status_len][status]
 *
 * @param[out] _buffer Destination serialization buffer.
 * @param[in] _status Exit-group status.
 *
 * @return ProtocolStatus result.
 */
int ProtocolBuildExitGroupResp(uint8_t* _buffer, ExitGroupRespStatus _status);


/**
 * @brief Parses exit-group response message.
 *
 * @param[in] _buffer Serialized message buffer.
 * @param[out] _status Parsed exit-group status.
 *
 * @return ProtocolStatus result.
 */
ProtocolStatus ProtocolParseExitGroupResp(uint8_t* _buffer, ExitGroupRespStatus* _status);


#endif /* __PROTOCOL_H__ */