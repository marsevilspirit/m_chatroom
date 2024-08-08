#include "clientService.h"
#include "util.h"
#include "../public.h"
#include "../model/user.h"

#include <iostream>
#include <limits>
#include <semaphore.h>
#include <atomic>

sem_t login_sem;
std::atomic<bool> login_flag(false);

User CurrentUser;
std::vector<User> currentUserFriendList;

static int displayFriendList() {
    if (currentUserFriendList.empty()) {
        std::cout << "Friend list is empty." << std::endl;
        return 0;
    }

    std::cout << "Friend List:" << std::endl;
    for (const auto& user : currentUserFriendList) {
        std::cout << "id: " << user.getId() << ", Name: " << user.getName() << ", state: " << user.getState() << std::endl;
    }
    return 1;
}

static void clearInputBuffer() 
{
    std::cin.clear(); // 清除输入状态标志
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 忽略剩余输入
}

void handleServerMessage(json &js, Timestamp time){
    int msgid = js["msgid"].get<int>();
    switch (msgid){
        case REG_SUCCESS:        std::cout << "注册成功, 用户id:" << js["id"].get<int>() << std::endl;                                  break;
        case REG_FAIL:           std::cout << "注册失败, 用户名已存在" << std::endl;                                                    break;
        case LOGIN_SUCCESS:      std::cout << "登录成功" << std::endl; login_flag = true;  UserInit(js); sem_post(&login_sem);          break;
        case LOGIN_REPEAT:       std::cout << "重复登录" << std::endl; login_flag = false; sem_post(&login_sem);                        break;
        case LOGIN_FAIL:         std::cout << "登录失败, 用户名或密码错误" << std::endl; login_flag = false; sem_post(&login_sem);      break;
        case ADD_FRIEND_SUCCESS: std::cout << "添加成功" << std::endl; updateFriendList(js); displayFriendList();                       break;
        case ADD_FRIEND_REQUEST: std::cout << "你向对方发起好友申请" << std::endl;                                                      break;
        case UPDATE_FRIEND_LIST: std::cout << "更新好友列表" << std::endl;  updateFriendList(js);                                       break;
        case DELETE_FRIEND_SUCCESS: std::cout << "删除好友成功" << std::endl;  deleteOneFriendList(js);                                 break;
        case DELETED_FRIEND_SUCCESS: std::cout << "你被一位好友删除" <<std::endl; deleteOneFriendList(js); break;
        case DELETE_FRIEND_FAIL:  std::cout << "删除好友失败，你与对方不是好友" << std::endl; break;
        case FRIEND_ONLINE:      handleFriendOnline(js);  break;
        case PRIVATE_CHAT:        std::cout << "msg: " << js["msg"] << std::endl; break;
        case FRIEND_REQUEST_LIST: displayRequestList(js); break;
        case TEST: std::cout << "收到test json send" <<std::endl; break;
    }
}

