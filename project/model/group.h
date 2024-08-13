//Created by mars on 8/8/24

#ifndef GROUP_H
#define GROUP_H

#include "groupuser.h"
#include <string>
#include <vector>

// User表的ORM类
class Group
{
public:
    Group(int id = -1, std::string name = "", std::string desc = "")
    {
        m_id = id;
        m_name = name;
    }

    void setId(int id) { m_id = id; }
    void setName(std::string name) { m_name = name; }
    void setGroupRole(std::string role) { m_grouprole = role; }

    int getId() { return m_id; }
    std::string getName() { return m_name; }
    std::vector<GroupUser> &getUsers() { return m_users; }
    std::string getGroupRole() { return m_grouprole; }

private:
    int m_id;
    std::string m_name;
    std::string m_grouprole;
    std::vector<GroupUser> m_users;    //把群组的成员都放入vector当中，users给业务层去使用
};

#endif
