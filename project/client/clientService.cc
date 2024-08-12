#include "clientService.h"
#include "client.h"
#include "util.h"
#include "../public.h"
#include "../model/user.h"

#include <iostream>
#include <limits>
#include <semaphore.h> 
#include <atomic>
#include <unistd.h>
#include <termios.h>

sem_t reg_sem;
sem_t login_sem;
sem_t show_all_user_list;
sem_t show_friend_list;
sem_t show_block_list;
sem_t show_request_list;
sem_t show_own_group_list;
sem_t show_all_group_list;
sem_t show_file_list;
sem_t add_someone_to_group;
sem_t set_manager;
sem_t check_group_member;
sem_t receive_file;
sem_t check_if_block;

std::atomic<bool> login_flag(false);
std::atomic<bool> if_block(false);
std::atomic<bool> if_friend(false);
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
        case REG_SUCCESS:                       std::cout << "注册成功, 用户id:" << js["id"] << std::endl;                         
                                                sem_post(&reg_sem);
                                                break;
        case REG_FAIL:                          std::cout << "注册失败, 用户名已存在" << std::endl;                                
                                                sem_post(&reg_sem);
                                                break;
        case LOGIN_SUCCESS:                     std::cout << "登录成功" << std::endl; 
                                                login_flag = true;  
                                                UserInit(js); 
                                                sem_post(&login_sem);                                                              
                                                break;
        case LOGIN_REPEAT:                      std::cout << "重复登录" << std::endl; 
                                                login_flag = false; 
                                                sem_post(&login_sem);                                                              
                                                break;
        case LOGIN_FAIL:                        std::cout << "登录失败, 用户名或密码错误" << std::endl; 
                                                login_flag = false; 
                                                sem_post(&login_sem);                                                              
                                                break;
        case FRIEND_ONLINE:                     std::cout << "id:" << js["id"] << " name:" << js["name"] << " 上线" << std::endl; 
                                                break;
        case DISPLAY_ALLUSER_LIST:              displayAllUserList(js); 
                                                sem_post(&show_all_user_list);                                                   
                                                break;
        case ADD_FRIEND_SUCCESS:                std::cout << "添加成功" << std::endl;                                              
                                                break;
        case ADD_FRIEND_REQUEST:                std::cout << "你向对方发起好友申请" << std::endl;                                  
                                                break;
        case THE_OTHER_AGREE_FRIEND_REQUEST:    std::cout << js["friend_name"] << "同意了你的好友申请" << std::endl;                               
                                                break;
        case UPDATE_FRIEND_LIST:                std::cout << "更新好友列表" << std::endl;                                          
                                                break;
        case DELETE_FRIEND_SUCCESS:             std::cout << "删除好友成功" << std::endl;                                       
                                                break;
        case DELETED_FRIEND_SUCCESS:            std::cout << "你被一位好友删除" <<std::endl;                                   
                                                break;
        case DELETE_FRIEND_FAIL:                std::cout << "删除好友失败，你与对方不是好友" << std::endl;                       
                                                break;
        case BLOCK_FRIEND_FAIL:                 std::cout << "屏蔽好友失败，你与对方不是好友" << std::endl;                        
                                                break;
        case BLOCK_FRIEND_SUCCESS:              std::cout << "屏蔽好友成功" << std::endl; 
                                                checkIfBlock(client, private_chat_id);                                           
                                                break;
        case UNBLOCK_FRIEND_FAIL:               std::cout << "解除屏蔽好友失败，你并未屏蔽对方" << std::endl;                    
                                                break;
        case UNBLOCK_FRIEND_SUCCESS:            std::cout << "解除屏蔽好友成功" << std::endl; 
                                                checkIfBlock(client, private_chat_id);                                         
                                                break;
        case PRIVATE_CHAT:                      handlePrivateChat(js);                                                             
                                                break;
        case GROUP_CHAT:                        handleGroupChat(js);                                                               
                                                break;
        case FRIEND_REQUEST_LIST:               displayRequestList(js); 
                                                sem_post(&show_request_list);                                                     
                                                break;
        case DISPLAY_FRIEND_LIST:               displayFriendList(js); 
                                                sem_post(&show_friend_list);                                                      
                                                break;
        case BLOCK_FRIEND_LIST:                 displayBlockList(js); 
                                                sem_post(&show_block_list);                                                        
                                                break;
        case CHECK_BLOCK:                       if_block = (js["state1"] == "block") || (js["state2"] == "block"); 
                                                if_friend = (js["state1"] == "friend") || (js["state2"] == "friend") || if_block; 
                                                sem_post(&check_if_block);                                                         
                                                break;
        case CREATE_GROUP_SUCCESS:              std::cout << "创建群聊成功" << std::endl;                                        
                                                break;
        case CREATE_GROUP_FAIL:                 std::cout << "创建群聊失败, 已有同名群聊" << std::endl;                             
                                                break;
        case DISPLAY_ALLGROUP_LIST:             displayAllGroupList(js); 
                                                sem_post(&show_all_group_list);                                                 
                                                break;
        case REQUEST_GROUP_SUCCESS:             std::cout << "提交加入群聊申请成功" << std::endl;                               
                                                break;
        case REQUEST_GROUP_FAIL:                std::cout << "提交加入群聊申请失败" << std::endl;                                  
                                                break;
        case DISPLAY_GROUP_REQUEST:             if_master_ormanager = displayGroupRequestList(js); 
                                                sem_post(&add_someone_to_group);                                                
                                                break;
        case DISPLAY_GROUP_REQUEST_FAIL:        std::cout << "你不是群主或管理员" << std::endl;
                                                if_master_ormanager = false; 
                                                sem_post(&add_someone_to_group);                                           
                                                break;
        case ADD_GROUP_SUCCESS:                 std::cout << "添加群成员成功" << std::endl;                                         
                                                break;
        case ADD_GROUP_FAIL:                    std::cout << "添加群成员失败" << std::endl;                                            
                                                break;
        case QUIT_GROUP_FAIL:                   std::cout << "退出群聊失败" << std::endl;                                             
                                                break;
        case QUIT_GROUP_SUCCESS:                std::cout << "退出群聊成功" << std::endl;                                          
                                                break;
        case DISPLAY_OWN_GROUP_LIST:            displayOwnGroupList(js); 
                                                sem_post(&show_own_group_list);                                                
                                                break;
        case DISPLAY_GROUP_MEMBER_LIST:         displayGroupMemberList(js);                                                 
                                                break;
        case DISPLAY_GROUP_MEMBER_FAIL:         std::cout << "你不是群成员" << std::endl;                                   
                                                break;
        case DISPLAY_SET_MANAGER_MEMBER_FAIL:   std::cout << "你不是群主" << std::endl; 
                                                if_master = false; 
                                                sem_post(&set_manager);                                               
                                                break;
        case DISPLAY_SET_MANAGER_MEMBER_LIST:   displayGroupMemberList(js); 
                                                if_master = true; 
                                                sem_post(&set_manager);                                               
                                                break;
        case SET_MANAGER_SUCCESS:               std::cout << "设置管理员成功" << std::endl;                                       
                                                break;
        case SET_MANAGER_FAIL:                  std::cout << "设置管理员失败" << std::endl;                                          
                                                break;
        case CANCEL_MANAGER_FAIL:               std::cout << "对方不是管理员" << std::endl;                                       
                                                break;
        case CANCEL_MANAGER_SUCCESS:            std::cout << "取消管理员成功" << std::endl; 
                                                break;
        case KICK_SOMEONE_FAIL:                 std::cout << "踢人失败, 权限不够" << std::endl; 
                                                break;
        case KICK_SOMEONE_SUCCESS:              std::cout << "踢人成功" << std::endl; 
                                                break;
        case CHECK_IF_GROUP_MEMBER:             if_group_member = js["state"]; 
                                                sem_post(&check_group_member); 
                                                break;
        case MASTER_DELETE_GROUP_FAIL:          std::cout << "你没有权限解散群聊" << std::endl; 
                                                break;
        case MASTER_DELETE_GROUP_SUCCESS:       std::cout << "解散群聊成功" << std::endl; 
                                                break;
        case SEND_FILE_DATABASE_SUCCESS:        std::cout << "文件发送成功" << std::endl; 
                                                break;
        case VIEW_FILE:                         displayFileList(js); 
                                                sem_post(&show_file_list); 
                                                break;
        case RECEIVE_FILE_FINISH:               std::cout << "文件接收完成" << std::endl; 
                                                sem_post(&receive_file); 
                                                break;
        case DISPLAY_PRIVATE_HISTORY:           displayPrivateChatHistory(js); 
                                                break;
        case DISPLAY_GROUP_HISTORY:             displayGroupChatHistory(js); 
                                                break;
        case CLIENT_LONGTIME_EXIT:              std::cout << "你太长时间未操作，服务器断开连接" << std::endl; 
                                                client->disconnect(); 
                                                ExitChatRoom();                                                      
                                                break;
    }
}

