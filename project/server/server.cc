#include "server.h"
#include "../json.hpp"
#include "service.h"
#include "../m_netlib/Log/mars_logger.h"

#include <iostream>
#include <string>
#include <functional>

using namespace mars;
using namespace mars::net;
using namespace mars::base;

using json = nlohmann::json;

Server::Server(EventLoop *loop, const InetAddress &listenAddr)
    : m_server(loop, listenAddr),
      m_loop(loop)
{
    m_server.setConnectionCallback(std::bind(&Server::onConnection, this, std::placeholders::_1));
    m_server.setMessageCallback(std::bind(&Server::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    m_server.setThreadNum(4);
}

void Server::start()
{
    m_server.start();
}

void Server::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        std::cout << "onConnection(): new connection [" << conn->name() << "] from " << conn->peerAddress().toHostPort() << std::endl;
        Service::getInstance()->needEnter(conn, Timestamp::now());
    }
    else
    {
        std::cout << "onConnection(): connection [" << conn->name() << "] is down" << std::endl;
        Service::getInstance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void Server::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    std::string str = buf->retrieveAsString(); // 这里可能会出现问题 原来是 buf->retrieveAllAsString()

    LogDebug("Message: {}", str);

    json js = json::parse(str);

    // 通过消息id获取对应的处理器
    int msgid = js["msgid"].get<int>();
    auto msgHandler = Service::getInstance()->getHandler(msgid);

    // 回调消息对应的处理器
    msgHandler(conn, js, time);
}
