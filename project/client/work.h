//Created by mars on 7/8/24

#ifndef MARS_WORK_H
#define MARS_WORK_H

#include <functional>
#include <unordered_map>
#include <mutex>

#include "../m_netlib/Net/TcpConnection.h"
#include "../json.hpp"

using json = nlohmann::json;

using namespace mars;
using namespace mars::net;
using namespace mars::base;

using MsgHandler = std::function<void(const TcpConnectionPtr& conn, json &js, Timestamp)>;

class Work {
public:
    // 获取单例对象
    static Work* getInstance();

    void needEnter(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 注册消息以及对应的回调操作
    void reg(const TcpConnectionPtr &conn, Timestamp time);

    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void enterChatroom(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);

private:
    Work();

    std::unordered_map<int, MsgHandler> m_handlersMap;
};


#endif // MARS_WORK_H
