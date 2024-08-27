#include "redis.h"
#include "redisPool.h"


Redis::Redis() : m_context(nullptr) {}

Redis::~Redis() {}

bool Redis::connect() {
    // 初始化连接池（可选）

    const std::string redis_host = std::getenv("REDIS_HOST") ? std::getenv("REDIS_HOST") : "";
    std::cout << "REDIS_HOST: " << redis_host << std::endl;

    if(redis_host.empty()) {
        LogError("REDIS_HOST is not set");
        exit(1);
    }

    RedisPool::getInstance().init(redis_host, 6379, 10);
    return true;
}

bool Redis::rpush(const std::string &key, const std::string &value) {
    redisContext* context = RedisPool::getInstance().getConnection();
    if (!context) {
        return false;
    }

    std::unique_ptr<redisReply, RedisReplyDeleter> reply(
        (redisReply*)redisCommand(context, "RPUSH %s %s", key.c_str(), value.c_str()));

    RedisPool::getInstance().releaseConnection(context);

    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        LogWarn("RPUSH to Redis failed: {}", reply ? reply->str : "null reply");
        return false;
    }
    return true;
}

std::vector<std::string> Redis::lrange(const std::string &key, int start, int end) {
    redisContext* context = RedisPool::getInstance().getConnection();
    if (!context) {
        return {};
    }

    std::unique_ptr<redisReply, RedisReplyDeleter> reply(
        (redisReply*)redisCommand(context, "LRANGE %s %d %d", key.c_str(), start, end));
    std::vector<std::string> result;

    RedisPool::getInstance().releaseConnection(context);

    if (reply && reply->type == REDIS_REPLY_ARRAY) {
        for (size_t i = 0; i < reply->elements; i++) {
            result.push_back(reply->element[i]->str);
        }
    } else {
        LogWarn("LRANGE from Redis failed: {}", reply ? reply->str : "null reply");
    }
    return result;
}

void Redis::ltrim(const std::string &key, int start, int end) {
    redisContext* context = RedisPool::getInstance().getConnection();
    if (!context) {
        return;
    }

    std::unique_ptr<redisReply, RedisReplyDeleter> reply(
        (redisReply*)redisCommand(context, "LTRIM %s %d %d", key.c_str(), start, end));

    RedisPool::getInstance().releaseConnection(context);

    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        LogWarn("LTRIM in Redis failed: {}", reply ? reply->str : "null reply");
    }
}
