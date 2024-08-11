#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.h"
#include "user.h"
#include <string>
#include <vector>

// 维护群组信息的操作接口方法
class GroupModel
{
public:
    // 创建群组
    bool createGroup(Group &group);
    // 加入群组
    bool addGroup(int userid, int groupid, std::string role);

    void modifyGroupRole(int userid, int groupid, std::string role);

    std::vector<Group> queryAllGroup();

    std::vector<Group> queryOwnGroup(int userid);

    std::vector<User> queryGroupRequest(int groupid);

    std::vector<User> queryGroupMember(int groupid);

    std::string queryGroupRole(int userid, int groupid);

    std::string queryGroupName(int groupid);

    bool ifMaster(int userid, int groupid);

    bool ifManager(int userid, int groupid);

    bool ifMasterOrManager(int userid, int groupid);

    bool ifManagerOrNormal(int userid, int groupid);

    bool ifMasterOrManagerORnormal(int userid, int groupid);

    void deleteGroupMember(int userid, int groupid);

    void deleteGroup(int groupid);
};

#endif
