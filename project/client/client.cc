#include "client.h"
#include "../m_netlib/Log/mars_logger.h"
#include "../json.hpp"
#include "work.h"

using namespace mars;
using namespace mars::net;
using namespace mars::base;

using json = nlohmann::json;

Client::Client(EventLoop *loop, const InetAddress &serverAddr)
    : m_client(loop, serverAddr)
{
    m_client.setConnectionCallback(std::bind(&Client::onConnection, this, std::placeholders::_1));
    m_client.setMessageCallback(std::bind(&Client::onMessage, this, std::placeholders::_1, std::placeholders::_2));
}

void Client::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        LogDebug("onConnection(): new connection [{}] from {}", conn->name().c_str(), conn->peerAddress().toHostPort().c_str());
    }
    else
    {
        LogDebug("onConnection(): connection [{}] is down", conn->name().c_str());
        conn->shutdown();
    }
}

void Client::onMessage(const TcpConnectionPtr &conn, Buffer *buf)
{
    std::string str = buf->retrieveAsString();

    LogDebug("Message: {}", str);

    json js = json::parse(str);

    // 通过消息id获取对应的处理器
    int msgid = js["msgid"].get<int>();
    auto msgHandler = Work::getInstance()->getHandler(msgid);

    // 回调消息对应的处理器
    msgHandler(conn, js, Timestamp::now());
}
