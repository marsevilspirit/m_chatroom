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
    void addGroup(int userid, int groupid, std::string role);

    void modifyGroupRole(int userid, int groupid, std::string role);

    std::vector<Group> queryAllGroup();

    std::vector<User> queryGroupRequest(int groupid);

    bool ifMasterOrManager(int userid, int groupid);
};

#endif
