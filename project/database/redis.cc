#include "redis.h"

Redis::Redis() : m_context(nullptr) {}

Redis::~Redis() {
    if (m_context != nullptr) {
        redisFree(m_context);
    }
}

bool Redis::connect() {
    m_context = redisConnect("127.0.0.1", 6379);
    if (m_context == nullptr || m_context->err) {
        if (m_context) {
            LogWarn("Redis connection error: {}", m_context->errstr);
        } else {
            LogWarn("Cannot allocate Redis context");
        }
        return false;
    }
    LogInfo("connected redis success");
    return true;
}

bool Redis::rpush(const std::string &key, const std::string &value) {
    redisReply* reply = (redisReply*)redisCommand(m_context, "RPUSH %s %s", key.c_str(), value.c_str());
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        LogWarn("RPUSH to Redis failed: {}", reply ? reply->str : "null reply");
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

std::vector<std::string> Redis::lrange(const std::string &key, int start, int end) {
    redisReply* reply = (redisReply*)redisCommand(m_context, "LRANGE %s %d %d", key.c_str(), start, end);
    std::vector<std::string> result;
    if (reply && reply->type == REDIS_REPLY_ARRAY) {
        for (size_t i = 0; i < reply->elements; i++) {
            result.push_back(reply->element[i]->str);
        }
    } else {
        LogWarn("LRANGE from Redis failed: {}", reply ? reply->str : "null reply");
    }
    if (reply) freeReplyObject(reply);
    return result;
}

void Redis::ltrim(const std::string &key, int start, int end) {
    redisReply* reply = (redisReply*)redisCommand(m_context, "LTRIM %s %d %d", key.c_str(), start, end);
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        LogWarn("LTRIM in Redis failed: {}", reply ? reply->str : "null reply");
    }
    if (reply) freeReplyObject(reply);
}
