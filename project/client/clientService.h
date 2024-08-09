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

void tellServerWantToLookAllUserList(Client &client);

void displayAllUserList(json &js);

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

void createGroup(Client &client);

void tellServerWantToLookAllGroupList(Client &client);

void displayAllGroupList(json &js);

void requestEnterGroup(Client &client);

void tellServerWantToLookGroupRequestList(Client &client, int groupid);

int displayGroupRequestList(json &js);

void addSomeoneToGroup(Client &client);

void quitGroup(Client &client);

void tellServerShowOwnGroupList(Client &client);

void displayOwnGroupList(json &js);

void tellServerWantToLookGroupMemberList(Client &client, int groupid);

void GroupMemberList(Client &client);

void displayGroupMemberList(json &js);

void setGroupManager(Client &client);

void tellServerWantToSetManagerGroupMemberList(Client &client, int groupid);

void cancelGroupManager(Client &client);

void kickSomeoneInGroup(Client &client);

void groupChat(Client &client);

void handleGroupChat(json &js);

void masterDeleteGroup(Client &client);

void sendfile(Client &client);

void viewfile(Client &client);

void receivefile(Client &client);

void displayFileList(json &js);

void ExitChatRoom();

#endif //SERVER_H
