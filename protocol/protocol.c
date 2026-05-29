#include "protocol.h"


/*===========================================================================*/
/*=========================== STATIC DECLARATIONS ===========================*/
/*===========================================================================*/

static void ProtocolWriteUint8(uint8_t* _buffer, uint8_t* _index, uint8_t _value);
static void ProtocolWriteBuffer(uint8_t* _buffer, uint8_t* _index, const void* _src, uint8_t _size);
static void ProtocolWriteString(uint8_t* _buffer, uint8_t* _index, const char* _str);

static uint8_t ProtocolReadUint8(uint8_t* _buffer, uint8_t* _index);
static void ProtocolReadBuffer(uint8_t* _buffer, uint8_t* _index, void* _dest, uint8_t _size);
static void ProtocolReadString(uint8_t* _buffer, uint8_t* _index, char* _dest);

static MessageType ProtocolGetMsgType(uint8_t* _buffer);


/*===========================================================================*/
/*============================== AUTH REQUESTS ==============================*/
/*===========================================================================*/

int ProtocolBuildAuthReq(uint8_t* _buffer, MessageType _msg_type, const char* _username, const char* _password)
{
    uint8_t data_size , uname_size, pass_size;
    uint8_t msg_index = 2;

    if (_buffer == NULL || _username == NULL || _password == NULL)
        return -1;

    if (_msg_type != MSG_REG_REQ && _msg_type != MSG_LOGIN_REQ)
        return -1;

    uname_size = strlen(_username);
    pass_size = strlen(_password);

    if (uname_size > UNAME_MAX_LEN || pass_size > PSWD_MAX_LEN)
        return -1;

    data_size = uname_size + pass_size + 2;

    _buffer[0] = _msg_type;
    _buffer[1] = data_size;

    ProtocolWriteString(_buffer, &msg_index, _username);

    ProtocolWriteString(_buffer, &msg_index, _password);

    return msg_index;
}


ProtocolStatus ProtocolParseAuthReq(uint8_t* _buffer, char* _username, char* _password)
{
    uint8_t msg_index = 2;
    MessageType msg_type;

    if (_buffer == NULL || _username == NULL || _password == NULL)
        return PROTOCOL_SERIALIZATION_ERR;

    msg_type = ProtocolGetMsgType(_buffer);

    if (msg_type != MSG_REG_REQ && msg_type != MSG_LOGIN_REQ)
        return PROTOCOL_INVALID_ARGUMENTS;

    ProtocolReadString(_buffer, &msg_index, _username);

    ProtocolReadString(_buffer, &msg_index, _password);

    return PROTOCOL_SUCCESS;
}


/*===========================================================================*/
/*============================= AUTH RESPONSES ==============================*/
/*===========================================================================*/

int ProtocolBuildAuthResp(uint8_t* _buffer, MessageType _msg_type, uint8_t _status)
{
    uint8_t msg_index = 2;

    if (_buffer == NULL)
        return -1;

    if (_msg_type != MSG_REG_RESP && _msg_type != MSG_LOGIN_RESP)
        return -1;

    _buffer[0] = _msg_type;
    _buffer[1] = 1;

    ProtocolWriteUint8(_buffer, &msg_index, _status);

    return msg_index;
}


ProtocolStatus ProtocolParseAuthResp(uint8_t* _buffer, uint8_t* _status)
{
    uint8_t msg_index = 2;
    MessageType msg_type;

    if (_buffer == NULL || _status == NULL)
        return PROTOCOL_SERIALIZATION_ERR;

    msg_type = ProtocolGetMsgType(_buffer);

    if (msg_type != MSG_REG_RESP && msg_type != MSG_LOGIN_RESP)
        return PROTOCOL_INVALID_ARGUMENTS;

    *_status = ProtocolReadUint8(_buffer, &msg_index);

    return PROTOCOL_SUCCESS;
}


/*===========================================================================*/
/*============================= LOGOUT REQUEST ==============================*/
/*===========================================================================*/

int ProtocolBuildLogoutReq(uint8_t* _buffer)
{
    if (_buffer == NULL)
        return -1;

    _buffer[0] = MSG_LOGOUT_REQ;
    _buffer[1] = 0;

    return HEADER_SIZE;
}


