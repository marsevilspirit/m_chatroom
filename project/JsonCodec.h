//Created by mars on 7/8/24

#ifndef  JSON_CODEC_H
#define  JSON_CODEC_H

#include "json.hpp"
#include "m_netlib/Net/TcpConnection.h"
#include "m_netlib/Base/Timestamp.h"
#include "m_netlib/Log/mars_logger.h"

using namespace mars;
using namespace mars::net;
using namespace mars::base;

using json = nlohmann::json;

class JsonCodec {
public:
    using JsonMessageCallback = std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

    explicit JsonCodec(JsonMessageCallback cb)
        : m_messageCallback(std::move(cb)) {}

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
        while (buf->readableBytes() > 0) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                std::string jsonString(buf->peek(), crlf);
                buf->retrieveUntil(crlf + 2); // +2 to remove "\r\n"

                try {
                    json js = json::parse(jsonString);
                    m_messageCallback(conn, js, time);
                } catch (const json::parse_error& e) {
                    LogError("json parse error: {}", e.what());
                    conn->shutdown();
                }
            } else {
                break; // not enough data to form a complete message
            }
        }
    }

    void send(const TcpConnectionPtr& conn, const json& message) {
        std::string jsonString = message.dump();
        jsonString.append("\r\n"); // Append CRLF to indicate end of message
        conn->send(jsonString);
    }

private:
    JsonMessageCallback m_messageCallback;
};

#endif //JSON_CODEC_H
