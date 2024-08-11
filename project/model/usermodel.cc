#include "usermodel.h"
#include "../database/mysqlPool.h"

// User表的增加方法
bool UserModel::insert(User &user)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        if (mysql->update(sql))
        {
            // 获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(mysql->getConnection()));

            // 释放连接回到连接池
            MysqlPool::getInstance().releaseConnection(mysql);

            return true;
        }

        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return false;
}


// 根据用户号码查询用户信息
User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);

    User user;
    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        MYSQL_RES *res = mysql->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
            }
            mysql_free_result(res);
        }
        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return user;
}

User UserModel::query(std::string name)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from user where name = '%s'", name.c_str());

    User user;
    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        MYSQL_RES *res = mysql->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
            }
            mysql_free_result(res);
        }
        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return user;
}
    

std::vector<User> UserModel::query()
{
    char sql[1024] = "select * from user";

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
                user.setState(row[3]);
                userVec.push_back(user);
            }
            mysql_free_result(res);
        }
        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return userVec;
}


// 更新用户的状态信息
bool UserModel::updateState(User user)
{
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        bool result = mysql->update(sql);
        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
        return result;
    }

    return false;
}

// 重置用户的状态信息
void UserModel::resetState()
{
    char sql[1024] = "update user set state = 'offline' where state = 'online'";

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        mysql->update(sql);
        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }
}
