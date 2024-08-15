//Created by mars on 3/8/24

#ifndef MARS_NET_TCPCONNECTION_H
#define MARS_NET_TCPCONNECTION_H

#include "../Base/noncopyable.h"
#include "EventLoop.h"
#include "../Socket/Socket.h"

#include <any>
#include <memory>

namespace mars {
namespace net {

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>{
public:
    TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return m_loop;}
    const std::string& name() const { return m_name;}
    const InetAddress& localAddress() const { return m_localAddr;}
    const InetAddress& peerAddress() const { return m_peerAddr;}
    bool connected() const { return m_state == kConnected;}

    void setConnectionCallback(const ConnectionCallback& cb){ m_connectionCallback = cb;}
    void setMessageCallback(const MessageCallback& cb){ m_messageCallback = cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback& cb){ m_writeCompleteCallback = cb;}
    void setCloseCallback(const CloseCallback& cb){ m_closeCallback = cb;}

    void connectEstablished();

    void connectDestroyed();

    void send(Buffer* buf);
    void send(const std::string& message);

    void shutdown();
    void forceClose();
    void setTcpNoDelay(bool on);
    void setTcpKeepAlive(bool on);

    void setContext(const std::any& context){ m_context = context; }
    const std::any& getContext() const { return m_context;}
    std::any* getMutableContext() { return &m_context; }

    int getFd() const { return m_socket->fd();}

private:
    enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected };

    void setState(StateE s){ m_state = s;}
    void handleRead(base::Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop(const std::string& messafe);
    void shutdownInLoop();
    void forceCloseInLoop();

    EventLoop* m_loop;
    std::string m_name;
    StateE m_state;
    std::unique_ptr<Socket> m_socket;
    std::unique_ptr<Channel> m_channel;
    InetAddress m_localAddr;
    InetAddress m_peerAddr;
    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    CloseCallback m_closeCallback;
    Buffer m_inputBuffer;
    Buffer m_outputBuffer;
    std::any m_context;
};

} //namespace net
} //namespace mars

#endif //MARS_NET_TCPCONNECTION_H
