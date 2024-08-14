#ifndef REDIS_POOL_H
#define REDIS_POOL_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <hiredis/hiredis.h>
#include "../m_netlib/Log/mars_logger.h"

class RedisConnectionPool {
public:
    static RedisConnectionPool& getInstance() {
        static RedisConnectionPool instance;
        return instance;
    }

    // 初始化连接池（可选）
    void init(const std::string& host, int port, size_t poolSize) {
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
                    m_pool.push(context);
                }
            }
            m_initialized = true;
        }
    }

    redisContext* getConnection() {
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

    void releaseConnection(redisContext* context) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pool.push(context);
        m_condVar.notify_one();
    }

    void cleanup() {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_pool.empty()) {
            redisContext* context = m_pool.front();
            redisFree(context);
            m_pool.pop();
        }
    }

private:
    RedisConnectionPool() : m_host("127.0.0.1"), m_port(6379), m_poolSize(10), m_initialized(false) {}
    ~RedisConnectionPool() { cleanup(); }

    std::string m_host;
    int m_port;
    size_t m_poolSize;
    bool m_initialized;
    std::queue<redisContext*> m_pool;
    std::mutex m_mutex;
    std::condition_variable m_condVar;
};

#endif // REDIS_POOL_H
