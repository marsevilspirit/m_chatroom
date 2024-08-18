//Created by mars on 10/8/24

#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <string>
#include <vector>
#include "../m_netlib/Log/mars_logger.h"

class Redis {
public:
    Redis();
    ~Redis();

    bool connect();
    bool rpush(const std::string &key, const std::string &value);
    std::vector<std::string> lrange(const std::string &key, int start, int end);
    void ltrim(const std::string &key, int start, int end);

private:
    redisContext* m_context;

    struct RedisReplyDeleter {
        void operator()(redisReply* reply) const {
            if (reply) {
                freeReplyObject(reply);
            }
        }
    };
};

#endif //REDIS_H
