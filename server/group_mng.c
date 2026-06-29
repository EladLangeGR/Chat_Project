#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "group_mng.h"
#include "gen_hash.h"
#include "queue.h"

#define GROUP_HASH_SIZE  50
#define MAX_GROUPS       50
#define MEMBERS_HASH_SIZE 20

struct GroupMng {
    HashMap* m_groups;   /* key: group->name (char*) -> value: Group* */
    Queue*   m_freeIPs;  /* queue of available MC IP strings (char*) */
};

/*===========================================================================*/
/*                         STATIC HELPER FUNCTIONS                           */
/*===========================================================================*/

static size_t StringHash(void* _key)
{
    unsigned char* str = (unsigned char*)_key;
    size_t hash = 5381;
    while (*str)
        hash = hash * 33 + *str++;
    return hash;
}

static int StringEqual(void* _a, void* _b)
{
    return strcmp((const char*)_a, (const char*)_b) == 0;
}

/* Populate the free-IP queue with addresses 239.0.0.1 .. 239.0.0.MAX_GROUPS */
static void InitMCIPPool(Queue* _queue)
{
    int i;
    char* ip;
    for (i = 1; i <= MAX_GROUPS; i++) {
        ip = (char*)malloc(16);
        if (!ip) continue;
        snprintf(ip, 16, "239.0.0.%d", i);
        QueueInsert(_queue, ip);
    }
}

/* Free a Group and return its MC IP to the pool */
static void DestroyGroup(GroupMng* _mng, Group* _group)
{
    void* pKey;
    void* pValue;
    char* ip;

    /* Return the MC IP to the free pool */
    ip = (char*)malloc(16);
    if (ip) {
        strncpy(ip, _group->mc_ip, 15);
        ip[15] = '\0';
        QueueInsert(_mng->m_freeIPs, ip);
    }

    /* Remove group from the hash (keys/values freed below) */
    HashMap_Remove(_mng->m_groups, _group->name, &pKey, &pValue);

    /* members hash: don't free User* values – owned by UserMng */
    HashMap_Destroy(&_group->members, NULL, NULL);
    free(_group);
}

/* Callback used when GroupMng_Destroy wipes all groups */
static void FreeGroupValue(void* _group)
{
    Group* g = (Group*)_group;
    if (!g) return;
    HashMap_Destroy(&g->members, NULL, NULL);
    free(g);
}

/*===========================================================================*/
/*                              PUBLIC API                                   */
/*===========================================================================*/

GroupMng* GroupMng_Create(void)
{
    GroupMng* mng = (GroupMng*)malloc(sizeof(GroupMng));
    if (!mng) return NULL;

    mng->m_groups = HashMap_Create(GROUP_HASH_SIZE, StringHash, StringEqual);
    if (!mng->m_groups) {
        free(mng);
        return NULL;
    }

    mng->m_freeIPs = QueueCreate(MAX_GROUPS);
    if (!mng->m_freeIPs) {
        HashMap_Destroy(&mng->m_groups, NULL, NULL);
        free(mng);
        return NULL;
    }

    InitMCIPPool(mng->m_freeIPs);
    return mng;
}

void GroupMng_Destroy(GroupMng** _mng)
{
    void* ip;
    if (!_mng || !*_mng) return;

    HashMap_Destroy(&(*_mng)->m_groups, NULL, FreeGroupValue);

    /* Free any remaining IP strings still in the pool */
    while (QueueRemove((*_mng)->m_freeIPs, &ip) == QUEUE_SUCCESS)
        free(ip);
    QueueDestroy(&(*_mng)->m_freeIPs, NULL);

    free(*_mng);
    *_mng = NULL;
}

CreateGroupRespStatus GroupMng_CreateGroup(GroupMng*   _mng,
                                            const char* _groupName,
                                            const char* _username,
                                            User*       _user,
                                            char*       _mc_ip_out)
{
    Group* group;
    void*  found;
    void*  ip_ptr;

    if (!_mng || !_groupName || !_username || !_user || !_mc_ip_out)
        return CREATE_GROUP_INVALID_ARGUMENTS;

    /* Duplicate name check */
    if (HashMap_Find(_mng->m_groups, (void*)_groupName, &found) == MAP_SUCCESS)
        return CREATE_GROUP_ALREADY_EXISTS;

    /* Get a free MC IP */
    if (QueueRemove(_mng->m_freeIPs, &ip_ptr) != QUEUE_SUCCESS)
        return CREATE_GROUP_NO_AVAILABLE_MC_ADDR;

    group = (Group*)malloc(sizeof(Group));
    if (!group) {
        QueueInsert(_mng->m_freeIPs, ip_ptr); /* return IP */
        return CREATE_GROUP_INVALID_ARGUMENTS;
    }

    strncpy(group->name,  _groupName,     GROUP_NAME_MAX_LEN);
    group->name[GROUP_NAME_MAX_LEN] = '\0';
    strncpy(group->mc_ip, (char*)ip_ptr,  15);
    group->mc_ip[15]   = '\0';
    group->mc_port      = MC_PORT;
    group->member_count = 0;
    free(ip_ptr); /* already copied */

    group->members = HashMap_Create(MEMBERS_HASH_SIZE, StringHash, StringEqual);
    if (!group->members) {
        free(group);
        return CREATE_GROUP_INVALID_ARGUMENTS;
    }

    if (HashMap_Insert(_mng->m_groups, group->name, group) != MAP_SUCCESS) {
        HashMap_Destroy(&group->members, NULL, NULL);
        free(group);
        return CREATE_GROUP_INVALID_ARGUMENTS;
    }

    /* Automatically add creator as first member */
    HashMap_Insert(group->members, _user->name, _user);
    group->member_count++;

    strncpy(_mc_ip_out, group->mc_ip, 15);
    _mc_ip_out[15] = '\0';
    return CREATE_GROUP_SUCCESS;
}

