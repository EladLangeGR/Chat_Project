#include <stdio.h>
#include "ui.h"






/*===========================================================================*/
/*=========================== STATIC DECLARATIONS ===========================*/
/*===========================================================================*/

static void UIPrintStartMenu();

static void UIPrintMainMenu();

static void UIHandleStartMenu(UIState* _state);

static void UIHandleMainMenu(UIState* _state);

static void UIHandleRegister(UIState* _state);

static void UIHandleLogin(UIState* _state);

static void UIHandleCreateGroup();

static void UIHandleJoinGroup();

static void UIHandleExitGroup();

static void UIGetString(char* _buffer, size_t _size);

static void UIClearInputBuffer();


/*===========================================================================*/
/*================================ UI RUN ==================================*/
/*===========================================================================*/

void UIRun()
{
    UIState state;
    state = UI_START_MENU;

    while (state != UI_EXIT)
    {
        switch(state)
        {
            case UI_START_MENU:

                UIHandleStartMenu(&state);

                break;

            case UI_MAIN_MENU:

                UIHandleMainMenu(&state);

                break;

            default:

                state = UI_EXIT;

                break;
        }
    }
}


/*===========================================================================*/
/*============================ START MENU FLOW ==============================*/
/*===========================================================================*/

static void UIHandleStartMenu(UIState* _state)
{
    int option;

    UIPrintStartMenu();

    scanf("%d", &option);

    UIClearInputBuffer();

    switch(option)
    {
        case 1:

            UIHandleRegister(_state);

            break;

        case 2:

            UIHandleLogin(_state);

            break;

        case 3:

            *_state = UI_EXIT;

            break;

        default:

            printf("Invalid option\n");

            break;
    }
}


static void UIPrintStartMenu()
{
    printf("\n");
    printf("====================================\n");
    printf("          CHAT APPLICATION\n");
    printf("====================================\n");
    printf("1. Register\n");
    printf("2. Login\n");
    printf("3. Exit\n");
    printf("====================================\n");
    printf("Select option: ");
}


/*===========================================================================*/
/*============================= MAIN MENU FLOW ==============================*/
/*===========================================================================*/

static void UIHandleMainMenu(UIState* _state)
{
    int option;

    UIPrintMainMenu();

    scanf("%d", &option);

    UIClearInputBuffer();

    switch(option)
    {
        case 1:

            UIHandleCreateGroup();

            break;

        case 2:

            UIHandleJoinGroup();

            break;

        case 3:

            UIHandleExitGroup();

            break;

        case 4:

            ClientMngLogout();

            *_state = UI_START_MENU;

            break;

        default:

            printf("Invalid option\n");

            break;
    }
}


static void UIPrintMainMenu()
{
    printf("\n");
    printf("====================================\n");
    printf("1. Create Group\n");
    printf("2. Join Group\n");
    printf("3. Exit Group\n");
    printf("4. Logout\n");
    printf("====================================\n");
    printf("Select option: ");
}


/*===========================================================================*/
/*============================== AUTH ACTIONS ===============================*/
/*===========================================================================*/

static void UIHandleRegister(UIState* _state)
{
    char username[UNAME_MAX_LEN + 1];
    char password[PSWD_MAX_LEN + 1];

    RegRespStatus reg_status;

    printf("Enter username: ");

    UIGetString(username, sizeof(username));

    printf("Enter password: ");

    UIGetString(password, sizeof(password));

    reg_status = ClientMngRegister(username, password);

    switch(reg_status)
    {
        case REG_SUCCESS:

            printf("Registration successful\n");

            *_state = UI_MAIN_MENU;

            break;

        case REG_USER_EXISTS:

            printf("User already exists\n");

            break;

        case REG_INVALID_ARGUMENTS:

            printf("Invalid username or password\n");

            break;

        case REG_SYSTEM_ERROR:

            printf("System error\n");

            break;

        case REG_SEND_ERROR:

            printf("Send error\n");

            break;

        case REG_RECV_ERROR:

            printf("Recv error\n");

            break;

        case REG_PARSE_ERROR:

            printf("Parse error\n");

            break;

        default:

            printf("Unknown registration error\n");

            break;
    }
}


static void UIHandleLogin(UIState* _state)
{
    char username[UNAME_MAX_LEN + 1];
    char password[PSWD_MAX_LEN + 1];

    LoginRespStatus login_status;

    printf("Enter username: ");

    UIGetString(username, sizeof(username));

    printf("Enter password: ");

    UIGetString(password, sizeof(password));

    login_status = ClientMngLogin(username, password);

    switch(login_status)
    {
        case LOGIN_SUCCESS:

            printf("Login successful\n");

            *_state = UI_MAIN_MENU;

            break;

        case LOGIN_USER_NOT_FOUND:

            printf("User does not exist\n");

            break;

        case LOGIN_WRONG_PASSWORD:

            printf("Wrong password\n");

            break;

        case LOGIN_USER_ALREADY_ACTIVE:

            printf("User already logged in\n");

            break;

        case LOGIN_INVALID_ARGUMENTS:

            printf("Invalid username or password\n");

            break;

        case LOGIN_SYSTEM_ERROR:

            printf("System error\n");

            break;

        default:

            printf("Unknown login error\n");

            break;
    }
}

/*===========================================================================*/
/*============================== GROUP ACTIONS ==============================*/
/*===========================================================================*/

static void UIHandleCreateGroup(void)
{
    char group_name[GROUP_NAME_MAX_LEN + 1];
    CreateGroupRespStatus status;

    printf("Enter group name: ");
    scanf("%s", group_name);

    status = ClientMngCreateGroup(group_name);

    switch (status)
    {
        case CREATE_GROUP_SUCCESS:
            printf("Group created successfully\n");
            break;

        case CREATE_GROUP_ALREADY_EXISTS:
            printf("Group already exists\n");
            break;

        default:
            printf("Failed creating group\n");
            break;
    }
}

static void UIHandleJoinGroup(void)
{
    char group_name[GROUP_NAME_MAX_LEN + 1];
    JoinGroupRespStatus status;

    printf("Enter group name: ");
    scanf("%s", group_name);

    status = ClientMngJoinGroup(group_name);

    switch (status)
    {
        case JOIN_GROUP_SUCCESS:
            printf("Group Joined successfully\n");
            break;

        case JOIN_GROUP_ALREADY_JOINED:
            printf("Already Joined to group\n");
            break;

        case JOIN_GROUP_DOES_NOT_EXIST:
            printf("Group does not exist\n");
            break;

        default:
            printf("Failed joining group\n");
            break;
    }
}

static void UIHandleExitGroup(void)
{
    char group_name[GROUP_NAME_MAX_LEN + 1];
    ExitGroupRespStatus status;

    printf("Enter group name: ");
    scanf("%s", group_name);

    status = ClientMngExitGroup(group_name);

    switch (status)
    {
        case EXIT_GROUP_SUCCESS:
            printf("Exited Group successfully\n");
            break;

        case EXIT_GROUP_NOT_JOINED:
            printf("Group not joined\n");
            break;

        case EXIT_GROUP_DOES_NOT_EXIST:
            printf("Group does not exist\n");
            break;

        default:
            printf("Failed joining group\n");
            break;
    }
}


/*===========================================================================*/
/*============================= HELPER FUNCTIONS ============================*/
/*===========================================================================*/

static void UIGetString(char* _buffer, size_t _size)
{
    fgets(_buffer, _size, stdin);

    _buffer[strcspn(_buffer, "\n")] = '\0';
}


static void UIClearInputBuffer()
{
    while (getchar() != '\n');
}