#include "groupmodel.h"
#include "../database/database.h"
#include <iostream>

// 创建群组
bool GroupModel::createGroup(Group &group)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname) values('%s')",
            group.getName().c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}

// 加入群组
void GroupModel::addGroup(int userid, int groupid, std::string role)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser values(%d, %d, '%s')",
            groupid, userid, role.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

void GroupModel::modifyGroupRole(int userid, int groupid, std::string role){
    char sql[1024] = {0};
    sprintf(sql, "update GroupUser set grouprole = '%s' where userid = %d and groupid = %d", role.c_str(), userid, groupid);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

std::vector<Group> GroupModel::queryAllGroup(){
    char sql[1024] = {0};
    sprintf(sql, "select * from AllGroup");

    std::vector<Group> groupVec;

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
            return groupVec;
        }
    }

    return groupVec;
}

std::vector<User> GroupModel::queryGroupRequest(int groupid){
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name, b.grouprole from user a inner join GroupUser b on a.id = b.userid where b.groupid = %d and b.grouprole = 'request'", groupid);

    std::vector<User> userVec;

    std::cout << "queryGroupRequest sql: " << sql << std::endl;

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                userVec.push_back(user);
                std::cout << "id: " << user.getId() << " name: " << user.getName() << " state: " << user.getState() << std::endl;
            }
            mysql_free_result(res);
            return userVec;
        }
    }

    return userVec;
}

bool GroupModel::ifMasterOrManager(int userid, int groupid){
    char sql[1024] = {0};
    sprintf(sql, "select * from GroupUser where userid = %d and groupid = %d and (grouprole = 'master' or grouprole = 'manager')", userid, groupid);

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            if ((row = mysql_fetch_row(res)) != nullptr)
            {
                mysql_free_result(res);
                return true;
            }
        }
    }

    return false;
}
