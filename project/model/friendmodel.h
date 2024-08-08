//Created by mars on 7/8/24

#ifndef FRIEND_MODEL_H
#define FRIEND_MODEL_H

#include "user.h"

#include <vector>
#include <string>

class FriendModel{
public:
    // 添加好友关系
    void insert(int userid, int friendid, std::string state);

    void modify(int userid, int friendid, std::string state);

    void deleteEach(int userid, int friendid);

    // 返回用户好友列表
    std::vector<User> query(int userid);

    std::vector<User> requestQuery(int friendid);

    std::vector<User> blockQuery(int userid);

    std::string getState(int userid, int friendid);
};

#endif // FRIEND_MODEL_H
