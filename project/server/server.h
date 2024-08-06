#ifndef MARS_SERVER_H
#define MARS_SERVER_H

#include "../m_netlib/Net/TcpServer.h"
#include "../m_netlib/Net/EventLoop.h"

using namespace mars;
using namespace mars::net;

class Server{
public:
    Server(EventLoop* loop, const InetAddress& listenAddr);
    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, base::Timestamp time);

    TcpServer m_server;
    EventLoop* m_loop;
};

#endif // MARS_SERVER_H
