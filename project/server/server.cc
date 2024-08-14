#include "server.h"
#include "../json.hpp"
#include "service.h"
#include "../m_netlib/Log/mars_logger.h"

#include <iostream>
#include <string>
#include <functional>

using namespace mars;
using namespace mars::net;

using json = nlohmann::json;

Server::Server(EventLoop *loop, const InetAddress &listenAddr)
    : m_server(loop, listenAddr),
      m_loop(loop),
      m_codec(std::bind(&Server::onJsonMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
{
    m_server.setConnectionCallback(std::bind(&Server::onConnection, this, std::placeholders::_1));
    m_server.setMessageCallback(std::bind(&JsonCodec::onMessage, &m_codec, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    m_server.setThreadNum(32);

    Service::getInstance()->groupUserListMapInit();
    Service::getInstance()->UserstateInit();
}

void Server::start()
{
    m_server.start();
}

void Server::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        LogInfo("onConnection(): new connection [{}] from {}", conn->name(), conn->peerAddress().toHostPort())
    }
    else
    {
        LogInfo("onConnection(): connection [{}] is down", conn->name())
        Service::getInstance()->clientClose(conn);
        conn->shutdown();
    }
}

void Server::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    std::string str = buf->retrieveAsString(); // 这里可能会出现问题 原来是 buf->retrieveAllAsString()

    json js = json::parse(str);

    // 通过消息id获取对应的处理器
    int msgid = js["msgid"].get<int>();
    auto msgHandler = Service::getInstance()->getHandler(msgid);

    // 回调消息对应的处理器
    msgHandler(conn, js, time);
}

void Server::onJsonMessage(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    Service::getInstance()->setConnStatus(conn, true);

    // 通过消息id获取对应的处理器
    int msgid = js["msgid"].get<int>();

    auto JsonMsgHandler = Service::getInstance()->getHandler(msgid);

    // 回调消息对应的处理器
    JsonMsgHandler(conn, js, time);
}