ProtocolStatus ProtocolParseLogoutReq(uint8_t* _buffer)
{
    if (_buffer == NULL)
        return PROTOCOL_SERIALIZATION_ERR;

    if (ProtocolGetMsgType(_buffer) != MSG_LOGOUT_REQ)
        return PROTOCOL_INVALID_ARGUMENTS;

    if (_buffer[1] != 0)
        return PROTOCOL_INVALID_ARGUMENTS;

    return PROTOCOL_SUCCESS;
}


/*===========================================================================*/
/*============================= LOGOUT RESPONSE =============================*/
/*===========================================================================*/

int ProtocolBuildLogoutResp(uint8_t* _buffer, LogoutRespStatus _status)
{
    uint8_t msg_index = 2;

    if (_buffer == NULL)
        return -1;

    _buffer[0] = MSG_LOGOUT_RESP;
    _buffer[1] = 2;

    ProtocolWriteUint8(_buffer, &msg_index, 1);

    ProtocolWriteUint8(_buffer, &msg_index, _status);

    return msg_index;
}


ProtocolStatus ProtocolParseLogoutResp(uint8_t* _buffer, LogoutRespStatus* _status)
{
    uint8_t msg_index = 2;
    uint8_t status_len;

    if (_buffer == NULL || _status == NULL)
        return PROTOCOL_SERIALIZATION_ERR;

    if (ProtocolGetMsgType(_buffer) != MSG_LOGOUT_RESP)
        return PROTOCOL_INVALID_ARGUMENTS;

    status_len = ProtocolReadUint8(_buffer, &msg_index);

    if (status_len != 1)
        return PROTOCOL_INVALID_ARGUMENTS;

    *_status = ProtocolReadUint8(_buffer, &msg_index);

    return PROTOCOL_SUCCESS;
}


/*===========================================================================*/
/*============================= GROUP REQUESTS ==============================*/
/*===========================================================================*/

int ProtocolBuildGroupReq(uint8_t* _buffer, MessageType _msg_type, const char* _group_name)
{
    uint8_t data_size, group_name_size;
    uint8_t msg_index = 2;

    if (_buffer == NULL || _group_name == NULL)
        return -1;

    if (_msg_type != MSG_CREATE_GROUP_REQ &&
        _msg_type != MSG_JOIN_GROUP_REQ &&
        _msg_type != MSG_EXIT_GROUP_REQ)
    {
        return -1;
    }

    group_name_size = strlen(_group_name);

    if (group_name_size > GROUP_NAME_MAX_LEN || group_name_size == 0)
        return -1;

    data_size = group_name_size + 1;

    _buffer[0] = _msg_type;
    _buffer[1] = data_size;

    ProtocolWriteString(_buffer, &msg_index, _group_name);

    return msg_index;
}


ProtocolStatus ProtocolParseGroupReq(uint8_t* _buffer,  char* _group_name)
{
    uint8_t msg_index = 2;
    MessageType msg_type;

    if (_buffer == NULL || _group_name == NULL)
        return PROTOCOL_SERIALIZATION_ERR;

    msg_type = ProtocolGetMsgType(_buffer);

    if (msg_type != MSG_CREATE_GROUP_REQ &&
        msg_type != MSG_JOIN_GROUP_REQ &&
        msg_type != MSG_EXIT_GROUP_REQ)
    {
        return PROTOCOL_INVALID_ARGUMENTS;
    }

    ProtocolReadString(_buffer, &msg_index, _group_name);

    return PROTOCOL_SUCCESS;
}


/*===========================================================================*/
/*============================= GROUP RESPONSES =============================*/
/*===========================================================================*/

int ProtocolBuildGroupResp(uint8_t* _buffer, MessageType _msg_type, uint8_t _status,const char* _mc_ip, const char* _mc_port)
{
    uint8_t data_size, ip_size, port_size;
    uint8_t msg_index = 2;

    if (_buffer == NULL || _mc_ip == NULL || _mc_port == NULL)
        return -1;

    if (_msg_type != MSG_CREATE_GROUP_RESP &&
        _msg_type != MSG_JOIN_GROUP_RESP)
    {
        return -1;
    }

    ip_size = strlen(_mc_ip);

    port_size = strlen(_mc_port);

    data_size = 4 + ip_size + port_size;

    _buffer[0] = _msg_type;
    _buffer[1] = data_size;

    ProtocolWriteUint8(_buffer, &msg_index, 1);

    ProtocolWriteUint8(_buffer, &msg_index, _status);

    ProtocolWriteString(_buffer, &msg_index, _mc_ip);

    ProtocolWriteString(_buffer,&msg_index, _mc_port);

    return msg_index;
}


