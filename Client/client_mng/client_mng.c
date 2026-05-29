#include "client_mng.h"
#include <stdio.h>


static void PrintProtocolMessage(uint8_t* _buffer, int _size);

struct Client
{
    ClientState state;

    int server_socket_fd;

    char username[UNAME_MAX_LEN + 1];

    char* address;

    uint16_t port;

    uint8_t send_buffer[MAX_BUFFER_SIZE];

    uint8_t recv_buffer[MAX_BUFFER_SIZE];

    List* groups;
};



struct Group
{
    char name[GROUP_NAME_MAX_LEN + 1];

    char mc_address[20];

    uint16_t mc_port;

    pid_t sender_pid;

    pid_t receiver_pid;

};

static Client* client;


/*===========================================================================*/
/*============================= CLIENT MNG INIT =============================*/
/*===========================================================================*/

ClientMngStatus ClientMngInit()
{
    client = malloc(sizeof(Client));

    if (client == NULL)
    {
        return CM_ALLOCATION_ERROR;
    }

    client->address = malloc(strlen(ADDRESS)+1);

    if (client->address == NULL)
    {
        free(client);
        return CM_ALLOCATION_ERROR;
    }

    client->state = CLIENT_DISCONNECTED;

    client->server_socket_fd = -1;

    strcpy(client->address, SERVER_IP);

    client->port = SERVER_PORT;

    return CM_SUCCESS;
}


/*===========================================================================*/
/*============================ CLIENT MNG DESTROY ===========================*/
/*===========================================================================*/

void ClientMngDestroy()
{
    if (client == NULL)
    {
        return;
    }

    free(client->address);

    free(client);
}


RegRespStatus ClientMngRegister(const char* _username, const char* _password)
{

    RegRespStatus auth_resp;
    int req_msg_size;

    if (client->state == CLIENT_DISCONNECTED)
    {
        if (ClientConnectToServer(&(client->server_socket_fd), client->address, client->port) != CN_SUCCESS)
            return REG_SYSTEM_ERROR;

        client->state = CLIENT_CONNECTED;
    }

    if ((req_msg_size = ProtocolBuildAuthReq(client->send_buffer, MSG_REG_REQ, _username, _password)) < 0)
        return REG_SYSTEM_ERROR;

    PrintProtocolMessage(client->send_buffer, req_msg_size);

    if (ClientNetSend(client->server_socket_fd, client->send_buffer, req_msg_size) != CN_SUCCESS)
        return REG_SEND_ERROR;


    if (ClientNetRecv(client->server_socket_fd, client->recv_buffer) != CN_SUCCESS)
        return REG_RECV_ERROR;

    ProtocolParseAuthResp(client->recv_buffer, (uint8_t*)&auth_resp);
    PrintProtocolMessage(client->recv_buffer, client->recv_buffer[1]);
    
    if (ProtocolParseAuthResp(client->recv_buffer, (uint8_t*)&auth_resp) != PROTOCOL_SUCCESS)
        return REG_PARSE_ERROR;

    

    if (auth_resp == REG_SUCCESS)
    {
        client->state = CLIENT_LOGGED_IN;
        strcpy(client->username, _username);
    }
        
    return auth_resp;
}

LoginRespStatus ClientMngLogin(const char* _username, const char* _password)
{
    LoginRespStatus auth_resp;
    int req_msg_size;

    if (client->state == CLIENT_DISCONNECTED)
    {
        if (ClientConnectToServer(&(client->server_socket_fd), client->address, client->port) != CN_SUCCESS)
            return LOGIN_SYSTEM_ERROR;

        client->state = CLIENT_CONNECTED;
    }

    if ((req_msg_size = ProtocolBuildAuthReq(client->send_buffer, MSG_LOGIN_REQ, _username, _password)) < 0)
        return LOGIN_SYSTEM_ERROR;


    if (ClientNetSend(client->server_socket_fd, client->send_buffer, req_msg_size) != CN_SUCCESS)
        return LOGIN_SYSTEM_ERROR;


    if (ClientNetRecv(client->server_socket_fd, client->recv_buffer) != CN_SUCCESS)
        return LOGIN_SYSTEM_ERROR;

    if (ProtocolParseAuthResp(client->recv_buffer, (uint8_t*)&auth_resp) != PROTOCOL_SUCCESS)
        return LOGIN_SYSTEM_ERROR;

    if (auth_resp == LOGIN_SUCCESS)
        {
            client->state = CLIENT_LOGGED_IN;
            strcpy(client->username, _username);
        }
    return auth_resp;
}

LogoutRespStatus ClientMngLogout()
{
    LogoutRespStatus logout_resp;
    int req_msg_size;

    if (client->state == CLIENT_CONNECTED)
        return LOGOUT_SUCCESS;
    
    if ((req_msg_size = ProtocolBuildLogoutReq(client->send_buffer)) < 0)
        return LOGOUT_SYSTEM_ERROR;

    if (ClientNetSend(client->server_socket_fd, client->send_buffer, req_msg_size) != CN_SUCCESS)
        return LOGOUT_SYSTEM_ERROR;

    if (ClientNetRecv(client->server_socket_fd, client->recv_buffer) != CN_SUCCESS)
        return LOGOUT_SYSTEM_ERROR;

    if (ProtocolParseLogoutResp(client->recv_buffer, &logout_resp) != PROTOCOL_SUCCESS)
        return LOGOUT_SYSTEM_ERROR;

    if (logout_resp == LOGOUT_SUCCESS)
    {
        ClientDisconnectFromServer(&(client->server_socket_fd));
        client->state = CLIENT_DISCONNECTED;
        memset(client->username, 0, sizeof(client->username));
    }

    return logout_resp;
}

