#ifndef MARS_SERVER_H
#define MARS_SERVER_H

#include "../m_netlib/Net/TcpServer.h"
#include "../m_netlib/Net/EventLoop.h"
#include "../m_netlib/Base/noncopyable.h"
#include "../JsonCodec.h"


using namespace mars;
using namespace mars::net;
using json = nlohmann::json;

class Server : noncopyable{
public:
    Server(EventLoop* loop, const InetAddress& listenAddr);
    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, base::Timestamp time);
    void onJsonMessage(const TcpConnectionPtr& conn, json& js, base::Timestamp time);

    TcpServer m_server;
    EventLoop* m_loop;
    JsonCodec m_codec;
};

#endif // MARS_SERVER_H
