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
    //m_handlersMap[NEED_ENTER] = std::bind(&Work::needEnter, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    //m_handlersMap[LOGIN_SUCCESS] = std::bind(&Work::enterChatroom, this,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    //m_handlersMap[ENTER_CHATROOM] = std::bind(&Work::enterChatroom, this,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[BACK_FRIEND_OP] = std::bind(&Work::friendOptions, this,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[PRIVATE_CHAT] = std::bind(&Work::receivePrivateChat, this,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

static void clearInputBuffer() 
{
    std::cin.clear(); // 清除输入状态标志
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 忽略剩余输入
}

void Work::enterChatroom() {

    while(1)
    {

        std::cout << "1.注册     2.登录" << std::endl;
        std::cout << "3.退出" << std::endl;
        std::cout << "请输入:";

        int choice;
        std::cin >> choice;
        clearInputBuffer(); // 清空输入缓冲区

        switch (choice){
            case 1: reg(conn, time); break;
            case 2: login(conn, js, time); break;
            case 3: exit(EXIT_SUCCESS); break;
        }
    }
}

void Work::reg(const TcpConnectionPtr &conn, Timestamp time) {
    std::string name, password;

    std::cout << "用户名:";
    std::cin >> name;
    std::cout << "密码:";
    std::cin >> password;

    password = passwordToSha256(password);

    json response;
    response["msgid"] = REG_MSG;
    response["name"]  = name;
    response["password"] = password;
    conn->send(response.dump().append("\r\n"));
}

void Work::login(const TcpConnectionPtr &conn, json &js, Timestamp time){
    std::string name, password;

    std::cout << "用户名:";
    std::cin >> name;
    std::cout << "密码:";
    std::cin >> password;

    password = passwordToSha256(password);

    json response;
    response["msgid"] = LOGIN_MSG;
    response["name"] = name;
    response["password"] = password;
    conn->send(response.dump().append("\r\n"));
}

void Work::enterChatroom(const TcpConnectionPtr &conn, json &js, Timestamp time){
    if (js["msgid"] == LOGIN_SUCCESS){
        m_currentUser.setId(js["id"]);
        m_currentUser.setName(js["name"]);
        std::cout << "当前用户 name:" << m_currentUser.getName() << " id:" << m_currentUser.getId() << std::endl;
        // 记录当前用户的好友列表信息
        if (js.contains("friends"))
        {
            // 初始化
            m_friendList.clear();

            std::cout << "好友列表:" << std::endl;

            std::vector<std::string> vec = js["friends"];
            for (std::string &str : vec)
            {
                json js = json::parse(str);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                m_friendList.push_back(user);
                std::cout << user.getId() << " " << user.getName() << " " << user.getState() << std::endl;
            }
        }
    }

    if (js["msg"] != nullptr){
        std::cout << js["msg"] << std::endl;
        std::cout << "你已进入聊天室" << std::endl;
    }

    std::cout << "command界面\n";
    std::cout << "1.私聊      2.群聊\n";
    std::cout << "3.好友操作  4.群操作\n";
    std::cout << "5.退出\n";
    std::cout << "请输入：";

    int choice;
    std::cin >> choice;
    clearInputBuffer();

    switch (choice){
        case 1: privateChat(conn, js, time);          return;
        case 2: std::cout << "进入群聊" << std::endl; return;
        case 3: friendOptions(conn, js, time); return;
        case 4: std::cout << "进入群操作" << std::endl; return;
        case 5: exit(EXIT_SUCCESS);
        default: 
                json response;
                response["msgid"] = ENTER_CHATROOM;
                conn->send(response.dump().append("\r\n"));
                return;
    }
}

