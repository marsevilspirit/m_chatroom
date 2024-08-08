//Created by mars on 8/8/24

#ifndef SERVER_H
#define SERVER_H

#include "client.h"
#include "../m_netlib/Base/Timestamp.h"

void handleServerMessage(Client* client, json &js, Timestamp time);

void UserInit(json &js);

void EnterChatRoom(Client &client);

void reg(Client &client);

bool login(Client &client);

void EnterCommandMenu(Client &client);

void addFriend(Client &client);

void deleteFriend(Client &client);

void blockFriend(Client &client);

void unblockFriend(Client &client);

void tellServerWantToLookRequestList(Client& client);

void tellServerWantToLookFriendList(Client& client);

void tellServerWantToLookBlockList(Client &client);

void displayFriendList(json &js);

void displayRequestList(json &js);

void displayBlockList(json &js);

void checkIfBlock(Client &client, int id_to_chat);

void checkIfBlock(Client* client, int id_to_chat);

void privateChat(Client &client);

void handlePrivateChat(json &js);

void ExitChatRoom();

#endif //SERVER_H
