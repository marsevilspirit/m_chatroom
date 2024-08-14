#include "service.h"
#include "../public.h"
#include "../m_netlib/Log/mars_logger.h"
#include "../ServerJsonCodec.h"
#include "../model/user.h"
#include "../model/group.h"
#include "../model/historyCasheManager.h"
#include "../model/privatechathistory.h"
#include <cstdio>
#include <mutex>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

extern std::shared_ptr<CacheManager> cacheManager;

Service *Service::getInstance() {
    static Service service;
    return &service;
}


Service::Service(){
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
    m_handlersMap[MASTER_DELETE_GROUP] = std::bind(&Service::handleMasterDeleteGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[SEND_FILE_DATABASE]  = std::bind(&Service::handleSendFileDataBase, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[VIEW_FILE] = std::bind(&Service::handleDisplayFileList, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[RECEIVE_FILE] = std::bind(&Service::handleReceiveFile, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[PRIVATE_CHAT_HISTORY] = std::bind(&Service::handleDisplayPrivateHistory, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_handlersMap[GROUP_CHAT_HISTORY] = std::bind(&Service::handleDisplayGroupHistory, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void Service::groupUserListMapInit(){
    LogInfo("groupUserList init")

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
    cacheManager->flushCacheToDatabase();  // 刷新缓存到数据库
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

    LogInfo("{} want to reg", name)

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

    LogInfo("{} want to login", name)

    User user = m_userModel.query(name);
    LogTrace("user id:{} name:{} pwd:{} state:{}", user.getId(), user.getName(), user.getPwd(), user.getState())
    if (user.getName() == name && user.getPwd() == pwd){
        if (user.getState() == "online"){
            json response;
            response["msgid"] = LOGIN_REPEAT;
            conn->send(response.dump().append("\r\n"));
        } else {

            m_userConnMap[user.getId()] = conn;
            m_connUserMap[conn] = user.getId();
            m_connStatusMap[conn] = true;

            user.setState("online");
            m_userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_SUCCESS;
            response["msg"] = "登录成功";
            response["id"] = user.getId();
            response["name"] = user.getName();

            conn->send(response.dump().append("\r\n"));

            // 向在线好友发送上线通知
            std::vector<User> userVec = m_friendModel.query(user.getId());
            for (User &userTmp : userVec)
            {
                auto it = m_userConnMap.find(userTmp.getId());
                if (it != m_userConnMap.end())
                {
                    json response;
                    response["msgid"] = FRIEND_ONLINE;
                    response["id"] = user.getId();
                    response["name"] = user.getName();
                    it->second->send(response.dump().append("\r\n"));
                }
            }
        }
    } else { // 不存在
        json response;
        response["msgid"] = LOGIN_FAIL;
        response["msg"] = "登录失败, 用户不存在";
        conn->send(response.dump().append("\r\n"));
    }
}

void Service::handleDisplayAllUserList(const TcpConnectionPtr &conn, json &js, Timestamp time){
    LogInfo("{} want to display all user list", m_connUserMap[conn])

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

    LogInfo("{} want to add friend {}", m_connUserMap[conn], add_name)

    User add_user = m_userModel.query(add_name);

    int from_id = m_connUserMap[conn];
    int to_id = add_user.getId();

    LogTrace("from_id:{} to_id:{}", from_id, to_id)

    std::string state1 = m_friendModel.getState(from_id, to_id);
    std::string state2 = m_friendModel.getState(to_id, from_id);

    LogTrace("state1:{} state2:{}", state1, state2)

    if ((state1 == "" && state2 == "") || (state1 == "request" && state2 == "")) // 发起好友请求
    {
        m_friendModel.insert(from_id, to_id, "request");
        json response;
        response["msgid"] = ADD_FRIEND_REQUEST;
        conn->send(response.dump().append("\r\n"));

        json response2;
        response2["msgid"] = TO_ADD_FRIEND_REQUEST;
        auto it = m_userConnMap.find(to_id);
        if (it != m_userConnMap.end())
        {
            it->second->send(response2.dump().append("\r\n"));
        }
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
            // 向对方通知同意添加好友
            auto it = m_userConnMap.find(to_id);
            if (it != m_userConnMap.end())
            {
                json to_response;
                to_response["msgid"] = THE_OTHER_AGREE_FRIEND_REQUEST;
                to_response["friend_name"] = m_userModel.query(from_id).getName();
                it->second->send(to_response.dump().append("\r\n"));
            }
        }
        return;
    }
}

void Service::handleDeleteFriend(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int delete_id = js["del_id"];

    LogInfo("{} want to delete friend {}", m_connUserMap[conn], delete_id)

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

    LogInfo("{} want to block friend {}", user_id, block_id)

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

    LogInfo("{} want to unblock friend {}", user_id, unblock_id)

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

    LogInfo("{} want to check block state of friend {}", user_id, check_id)

    LogTrace("check_id:{} user_id:{}", check_id, user_id)

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
        LogInfo("m_userConnMap is null")
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

    cacheManager->storePrivateMessageInCache(std::to_string(m_connUserMap[conn]), std::to_string(to_id), msg);

    auto it = m_userConnMap.find(to_id); 
    if (it != m_userConnMap.end())
    {
        TcpConnectionPtr to_conn = it->second;
        to_conn->send(response.dump().append("\r\n"));
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

    json response;
    response["msgid"] = FRIEND_REQUEST_LIST;
    response["requests"] = vec;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleCreateGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    std::string group_name = js["groupname"];
    int userid = m_connUserMap[conn];

    LogInfo("{} want to create group {}", userid, group_name)

    Group group;
    group.setName(group_name);

    if (m_groupModel.createGroup(group)){

        m_groupModel.addGroup(userid, group.getId(), "master");
        m_groupUserListMap[group.getId()].push_back(userid);

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

    LogInfo("{} want to add group {}", userid, groupid)

    json response;

    if(m_groupModel.addGroup(userid, groupid, "request")){
        response["msgid"] = REQUEST_GROUP_SUCCESS;
        conn->send(response.dump().append("\r\n"));

        response_to_master_or_manager(groupid, userid);
        return;
    }

    response["msgid"] = REQUEST_GROUP_FAIL;
    conn->send(response.dump().append("\r\n"));
}

void Service::response_to_master_or_manager(int groupid, int userid){
    std::vector<User> userVec = m_groupModel.queryGroupMember(groupid);

    if (userVec.size() == 0)
    {
        LogInfo("no member in group")
        return;
    }

    for (User &user : userVec)
    {
        std::cout << user.getId() << " " << user.getName() << " " << user.getState() << std::endl;
        if (user.getState() == "master" || user.getState() == "manager")
        {
            auto it = m_userConnMap.find(user.getId());
            if (it != m_userConnMap.end())
            {
                json response;
                response["msgid"] = GROUP_REQUEST;
                response["groupid"] = groupid;
                response["userid"] = userid;
                it->second->send(response.dump().append("\r\n"));
            }
        }
    }
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

    LogInfo("{} want to add {} to group {}", m_connUserMap[conn], userid, groupid)

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

    json response2;
    response2["msgid"] = GROUP_REQUEST_ACCEPTED;
    response2["groupid"] = groupid;
    response2["accept_id"] = m_connUserMap[conn];
    auto it = m_userConnMap.find(userid);
    if (it != m_userConnMap.end())
    {
        it->second->send(response2.dump().append("\r\n"));
    }
}

void Service::handleQuitGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["groupid"];
    int userid = m_connUserMap[conn];

    LogInfo("{} want to quit group {}", userid, groupid)

    if(m_groupModel.ifManagerOrNormal(m_connUserMap[conn], groupid) == false){
        json response;
        response["msgid"] = QUIT_GROUP_FAIL;
        conn->send(response.dump().append("\r\n"));
        return;
    }

    m_groupModel.deleteGroupMember(userid, groupid);

    m_groupUserListMap.find(groupid)->second.erase(std::remove(m_groupUserListMap.find(groupid)->second.begin(), m_groupUserListMap.find(groupid)->second.end(), userid), m_groupUserListMap.find(groupid)->second.end());

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
        if(group.getGroupRole() == "request" || group.getGroupRole() == ""){
            continue;
        }

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

    LogInfo("{} want to set {} as manager in group {}", m_connUserMap[conn], userid, groupid)

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

    LogInfo("{} want to cancel {} as manager in group {}", m_connUserMap[conn], userid, groupid)

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

    LogInfo("{} want to kick {} in group {}", m_connUserMap[conn], userid, groupid)

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

    json response;
    response["msgid"] = CHECK_IF_GROUP_MEMBER;
    response["state"] = !((state == "")|| (state == "request"));
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

    cacheManager->storeGroupMessageInCache(std::to_string(groupid), std::to_string(m_connUserMap[conn]), msg);

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
        } 
    }
}

void Service::handleMasterDeleteGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["groupid"];
    int userid = m_connUserMap[conn];

    if(m_groupModel.ifMaster(userid, groupid) == false){
        json response;
        response["msgid"] = MASTER_DELETE_GROUP_FAIL;
        conn->send(response.dump().append("\r\n"));
        return;
    }

    m_groupModel.deleteGroup(groupid);
    m_groupUserListMap.erase(groupid);

    json response;
    response["msgid"] = MASTER_DELETE_GROUP_SUCCESS;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleSendFileDataBase(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["sender"];
    int recv_id = js["receiver"];
    std::string filename = js["filename"];

    m_fileModel.insertFile(userid, recv_id, filename);

    json response;
    response["msgid"] = SEND_FILE_DATABASE_SUCCESS;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleDisplayFileList(const TcpConnectionPtr &conn, json &js, Timestamp time){
    std::vector<File> fileVec = m_fileModel.query(m_connUserMap[conn]);

    std::vector<std::string> vec;
    for (File &file : fileVec)
    {
        json js;
        js["sender"] = file.getSenderId();
        js["receiver"] = file.getReceiverId();
        js["filename"] = file.getFileName();
        js["time"] = file.getCreateTime();
        vec.push_back(js.dump());
    }

    json response;
    response["msgid"] = VIEW_FILE;
    response["files"] = vec;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleReceiveFile(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    LogInfo("Receive file")

    std::string filename = js["filename"];
    std::string filePath = "./received_files/" + filename;

    // 打开文件
    int fileFd = open(filePath.c_str(), O_RDONLY);
    if (fileFd < 0) {
        LogError("Failed to open file: {} - {}", filePath, strerror(errno))
        json response;
        response["msgid"] = RECEIVE_FILE_FAIL;
        conn->send(response.dump().append("\r\n"));
        return;
    }

    // 获取文件大小
    struct stat fileStat;
    if (fstat(fileFd, &fileStat) < 0) {
        LogError("Failed to get file stats: {}", strerror(errno))
        close(fileFd);
        return;
    }
    size_t fileSize = fileStat.st_size;

    LogInfo("fileSize: {}", fileSize)

    // 移除当前连接与用户映射，防止群聊信息干扰
    m_userConnMap.erase(m_connUserMap[conn]);

    // 发送文件元数据
    json metadata;
    metadata["msgid"] = SEND_FILE_SERVER;  // 标识这是一个文件传输
    metadata["filename"] = filename;
    metadata["filesize"] = fileSize;
    conn->send(metadata.dump().append("\r\n"));

    usleep(10000);

    // 获取连接的文件描述符
    int connFd = conn->getFd();

    // 获取当前文件描述符的标志
    int flags = fcntl(connFd, F_GETFL, 0);
    if (flags < 0) {
        LogError("Failed to get file descriptor flags: {}", strerror(errno))
        return;
    }

    // 设置为阻塞模式
    if (fcntl(connFd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
        LogError("Failed to set blocking mode: {}", strerror(errno))
        return;
    }

    // 逐块发送文件数据
    off_t offset = 0;
    size_t remaining = fileSize;
    while (remaining > 0) {
        ssize_t sent = sendfile(connFd, fileFd, &offset, remaining);
        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 暂时不可用，稍后重试
                usleep(10000); // 10ms 延迟
                continue;
            } else {
                LogError("Sendfile error: {}", strerror(errno))
                close(fileFd);
                return;
            }
        }
        remaining -= sent;

        LogInfo("Progress: {}/{} bytes", fileSize - remaining, fileSize)
    }

    close(fileFd);
    LogInfo("File sent successfully.")

    // 恢复为非阻塞模式
    if (fcntl(connFd, F_SETFL, flags | O_NONBLOCK) < 0) {
        LogError("Failed to restore non-blocking mode: {}", strerror(errno))
        return;
    }

    // 重新添加当前连接与用户的映射
    m_userConnMap[m_connUserMap[conn]] = conn;

    // 发送传输结束标识
    json endMessage;
    endMessage["msgid"] = SEND_FILE_END;
    conn->send(endMessage.dump().append("\r\n"));

    // 发送文件接收完成的消息
    json response;
    response["msgid"] = RECEIVE_FILE_FINISH;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleDisplayPrivateHistory(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = m_connUserMap[conn];
    int friendid = js["id"];

    std::vector<PrivateChatHistory> historyVec = m_historyModel.queryPrivateHistory(userid, friendid, 50);

    std::vector<std::string> vec;
    for (PrivateChatHistory &history : historyVec)
    {
        json js;
        js["from"] = history.getSenderId();
        js["to"] = history.getReceiverId();
        js["msg"] = history.getMessage();
        js["time"] = history.getSendTime();
        vec.push_back(js.dump());
    }

    // 逆转向量顺序
    std::reverse(vec.begin(), vec.end());

    json response;
    response["msgid"] = DISPLAY_PRIVATE_HISTORY;
    response["history"] = vec;
    conn->send(response.dump().append("\r\n"));
}

void Service::handleDisplayGroupHistory(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int groupid = js["groupid"];

    std::vector<GroupChatHistory> historyVec = m_historyModel.queryGroupHistory(groupid, 50);

    std::vector<std::string> vec;
    for (GroupChatHistory &history : historyVec)
    {
        json js;
        js["from"] = history.getSenderId();
        js["groupid"] = history.getGroupId();
        js["msg"] = history.getMessage();
        js["time"] = history.getSendTime();
        vec.push_back(js.dump());
    }

    // 逆转向量顺序
    std::reverse(vec.begin(), vec.end());

    json response;
    response["msgid"] = DISPLAY_GROUP_HISTORY;
    response["history"] = vec;
    conn->send(response.dump().append("\r\n"));
}

void Service::checkIfConnAlive() {
    std::lock_guard<std::mutex> lock(m_connMutex);

    if (m_connStatusMap.empty()) {
        return;
    }

    std::vector<std::shared_ptr<TcpConnection>> connectionsToDelete;

    // 首先收集需要删除的连接
    for (auto it = m_connStatusMap.begin(); it != m_connStatusMap.end(); ++it) {
        if (it->second == false) {
            connectionsToDelete.push_back(it->first);
        } else {
            it->second = false; // 重置心跳标志
        }
    }

    // 然后删除那些已经确定需要删除的连接
    for (const auto& conn : connectionsToDelete) {
        int id = m_connUserMap[conn];
        LogInfo("心跳检测到用户 {} 掉线", id);

        json response;
        response["msgid"] = CLIENT_LONGTIME_EXIT;
        auto userConn = m_userConnMap.find(id);
        if (userConn != m_userConnMap.end()) {
            userConn->second->send(response.dump().append("\r\n"));
        }

        usleep(10000);

        conn->shutdown();

        m_userConnMap.erase(id);
        m_connUserMap.erase(conn);
        m_connStatusMap.erase(conn);

        User user = m_userModel.query(id);
        user.setState("offline");
        m_userModel.updateState(user);
    }
}

// 处理客户端异常退出
void Service::clientClose(const TcpConnectionPtr &conn){
    LogInfo("client {} close", m_connUserMap[conn]);

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
                m_connStatusMap.erase(it->second);
                break;
            }
        }
    }

    // 更新用户的状态信息
    if (user.getId() != -1)   //没找到这个 ，id就是默认的构造函数设置的-1，不等于-1说明是有效的用户
    {
        user.setState("offline");
        m_userModel.updateState(user);
    }
}
