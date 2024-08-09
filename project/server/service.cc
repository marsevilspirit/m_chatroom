#include "service.h"
#include "../public.h"
#include "../m_netlib/Log/mars_logger.h"
#include "../JsonCodec.h"
#include "../model/user.h"
#include "../model/group.h"
#include <cstdio>
#include <mutex>
#include <vector>

Service *Service::getInstance() {
    static Service service;
    return &service;
}


Service::Service(){
    groupUserListMapInit(); 

    m_handlersMap[REG_MSG] = std::bind(&Service::reg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[LOGIN_MSG] = std::bind(&Service::login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[DISPLAY_ALLUSER_LIST] = std::bind(&Service::handleDisplayAllUserList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[ADD_FRIEND] = std::bind(&Service::handleAddFriend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[DELETE_FRIEND] = std::bind(&Service::handleDeleteFriend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[PRIVATE_CHAT] = std::bind(&Service::handlePrivateChat, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[DISPLAY_FRIEND_LIST] = std::bind(&Service::handleDisplayFriendList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[FRIEND_REQUEST_LIST] = std::bind(&Service::handleFriendRequestList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[BLOCK_FRIEND] = std::bind(&Service::handleBlockFriend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[UNBLOCK_FRIEND] = std::bind(&Service::handleUnBlockFriend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[BLOCK_FRIEND_LIST] = std::bind(&Service::handleBlockFriendList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[CHECK_BLOCK] = std::bind(&Service::handleCheckBlock, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[CREATE_GROUP] = std::bind(&Service::handleCreateGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[DISPLAY_ALLGROUP_LIST] = std::bind(&Service::handleDisplayAllGroupList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[REQUEST_GROUP] = std::bind(&Service::requestAddGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[GROUP_REQUEST_LIST] = std::bind(&Service::handleGroupRequestList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[ADD_GROUP] = std::bind(&Service::handleAddSomeoneToGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[QUIT_GROUP] = std::bind(&Service::handleQuitGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3); 
    m_handlersMap[DISPLAY_OWN_GROUP_LIST] = std::bind(&Service::handleShowOwnGroupList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[DISPLAY_GROUP_MEMBER_LIST] = std::bind(&Service::handleShowGroupMemberList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[DISPLAY_GROUP_SET_MANAGER_MEMBER_LIST] = std::bind(&Service::handleDisplaySetManagerMemberList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[SET_MANAGER] = std::bind(&Service::handleSetManager, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[CANCEL_MANAGER] = std::bind(&Service::handleCancelManager, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[KICK_SOMEONE] = std::bind(&Service::handleKickSomeoneInGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[CHECK_GROUP_MEMBER] = std::bind(&Service::handleCheckIfGroupMember, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[GROUP_CHAT] = std::bind(&Service::handleGroupChat, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void Service::groupUserListMapInit(){
    std::vector<Group> groupVec = m_groupModel.queryAllGroup();
    for (Group &group : groupVec)
    {
        std::vector<User> userVec = m_groupModel.queryGroupMember(group.getId());
        std::vector<int> vec;
        for (User &user : userVec)
        {
            vec.push_back(user.getId());
        }
        m_groupUserListMap[group.getId()] = vec;
    }
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

            conn->send(response.dump().append("\r\n"));
        }
    } else { // 不存在
        json response;
        response["msgid"] = LOGIN_FAIL;
        response["msg"] = "登录失败, 用户不存在";
        conn->send(response.dump().append("\r\n"));
    }
}

void Service::handleDisplayAllUserList(const TcpConnectionPtr &conn, json &js, Timestamp time){
    std::vector<User> userVec = m_userModel.query();
    std::vector<std::string> vec;
    for (User &user : userVec)
    {
        json js;
        js["id"] = user.getId();
        js["name"] = user.getName();
        js["state"] = user.getState();
        vec.push_back(js.dump());
    }

    json response;
    response["msgid"] = DISPLAY_ALLUSER_LIST;
    response["users"] = vec;
    conn->send(response.dump().append("\r\n"));
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
        if (from_response["friend_state"] == "online")
        {
            //todo
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
        return;
    } else {
        json response;
        response["msgid"] = DELETE_FRIEND_FAIL;
        conn->send(response.dump().append("\r\n"));
    }
}

void Service::handleBlockFriend(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int block_id = js["block_id"];
    int user_id = m_connUserMap[conn]; 

    std::string state1 = m_friendModel.getState(user_id, block_id);

    if (state1 == "friend")
    {
        m_friendModel.modify(user_id, block_id, "block");
        json response;
        response["msgid"] = BLOCK_FRIEND_SUCCESS;
        conn->send(response.dump().append("\r\n"));
    } else {
        json response;
        response["msgid"] = BLOCK_FRIEND_FAIL;
        conn->send(response.dump().append("\r\n"));
    }
}

void Service::handleUnBlockFriend(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int unblock_id = js["unblock_id"];
    int user_id = m_connUserMap[conn]; 

    std::string state1 = m_friendModel.getState(user_id, unblock_id);

    if (state1 == "block")
    {
        m_friendModel.modify(user_id, unblock_id, "friend");
        json response;
        response["msgid"] = UNBLOCK_FRIEND_SUCCESS;
        conn->send(response.dump().append("\r\n"));
    } else {
        json response;
        response["msgid"] = UNBLOCK_FRIEND_FAIL;
        conn->send(response.dump().append("\r\n"));
    }
}

void Service::handleCheckBlock(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int check_id = js["check_id"];
    int user_id = m_connUserMap[conn]; 

    std::cout << "check_id:" << check_id << " user_id:" << user_id << '\n';

    std::string state1 = m_friendModel.getState(user_id, check_id);
    std::string state2 = m_friendModel.getState(check_id, user_id);

    json response;
    response["msgid"] = CHECK_BLOCK;
    response["state1"] = state1;
    response["state2"] = state2;
    conn->send(response.dump().append("\r\n"));

    auto it = m_userConnMap.find(check_id);
    if (it != m_userConnMap.end())
    {
        it->second->send(response.dump().append("\r\n"));
    } else {
        std::cout << "m_userConnMap is null" << std::endl;
    }
}

void Service::handleBlockFriendList(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int user_id = m_connUserMap[conn];
    std::vector<User> userVec = m_friendModel.blockQuery(user_id);

    std::vector<std::string> vec;
    for (User &user : userVec)
    {
        json js;
        js["id"] = user.getId();
        js["name"] = user.getName();
        vec.push_back(js.dump());
    }

    json response;
    response["msgid"] = BLOCK_FRIEND_LIST;
    response["blocks"] = vec;
    conn->send(response.dump().append("\r\n"));
}

void Service::handlePrivateChat(const TcpConnectionPtr &conn, json &js, Timestamp time){
    std::string msg = js["msg"];
    std::string from_name = js["from_name"];
    int to_id = js["id"];

    json response;
    response["msgid"] = PRIVATE_CHAT;
    response["msg"] = msg;
    response["from_name"] = from_name;
    response["time"] = time.toFormattedString();

    auto it = m_userConnMap.find(to_id); 
    if (it != m_userConnMap.end())
    {
        TcpConnectionPtr to_conn = it->second;
        to_conn->send(response.dump().append("\r\n"));
    } else {
        // 对方不在线
        // 存储离线消息
    }
}

void Service::handleDisplayFriendList(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int id = m_connUserMap[conn];
    std::vector<User> userVec = m_friendModel.query(id);

    std::vector<std::string> vec;
    for (User &user : userVec)
    {
        json js;
        js["id"] = user.getId();
        js["name"] = user.getName();
        js["state"] = user.getState();
        vec.push_back(js.dump());
    }

    json response;
    response["msgid"] = DISPLAY_FRIEND_LIST;
    response["friends"] = vec;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleFriendRequestList(const TcpConnectionPtr &conn, json &js, Timestamp time){
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

    for(std::string &str : vec){
        std::cout << str << std::endl;
    }

    json response;
    response["msgid"] = FRIEND_REQUEST_LIST;
    response["requests"] = vec;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleCreateGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    std::string group_name = js["groupname"];
    int userid = m_connUserMap[conn];

    Group group;
    group.setName(group_name);

    if (m_groupModel.createGroup(group)){
        m_groupModel.addGroup(userid, group.getId(), "master");
        json response;
        response["msgid"] = CREATE_GROUP_SUCCESS;
        conn->send(response.dump().append("\r\n"));
        return;
    } 

    json response;
    response["msgid"] = CREATE_GROUP_FAIL;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleDisplayAllGroupList(const TcpConnectionPtr &conn, json &js, Timestamp time){
    std::vector<Group> groupVec = m_groupModel.queryAllGroup();

    std::vector<std::string> vec;
    for (Group &group : groupVec)
    {
        json js;
        js["id"] = group.getId();
        js["name"] = group.getName();
        vec.push_back(js.dump());
    }

    json response;
    response["msgid"] = DISPLAY_ALLGROUP_LIST;
    response["groups"] = vec;
    conn->send(response.dump().append("\r\n"));
}

void Service::requestAddGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["add_group_id"];
    int userid = m_connUserMap[conn];

    m_groupModel.addGroup(userid, groupid, "request");

    json response;
    response["msgid"] = REQUEST_GROUP_SUCCESS;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleGroupRequestList(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["groupid"];

    if(m_groupModel.ifMasterOrManager(m_connUserMap[conn], groupid) == false){
        json response;
        response["msgid"] = DISPLAY_GROUP_REQUEST_FAIL;
        conn->send(response.dump().append("\r\n"));
        return;
    }

    std::vector<User> userVec = m_groupModel.queryGroupRequest(groupid);

    std::vector<std::string> vec;
    for (User &user : userVec)
    {
        json js;
        js["id"] = user.getId();
        js["name"] = user.getName();
        vec.push_back(js.dump());
    }

    json response;
    response["msgid"] = DISPLAY_GROUP_REQUEST;
    response["requests"] = vec;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleAddSomeoneToGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["groupid"];
    int userid = js["userid"];

    if(m_groupModel.ifMasterOrManager(m_connUserMap[conn], groupid) == false){
        json response;
        response["msgid"] = ADD_GROUP_FAIL;
        conn->send(response.dump().append("\r\n"));
        return;
    }

    m_groupModel.modifyGroupRole(userid, groupid, "normal");

    m_groupUserListMap[groupid].push_back(userid);

    json response;
    response["msgid"] = ADD_GROUP_SUCCESS;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleQuitGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["groupid"];
    int userid = m_connUserMap[conn];

    if(m_groupModel.ifManagerOrNormal(m_connUserMap[conn], groupid) == false){
        json response;
        response["msgid"] = QUIT_GROUP_FAIL;
        conn->send(response.dump().append("\r\n"));
        return;
    }

    m_groupModel.deleteGroupMember(userid, groupid);

    json response;
    response["msgid"] = QUIT_GROUP_SUCCESS;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleShowOwnGroupList(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = m_connUserMap[conn];
    std::vector<Group> groupVec = m_groupModel.queryOwnGroup(userid);

    std::vector<std::string> vec;
    for (Group &group : groupVec)
    {
        json js;
        js["id"] = group.getId();
        js["name"] = group.getName();
        vec.push_back(js.dump());
    }

    json response;
    response["msgid"] = DISPLAY_OWN_GROUP_LIST;
    response["groups"] = vec;
    conn->send(response.dump().append("\r\n"));
}


void Service::handleShowGroupMemberList(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["groupid"];

    if(m_groupModel.ifMasterOrManagerORnormal(m_connUserMap[conn], groupid) == false){
        json response;
        response["msgid"] = DISPLAY_GROUP_MEMBER_FAIL;
        conn->send(response.dump().append("\r\n"));
        return;
    }

    std::vector<User> userVec = m_groupModel.queryGroupMember(groupid);

    std::vector<std::string> vec;
    for (User &user : userVec)
    {
        json js;
        js["id"] = user.getId();
        js["name"] = user.getName();
        js["role"] = user.getState();
        vec.push_back(js.dump());
    }

    json response;
    response["msgid"] = DISPLAY_GROUP_MEMBER_LIST;
    response["members"] = vec;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleDisplaySetManagerMemberList(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["groupid"];

    if(m_groupModel.ifMaster(m_connUserMap[conn], groupid) == false){
        json response;
        response["msgid"] = DISPLAY_SET_MANAGER_MEMBER_FAIL;
        conn->send(response.dump().append("\r\n"));
        return;
    }

    std::vector<User> userVec = m_groupModel.queryGroupMember(groupid);

    std::vector<std::string> vec;
    for (User &user : userVec)
    {
        json js;
        js["id"] = user.getId();
        js["name"] = user.getName();
        js["role"] = user.getState();
        vec.push_back(js.dump());
    }

    json response;
    response["msgid"] = DISPLAY_SET_MANAGER_MEMBER_LIST;
    response["members"] = vec;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleSetManager(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["groupid"];
    int userid = js["userid"];

    std::string state = m_groupModel.queryGroupRole(userid, groupid);

    if(state != "normal")
    {
        json response;
        response["msgid"] = SET_MANAGER_FAIL;
        conn->send(response.dump().append("\r\n"));
        return;
    }

    m_groupModel.modifyGroupRole(userid, groupid, "manager");

    json response;
    response["msgid"] = SET_MANAGER_SUCCESS;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleCancelManager(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["groupid"];
    int userid = js["userid"];

    if(m_groupModel.ifManager(userid, groupid) == false){
        json response;
        response["msgid"] = CANCEL_MANAGER_FAIL;
        conn->send(response.dump().append("\r\n"));
        return;
    }

    m_groupModel.modifyGroupRole(userid, groupid, "normal");

    json response;
    response["msgid"] = CANCEL_MANAGER_SUCCESS;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleKickSomeoneInGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["groupid"];
    int userid = js["userid"];

    std::string kick_state = m_groupModel.queryGroupRole(m_connUserMap[conn], groupid); 
    std::string be_kick_state = m_groupModel.queryGroupRole(userid, groupid);

    if (kick_state == "master"){
        m_groupModel.deleteGroupMember(userid, groupid);
        m_groupUserListMap[groupid].erase(std::remove(m_groupUserListMap[groupid].begin(), m_groupUserListMap[groupid].end(), userid), m_groupUserListMap[groupid].end());

        json response;
        response["msgid"] = KICK_SOMEONE_SUCCESS;
        conn->send(response.dump().append("\r\n"));
    } else if(kick_state == "manager"){
        if(be_kick_state == "manager" || be_kick_state == "master"){
            json response;
            response["msgid"] = KICK_SOMEONE_FAIL;
            conn->send(response.dump().append("\r\n"));
            return;
        }
        m_groupModel.deleteGroupMember(userid, groupid);
        m_groupUserListMap[groupid].erase(std::remove(m_groupUserListMap[groupid].begin(), m_groupUserListMap[groupid].end(), userid), m_groupUserListMap[groupid].end());

        json response;
        response["msgid"] = KICK_SOMEONE_SUCCESS;
        conn->send(response.dump().append("\r\n"));
    } else {
        json response;
        response["msgid"] = KICK_SOMEONE_FAIL;
        conn->send(response.dump().append("\r\n"));
    }
}

void Service::handleCheckIfGroupMember(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["groupid"];
    int userid = m_connUserMap[conn];

    std::string state = m_groupModel.queryGroupRole(userid, groupid);

    if(state == ""){
        state = "not";
    } else {
        state = "yes";
    }

    json response;
    response["msgid"] = CHECK_IF_GROUP_MEMBER;
    response["state"] = state == "yes";
    conn->send(response.dump().append("\r\n"));
}

void Service::handleGroupChat(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["groupid"];
    std::string msg = js["msg"];
    std::string from_name = js["from_name"];
    
    std::vector<int> userVec = m_groupUserListMap[groupid];

    json response;
    response["msgid"] = GROUP_CHAT;
    response["msg"] = msg;
    response["from_name"] = from_name;
    response["time"] = time.toFormattedString();
    response["groupid"] = groupid;

    for (int &userid : userVec)
    {
        if (userid == m_connUserMap[conn])
        {
            continue;
        }

        auto it = m_userConnMap.find(userid);
        if (it != m_userConnMap.end())
        {
            it->second->send(response.dump().append("\r\n"));
        } else {
            // 存储离线消息
        }
    }
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
                m_connUserMap.erase(it->second);
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