void UserInit(json &js){
    CurrentUser.setId(js["id"]);
    CurrentUser.setName(js["name"]);

    std::cout << "当前用户 name:" << CurrentUser.getName() << " id:" << CurrentUser.getId() << std::endl;

    std::cout << "好友列表:" << std::endl;
    if(js.contains("friends")){
        currentUserFriendList.clear();
        std::vector<std::string> vec = js["friends"];
        for(std::string &str : vec){
            json js = json::parse(str);
            User user;
            user.setId(js["friend_id"]);
            user.setName(js["friend_name"]);
            user.setState(js["friend_state"]);
            currentUserFriendList.push_back(user);
            std::cout << "id: " << user.getId() << " name: " << user.getName() << " state: " << user.getState() << std::endl;
        }
    }
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
        std::cout << "8.创建群聊    10.申请加入群聊\n";
        std::cout << "11.退出群聊   12.解散群聊\n";
        std::cout << "13.群聊列表   14.设置管理员\n";
        std::cout << "15.取消管理员 16.踢人\n";
        std::cout << "17.群聊成员   18.群成员申请列表\n";
        std::cout << "19.发送文件   20.查看文件\n";
        std::cout << "21.接收文件   22.退出\n";
        std::cout << "\n";

        int choice;
        std::cin >> choice;
        clearInputBuffer();

        switch (choice){
            case 1:  privateChat(client);              break;
            case 2:  std::cout << "群聊" << std::endl; break;
            case 3:  addFriend(client);    break;
            case 4:  deleteFriend(client); break;
            case 5:  std::cout << "屏蔽好友" << std::endl; break;
            case 6:  std::cout << "解除屏蔽" << std::endl; break;
            case 7:  displayFriendList(); break;
            case 8:  tellServerWantToLookRequestList(client); break;
            case 9:  std::cout << "创建群聊" << std::endl;     break;
            case 10: std::cout << "申请加入群聊" << std::endl; break;
            case 11: std::cout << "退出群聊" << std::endl; break;
            case 12: std::cout << "解散群聊" << std::endl; break;
            case 13: std::cout << "群聊列表" << std::endl; break;
            case 14: std::cout << "设置管理员" << std::endl; break;
            case 15: std::cout << "取消管理员" << std::endl; break;
            case 16: std::cout << "踢人" << std::endl; break;
            case 17: std::cout << "群聊成员" << std::endl; break;
            case 18: std::cout << "群成员申请列表" << std::endl; break;
            case 19: std::cout << "发送文件" << std::endl; break;
            case 20: std::cout << "查看文件" << std::endl; break;
            case 21: std::cout << "接收文件" << std::endl; break;
            case 22: ExitChatRoom();                  break;
        }
    }
}

void addFriend(Client &client){
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


void updateFriendList(json &js){
    User user;
    user.setId(js["friend_id"]);
    user.setName(js["friend_name"]);
    user.setState(js["friend_state"]);

    std::cout << "id:" << user.getId() << "name:" << user.getName() << " " << user.getState() << std::endl;

    currentUserFriendList.push_back(user);
}

void deleteFriend(Client &client){
    if(displayFriendList() == 0){
        std::cout << "没有好友可以删除\n";
        return;
    }

    int id;
    std::cout << "请输入要删除的好友id:";
    std::cin >> id;
    clearInputBuffer();

    json response;
    response["msgid"] = DELETE_FRIEND;
    response["del_id"] = id;

    client.send(response.dump().append("\r\n"));
}

void deleteOneFriendList(json &js) {
    int idToDelete = js["id"].get<int>();
    auto it = std::remove_if(currentUserFriendList.begin(), currentUserFriendList.end(),
                             [idToDelete](const User &user) {
                                 return user.getId() == idToDelete;
                             });

        currentUserFriendList.erase(it, currentUserFriendList.end());
}

void tellServerWantToLookRequestList(Client &client){
    json response;
    response["msgid"] = FRIEND_REQUEST_LIST;
    client.send(response.dump().append("\r\n"));
}

void displayRequestList(json &js){
    std::vector<std::string> vec = js["requests"];
    for(std::string &str : vec){
        json js = json::parse(str);
        std::cout << "id:" << js["id"] << " name:" << js["name"] << std::endl;
    }
}

void handleFriendOnline(json &js){
    for(auto& user : currentUserFriendList){
        if (user.getId() == js["id"]){
            user.setState("online");
            std::cout << "你的朋友" << user.getName() << "上线" << std::endl;
        }
    }
}

void privateChat(Client &client){
    displayFriendList();
    std::cout << "请输入好友id:";
    int id_to_chat;
    std::cin >> id_to_chat;
    std::string msg;
    json response;

    while(1){
    
        std::cin >> msg;

        if(msg == "quit"){
            break;
        }

        response["msgid"] = PRIVATE_CHAT;
        response["id"] = id_to_chat;
        response["msg"] = msg;

        client.send(response.dump().append("\r\n"));
    }
}

void ExitChatRoom(){
    sem_destroy(&login_sem);
    exit(EXIT_SUCCESS);
}
