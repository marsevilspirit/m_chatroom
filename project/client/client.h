//Created by mars on 6/8/24

#ifndef CLIENT_H
#define CLIENT_H

#include "../m_netlib/Net/TcpClient.h"
#include "../m_netlib/Net/EventLoop.h"

using namespace mars;
using namespace mars::net;

class Client {
public:
    Client(EventLoop *loop, const InetAddress &serverAddr);

    void connect() {
        m_client.connect();
    }

private:
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf);

    TcpClient m_client;
};

#endif //CLIENT_H
