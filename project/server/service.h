//Created by mars on 6/8/24

#ifndef MARS_SERVICE_H
#define MARS_SERVICE_H

#include <functional>
#include <unordered_map>
#include <mutex>

#include "../model/usermodel.h"
#include "../m_netlib/Net/TcpConnection.h"
#include "../json.hpp"
#include "../model/friendmodel.h"
#include "../model/groupmodel.h"

using json = nlohmann::json;

using namespace mars;
using namespace mars::net;
using namespace mars::base;

using MsgHandler = std::function<void(const TcpConnectionPtr& conn, json &js, Timestamp)>;

class Service {
public:
    // 获取单例对象
    static Service* getInstance();

    void groupUserListMapInit();

    // 想要进入房间
    void wantEnter(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 注册消息以及对应的回调操作
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleDisplayAllUserList(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleAddFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleDeleteFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handlePrivateChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleDisplayFriendList(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleFriendRequestList(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleBlockFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleUnBlockFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleBlockFriendList(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleCheckBlock(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleCreateGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleDisplayAllGroupList(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void requestAddGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleGroupRequestList(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleAddSomeoneToGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleQuitGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleShowOwnGroupList(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleShowGroupMemberList(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleDisplaySetManagerMemberList(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleSetManager(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleCancelManager(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleKickSomeoneInGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleCheckIfGroupMember(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleGroupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

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
    std::unordered_map<TcpConnectionPtr, int> m_connUserMap;
    std::unordered_map<int, std::vector<int>> m_groupUserListMap;
    std::mutex m_connMutex;

    UserModel m_userModel;
    FriendModel m_friendModel;
    GroupModel m_groupModel;
};

#endif //MARS_SERVICE_H
