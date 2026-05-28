#ifndef __GROUP_H__
#define __GROUP_H__

#include <stdint.h>
#include "protocol.h"
#include "gen_hash.h"

#define MC_PORT 5000

typedef struct Group {
    char     name[GROUP_NAME_MAX_LEN + 1];
    char     mc_ip[16];      /* e.g. "239.0.0.1" */
    uint16_t mc_port;
    int      member_count;
    HashMap* members;        /* key: username (char*) -> value: User* */
} Group;

#endif /* __GROUP_H__ */
