//Created by mars on 12/8/24

#ifndef MYSQLPOOL_H
#define MYSQLPOOL_H

#include "database.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

class MysqlPool {
public:
    // 获取连接池的唯一实例
    static MysqlPool& getInstance(size_t poolSize = 10) {
        static MysqlPool instance(poolSize);
        return instance;
    }

    // 禁止拷贝构造和赋值
    MysqlPool(const MysqlPool&) = delete;
    MysqlPool& operator=(const MysqlPool&) = delete;

    // 获取一个可用连接
    std::shared_ptr<MySQL> getConnection();

    // 释放连接回到连接池
    void releaseConnection(std::shared_ptr<MySQL> connection);

private:
    // 私有化构造函数
    MysqlPool(size_t poolSize);
    ~MysqlPool();

    std::queue<std::shared_ptr<MySQL>> m_pool; 
    std::mutex m_mutex; 
    std::condition_variable m_condVar;  // 用于通知连接可用的条件变量
    size_t m_poolSize;  // 连接池大小
};

#endif // MYSQLPOOL_H
