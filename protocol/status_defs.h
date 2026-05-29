#ifndef __STATUS_DEFS_H__
#define __STATUS_DEFS_H__


/*===========================================================================*/
/*================================ CONSTANTS ================================*/
/*===========================================================================*/

#define MAX_BUFFER_SIZE       256

#define HEADER_SIZE           2

#define UNAME_MAX_LEN         100
#define PSWD_MAX_LEN          100
#define GROUP_NAME_MAX_LEN    100

#define SERVER_IP             "127.0.0.1"
#define SERVER_PORT           2222


/*===========================================================================*/
/*============================== MESSAGE TYPES ==============================*/
/*===========================================================================*/

/*
Protocol Header Format:

[type][payload_size]

type         -> message type identifier
payload_size -> payload size in bytes (does not include header)
*/

typedef enum
{
    MSG_REG_REQ,
    MSG_REG_RESP,

    MSG_LOGIN_REQ,
    MSG_LOGIN_RESP,

    MSG_LOGOUT_REQ,
    MSG_LOGOUT_RESP,

    MSG_CREATE_GROUP_REQ,
    MSG_CREATE_GROUP_RESP,

    MSG_JOIN_GROUP_REQ,
    MSG_JOIN_GROUP_RESP,

    MSG_EXIT_GROUP_REQ,
    MSG_EXIT_GROUP_RESP

} MessageType;



/*===========================================================================*/
/*============================ REGISTER RESPONSES ===========================*/
/*===========================================================================*/

typedef enum
{
    REG_SUCCESS,

    REG_USER_EXISTS,

    REG_INVALID_ARGUMENTS,

    REG_SYSTEM_ERROR,

    REG_PARSE_ERROR,

    REG_RECV_ERROR,

    REG_SEND_ERROR

} RegRespStatus;


/*===========================================================================*/
/*============================= LOGIN RESPONSES =============================*/
/*===========================================================================*/

typedef enum
{
    LOGIN_SUCCESS,

    LOGIN_NO_SUCH_USER,

    LOGIN_WRONG_PASSWORD,

    LOGIN_USER_ALREADY_ACTIVE,

    LOGIN_INVALID_ARGUMENTS,

    LOGIN_USER_NOT_FOUND,

    

    LOGIN_SYSTEM_ERROR

} LoginRespStatus;


/*===========================================================================*/
/*============================ LOGOUT RESPONSES =============================*/
/*===========================================================================*/

typedef enum
{
    LOGOUT_SUCCESS,

    LOGOUT_USER_NOT_ACTIVE,

    LOGOUT_INVALID_ARGUMENTS,

    LOGOUT_SYSTEM_ERROR

} LogoutRespStatus;


/*===========================================================================*/
/*========================= CREATE GROUP RESPONSES ==========================*/
/*===========================================================================*/

typedef enum
{
    CREATE_GROUP_SUCCESS,

    CREATE_GROUP_ALREADY_EXISTS,

    CREATE_GROUP_INVALID_ARGUMENTS,

    CREATE_GROUP_NO_AVAILABLE_MC_ADDR,

    CREATE_GROUP_SYSTEM_ERROR

} CreateGroupRespStatus;


/*===========================================================================*/
/*=========================== JOIN GROUP RESPONSES ==========================*/
/*===========================================================================*/

typedef enum
{
    JOIN_GROUP_SUCCESS,

    JOIN_GROUP_DOES_NOT_EXIST,

    JOIN_GROUP_ALREADY_JOINED,

    JOIN_GROUP_INVALID_ARGUMENTS,

    JOIN_GROUP_SYSTEM_ERROR

} JoinGroupRespStatus;


/*===========================================================================*/
/*=========================== EXIT GROUP RESPONSES ==========================*/
/*===========================================================================*/

typedef enum
{
    EXIT_GROUP_SUCCESS,

    EXIT_GROUP_DOES_NOT_EXIST,

    EXIT_GROUP_USER_NOT_MEMBER,

    EXIT_GROUP_INVALID_ARGUMENTS,

    EXIT_GROUP_SYSTEM_ERROR,

    EXIT_GROUP_NOT_JOINED

} ExitGroupRespStatus;

#endif