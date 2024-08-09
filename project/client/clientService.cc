#include "clientService.h"
#include "client.h"
#include "util.h"
#include "../public.h"
#include "../model/user.h"

#include <iostream>
#include <limits>
#include <semaphore.h>
#include <atomic>

sem_t login_sem;
sem_t add_someone_to_group;
sem_t set_manager;
sem_t check_group_member;
std::atomic<bool> login_flag(false);
std::atomic<bool> if_block(false);
std::atomic<bool> if_master_ormanager(false);
std::atomic<bool> if_master(false);
std::atomic<bool> if_group_member(false);

std::atomic<int> private_chat_id(-1);

User CurrentUser;

static void clearInputBuffer() 
{
    std::cin.clear(); // 清除输入状态标志
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 忽略剩余输入
}

void handleServerMessage(Client* client, json &js, Timestamp time){
    int msgid = js["msgid"].get<int>();
    switch (msgid){
        case REG_SUCCESS:        std::cout << "注册成功, 用户id:" << js["id"].get<int>() << std::endl;                                  break;
        case REG_FAIL:           std::cout << "注册失败, 用户名已存在" << std::endl;                                                    break;
        case LOGIN_SUCCESS:      std::cout << "登录成功" << std::endl; login_flag = true;  UserInit(js); sem_post(&login_sem);          break;
        case LOGIN_REPEAT:       std::cout << "重复登录" << std::endl; login_flag = false; sem_post(&login_sem);                        break;
        case LOGIN_FAIL:         std::cout << "登录失败, 用户名或密码错误" << std::endl; login_flag = false; sem_post(&login_sem);      break;
        case DISPLAY_ALLUSER_LIST: displayAllUserList(js); break;
        case ADD_FRIEND_SUCCESS: std::cout << "添加成功" << std::endl;                                                                  break;
        case ADD_FRIEND_REQUEST: std::cout << "你向对方发起好友申请" << std::endl;                                                      break;
        case UPDATE_FRIEND_LIST: std::cout << "更新好友列表" << std::endl;  break;
        case DELETE_FRIEND_SUCCESS: std::cout << "删除好友成功" << std::endl;  break;
        case DELETED_FRIEND_SUCCESS: std::cout << "你被一位好友删除" <<std::endl; break;
        case DELETE_FRIEND_FAIL:  std::cout << "删除好友失败，你与对方不是好友" << std::endl; break;
        case BLOCK_FRIEND_FAIL:  std::cout << "屏蔽好友失败，你与对方不是好友" << std::endl; break;
        case BLOCK_FRIEND_SUCCESS: std::cout << "屏蔽好友成功" << std::endl; checkIfBlock(client, private_chat_id); break;
        case UNBLOCK_FRIEND_FAIL:  std::cout << "解除屏蔽好友失败，你并未屏蔽对方" << std::endl; break;
        case UNBLOCK_FRIEND_SUCCESS: std::cout << "解除屏蔽好友成功" << std::endl; checkIfBlock(client, private_chat_id); break;
        case PRIVATE_CHAT:       handlePrivateChat(js);   break;
        case GROUP_CHAT:         handleGroupChat(js); break;
        case FRIEND_REQUEST_LIST: displayRequestList(js); break;
        case DISPLAY_FRIEND_LIST: displayFriendList(js); break;
        case BLOCK_FRIEND_LIST:  displayBlockList(js); break;
        case CHECK_BLOCK:        if_block = (js["state1"] == "block") || (js["state2"] == "block"); std::cout << "if_block: " << if_block << std::endl;  break;
        case CREATE_GROUP_SUCCESS: std::cout << "创建群聊成功" << std::endl; break;
        case CREATE_GROUP_FAIL: std::cout << "创建群聊失败" << std::endl; break;
        case DISPLAY_ALLGROUP_LIST: displayAllGroupList(js); break;
        case REQUEST_GROUP_SUCCESS: std::cout << "提交加入群聊申请成功" << std::endl; break;
        case DISPLAY_GROUP_REQUEST: if_master_ormanager = displayGroupRequestList(js); sem_post(&add_someone_to_group);  break;
        case DISPLAY_GROUP_REQUEST_FAIL: std::cout << "你不是群主或管理员" << std::endl;if_master_ormanager = false; sem_post(&add_someone_to_group); break;
        case ADD_GROUP_SUCCESS: std::cout << "添加群成员成功" << std::endl; break;
        case ADD_GROUP_FAIL: std::cout << "添加群成员失败" << std::endl; break;
        case QUIT_GROUP_FAIL: std::cout << "退出群聊失败" << std::endl; break;
        case QUIT_GROUP_SUCCESS: std::cout << "退出群聊成功" << std::endl; break;
        case DISPLAY_OWN_GROUP_LIST: displayOwnGroupList(js); break;
        case DISPLAY_GROUP_MEMBER_LIST: displayGroupMemberList(js); break;
        case DISPLAY_GROUP_MEMBER_FAIL: std::cout << "你不是群成员" << std::endl; break;
        case DISPLAY_SET_MANAGER_MEMBER_FAIL: std::cout << "你不是群主" << std::endl; if_master = false; sem_post(&set_manager); break;
        case DISPLAY_SET_MANAGER_MEMBER_LIST: displayGroupMemberList(js); if_master = true; sem_post(&set_manager); break;
        case SET_MANAGER_SUCCESS: std::cout << "设置管理员成功" << std::endl; break;
        case SET_MANAGER_FAIL: std::cout << "设置管理员失败" << std::endl; break;
        case CANCEL_MANAGER_FAIL: std::cout << "对方不是管理员" << std::endl; break;
        case CANCEL_MANAGER_SUCCESS: std::cout << "取消管理员成功" << std::endl; break;
        case KICK_SOMEONE_FAIL: std::cout << "踢人失败, 权限不够" << std::endl; break;
        case KICK_SOMEONE_SUCCESS: std::cout << "踢人成功" << std::endl; break;
        case CHECK_IF_GROUP_MEMBER: if_group_member = js["state"]; sem_post(&check_group_member); break;
        case TEST: std::cout << "收到test json send" <<std::endl; break;
    }
}

