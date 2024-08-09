#include "database.h"
#include "../m_netlib/Log/mars_logger.h"

// 数据库配置信息
static std::string server = "127.0.0.1";
static std::string user = "root";
static std::string password = "661188";
static std::string dbname = "chatroom2";

// 初始化数据库连接
MySQL::MySQL()
{
    m_conn = mysql_init(nullptr);
}

// 释放数据库连接资源
MySQL::~MySQL()
{
    if (m_conn != nullptr)
        mysql_close(m_conn);
}

// 连接数据库
bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(m_conn, server.c_str(), user.c_str(),
                                  password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        mysql_query(m_conn, "set names gbk");
        LogInfo("connect mysql success!");
    }
    else
    {
        LogInfo("connect mysql fail!");
    }

    return p;
}

// 更新操作
bool MySQL::update(const std::string &sql)
{
    if (mysql_query(m_conn, sql.c_str()))
    {
        LogInfo("{} 更新失败", sql);
        return false;
    }

    return true;
}

// 查询操作
MYSQL_RES *MySQL::query(const std::string& sql)
{
    if (mysql_query(m_conn, sql.c_str()))
    {
        LogInfo("{} 查询失败", sql);
        return nullptr;
    }
    
    return mysql_use_result(m_conn);
}

// 获取连接
MYSQL* MySQL::getConnection()
{
    return m_conn;
}