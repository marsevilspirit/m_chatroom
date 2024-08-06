#include "service.h"
#include "../public.h"
#include "../m_netlib/Log/mars_logger.h"
#include <mutex>

Service *Service::getInstance() {
    static Service service;
    return &service;
}

Service::Service(){
    m_handlersMap[REG_MSG] = std::bind(&Service::reg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[LOGIN_MSG] = std::bind(&Service::login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[ENTER_CHATROOM] = std::bind(&Service::tempEcho, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void Service::reset() {
    // 把online状态的用户，设置成offline
    LogWarn("server reset");
    m_userModel.resetState();
}

MsgHandler Service::getHandler(int msgid) {
    auto it = m_handlersMap.find(msgid);
    if (it == m_handlersMap.end())
    {
        return [=](const TcpConnectionPtr& conn, json& js, Timestamp time){
            LogError("msgid: %d can not find handler!", msgid);
        };
    }
    else
    {
        return m_handlersMap[msgid];
    }
}

void Service::needEnter(const TcpConnectionPtr &conn, Timestamp time) {
    json response;
    response["msgid"] = NEED_ENTER;
    conn->send(response.dump());
}

// 注册消息以及对应的回调操作
void Service::reg(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    std::string name = js["name"];
    std::string pwd  = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
     
    bool state = m_userModel.insert(user);
    if (state){
        json response;
        response["msgid"] = NEED_ENTER;
        response["msg"] = "注册成功";
        response["id"] = user.getId();
        conn->send(response.dump());
    } else {
        json response;
        response["msgid"] = NEED_ENTER;
        response["msg"] = "注册失败";
        conn->send(response.dump());
    }
}

void Service::login(const TcpConnectionPtr &conn, json &js, Timestamp time){
    std::string name = js["name"];
    std::string pwd = js["password"];

    User user = m_userModel.query(name);
    if (user.getName() == name && user.getPwd() == pwd){
        if (user.getState() == "online"){
            json response;
            response["msgid"] = NEED_ENTER;
            response["msg"] = "该用户已经登录";
            conn->send(response.dump());
        } else {
            {
                std::lock_guard<std::mutex> lock(m_connMutex);
                m_userConnMap[user.getId()] = conn;
            }

            user.setState("online");
            m_userModel.updateState(user);

            json response;
            response["msgid"] = ENTER_CHATROOM;
            response["msg"] = "登录成功";
            response["id"] = user.getId();
            response["name"] = user.getName();

            conn->send(response.dump());
        }
    } else { // 不存在
        json response;
        response["msgid"] = NEED_ENTER;
        response["msg"] = "登录失败";
        conn->send(response.dump());
    }
}

void Service::tempEcho(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    std::cout << js["msg"] << std::endl;

    json response;
    response["msgid"] = ENTER_CHATROOM;
    conn->send(response.dump());
}

// 处理客户端异常退出
void Service::clientCloseException(const TcpConnectionPtr &conn){
    User user;
    {
        std::lock_guard<std::mutex> lock(m_connMutex);
        for (auto it = m_userConnMap.begin(); it != m_userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从map表删除用户的链接信息
                user.setId(it->first);
                m_userConnMap.erase(it);
                break;
            }
        }
    }

    LogInfo("User {} closed", user.getName());

    // 更新用户的状态信息
    if (user.getId() != -1)   //没找到这个 ，id就是默认的构造函数设置的-1，不等于-1说明是有效的用户
    {
        user.setState("offline");
        m_userModel.updateState(user);
    }
}