void UserInit(json &js){
    CurrentUser.setId(js["id"]);
    CurrentUser.setName(js["name"]);

    std::cout << "当前用户 name: " << CurrentUser.getName() << " id: " << CurrentUser.getId() << std::endl;
}


void EnterChatRoom(Client &client){

    sem_init(&login_sem, 0, 0);

    while(1)
    {

        std::cout << "1.注册     2.登录" << std::endl;
        std::cout << "3.退出" << std::endl;
        std::cout << "请输入:";

        int choice;
        std::cin >> choice;
        clearInputBuffer(); // 清空输入缓冲区

        switch (choice){
            case 1: reg(client);                                  break;
            case 2: if(login(client)) {EnterCommandMenu(client);} break;
            case 3: sem_destroy(&login_sem); exit(EXIT_SUCCESS);  break;
        }
    }   

}

void reg(Client &client){
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
    client.send(response.dump().append("\r\n"));
}

bool login(Client &client){
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
    client.send(response.dump().append("\r\n"));

    sem_wait(&login_sem);

    if(login_flag){
        return true;
    } else {
        return false;
    }
}

void EnterCommandMenu(Client &client){
    while(1){
        std::cout << "\n";
        std::cout << "command界面\n";
        std::cout << "1.私聊        2.群聊\n";
        std::cout << "3.添加好友    4.删除好友\n";
        std::cout << "5.屏蔽好友    6.解除屏蔽\n";
        std::cout << "7.好友列表    8.好友申请列表\n";
        std::cout << "9.创建群聊    10.申请加入群聊\n";
        std::cout << "11.退出群聊   12.解散群聊\n";
        std::cout << "13.群聊列表   14.设置管理员\n";
        std::cout << "15.取消管理员 16.踢人\n";
        std::cout << "17.群聊成员表 18.群成员申请列表\n";
        std::cout << "19.发送文件   20.查看文件\n";
        std::cout << "21.接收文件   22.退出\n";
        std::cout << "\n";

        int choice;
        std::cin >> choice;
        clearInputBuffer();

        switch (choice){
            case 1:  privateChat(client);              break;
            case 2:  groupChat(client);    break;
            case 3:  addFriend(client);    break;
            case 4:  deleteFriend(client); break;
            case 5:  blockFriend(client);              break;
            case 6:  unblockFriend(client);  break;
            case 7:  tellServerWantToLookFriendList(client); break;
            case 8:  tellServerWantToLookRequestList(client); break;
            case 9:  createGroup(client);         break;
            case 10: requestEnterGroup(client);   break;
            case 11: quitGroup(client);           break;
            case 12: std::cout << "解散群聊" << std::endl; break;
            case 13: tellServerWantToLookAllGroupList(client); break;
            case 14: setGroupManager(client);     break;
            case 15: cancelGroupManager(client);   break;
            case 16: kickSomeoneInGroup(client);   break;
            case 17: GroupMemberList(client); break;
            case 18: addSomeoneToGroup(client); break;
            case 19: std::cout << "发送文件" << std::endl; break;
            case 20: std::cout << "查看文件" << std::endl; break;
            case 21: std::cout << "接收文件" << std::endl; break;
            case 22: ExitChatRoom();                  break;
        }
    }
}

