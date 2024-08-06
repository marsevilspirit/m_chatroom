//Created by mars on 6/8/24

#ifndef MARS_SERVICE_H
#define MARS_SERVICE_H

#include <functional>
#include <unordered_map>
#include <mutex>

#include "../model/usermodel.h"
#include "../m_netlib/Net/TcpConnection.h"
#include "../json.hpp"

using json = nlohmann::json;

using namespace mars;
using namespace mars::net;
using namespace mars::base;

using MsgHandler = std::function<void(const TcpConnectionPtr& conn, json &js, Timestamp)>;

class Service {
public:
    // 获取单例对象
    static Service* getInstance();

    void needEnter(const TcpConnectionPtr &conn, Timestamp time);

    // 想要进入房间
    void wantEnter(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 注册消息以及对应的回调操作
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void tempEcho(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);

    // 重置
    void reset();

    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
private:
    Service();

    std::unordered_map<int, MsgHandler> m_handlersMap;
    std::unordered_map<int, TcpConnectionPtr> m_userConnMap;
    std::mutex m_connMutex;

    UserModel m_userModel;
};

#endif //MARS_SERVICE_H
