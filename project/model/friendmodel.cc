#include "friendmodel.h"
#include "../database/mysqlPool.h"
#include <mysql/mysql.h>
#include <ostream>
#include <iostream>

void FriendModel::insert(int userid, int friendid, std::string state) {
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d, %d, '%s')", userid, friendid, state.c_str());

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        mysql->update(sql);
        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }
}

void FriendModel::modify(int userid, int friendid, std::string state) {
    char sql[1024] = {0};
    sprintf(sql, "update friend set state='%s' where userid=%d and friendid=%d", state.c_str(), userid, friendid);

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        mysql->update(sql);
        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }
}

void FriendModel::deleteEach(int userid, int friendid) {
    char sql[1024] = {0};
    sprintf(sql, "DELETE FROM friend WHERE (userid = %d AND friendid = %d) OR (userid = %d AND friendid = %d)", userid, friendid, friendid, userid);

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        mysql->update(sql);
        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }
}     

// 返回用户好友列表
std::vector<User> FriendModel::query(int userid) {
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.name,a.state,b.state from user a inner join friend b on b.friendid = a.id where b.userid=%d", userid);

    std::vector<User> vec;
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
                if (std::string(row[3]) == "block"){
                    continue;
                }

                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
        }
        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return vec;
}

// 返回用户好友列表
std::vector<User> FriendModel::requestQuery(int friendid) {
    char sql[1024] = {0};
    sprintf(sql, "SELECT * FROM friend WHERE friendid = %d AND state = 'request'", friendid);

    std::vector<int> vec;
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
                vec.push_back(std::stoi(row[0]));
            }
            mysql_free_result(res);
        }
        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    std::vector<User> vec2;

    if (!vec.empty())
    {
        char sql[1024] = {0};
        for (int id : vec)
        {
            sprintf(sql, "SELECT * FROM user WHERE id = %d", id);
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
                        vec2.push_back(user);
                    }
                    mysql_free_result(res);
                }
                // 释放连接回到连接池
                MysqlPool::getInstance().releaseConnection(mysql);
            }
        }
    }

    return vec2;
}
// 返回用户好友列表
std::vector<User> FriendModel::blockQuery(int userid) {
    char sql[1024] = {0};
    sprintf(sql, "SELECT * FROM friend WHERE userid = %d AND state = 'block'", userid);

    std::vector<int> vec;
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
                vec.push_back(std::stoi(row[1]));
            }
            mysql_free_result(res);
        }
        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    std::vector<User> vec2;

    if (!vec.empty())
    {
        char sql[1024] = {0};
        for (int id : vec)
        {
            sprintf(sql, "SELECT * FROM user WHERE id = %d", id);
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
                        vec2.push_back(user);
                    }
                    mysql_free_result(res);
                }
                // 释放连接回到连接池
                MysqlPool::getInstance().releaseConnection(mysql);
            }
        }
    }

    return vec2;
}

std::string FriendModel::getState(int userid, int friendid) {
    char sql[1024] = {0};
    sprintf(sql, "select state from friend where userid = %d and friendid = %d", userid, friendid);

    std::string state;

    // 从连接池获取一个数据库连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (mysql)
    {
        MYSQL_RES *res = mysql->query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                state = row[0];
            }
            mysql_free_result(res);
        }
        // 释放连接回到连接池
        MysqlPool::getInstance().releaseConnection(mysql);
    }

    return state;
}
