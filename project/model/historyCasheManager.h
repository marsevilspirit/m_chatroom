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
        LogTrace("Flushing cache to database...");
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
        std::vector<std::string> cachedMessages = redis.lrange("private_chat_cache", 0, -1);

        if (cachedMessages.empty()) {
            return;
        }

        auto mysql = MysqlPool::getInstance().getConnection();
        if (!mysql) {
            LogWarn("Failed to get MySQL connection from pool");
            return;
        }

        for (const auto &record_str : cachedMessages) {
            // 使用 nlohmann::json 解析 JSON 字符串
            json record = json::parse(record_str);

            // 提取字段
            std::string sender_id = record["sender_id"];
            std::string receiver_id = record["receiver_id"];
            std::string message = record["message"];
            std::string timestamp = record["timestamp"];

            std::string escapedMessage = escapeSingleQuotes(message);

            std::string sql = "INSERT INTO private_chat_history(sender_id, receiver_id, message, timestamp) VALUES('"
                      + sender_id + "', '" + receiver_id + "', '" + escapedMessage + "', '" + timestamp + "')";

            if (!mysql->update(sql)) {
                LogWarn("Failed to insert private message into MySQL");
            }
        }
        // 清空 Redis 缓存
        redis.ltrim("private_chat_cache", 1, 0);

        MysqlPool::getInstance().releaseConnection(mysql);
    }

    void flushGroupChatCache() {
        std::vector<std::string> cachedMessages = redis.lrange("group_chat_cache", 0, -1);

        if (cachedMessages.empty()) {
            return;
        }

        auto mysql = MysqlPool::getInstance().getConnection();
        if (!mysql) {
            LogWarn("Failed to get MySQL connection from pool");
            return;
        }

        for (const auto &record_str : cachedMessages) {
            // 使用 nlohmann::json 解析 JSON 字符串
            json record = json::parse(record_str);

            // 提取字段
            std::string group_id = record["group_id"];
            std::string sender_id = record["sender_id"];
            std::string message = record["message"];
            std::string timestamp = record["timestamp"];

            std::string escapedMessage = escapeSingleQuotes(message);

            std::string sql = "INSERT INTO group_chat_history(group_id, sender_id, message, timestamp) VALUES('"
                      + group_id + "', '" + sender_id + "', '" + escapedMessage + "', '" + timestamp + "')";

            if (!mysql->update(sql)) {
                LogWarn("Failed to insert group message into MySQL");
            }
        }
        // 清空 Redis 缓存
        redis.ltrim("group_chat_cache", 1, 0);

        MysqlPool::getInstance().releaseConnection(mysql);
    }
};

#endif //HISTORYCACHEMANAGER_H
