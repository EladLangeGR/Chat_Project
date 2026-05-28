#ifndef __USER_H__
#define __USER_H__

#include "protocol.h"

typedef struct User {
    char name    [UNAME_MAX_LEN + 1];
    char password[PSWD_MAX_LEN  + 1];
    int  sockfd;      /* TCP socket, -1 if disconnected */
    int  is_active;   /* 1 = logged in, 0 = registered but offline */
} User;

#endif /* __USER_H__ */