JoinGroupRespStatus GroupMng_JoinGroup(GroupMng*   _mng,
                                        const char* _groupName,
                                        const char* _username,
                                        User*       _user,
                                        char*       _mc_ip_out)
{
    Group* group;
    void*  found;

    if (!_mng || !_groupName || !_username || !_user || !_mc_ip_out)
        return JOIN_GROUP_INVALID_ARGUMENTS;

    if (HashMap_Find(_mng->m_groups, (void*)_groupName, (void**)&group) != MAP_SUCCESS)
        return JOIN_GROUP_DOES_NOT_EXIST;

    if (HashMap_Find(group->members, (void*)_username, &found) == MAP_SUCCESS)
        return JOIN_GROUP_ALREADY_JOINED;

    HashMap_Insert(group->members, _user->name, _user);
    group->member_count++;

    strncpy(_mc_ip_out, group->mc_ip, 15);
    _mc_ip_out[15] = '\0';
    return JOIN_GROUP_SUCCESS;
}

ExitGroupRespStatus GroupMng_ExitGroup(GroupMng*   _mng,
                                        const char* _groupName,
                                        const char* _username)
{
    Group* group;
    void*  pKey;
    void*  pValue;

    if (!_mng || !_groupName || !_username)
        return EXIT_GROUP_INVALID_ARGUMENTS;

    if (HashMap_Find(_mng->m_groups, (void*)_groupName, (void**)&group) != MAP_SUCCESS)
        return EXIT_GROUP_DOES_NOT_EXIST;

    if (HashMap_Remove(group->members, (void*)_username, &pKey, &pValue) != MAP_SUCCESS)
        return EXIT_GROUP_USER_NOT_MEMBER;

    group->member_count--;

    if (group->member_count == 0)
        DestroyGroup(_mng, group); /* group ptr is freed inside */

    return EXIT_GROUP_SUCCESS;
}

/*---------------------------------------------------------------------------*/
/* GroupMng_RemoveUserFromAll                                                 */
/*---------------------------------------------------------------------------*/

/* Context passed to the ForEach callback */
typedef struct {
    const char* username;
    char        to_destroy[MAX_GROUPS][GROUP_NAME_MAX_LEN + 1];
    int         count;
} RemoveAllCtx;

static int RemoveFromGroupCb(const void* _key, void* _value, void* _context)
{
    Group*       group = (Group*)_value;
    RemoveAllCtx* ctx  = (RemoveAllCtx*)_context;
    void* pKey;
    void* pValue;

    (void)_key;

    if (HashMap_Remove(group->members, (void*)ctx->username, &pKey, &pValue) == MAP_SUCCESS) {
        group->member_count--;
        if (group->member_count == 0 && ctx->count < MAX_GROUPS) {
            strncpy(ctx->to_destroy[ctx->count], group->name, GROUP_NAME_MAX_LEN);
            ctx->count++;
        }
    }
    return 1; /* continue iteration */
}

void GroupMng_RemoveUserFromAll(GroupMng* _mng, const char* _username)
{
    RemoveAllCtx ctx;
    Group*       group;
    int          i;

    if (!_mng || !_username) return;

    memset(&ctx, 0, sizeof(ctx));
    ctx.username = _username;

    /* Pass 1: remove user from every group, collect empty group names */
    HashMap_ForEach(_mng->m_groups, RemoveFromGroupCb, &ctx);

    /* Pass 2: destroy groups that became empty */
    for (i = 0; i < ctx.count; i++) {
        if (HashMap_Find(_mng->m_groups, ctx.to_destroy[i], (void**)&group) == MAP_SUCCESS)
            DestroyGroup(_mng, group);
    }
}
