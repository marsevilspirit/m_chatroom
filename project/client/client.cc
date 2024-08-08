#include "client.h"
#include "../m_netlib/Log/mars_logger.h"
#include "../json.hpp"
#include "clientService.h"

using namespace mars;
using namespace mars::net;
using namespace mars::base;

using json = nlohmann::json;

Client::Client(EventLoop *loop, const InetAddress &serverAddr)
    : m_client(loop, serverAddr),
      m_codec(std::bind(&Client::onJsonMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
{
    m_client.setConnectionCallback(std::bind(&Client::onConnection, this, std::placeholders::_1));
    m_client.setMessageCallback(std::bind(&JsonCodec::onMessage, &m_codec, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void Client::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        LogDebug("onConnection(): new connection [{}] from {}", conn->name().c_str(), conn->peerAddress().toHostPort().c_str());
        m_connection = conn;
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

}

void Client::onJsonMessage(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    handleServerMessage(this, js, time);
}