CreateGroupRespStatus ClientMngCreateGroup(const char* _group_name)
{
    CreateGroupRespStatus create_resp;
    int req_msg_size;
    char mc_address[20];
    uint16_t mc_port;

    if (client->state != CLIENT_LOGGED_IN)
    {
        return CREATE_GROUP_SYSTEM_ERROR;
    }

    req_msg_size = ProtocolBuildGroupReq(client->send_buffer,MSG_CREATE_GROUP_REQ, _group_name);

    if (req_msg_size < 0)
    {
        return CREATE_GROUP_SYSTEM_ERROR;
    }

    if (ClientNetSend(client->server_socket_fd, client->send_buffer, req_msg_size) != CN_SUCCESS)
    {
        return CREATE_GROUP_SYSTEM_ERROR;
    }

    if (ClientNetRecv(client->server_socket_fd, client->recv_buffer) != CN_SUCCESS)
    {
        return CREATE_GROUP_SYSTEM_ERROR;
    }

    if (ProtocolParseGroupResp(client->recv_buffer, (uint8_t*)&create_resp, mc_address, &mc_port) != PROTOCOL_SUCCESS)
    {
        return CREATE_GROUP_SYSTEM_ERROR;
    }

    if (create_resp == CREATE_GROUP_SUCCESS)
    {
        GroupAdd(_group_name, mc_address, mc_port);

        LaunchGroupProcesses(_group_name, mc_address, mc_port);
    }

    return create_resp;
}

JoinGroupRespStatus ClientMngJoinGroup(const char* _group_name)
{
    JoinGroupRespStatus join_resp;
    int req_msg_size;
    char mc_address[20];
    uint16_t mc_port;

    if (client->state != CLIENT_LOGGED_IN)
    {
        return JOIN_GROUP_SYSTEM_ERROR;
    }

    if (GroupExists(_group_name))
    {
        return JOIN_GROUP_ALREADY_JOINED;
    }

    req_msg_size = ProtocolBuildGroupReq(client->send_buffer, MSG_JOIN_GROUP_REQ, _group_name);

    if (req_msg_size < 0)
    {
        return JOIN_GROUP_SYSTEM_ERROR;
    }

    if (ClientNetSend(client->server_socket_fd, client->send_buffer, req_msg_size) != CN_SUCCESS)
    {
        return JOIN_GROUP_SYSTEM_ERROR;
    }

    if (ClientNetRecv(client->server_socket_fd, client->recv_buffer) != CN_SUCCESS)
    {
        return JOIN_GROUP_SYSTEM_ERROR;
    }

    if (ProtocolParseGroupResp(client->recv_buffer, (uint8_t*)&join_resp, mc_address, &mc_port) != PROTOCOL_SUCCESS)
    {
        return JOIN_GROUP_SYSTEM_ERROR;
    }

    if (join_resp == JOIN_GROUP_SUCCESS)
    {
        GroupAdd(_group_name, mc_address, mc_port);

        LaunchGroupProcesses(_group_name, mc_address, mc_port);
    }

    return join_resp;
}


ExitGroupRespStatus ClientMngExitGroup(const char* _group_name)
{
    ExitGroupRespStatus exit_resp;
    int req_msg_size;
    Group* group;

    if (client->state != CLIENT_LOGGED_IN)
    {
        return EXIT_GROUP_SYSTEM_ERROR;
    }

    group = GroupFind(_group_name);

    if (group == NULL)
    {
        return EXIT_GROUP_NOT_JOINED;
    }

    req_msg_size = ProtocolBuildGroupReq(client->send_buffer, MSG_EXIT_GROUP_REQ, _group_name);

    if (req_msg_size < 0)
    {
        return EXIT_GROUP_SYSTEM_ERROR;
    }

    if (ClientNetSend(client->server_socket_fd, client->send_buffer, req_msg_size) != CN_SUCCESS)
    {
        return EXIT_GROUP_SYSTEM_ERROR;
    }

    if (ClientNetRecv(client->server_socket_fd, client->recv_buffer) != CN_SUCCESS)
    {
        return EXIT_GROUP_SYSTEM_ERROR;
    }

    if (ProtocolParseExitGroupResp(client->recv_buffer, (uint8_t*)&exit_resp) != PROTOCOL_SUCCESS)
    {
        return EXIT_GROUP_SYSTEM_ERROR;
    }

    if (exit_resp == EXIT_GROUP_SUCCESS)
    {
        kill(group->sender_pid, SIGTERM);

        kill(group->receiver_pid, SIGTERM);

        GroupRemove(_group_name);
    }

    return exit_resp;
}





static void PrintProtocolMessage(uint8_t* _buffer, int _size)
{
    int i;

    printf("\n====================================\n");
    printf("Protocol Message (%d bytes)\n", _size);
    printf("====================================\n");

    for (i = 0; i < _size; i++)
    {
        printf("[%02X]", _buffer[i]);
    }

    printf("\n");
    printf("====================================\n");
}