void Work::privateChat(const TcpConnectionPtr &conn, json &js, Timestamp time) {

    displayFriendList();

    std::string to_id;
    std::cout << "请输入对方的id:";
    std::cin >> to_id;

    bool isFriend = false;
    bool isOnline = false;
    for( const auto& user : m_friendList) {
        if (std::stoi(to_id) == user.getId()) {
            user.getState() == "online" ? isOnline = true : isOnline = false;
            isFriend = true;
            break;
        }
    }

    if (!isFriend) {
        std::cout << "对方不是你的好友" << std::endl;
        json response;
        response["msgid"] = ENTER_CHATROOM;
        conn->send(response.dump().append("\r\n"));
    }

    if (!isOnline) {
        std::cout << "对方现在不在线" << std::endl;
    }

    if (to_id == m_currentUser.getName())
    {
        std::cout << "不能给自己发消息" << std::endl;
        json response;
        response["msgid"] = ENTER_CHATROOM;
        conn->send(response.dump().append("\r\n"));
        return;
    }

    while(1)
    {
        std::string msg;
        std::cout << "请输入消息:";
        std::cin >> msg;

        if (msg == "exit")
        {
            json response;
            response["msgid"] = ENTER_CHATROOM;
            conn->send(response.dump().append("\r\n"));
            return;
        }

        json response;
        response["msgid"] = PRIVATE_CHAT;
        response["from_name"] = m_currentUser.getName();
        response["to_id"] = to_id;
        response["msg"] = msg;

        std::cout << "msg" << msg << std::endl;

        conn->send(response.dump().append("\r\n"));
    }
}

void Work::receivePrivateChat(const TcpConnectionPtr &conn, json &js, Timestamp time){
    std::cout << "收到私聊消息" << std::endl;
    std::cout << "from " << js["from_name"] << " [" << time.toFormattedString() << "]" << std::endl;
    std::cout << "msg:" << js["msg"] << std::endl;
}

void Work::friendOptions(const TcpConnectionPtr &conn, json &js, Timestamp time){

    if (js["msgid"] == BACK_FRIEND_OP){
        std::cout << js["msg"] << std::endl;
    }

    js["msg"] = nullptr;

    std::cout << "     好友操作界面" << std::endl;
    std::cout << "1.添加好友 2.删除好友\n";
    std::cout << "3.屏蔽好友 4.解除屏蔽\n";
    std::cout << "5.好友列表 6.申请列表\n";
    std::cout << "7.退出\n";
    std::cout << "请输入：";

    int choice;
    std::cin >> choice;
    clearInputBuffer();

    switch (choice){
        case 1: addFriend(conn, js, time); return;
        case 2: deleteFriend(conn, js, time); return;
        case 3: std::cout << "屏蔽好友" << std::endl; return;
        case 4: std::cout << "进入群操作" << std::endl; return;
        case 5: std::cout << "好友列表" << std::endl; return;
        case 6: std::cout << "申请列表" << std::endl; return;
        case 7: 
                return;
                {
                    json response;
                    response["msgid"] = ENTER_CHATROOM;
                    conn->send(response.dump().append("\r\n"));
                    return;
                }
        default:
                {
                    json response;
                    response["msgid"] = BACK_FRIEND_OP;
                    conn->send(response.dump().append("\r\n"));
                    return;
                }

    }
}

void Work::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time){
    std::cout << "请输入对方的用户名:";

    std::string add_name;
    std::cin >> add_name;

    if (add_name == m_currentUser.getName())
    {
        std::cout << "不能添加自己为好友" << std::endl;
        json response;
        response["msgid"] = BACK_FRIEND_OP;
        conn->send(response.dump().append("\r\n"));
        return;
    }

    json response;
    response["msgid"] = ADD_FRIEND;
    response["add_name"] = add_name;

    conn->send(response.dump().append("\r\n"));
}

void Work::displayFriendList() const {
    if (m_friendList.empty()) {
        std::cout << "Friend list is empty." << std::endl;
        return;
    }

    std::cout << "Friend List:" << std::endl;
    for (const auto& user : m_friendList) {
        std::cout << "id: " << user.getId() << ", Name: " << user.getName() << std::endl;
    }
}

void Work::deleteFriend(const TcpConnectionPtr &conn, json &js, Timestamp time){
    displayFriendList();
    std::string del_name;

    std::cout << "请输入要删除的好友名字(exit取消):";
    std::cin >> del_name;

    if (del_name == "exit"){
        json response1;
        response1["msgid"] = BACK_FRIEND_OP;
        conn->send(response1.dump().append("\r\n"));
        return;
    }

    json response;
    response["msgid"] = DELETE_FRIEND;
    response["del_name"] = del_name;

    conn->send(response.dump().append("\r\n"));
}

MsgHandler Work::getHandler(int msgid){
    auto it = m_handlersMap.find(msgid);
    if (it == m_handlersMap.end())
    {
        return [=](const TcpConnectionPtr& conn, json& js, Timestamp time){
            LogError("msgid: {} can not find handler!", msgid);
        };
    }
    else
    {
        return m_handlersMap[msgid];
    }
}
