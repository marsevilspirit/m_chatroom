#include "redisPool.h"

void RedisPool::init(const std::string& host, int port, size_t poolSize) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_initialized) {
        m_host = host;
        m_port = port;
        m_poolSize = poolSize;
        for (size_t i = 0; i < m_poolSize; ++i) {
            redisContext* context = redisConnect(host.c_str(), port);
            if (context == nullptr || context->err) {
                if (context) {
                    LogWarn("Redis connection error: {}", context->errstr);
                } else {
                    LogWarn("Cannot allocate Redis context");
                }
            } else {
                LogInfo("connect redis success!")
                m_pool.push(context);
            }
        }
        m_initialized = true;
    }
}

redisContext* RedisPool::getConnection() {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_initialized) {
        init(m_host, m_port, m_poolSize);
    }
    while (m_pool.empty()) {
        m_condVar.wait(lock);
    }
    redisContext* context = m_pool.front();
    m_pool.pop();
    return context;
}

void RedisPool::releaseConnection(redisContext* context) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pool.push(context);
    m_condVar.notify_one();
}

void RedisPool::cleanup() {
    std::lock_guard<std::mutex> lock(m_mutex);
    while (!m_pool.empty()) {
        redisContext* context = m_pool.front();
        redisFree(context);
        m_pool.pop();
    }
}
