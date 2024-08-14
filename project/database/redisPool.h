#ifndef REDIS_POOL_H
#define REDIS_POOL_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <hiredis/hiredis.h>
#include "../m_netlib/Log/mars_logger.h"

class RedisPool {
public:
    static RedisPool& getInstance() {
        static RedisPool instance;
        return instance;
    }

    // 初始化连接池（可选）
    void init(const std::string& host, int port, size_t poolSize);

    redisContext* getConnection();

    void releaseConnection(redisContext* context);

    void cleanup();

private:
    RedisPool() : m_host("127.0.0.1"), m_port(6379), m_poolSize(10), m_initialized(false) {}
    ~RedisPool() { cleanup(); }

    std::string m_host;
    int m_port;
    size_t m_poolSize;
    bool m_initialized;
    std::queue<redisContext*> m_pool;
    std::mutex m_mutex;
    std::condition_variable m_condVar;
};

#endif // REDIS_POOL_H
