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

        std::cout << "private chat record: " << chat_record_str << "\n";

        if (!redis.rpush("private_chat_cache", chat_record_str)) {
            LogInfo("Failed to store private message in Redis cache");
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

        std::cout << "group chat record: " << chat_record_str << "\n";

        if (!redis.rpush("group_chat_cache", chat_record_str)) {
            LogInfo("Failed to store group message in Redis cache");
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

    void flushPrivateChatCache() {
        std::vector<std::string> cachedMessages = redis.lrange("private_chat_cache", 0, -1);

        if (cachedMessages.empty()) {
            return;
        }

        auto mysql = MysqlPool::getInstance().getConnection();
        if (!mysql) {
            LogInfo("Failed to get MySQL connection from pool");
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

            char sql[1024];
            sprintf(sql, "INSERT INTO private_chat_history(sender_id, receiver_id, message, timestamp) VALUES('%s', '%s', '%s', '%s')",
                sender_id.c_str(), receiver_id.c_str(), message.c_str(), timestamp.c_str());

            if (!mysql->update(sql)) {
                LogInfo("Failed to insert private message into MySQL");
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
            LogInfo("Failed to get MySQL connection from pool");
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

            char sql[1024];
            sprintf(sql, "INSERT INTO group_chat_history(group_id, sender_id, message, timestamp) VALUES('%s', '%s', '%s', '%s')",
                group_id.c_str(), sender_id.c_str(), message.c_str(), timestamp.c_str());

            if (!mysql->update(sql)) {
                LogInfo("Failed to insert group message into MySQL");
            }
        }
        // 清空 Redis 缓存
        redis.ltrim("group_chat_cache", 1, 0);

        MysqlPool::getInstance().releaseConnection(mysql);
    }
};

#endif //HISTORYCACHEMANAGER_H