void tellServerWantToLookAllUserList(Client &client){
    json response;
    response["msgid"] = DISPLAY_ALLUSER_LIST;
    client.send(response.dump().append("\r\n"));
}

void displayAllUserList(json &js){
    std::cout << "用户列表:" << std::endl;
    std::vector<std::string> vec = js["users"];
    for(std::string &str : vec){
        json js = json::parse(str);
        std::cout << "id:" << js["id"] << " name:" << js["name"] << " state:" << js["state"] << std::endl;
    }
}

void addFriend(Client &client){
    tellServerWantToLookAllUserList(client);

    std::string name;
    std::cout << "请输入要添加的好友名:";
    std::cin >> name;
    clearInputBuffer();

    if(name == CurrentUser.getName()){
        std::cout << "不能添加自己为好友" << std::endl;
        return;
    }

    json response;
    response["msgid"] = ADD_FRIEND;
    response["add_name"] = name;

    client.send(response.dump().append("\r\n"));
}

void deleteFriend(Client &client){
    tellServerWantToLookFriendList(client);

    int id;
    std::cout << "请输入要删除的好友id:";
    std::cin >> id;
    clearInputBuffer();

    json response;
    response["msgid"] = DELETE_FRIEND;
    response["del_id"] = id;

    client.send(response.dump().append("\r\n"));
}

void blockFriend(Client& client){
    tellServerWantToLookFriendList(client);

    int id;
    std::cout << "请输入要屏蔽的好友id:";
    std::cin >> id;
    clearInputBuffer();

    json response;
    response["msgid"] = BLOCK_FRIEND;
    response["block_id"] = id;

    client.send(response.dump().append("\r\n"));
}

void unblockFriend(Client& client){
    tellServerWantToLookBlockList(client);

    int id;
    std::cout << "请输入要解除屏蔽的好友id:";
    std::cin >> id;
    clearInputBuffer();

    json response;
    response["msgid"] = UNBLOCK_FRIEND;
    response["unblock_id"] = id;

    client.send(response.dump().append("\r\n"));
}

void tellServerWantToLookRequestList(Client &client){
    json response;
    response["msgid"] = FRIEND_REQUEST_LIST;
    client.send(response.dump().append("\r\n"));
}

void tellServerWantToLookFriendList(Client &client){
    json response;
    response["msgid"] = DISPLAY_FRIEND_LIST;
    client.send(response.dump().append("\r\n"));
}

void tellServerWantToLookBlockList(Client &client){
    json response;
    response["msgid"] = BLOCK_FRIEND_LIST;
    client.send(response.dump().append("\r\n"));
}

void displayRequestList(json &js){
    std::cout << "好友申请列表:" << std::endl;
    std::vector<std::string> vec = js["requests"];
    for(std::string &str : vec){
        json js = json::parse(str);
        std::cout << "id:" << js["id"] << " name:" << js["name"] << std::endl;
    }
}

void displayFriendList(json &js){
    std::cout << "好友列表:" << std::endl;
    std::vector<std::string> vec = js["friends"];
    for(std::string &str : vec){
        json js = json::parse(str);
        std::cout << "id:" << js["id"] << " name:" << js["name"] << " state:" << js["state"] << std::endl;
    }
}

void displayBlockList(json &js){
    std::cout << "屏蔽列表:" << std::endl;
    std::vector<std::string> vec = js["blocks"];
    for(std::string &str : vec){
        json js = json::parse(str);
        std::cout << "id:" << js["id"] << " name:" << js["name"] << std::endl;
    }
}

void checkIfBlock(Client &client, int id_to_chat){
    json response;
    response["msgid"] = CHECK_BLOCK;
    response["check_id"] = id_to_chat;

    client.send(response.dump().append("\r\n"));
}

void checkIfBlock(Client* client, int id_to_chat){
    json response;
    response["msgid"] = CHECK_BLOCK;
    response["check_id"] = id_to_chat;

    client->send(response.dump().append("\r\n"));
}