ProtocolStatus ProtocolParseGroupResp(uint8_t* _buffer, uint8_t* _status, char* _mc_ip, char* _mc_port)
{
    uint8_t msg_index = 2;
    uint8_t status_len;
    MessageType msg_type;

    if (_buffer == NULL || _status == NULL || _mc_ip == NULL, _mc_port == NULL)
        return PROTOCOL_SERIALIZATION_ERR;

    msg_type = ProtocolGetMsgType(_buffer);

    if (msg_type != MSG_CREATE_GROUP_RESP &&
        msg_type != MSG_JOIN_GROUP_RESP)
    {
        return PROTOCOL_INVALID_ARGUMENTS;
    }

    status_len = ProtocolReadUint8(_buffer, &msg_index);

    if (status_len != 1)
        return PROTOCOL_INVALID_ARGUMENTS;

    *_status = ProtocolReadUint8(_buffer, &msg_index);

    ProtocolReadString(_buffer, &msg_index, _mc_ip);

    ProtocolReadString(_buffer, &msg_index, _mc_port);

    return PROTOCOL_SUCCESS;
}


/*===========================================================================*/
/*=========================== EXIT GROUP RESPONSE ===========================*/
/*===========================================================================*/

int ProtocolBuildExitGroupResp(uint8_t* _buffer, ExitGroupRespStatus _status)
{
    uint8_t msg_index = 2;

    if (_buffer == NULL)
        return -1;

    _buffer[0] = MSG_EXIT_GROUP_RESP;
    _buffer[1] = 2;

    ProtocolWriteUint8(_buffer, &msg_index, 1);

    ProtocolWriteUint8(_buffer, &msg_index, _status);

    return msg_index;
}


ProtocolStatus ProtocolParseExitGroupResp(uint8_t* _buffer, ExitGroupRespStatus* _status)
{
    uint8_t msg_index = 2;
    uint8_t status_len;

    if (_buffer == NULL || _status == NULL)
        return PROTOCOL_SERIALIZATION_ERR;

    if (ProtocolGetMsgType(_buffer) != MSG_EXIT_GROUP_RESP)
        return PROTOCOL_INVALID_ARGUMENTS;

    status_len = ProtocolReadUint8(_buffer, &msg_index);

    if (status_len != 1)
        return PROTOCOL_INVALID_ARGUMENTS;

    *_status = ProtocolReadUint8(_buffer, &msg_index);

    return PROTOCOL_SUCCESS;
}


/*===========================================================================*/
/*============================= HELPER FUNCTIONS ============================*/
/*===========================================================================*/

static MessageType ProtocolGetMsgType(uint8_t* _buffer)
{
    return (MessageType)_buffer[0];
}


static void ProtocolWriteUint8(uint8_t* _buffer, uint8_t* _index, uint8_t _value)
{
    _buffer[*_index] = _value;
    (*_index)++;
}


static void ProtocolWriteBuffer(uint8_t* _buffer, uint8_t* _index, const void* _src, uint8_t _size)
{
    memcpy(&_buffer[*_index], _src, _size);

    *(_index) += _size;
}


static void ProtocolWriteString(uint8_t* _buffer, uint8_t* _index, const char* _str)
{
    uint8_t len;

    len = strlen(_str);

    ProtocolWriteUint8(_buffer, _index, len);

    ProtocolWriteBuffer(_buffer, _index, _str, len);
}


static uint8_t ProtocolReadUint8(uint8_t* _buffer, uint8_t* _index)
{
    uint8_t value;

    value = _buffer[*_index];
    (*_index)++;

    return value;
}


static void ProtocolReadBuffer(uint8_t* _buffer, uint8_t* _index, void* _dest, uint8_t _size)
{
    memcpy(_dest, &_buffer[*_index], _size);

    *(_index) += _size;
}


static void ProtocolReadString(uint8_t* _buffer, uint8_t* _index, char* _dest)
{
    uint8_t len;

    len = ProtocolReadUint8(_buffer, _index);

    ProtocolReadBuffer(_buffer, _index, _dest, len);

    _dest[len] = '\0';
}

