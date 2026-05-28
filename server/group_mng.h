#ifndef __GROUP_MNG_H__
#define __GROUP_MNG_H__

#include "protocol.h"
#include "group.h"
#include "user.h"

typedef struct GroupMng GroupMng;

/**
 * @brief Create group manager (also initialises the free MC-IP pool)
 */
GroupMng* GroupMng_Create(void);

/**
 * @brief Destroy group manager and free all memory
 */
void GroupMng_Destroy(GroupMng** _mng);

/**
 * @brief Create a new group and add the creator as first member
 * @param[out] _mc_ip_out  Buffer (>=16 bytes) that receives the multicast IP
 * @return CreateGroupRespStatus
 */
CreateGroupRespStatus GroupMng_CreateGroup(GroupMng*   _mng,
                                           const char* _groupName,
                                           const char* _username,
                                           User*       _user,
                                           char*       _mc_ip_out);

/**
 * @brief Add a user to an existing group
 * @param[out] _mc_ip_out  Buffer (>=16 bytes) that receives the multicast IP
 * @return JoinGroupRespStatus
 */
JoinGroupRespStatus GroupMng_JoinGroup(GroupMng*   _mng,
                                       const char* _groupName,
                                       const char* _username,
                                       User*       _user,
                                       char*       _mc_ip_out);

/**
 * @brief Remove a user from a group. Destroys the group if it becomes empty.
 * @return ExitGroupRespStatus
 */
ExitGroupRespStatus GroupMng_ExitGroup(GroupMng*   _mng,
                                       const char* _groupName,
                                       const char* _username);

/**
 * @brief Remove a user from every group they belong to.
 *        Called on logout / disconnect.
 */
void GroupMng_RemoveUserFromAll(GroupMng* _mng, const char* _username);

#endif /* __GROUP_MNG_H__ */
