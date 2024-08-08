#include "service.h"
#include "../public.h"
#include "../m_netlib/Log/mars_logger.h"
#include "../JsonCodec.h"
#include "../model/user.h"
#include <cstdio>
#include <mutex>
#include <vector>

Service *Service::getInstance() {
    static Service service;
    return &service;
}

Service::Service(){
    m_handlersMap[REG_MSG] = std::bind(&Service::reg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[LOGIN_MSG] = std::bind(&Service::login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[ADD_FRIEND] = std::bind(&Service::handleAddFriend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[DELETE_FRIEND] = std::bind(&Service::handleDeleteFriend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[PRIVATE_CHAT] = std::bind(&Service::handlePrivateChat, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[FRIEND_REQUEST_LIST] = std::bind(&Service::handleFriendRequestList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
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
            LogError("msgid: {} can not find handler!", msgid);
        };
    }
    else
    {
        return m_handlersMap[msgid];
    }
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
        response["msgid"] = REG_SUCCESS;
        response["id"] = user.getId();
        conn->send(response.dump().append("\r\n"));
    } else {
        json response;
        response["msgid"] = REG_FAIL;
        conn->send(response.dump().append("\r\n"));
    }
}

void Service::login(const TcpConnectionPtr &conn, json &js, Timestamp time){
    std::string name = js["name"];
    std::string pwd = js["password"];

    User user = m_userModel.query(name);
    std::cout << "user id:" << user.getId() << " name:" << user.getName() << " pwd:" << user.getPwd() << " state:" << user.getState() << '\n';
    if (user.getName() == name && user.getPwd() == pwd){
        if (user.getState() == "online"){
            json response;
            response["msgid"] = LOGIN_REPEAT;
            conn->send(response.dump().append("\r\n"));
        } else {

            m_userConnMap[user.getId()] = conn;
            m_connUserMap[conn] = user.getId();

            user.setState("online");
            m_userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_SUCCESS;
            response["msg"] = "登录成功";
            response["id"] = user.getId();
            response["name"] = user.getName();

            std::vector<User> userVec = m_friendModel.query(user.getId());
            if (!userVec.empty()){
                std::vector<std::string> vec2;
                for (User &each_user : userVec)
                {
                    json js;       //把userVec里面的好友信息，遍历放到json里，再插入到vec2
                    js["friend_id"] = each_user.getId();
                    js["friend_name"] = each_user.getName();
                    js["friend_state"] = each_user.getState();
                    vec2.push_back(js.dump());

                    if(each_user.getState() == "online")
                    {
                        json res_update_state;
                        res_update_state["msgid"] = FRIEND_ONLINE;
                        res_update_state["id"] = user.getId();
                        m_userConnMap[each_user.getId()]->send(res_update_state.dump().append("\r\n"));
                    }
                }
                response["friends"] = vec2;
            }

            conn->send(response.dump().append("\r\n"));
        }
    } else { // 不存在
        json response;
        response["msgid"] = LOGIN_FAIL;
        response["msg"] = "登录失败, 用户不存在";
        conn->send(response.dump().append("\r\n"));
    }
}

void Service::handleAddFriend(const TcpConnectionPtr &conn, json &js, Timestamp time){
    std::string add_name = js["add_name"];

    User add_user = m_userModel.query(add_name);

    int from_id = m_connUserMap[conn];
    int to_id = add_user.getId();

    std::cout << "from_id:" << from_id << " to_id:" << to_id << '\n';

    std::string state1 = m_friendModel.getState(from_id, to_id);
    std::string state2 = m_friendModel.getState(to_id, from_id);

    std::cout << "state1:" << state1 << " state2:" << state2 << '\n';

    if ((state1 == "" && state2 == "") || (state1 == "request" && state2 == "")) // 发起好友请求
    {
        m_friendModel.insert(from_id, to_id, "request");
        json response;
        response["msgid"] = ADD_FRIEND_REQUEST;
        conn->send(response.dump().append("\r\n"));
        return;
    }

    if (state1 == "" && state2 == "request")
    {
        m_friendModel.insert(from_id, to_id, "friend");
        m_friendModel.modify(to_id, from_id, "friend");
        json from_response;
        from_response["msgid"] = ADD_FRIEND_SUCCESS;
        from_response["friend_id"] = to_id;
        from_response["friend_name"] = add_name;
        from_response["friend_state"] = m_userModel.query(to_id).getState();
        conn->send(from_response.dump().append("\r\n"));
        std::cout << "from_id"<< from_id << " is " << from_response["friend_state"] << std::endl;
        if (from_response["friend_state"] == "online")
        {
            std::cout << "update to_id " << to_id << "friend list" << std::endl;
            User from_user = m_userModel.query(from_id);
            json to_response;
            to_response["msgid"] = UPDATE_FRIEND_LIST;
            to_response["friend_id"] = from_id;
            to_response["friend_name"] = from_user.getName();
            to_response["friend_state"] = from_user.getState();

            m_userConnMap[to_id]->send(to_response.dump().append("\r\n"));
        }
        return;
    }
}

void Service::handleDeleteFriend(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int delete_id = js["del_id"];

    User delete_user = m_userModel.query(delete_id);

    int from_id = m_connUserMap[conn];
    int to_id = delete_user.getId();

    std::string state1 = m_friendModel.getState(from_id, to_id);
    std::string state2 = m_friendModel.getState(to_id, from_id);

    if (state1 == "friend" && state2 == "friend")
    {
        m_friendModel.deleteEach(from_id, to_id);
        json response;
        response["msgid"] = DELETE_FRIEND_SUCCESS;
        response["id"] = to_id;
        conn->send(response.dump().append("\r\n"));
        json response2;
        response2["msgid"] = DELETED_FRIEND_SUCCESS;
        response2["id"] = from_id;
        m_userConnMap[delete_id]->send(response2.dump().append("\r\n"));
        return;
    } else {
        json response;
        response["msgid"] = DELETE_FRIEND_FAIL;
        conn->send(response.dump().append("\r\n"));
    }
}

void Service::handlePrivateChat(const TcpConnectionPtr &conn, json &js, Timestamp time){
    std::string msg = js["msg"];
    int to_id = js["id"];

    std::cout << "msg: " << js["msg"] << " to " << js["id"] << '\n' << '\n';

    json response;
    response["msgid"] = PRIVATE_CHAT;
    response["msg"] = msg;

    m_userConnMap[to_id]->send(response.dump().append("\r\n"));
}

void Service::handleFriendRequestList(const TcpConnectionPtr &conn, json &js, Timestamp time){

    std::cout << "handleFriendRequestList" << std::endl;

    int id = m_connUserMap[conn];
    std::vector<User> userVec = m_friendModel.requestQuery(id);

    std::vector<std::string> vec;
    for (User &user : userVec)
    {
        json js;
        js["id"] = user.getId();
        js["name"] = user.getName();
        vec.push_back(js.dump());
    }

    std::cout << "vec size:" << vec.size() << std::endl;

    for(std::string &str : vec){
        std::cout << str << std::endl;
    }

    json response;
    response["msgid"] = FRIEND_REQUEST_LIST;
    response["requests"] = vec;
    conn->send(response.dump().append("\r\n"));
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
