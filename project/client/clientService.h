//Created by mars on 8/8/24

#ifndef SERVER_H
#define SERVER_H

#include "client.h"
#include "../m_netlib/Base/Timestamp.h"

void handleServerMessage(json &js, Timestamp time);

void UserInit(json &js);

void EnterChatRoom(Client &client);

void reg(Client &client);

bool login(Client &client);

void EnterCommandMenu(Client &client);

void addFriend(Client &client);

void updateFriendList(json &js);

void deleteFriend(Client &client);

void deleteOneFriendList(json &js);

void tellServerWantToLookRequestList(Client& client);

void displayRequestList(json &js);

void handleFriendOnline(json &js);

void privateChat(Client &client);

void ExitChatRoom();

#endif //SERVER_H
