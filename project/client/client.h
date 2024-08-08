//Created by mars on 6/8/24

#ifndef CLIENT_H
#define CLIENT_H

#include "../m_netlib/Net/TcpClient.h"
#include "../m_netlib/Net/EventLoop.h"
#include "../JsonCodec.h"

using namespace mars;
using namespace mars::net;

using json = nlohmann::json;

class Client {
public:
    Client(EventLoop *loop, const InetAddress &serverAddr);

    void connect() {
        m_client.connect();
    }

    void send(std::string &str) {
        m_connection->send(str);
    }

private:
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf);
    void onJsonMessage(const TcpConnectionPtr& conn, json& js, base::Timestamp time);

    TcpClient m_client;
    JsonCodec m_codec;
    TcpConnectionPtr m_connection;
};

#endif //CLIENT_H
