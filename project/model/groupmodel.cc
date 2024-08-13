#include "groupmodel.h"
#include "../database/mysqlPool.h"

// 创建群组
bool GroupModel::createGroup(Group &group)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname) values('%s')",
            group.getName().c_str());

    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql->update(sql))
    {
        group.setId(mysql_insert_id(mysql->getConnection()));
        MysqlPool::getInstance().releaseConnection(mysql);
        return true;
    }

    return false;
}

// 加入群组
bool GroupModel::addGroup(int userid, int groupid, std::string role)
{
    // 1. 组装 SQL 语句
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser values(%d, %d, '%s')",
            groupid, userid, role.c_str());

    // 2. 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    bool success = false;

    if (mysql) 
    {
        // 3. 执行 SQL 语句
        success = mysql->update(sql);
        
        // 4. 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return success;
}

void GroupModel::modifyGroupRole(int userid, int groupid, std::string role)
{
    // 1. 组装 SQL 语句
    char sql[1024] = {0};
    sprintf(sql, "update GroupUser set grouprole = '%s' where userid = %d and groupid = %d", 
            role.c_str(), userid, groupid);

    // 2. 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();

    if (mysql)
    {
        // 3. 执行 SQL 语句
        mysql->update(sql);
        
        // 4. 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }
}

std::vector<Group> GroupModel::queryAllGroup()
{
    char sql[1024] = {0};
    sprintf(sql, "select * from AllGroup");

    std::vector<Group> groupVec;

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        MYSQL_RES *res = mysql->query(sql);
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
        }

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return groupVec;
}

std::vector<Group> GroupModel::queryOwnGroup(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.groupname, b.grouprole from AllGroup a inner join GroupUser b on a.id = b.groupid where b.userid = %d", userid);

    std::vector<Group> groupVec;

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        MYSQL_RES *res = mysql->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setGroupRole(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return groupVec;
}


std::vector<User> GroupModel::queryGroupRequest(int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name, b.grouprole from user a inner join GroupUser b on a.id = b.userid where b.groupid = %d and b.grouprole = 'request'", groupid);

    std::vector<User> userVec;

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        MYSQL_RES *res = mysql->query(sql);
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
            }
            mysql_free_result(res);
        }

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return userVec;
}

std::vector<User> GroupModel::queryGroupMember(int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name, b.grouprole from user a inner join GroupUser b on a.id = b.userid where b.groupid = %d and b.grouprole != 'request'", groupid);

    std::vector<User> userVec;

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        MYSQL_RES *res = mysql->query(sql);
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
            }
            mysql_free_result(res);
        }

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return userVec;
}

bool GroupModel::ifMaster(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from GroupUser where userid = %d and groupid = %d and grouprole = 'master'", userid, groupid);

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        MYSQL_RES *res = mysql->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            if ((row = mysql_fetch_row(res)) != nullptr)
            {
                mysql_free_result(res);
                
                // 释放连接回到连接池
                MysqlPool::getInstance().releaseConnection(mysql);
                
                return true;
            }
        }

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return false;
}

std::string GroupModel::queryGroupRole(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select grouprole from GroupUser where userid = %d and groupid = %d", userid, groupid);

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        MYSQL_RES *res = mysql->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            if ((row = mysql_fetch_row(res)) != nullptr)
            {
                std::string role = row[0];
                mysql_free_result(res);

                // 释放连接回到连接池
                MysqlPool::getInstance().releaseConnection(mysql);

                return role;
            }
        }

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return "";
}


std::string GroupModel::queryGroupName(int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select groupname from AllGroup where id = %d", groupid);

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        MYSQL_RES *res = mysql->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            if ((row = mysql_fetch_row(res)) != nullptr)
            {
                std::string groupname = row[0];
                mysql_free_result(res);

                // 释放连接回到连接池
                MysqlPool::getInstance().releaseConnection(mysql);

                return groupname;
            }
        }

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return "";
}

bool GroupModel::ifManager(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from GroupUser where userid = %d and groupid = %d and grouprole = 'manager'", userid, groupid);

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        MYSQL_RES *res = mysql->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            if ((row = mysql_fetch_row(res)) != nullptr)
            {
                mysql_free_result(res);

                // 释放连接回到连接池
                MysqlPool::getInstance().releaseConnection(mysql);

                return true;
            }
        }

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return false;
}


bool GroupModel::ifMasterOrManager(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from GroupUser where userid = %d and groupid = %d and (grouprole = 'master' or grouprole = 'manager')", userid, groupid);

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        MYSQL_RES *res = mysql->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            if ((row = mysql_fetch_row(res)) != nullptr)
            {
                mysql_free_result(res);

                // 释放连接回到连接池
                MysqlPool::getInstance().releaseConnection(mysql);

                return true;
            }
        }

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return false;
}

bool GroupModel::ifManagerOrNormal(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from GroupUser where userid = %d and groupid = %d and (grouprole = 'manager' or grouprole = 'normal')", userid, groupid);

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        MYSQL_RES *res = mysql->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            if ((row = mysql_fetch_row(res)) != nullptr)
            {
                mysql_free_result(res);

                // 释放连接回到连接池
                MysqlPool::getInstance().releaseConnection(mysql);

                return true;
            }
        }

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return false;
}

bool GroupModel::ifMasterOrManagerORnormal(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from GroupUser where userid = %d and groupid = %d and (grouprole = 'master' or grouprole = 'manager' or grouprole = 'normal')", userid, groupid);

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        MYSQL_RES *res = mysql->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            if ((row = mysql_fetch_row(res)) != nullptr)
            {
                mysql_free_result(res);

                // 释放连接回到连接池
                MysqlPool::getInstance().releaseConnection(mysql);

                return true;
            }
        }

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return false;
}

void GroupModel::deleteGroupMember(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from GroupUser where userid = %d and groupid = %d", userid, groupid);

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        mysql->update(sql);

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }
}

void GroupModel::deleteGroup(int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from AllGroup where id = %d", groupid);

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        mysql->update(sql);

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    char sql2[1024] = {0};
    sprintf(sql2, "delete from GroupUser where groupid = %d", groupid);

    // 从连接池获取一个数据库连接
    auto mysql2 = MysqlPool::getInstance().getConnection();
    if (mysql2)
    {
        mysql2->update(sql2);

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql2);
    }
}
