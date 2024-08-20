//Created by mars on 12/8/24

#ifndef HISTORYCACHEMANAGER_H
#define HISTORYCACHEMANAGER_H

#include "../database/mysqlPool.h"
#include "../database/redis.h"
#include "../m_netlib/Log/mars_logger.h"
#include <vector>
#include <string>
#include "../json.hpp"

using json = nlohmann::json;

class CacheManager {
public:
    CacheManager() {
        if (!redis.connect()) {
            LogInfo("Failed to connect to Redis");
            exit(1);
        }
    }

    void storePrivateMessageInCache(const std::string &sender_id, const std::string &receiver_id, const std::string &message) {
        // 使用 nlohmann::json 生成 JSON 格式的字符串
        json chat_record;
        chat_record["sender_id"] = sender_id;
        chat_record["receiver_id"] = receiver_id;
        chat_record["message"] = message;
        chat_record["timestamp"] = getCurrentTimestamp();

        std::string chat_record_str = chat_record.dump(); // 将 JSON 对象序列化为字符串

        if (!redis.rpush("private_chat_cache", chat_record_str)) {
            LogWarn("Failed to store private message in Redis cache");
        }
    }

    void storeGroupMessageInCache(const std::string &group_id, const std::string &sender_id, const std::string &message) {
        // 使用 nlohmann::json 生成 JSON 格式的字符串
        json chat_record;
        chat_record["group_id"] = group_id;
        chat_record["sender_id"] = sender_id;
        chat_record["message"] = message;
        chat_record["timestamp"] = getCurrentTimestamp();

        std::string chat_record_str = chat_record.dump(); // 将 JSON 对象序列化为字符串

        if (!redis.rpush("group_chat_cache", chat_record_str)) {
            LogWarn("Failed to store group message in Redis cache");
        }
    }

    void flushCacheToDatabase() {
        LogInfo("Flushing cache to database...");
        flushPrivateChatCache();
        flushGroupChatCache();
    }

private:
    Redis redis;

    std::string getCurrentTimestamp() {
        time_t now = time(0);
        char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", localtime(&now));
        return std::string(buf);
    }

    std::string escapeSingleQuotes(std::string input) {
        size_t pos = 0;
        while ((pos = input.find("'", pos)) != std::string::npos) {
            input.replace(pos, 1, "''");
            pos += 2;  // 移动到下一个位置，避免重复替换
        }
    return input;
    }

void flushPrivateChatCache() {
    // 从 Redis 中获取缓存的消息
    std::vector<std::string> cachedMessages = redis.lrange("private_chat_cache", 0, -1);

    if (cachedMessages.empty()) {
        return;
    }

    // 清空 Redis 缓存，防止数据重复
    redis.ltrim("private_chat_cache", 1, 0);

    // 获取 MySQL 连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (!mysql) {
        LogWarn("Failed to get MySQL connection from pool");
        return;
    }

    // 开始事务
    mysql->startTransaction();

    size_t batchSize = 1000;  // 每批插入的消息条数
    size_t totalMessages = cachedMessages.size();

    for (size_t i = 0; i < totalMessages; i += batchSize) {
        std::stringstream ss;
        ss << "INSERT INTO private_chat_history (sender_id, receiver_id, message, timestamp) VALUES ";

        bool first = true;
        for (size_t j = i; j < std::min(i + batchSize, totalMessages); ++j) {
            const auto &record_str = cachedMessages[j];
            
            // 使用 nlohmann::json 解析 JSON 字符串
            nlohmann::json record = nlohmann::json::parse(record_str);

            // 提取字段
            std::string sender_id = record["sender_id"];
            std::string receiver_id = record["receiver_id"];
            std::string message = record["message"];
            std::string timestamp = record["timestamp"];

            std::string escapedMessage = escapeSingleQuotes(message);

            if (!first) {
                ss << ",";
            }
            first = false;
            ss << "('" << sender_id << "', '" << receiver_id << "', '" << escapedMessage << "', '" << timestamp << "')";
        }
        ss << ";";

        // 执行批量插入
        if (!mysql->update(ss.str())) {
            LogWarn("Failed to insert private messages into MySQL");
            mysql->rollback();
            MysqlPool::getInstance().releaseConnection(mysql);
            return;
        }
    }

    // 提交事务
    mysql->commit();

    // 释放 MySQL 连接
    MysqlPool::getInstance().releaseConnection(mysql);
}

void flushGroupChatCache() {
    // 从 Redis 中获取缓存的消息
    std::vector<std::string> cachedMessages = redis.lrange("group_chat_cache", 0, -1);

    if (cachedMessages.empty()) {
        return;
    }

    // 获取 MySQL 连接
    auto mysql = MysqlPool::getInstance().getConnection();
    if (!mysql) {
        LogWarn("Failed to get MySQL connection from pool");
        return;
    }

    // 开始事务
    mysql->startTransaction();

    // 批量插入构造
    std::stringstream ss;
    ss << "INSERT INTO group_chat_history (group_id, sender_id, message, timestamp) VALUES ";

    bool first = true;
    for (const auto &record_str : cachedMessages) {
        // 使用 nlohmann::json 解析 JSON 字符串
        nlohmann::json record = nlohmann::json::parse(record_str);

        // 提取字段
        std::string group_id = record["group_id"];
        std::string sender_id = record["sender_id"];
        std::string message = record["message"];
        std::string timestamp = record["timestamp"];

        std::string escapedMessage = escapeSingleQuotes(message);

        if (!first) {
            ss << ",";
        }
        first = false;
        ss << "('" << group_id << "', '" << sender_id << "', '" << escapedMessage << "', '" << timestamp << "')";
    }
    ss << ";";

    // 执行批量插入
    if (!mysql->update(ss.str())) {
        LogWarn("Failed to insert group messages into MySQL");
        mysql->rollback();

        redis.ltrim("private_chat_cache", 1, 0); // 先保证服务器不崩

        MysqlPool::getInstance().releaseConnection(mysql);
        return;
    }

    // 提交事务
    mysql->commit();

    // 清空 Redis 缓存
    redis.ltrim("group_chat_cache", 1, 0);

    // 释放 MySQL 连接
    MysqlPool::getInstance().releaseConnection(mysql);
}
};

#endif //HISTORYCACHEMANAGER_H