void privateChat(Client &client) {
    tellServerWantToLookFriendList(client);

    std::cout << "请输入好友id:";
    int id_to_chat;
    std::cin >> id_to_chat;
    clearInputBuffer();
    std::string msg;
    json response;

    checkIfBlock(client, id_to_chat);

    while (!if_block) {

        std::cout << "in while loop if_block: " << if_block << std::endl;

        private_chat_id = id_to_chat;

        std::getline(std::cin, msg);

        if (msg == "exit") {
            break;
        }

        response["msgid"] = PRIVATE_CHAT;
        response["id"] = id_to_chat;
        response["from_name"] = CurrentUser.getName();
        response["msg"] = msg;

        client.send(response.dump().append("\r\n"));
    }

    if (if_block) {
        std::cout << "你已被对方屏蔽" << std::endl;
    }
}

void handlePrivateChat(json &js){
    std::string from_name = js["from_name"];
    std::string msg = js["msg"];
    std::string time = js["time"];
    std::cout << time << " " << from_name << "说: " << msg << std::endl;
}

void createGroup(Client& client){
    std::cout << "请输入群名: " << std::endl;
    std::string groupname;
    std::cin >> groupname;
    clearInputBuffer();

    json response;
    response["msgid"] = CREATE_GROUP;
    response["groupname"] = groupname;

    client.send(response.dump().append("\r\n"));
}

void tellServerWantToLookAllGroupList(Client &client){
    json response;
    response["msgid"] = DISPLAY_ALLGROUP_LIST;
    client.send(response.dump().append("\r\n"));
}

void displayAllGroupList(json &js){
    std::cout << "群组列表:" << std::endl;
    std::vector<std::string> vec = js["groups"];
    for(std::string &str : vec){
        json js = json::parse(str);
        std::cout << "id:" << js["id"] << " name:" << js["name"] << std::endl;
    }
}

void requestEnterGroup(Client &client){
    tellServerWantToLookAllGroupList(client);
    
    std::cout << "请输入要加入的群id: " << std::endl;

    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    json response;
    response["msgid"] = REQUEST_GROUP;
    response["add_group_id"] = std::stoi(groupid);

    client.send(response.dump().append("\r\n"));
}

void tellServerWantToLookGroupRequestList(Client &client, int groupid){
    json response;
    response["msgid"] = GROUP_REQUEST_LIST;
    response["groupid"] = groupid;
    client.send(response.dump().append("\r\n"));
}

int displayGroupRequestList(json &js){
    std::cout << "群聊人员申请列表:" << std::endl;
    std::vector<std::string> vec = js["requests"];

    if(vec.size() == 0){
        std::cout << "无人申请" << std::endl;
        return 0;
    }

    for(std::string &str : vec){
        json js = json::parse(str);
        std::cout << "id:" << js["id"] << " name:" << js["name"] << std::endl;
    }

    return 1;
}

void addSomeoneToGroup(Client &client){
    sem_init(&add_someone_to_group, 0, 0);

    tellServerWantToLookAllGroupList(client);
    std::cout << "请输入要查看的群id: " << std::endl;


    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    tellServerWantToLookGroupRequestList(client, std::stoi(groupid));

    sem_wait(&add_someone_to_group);

    if(!if_master_ormanager){
        return;
    }

    std::cout << "请输入要加入的用户id: " << std::endl;

    std::string userid;
    std::cin >> userid;
    clearInputBuffer();

    json response;
    response["msgid"] = ADD_GROUP;
    response["groupid"] = std::stoi(groupid);
    response["userid"] = std::stoi(userid);

    client.send(response.dump().append("\r\n"));
}

void quitGroup(Client &client){
    tellServerShowOwnGroupList(client);

    std::cout << "请输入要退出的群id: " << std::endl;

    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    json response;
    response["msgid"] = QUIT_GROUP;
    response["groupid"] = std::stoi(groupid);

    client.send(response.dump().append("\r\n"));
}

void tellServerShowOwnGroupList(Client &client){
    json response;
    response["msgid"] = DISPLAY_OWN_GROUP_LIST;
    client.send(response.dump().append("\r\n"));
}

void displayOwnGroupList(json &js){
    std::cout << "自己所在的群聊列表:" << std::endl;
    std::vector<std::string> vec = js["groups"];
    for(std::string &str : vec){
        json js = json::parse(str);
        std::cout << "id:" << js["id"] << " name:" << js["name"] << std::endl;
    }
}

void GroupMemberList(Client &client){
    tellServerShowOwnGroupList(client);

    std::cout << "请输入要查看的群id: " << std::endl;

    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    tellServerWantToLookGroupMemberList(client, std::stoi(groupid));
}

void tellServerWantToLookGroupMemberList(Client &client, int groupid){
    json response;
    response["msgid"] = DISPLAY_GROUP_MEMBER_LIST;
    response["groupid"] = groupid;
    client.send(response.dump().append("\r\n"));
}

