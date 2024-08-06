#include "work.h"
#include "../public.h"
#include "../m_netlib/Log/mars_logger.h"
#include "util.h"

#include <cstdlib>
#include <stdlib.h>

Work *Work::getInstance() {
    static Work work;
    return &work;
}

Work::Work()
{
    m_handlersMap[NEED_ENTER] = std::bind(&Work::needEnter, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[ENTER_CHATROOM] = std::bind(&Work::enterChatroom, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

static void clearInputBuffer() 
{
    std::cin.clear(); // 清除输入状态标志
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 忽略剩余输入
}

void Work::needEnter(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    if (js["msg"] != nullptr)
    {
        std::cout << js["msg"] << std::endl;
    }

    std::cout << std::endl;


    std::cout << "1.注册     2.登录" << std::endl;
    std::cout << "3.退出" << std::endl;
    std::cout << "请输入:" << std::endl;

    int choice;
    std::cin >> choice;
    clearInputBuffer(); // 清空输入缓冲区

    switch (choice){
        case 1: reg(conn, time); break;
        case 2: login(conn, js, time); break;
        case 3: exit(EXIT_SUCCESS); break;
    }
}

void Work::reg(const TcpConnectionPtr &conn, Timestamp time) {
    std::string name, password;

    std::cout << "用户名: " << std::endl;
    std::cin >> name;
    std::cout << "密码: " << std::endl;
    std::cin >> password;

    password = passwordToSha256(password);

    json response;
    response["msgid"] = REG_MSG;
    response["name"]  = name;
    response["password"] = password;
    conn->send(response.dump());
}

void Work::login(const TcpConnectionPtr &conn, json &js, Timestamp time){
    std::string name, password;

    std::cout << "用户名: " << std::endl;
    std::cin >> name;
    std::cout << "密码: " << std::endl;
    std::cin >> password;

    password = passwordToSha256(password);

    json response;
    response["msgid"] = LOGIN_MSG;
    response["name"] = name;
    response["password"] = password;
    conn->send(response.dump());
}

void Work::enterChatroom(const TcpConnectionPtr &conn, json &js, Timestamp time){
    if (js["msg"] != nullptr){
        std::cout << js["msg"] << std::endl;
    std::cout << "你已进入聊天室" << std::endl;
    }

    std::string msg;

    std::cout << "请输入：" << std::endl;
    std::cin >> msg;

    json response;
    response["msgid"] = ENTER_CHATROOM;
    response["msg"] = msg;

    conn->send(response.dump());
}

MsgHandler Work::getHandler(int msgid){
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
