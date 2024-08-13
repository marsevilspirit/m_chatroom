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
#include "../model/filemodel.h"
#include "../model/historymodel.h"

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

    void response_to_master_or_manager(int groupid, int userid);

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

    void handleMasterDeleteGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleSendFile(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleSendFileDataBase(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleDisplayFileList(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleReceiveFile(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleDisplayPrivateHistory(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleDisplayGroupHistory(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void checkIfConnAlive();

    // 处理客户端退出
    void clientClose(const TcpConnectionPtr &conn);

    // 重置
    void reset();

    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);

    void setConnStatus(const TcpConnectionPtr &conn, bool status){
        std::lock_guard<std::mutex> lock(m_connMutex);
        m_connStatusMap[conn] = status;
    }
private:
    Service();

    std::unordered_map<int, MsgHandler> m_handlersMap;
    std::unordered_map<int, TcpConnectionPtr> m_userConnMap;
    std::unordered_map<TcpConnectionPtr, int> m_connUserMap;
    std::unordered_map<int, std::vector<int>> m_groupUserListMap;
    std::unordered_map<TcpConnectionPtr, bool> m_connStatusMap;
    std::mutex m_connMutex;

    UserModel m_userModel;
    FriendModel m_friendModel;
    GroupModel m_groupModel;
    FileModel m_fileModel;
    HistoryModel m_historyModel;
};

#endif //MARS_SERVICE_H