void displayGroupMemberList(json &js){
    std::cout << "群聊成员列表:" << std::endl;
    std::vector<std::string> vec = js["members"];
    for(std::string &str : vec){
        json js = json::parse(str);
        std::cout << "id:" << js["id"] << " name:" << js["name"] << " role:" << js["role"] << std::endl;
    }
}

void tellServerWantToSetManagerGroupMemberList(Client &client, int groupid){
    json response;
    response["msgid"] = DISPLAY_GROUP_SET_MANAGER_MEMBER_LIST;
    response["groupid"] = groupid;
    client.send(response.dump().append("\r\n"));
}

void setGroupManager(Client &client){
    sem_init(&set_manager, 0, 0);

    tellServerShowOwnGroupList(client);

    std::cout << "请输入要设置管理员的群id: " << std::endl;

    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    tellServerWantToSetManagerGroupMemberList(client, std::stoi(groupid));

    sem_wait(&set_manager);

    sem_destroy(&set_manager);

    if(!if_master){
        return;
    }

    std::cout << "请输入要设置为管理员的用户id: " << std::endl;

    std::string userid;
    std::cin >> userid;
    clearInputBuffer();

    if(std::stoi(userid) == CurrentUser.getId()){
        std::cout << "不能设置自己为管理员" << std::endl;
        return;
    }

    json response;
    response["msgid"] = SET_MANAGER;
    response["groupid"] = std::stoi(groupid);
    response["userid"] = std::stoi(userid);

    client.send(response.dump().append("\r\n"));
}

void cancelGroupManager(Client &client){
    sem_init(&set_manager, 0, 0);

    tellServerShowOwnGroupList(client);

    std::cout << "请输入要取消管理员的群id: " << std::endl;

    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    tellServerWantToSetManagerGroupMemberList(client, std::stoi(groupid));

    sem_wait(&set_manager);

    sem_destroy(&set_manager);

    if(!if_master){
        return;
    }

    std::cout << "请输入要取消管理员的用户id: " << std::endl;

    std::string userid;
    std::cin >> userid;
    clearInputBuffer();

    json response;
    response["msgid"] = CANCEL_MANAGER;
    response["groupid"] = std::stoi(groupid);
    response["userid"] = std::stoi(userid);

    client.send(response.dump().append("\r\n"));
}

void kickSomeoneInGroup(Client &client){

    tellServerShowOwnGroupList(client);

    std::cout << "请输入要踢出的群id: " << std::endl;

    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    tellServerWantToLookGroupMemberList(client, std::stoi(groupid));

    std::cout << "请输入要踢出的用户id: " << std::endl;

    std::string userid;
    std::cin >> userid;
    clearInputBuffer();

    if(std::stoi(userid) == CurrentUser.getId()){
        std::cout << "不能踢出自己" << std::endl;
        return;
    }

    json response;
    response["msgid"] = KICK_SOMEONE;
    response["groupid"] = std::stoi(groupid);
    response["userid"] = std::stoi(userid);

    client.send(response.dump().append("\r\n"));
}

static void checkIfGroupMember(Client &client, int groupid){
    json response;
    response["msgid"] = CHECK_GROUP_MEMBER;
    response["groupid"] = groupid;
    client.send(response.dump().append("\r\n"));
}

void groupChat(Client& client){
    sem_init(&check_group_member, 0, 0);
    tellServerShowOwnGroupList(client);

    std::cout << "请输入群id: " << std::endl;
    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    checkIfGroupMember(client, std::stoi(groupid));

    sem_wait(&check_group_member);
    sem_destroy(&check_group_member);

    if(!if_group_member){
        std::cout << "你不是群成员" << std::endl;
        return;
    }

    std::cout << "进入群聊" << std::endl;

    std::string msg;
    json response;

    while(1){
        std::getline(std::cin, msg);

        if(msg == "exit"){
            break;
        }

        response["msgid"] = GROUP_CHAT;
        response["groupid"] = std::stoi(groupid);
        response["from_name"] = CurrentUser.getName();
        response["msg"] = msg;

        client.send(response.dump().append("\r\n"));
    }
}

void handleGroupChat(json &js){
    std::string from_name = js["from_name"];
    std::string msg = js["msg"];
    std::string time = js["time"];
    int groupid = js["groupid"];
    std::cout << time << " " << from_name << "在群" << groupid << "说: " << msg << std::endl;
}

void ExitChatRoom(){
    sem_destroy(&login_sem);
    sem_destroy(&add_someone_to_group);
    exit(EXIT_SUCCESS);
}
