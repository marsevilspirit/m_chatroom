//Created by mars on 10/8/24

#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include "privatechathistory.h"
#include "groupchathistory.h"

#include <vector>


class HistoryModel {
public:
    std::vector<PrivateChatHistory> queryPrivateHistory(int sender_id, int receiver_id, int count);

    std::vector<GroupChatHistory> queryGroupHistory(int group_id, int count);
};

#endif //HISTORYMODEL_H
