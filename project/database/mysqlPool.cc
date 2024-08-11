#include "mysqlPool.h"

// 私有构造函数，初始化连接池并创建连接
MysqlPool::MysqlPool(size_t poolSize) : m_poolSize(poolSize) {
    for (size_t i = 0; i < m_poolSize; ++i) {
        std::shared_ptr<MySQL> conn = std::make_shared<MySQL>();
        if (conn->connect()) {
            m_pool.push(conn);
        }
    }
}

// 析构函数，销毁连接池
MysqlPool::~MysqlPool() {
    while (!m_pool.empty()) {
        m_pool.pop();
    }
}

// 获取一个可用连接
std::shared_ptr<MySQL> MysqlPool::getConnection() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condVar.wait(lock, [this] { return !m_pool.empty(); });

    auto connection = m_pool.front();
    m_pool.pop();
    return connection;
}

// 释放连接回到连接池
void MysqlPool::releaseConnection(std::shared_ptr<MySQL> connection) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_pool.push(connection);
    m_condVar.notify_one();
}