void UserInit(json &js){
    CurrentUser.setId(js["id"]);
    CurrentUser.setName(js["name"]);

    std::cout << "当前用户 name: " << CurrentUser.getName() << " id: " << CurrentUser.getId() << std::endl;
}

static void semInit(){
    sem_init(&reg_sem, 0, 0);
    sem_init(&login_sem, 0, 0);
    sem_init(&add_someone_to_group, 0, 0);
    sem_init(&set_manager, 0, 0);
    sem_init(&check_group_member, 0, 0);
    sem_init(&receive_file, 0, 0);
    sem_init(&show_all_user_list, 0, 0);
    sem_init(&show_friend_list, 0, 0);
    sem_init(&show_request_list, 0, 0);
    sem_init(&check_if_block, 0, 0);
    sem_init(&show_block_list, 0, 0);
    sem_init(&show_own_group_list, 0, 0);
    sem_init(&show_all_group_list, 0, 0);
    sem_init(&show_file_list, 0, 0);
}


void EnterChatRoom(Client &client){
    semInit();

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

std::string getPassword() {
    std::string password;
    char ch;
    struct termios oldt, newt;

    // 获取终端的当前属性
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // 关闭回显功能
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // 输入密码
    std::getline(std::cin, password);

    // 恢复终端的原始属性
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;

    return password;
}

bool isNumber(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

void reg(Client &client){
    std::string name, password, password2;

    std::cout << "用户名:";
    std::cin >> name;
    clearInputBuffer();
    std::cout << "密码:";
    password = getPassword();
    std::cout << "确定密码:";
    password2 = getPassword();

    if(password != password2){
        std::cout << "两次密码不一致" << std::endl;
        return;
    }

    password = passwordToSha256(password);

    json response;
    response["msgid"] = REG_MSG;
    response["name"]  = name;
    response["password"] = password;
    client.send(response.dump().append("\r\n"));

    sem_wait(&reg_sem);
}

bool login(Client &client){
    std::string name, password;

    std::cout << "用户名:";
    std::cin >> name;
    clearInputBuffer();
    std::cout << "密码:";
    password = getPassword();

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
            case 1:  privateChat(client);                           break;
            case 2:  groupChat(client);                             break;
            case 3:  addFriend(client);                             break;
            case 4:  deleteFriend(client);                          break;
            case 5:  blockFriend(client);                           break;
            case 6:  unblockFriend(client);                         break;
            case 7:  tellServerWantToLookFriendList(client);   
                     sem_wait(&show_friend_list);                   break;
            case 8:  addFriendRequest(client);                      break;
            case 9:  createGroup(client);                           break;
            case 10: requestEnterGroup(client);                     break;
            case 11: quitGroup(client);                             break;
            case 12: masterDeleteGroup(client);                     break;
            case 13: tellServerWantToLookAllGroupList(client); 
                     sem_wait(&show_all_group_list);                break;
            case 14: setGroupManager(client);                       break;
            case 15: cancelGroupManager(client);                    break;
            case 16: kickSomeoneInGroup(client);                    break;
            case 17: GroupMemberList(client);                       break;
            case 18: addSomeoneToGroup(client);                     break;
            case 19: sendfile(client);                              break;
            case 20: viewfile(client); 
                     sem_post(&show_file_list);                     break;
            case 21: receivefile(client);                           break;
            case 22: ExitChatRoom();                                break;
            default:
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

    sem_wait(&show_all_user_list);

    std::string name;
    std::cout << "请输入要添加的好友名(exit退出):";
    std::cin >> name;
    clearInputBuffer();

    if(name == "exit"){
        return;
    }

    if(name == CurrentUser.getName()){
        std::cout << "不能添加自己为好友" << std::endl;
        return;
    }

    json response;
    response["msgid"] = ADD_FRIEND;
    response["add_name"] = name;

    client.send(response.dump().append("\r\n"));
}


void addFriendRequest(Client &client){
    tellServerWantToLookRequestList(client);
    sem_wait(&show_request_list);

    std::string name;
    std::cout << "请输入要添加的好友名(exit退出):";
    std::cin >> name;
    clearInputBuffer();

    if(name == "exit"){
        return;
    }

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
    sem_wait(&show_friend_list);

    int id;
    std::cout << "请输入要删除的好友id(-1退出):";
    std::cin >> id;
    clearInputBuffer();

    if(id == -1){
        return;
    }

    json response;
    response["msgid"] = DELETE_FRIEND;
    response["del_id"] = id;

    client.send(response.dump().append("\r\n"));
}

void blockFriend(Client& client){
    tellServerWantToLookFriendList(client);
    sem_wait(&show_friend_list);

    int id;
    std::cout << "请输入要屏蔽的好友id(-1退出):";
    std::cin >> id;
    clearInputBuffer();

    if(id == -1){
        return;
    }

    json response;
    response["msgid"] = BLOCK_FRIEND;
    response["block_id"] = id;

    client.send(response.dump().append("\r\n"));
}

void unblockFriend(Client& client){
    tellServerWantToLookBlockList(client);
    sem_wait(&show_block_list);

    int id;
    std::cout << "请输入要解除屏蔽的好友id(-1退出):";
    std::cin >> id;
    clearInputBuffer();

    if(id == -1){
        return;
    }

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

    if(vec.size() == 0){
        std::cout << "无人申请" << std::endl;
        return;
    }

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

void tellServerWantToLookPrivateChatHistory(Client &client, int id_to_chat){
    json response;
    response["msgid"] = PRIVATE_CHAT_HISTORY;
    response["id"] = id_to_chat;

    client.send(response.dump().append("\r\n"));
}

void privateChat(Client &client) {
    tellServerWantToLookFriendList(client);
    sem_wait(&show_friend_list);

    std::cout << "请输入好友id(-1退出):";
    int id_to_chat;
    std::cin >> id_to_chat;
    clearInputBuffer();

    if(id_to_chat == -1){
        return;
    }

    std::string msg;
    json response;

    if_friend = false;
    checkIfBlock(client, id_to_chat);
    sem_wait(&check_if_block);

    if(!if_friend) {
        std::cout << "你与对方不是好友" << std::endl;
        return;
    }

    bool if_display_private_history = true;

    while (!if_block) {
        private_chat_id = id_to_chat;

        if(if_display_private_history){
            std::cout << "历史记录:" << std::endl;
            tellServerWantToLookPrivateChatHistory(client, id_to_chat);
            usleep(1000);
            if_display_private_history = false;
        }

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
    std::cout << "请输入群名(exit退出): " << std::endl;
    std::string groupname;
    std::cin >> groupname;
    clearInputBuffer();

    if(groupname == "exit") {
        return;
    }

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
    sem_wait(&show_all_group_list);

    std::cout << "请输入要加入的群id(-1退出): " << std::endl;

    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    if(groupid == "-1"){
        return;
    }

    if(!isNumber(groupid)){
        std::cout << "请输入数字" << std::endl;
        return;
    }

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
    tellServerWantToLookAllGroupList(client);
    sem_wait(&show_all_group_list);
    std::cout << "请输入要查看的群id(-1退出): " << std::endl;

    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    if(groupid == "-1"){
        return;
    }

    if(!isNumber(groupid)){
        std::cout << "请输入数字" << std::endl;
        return;
    }

    tellServerWantToLookGroupRequestList(client, std::stoi(groupid));

    sem_wait(&add_someone_to_group);

    if(!if_master_ormanager){
        return;
    }

    std::cout << "请输入要加入的用户id(-1退出): " << std::endl;

    std::string userid;
    std::cin >> userid;
    clearInputBuffer();

    if(userid == "-1"){
        return;
    }

    if(!isNumber(userid)) {
        std::cout << "请输入数字" << std::endl;
        return;
    }

    json response;
    response["msgid"] = ADD_GROUP;
    response["groupid"] = std::stoi(groupid);
    response["userid"] = std::stoi(userid);

    client.send(response.dump().append("\r\n"));
}

void quitGroup(Client &client){
    tellServerShowOwnGroupList(client);
    sem_wait(&show_own_group_list);

    std::cout << "请输入要退出的群id(-1退出): " << std::endl;

    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    if(groupid == "-1"){
        return;
    }

    if(!isNumber(groupid)) {
        std::cout << "请输入数字" << std::endl;
        return;
    }

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
    sem_wait(&show_own_group_list);

    std::cout << "请输入要查看的群id(-1退出): " << std::endl;

    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    if(groupid == "-1"){
        return;
    }

    if(!isNumber(groupid)){
        std::cout << "请输入数字" << std::endl;
        return;
    }

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
    tellServerShowOwnGroupList(client);
    sem_wait(&show_own_group_list);

    std::cout << "请输入要设置管理员的群id(-1退出): " << std::endl;

    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    if(groupid == "-1"){
        return;
    }

    if(!isNumber(groupid)){
        std::cout << "请输入数字" << std::endl;
        return;
    }

    tellServerWantToSetManagerGroupMemberList(client, std::stoi(groupid));

    sem_wait(&set_manager);


    if(!if_master){
        return;
    }

    std::cout << "请输入要设置为管理员的用户id(-1退出): " << std::endl;

    std::string userid;
    std::cin >> userid;
    clearInputBuffer();

    if(userid == "-1"){
        return;
    }

    if(!isNumber(userid)){
        std::cout << "请输入数字" << std::endl;
        return;
    }

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
    tellServerShowOwnGroupList(client);
    sem_wait(&show_own_group_list);

    std::cout << "请输入要取消管理员的群id(-1): " << std::endl;

    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    if(groupid == "-1"){
        return;
    }

    if(!isNumber(groupid)){
        std::cout << "请输入数字" << std::endl;
        return;
    }

    tellServerWantToSetManagerGroupMemberList(client, std::stoi(groupid));

    sem_wait(&set_manager);

    if(!if_master){
        return;
    }

    std::cout << "请输入要取消管理员的用户id(-1退出): " << std::endl;

    std::string userid;
    std::cin >> userid;
    clearInputBuffer();

    if(userid == "-1"){
        return;
    }

    if(!isNumber(userid)){
        std::cout << "请输入数字" << std::endl;
        return;
    }

    json response;
    response["msgid"] = CANCEL_MANAGER;
    response["groupid"] = std::stoi(groupid);
    response["userid"] = std::stoi(userid);

    client.send(response.dump().append("\r\n"));
}

void kickSomeoneInGroup(Client &client){
    tellServerShowOwnGroupList(client);
    sem_wait(&show_own_group_list);

    std::cout << "请输入要踢出的群id(-1退出): " << std::endl;

    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    if(groupid == "-1"){
        return;
    }

    if(!isNumber(groupid)){
        std::cout << "请输入数字" << std::endl;
        return;
    }

    tellServerWantToLookGroupMemberList(client, std::stoi(groupid));

    std::cout << "请输入要踢出的用户id(-1退出): " << std::endl;

    std::string userid;
    std::cin >> userid;
    clearInputBuffer();

    if(userid == "-1"){
        return;
    }

    if(!isNumber(userid)){
        std::cout << "请输入数字" << std::endl;
        return;
    }

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
    tellServerShowOwnGroupList(client);
    sem_wait(&show_own_group_list);

    std::cout << "请输入群id(-1退出): " << std::endl;
    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    if(groupid == "-1"){
        return;
    }

    if(!isNumber(groupid)){
        std::cout << "请输入数字" << std::endl;
        return;
    }

    checkIfGroupMember(client, std::stoi(groupid));

    sem_wait(&check_group_member);

    if(!if_group_member){
        std::cout << "你不是群成员" << std::endl;
        return;
    }

    std::cout << "进入群聊" << std::endl;

    std::string msg;
    json response;

    bool if_display_group_history = true;

    while(1){

        if(if_display_group_history){
            std::cout << "历史记录:" << std::endl;
            tellServerWantToLookGroupChatHistory(client, std::stoi(groupid));
            usleep(1000);
            if_display_group_history = false;
        }

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

void masterDeleteGroup(Client &client){
    tellServerShowOwnGroupList(client);
    sem_wait(&show_own_group_list);

    std::cout << "请输入要解散的群id(-1退出): " << std::endl;

    std::string groupid;
    std::cin >> groupid;
    clearInputBuffer();

    if(groupid == "-1"){
        return;
    }

    if(!isNumber(groupid)){
        std::cout << "请输入数字" << std::endl;
        return;
    }

    json response;
    response["msgid"] = MASTER_DELETE_GROUP;
    response["groupid"] = std::stoi(groupid);

    client.send(response.dump().append("\r\n"));
}

void sendfile(Client &client) {
    tellServerWantToLookFriendList(client);
    sem_wait(&show_friend_list);

    std::cout << "想发送给(id: -1 退出):" << std::endl;
    std::string receiver_id;
    std::cin >> receiver_id;

    if (receiver_id == "-1") {
        return;
    }

    if (!isNumber(receiver_id)) {
        std::cout << "请输入数字" << std::endl;
        return;
    }

    std::cout << "请输入文件路径: " << std::endl;
    std::string filePath;
    std::cin >> filePath;
    clearInputBuffer();

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // 发送文件元数据
    json metadata;
    metadata["msgid"] = SEND_FILE; // 标识这是一个文件传输
    metadata["filename"] = filePath.substr(filePath.find_last_of("/\\") + 1);
    metadata["filesize"] = fileSize;
    client.send(metadata.dump().append("\r\n"));

    // 逐块发送文件数据
    char buffer[40960];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        client.send(std::string(buffer, file.gcount()));
        usleep(10);
    }

    file.close();
    std::cout << "File sent successfully." << std::endl;

    // 发送传输结束标识
    json endMessage;
    endMessage["msgid"] = SEND_FILE_END;
    client.send(endMessage.dump().append("\r\n"));

    json response;
    response["msgid"] = SEND_FILE_DATABASE;
    response["filename"] = metadata["filename"];
    response["sender"] = CurrentUser.getId();
    response["receiver"] = std::stoi(receiver_id);

    client.send(response.dump().append("\r\n"));
}

void viewfile(Client &client) {
    json response;
    response["msgid"] = VIEW_FILE;
    client.send(response.dump().append("\r\n"));
}

void displayFileList(json &js) {
    std::cout << "文件列表:" << std::endl;
    std::vector<std::string> vec = js["files"];
    for (std::string &str : vec) {
        json js = json::parse(str);
        std::cout << "filename:" << js["filename"] << " sender:" << js["sender"] << " receiver:" << js["receiver"] << " time:" << js["time"] << std::endl;
    }
}

void receivefile(Client &client){
    viewfile(client);
    sem_wait(&show_file_list);

    std::cout << "接收文件(name exit退出):" << std::endl;
    std::string filename;
    std::cin >> filename;

    if(filename == "exit"){
        return;
    }

    json response;
    response["msgid"] = RECEIVE_FILE;
    response["filename"] = filename;
    client.send(response.dump().append("\r\n"));

    sem_wait(&receive_file);
}

void displayPrivateChatHistory(json &js){
    std::vector<std::string> vec = js["history"];
    for(std::string &str : vec){
        json js = json::parse(str);
        std::cout << js["time"] << " " << js["from"] <<" 对 " << js["to"] << "说: \n" << js["msg"] << std::endl;
    }
}

void tellServerWantToLookGroupChatHistory(Client &client, int groupid){
    json response;
    response["msgid"] = GROUP_CHAT_HISTORY;
    response["groupid"] = groupid;
    client.send(response.dump().append("\r\n"));
}

void displayGroupChatHistory(json &js){
    std::vector<std::string> vec = js["history"];
    for(std::string &str : vec){
        json js = json::parse(str);
        std::cout << js["time"] << " " << js["from"] << "在群" << js["groupid"] << "说: \n" << js["msg"] << std::endl;
    }
}

void ExitChatRoom(){
    sem_destroy(&reg_sem);
    sem_destroy(&login_sem);
    sem_destroy(&add_someone_to_group);
    sem_destroy(&set_manager);
    sem_destroy(&check_group_member);
    sem_destroy(&receive_file);
    sem_destroy(&show_all_user_list);
    sem_destroy(&show_friend_list);
    sem_destroy(&check_if_block);
    sem_destroy(&show_block_list);
    sem_destroy(&show_request_list);
    sem_destroy(&show_own_group_list);
    sem_destroy(&show_all_group_list);
    sem_destroy(&show_file_list);
    exit(EXIT_SUCCESS);
}
