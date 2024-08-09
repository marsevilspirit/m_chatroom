#include "historymodel.h"
#include "../database/database.h"

std::vector<PrivateChatHistory> HistoryModel::queryPrivateHistory(int sender_id, int receiver_id, int count){
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "SELECT * FROM private_chat_history WHERE (sender_id=%d AND receiver_id=%d) OR (sender_id=%d AND receiver_id=%d) ORDER BY timestamp DESC LIMIT %d", 
        sender_id, receiver_id, receiver_id, sender_id, count);

    // 2. 执行sql语句
    std::vector<PrivateChatHistory> history_vec;
    MySQL mysql;
    if (mysql.connect()){
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr){
            // 把res中的数据遍历出来，放到history_vec中
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr){
                PrivateChatHistory history;
                history.setSenderId(atoi(row[1]));
                history.setReceiverId(atoi(row[2]));
                history.setMessage(row[3]);
                history.setSendTime(row[4]);
                history_vec.push_back(history);
            }
            mysql_free_result(res);
        }
    }
    return history_vec;
}

std::vector<GroupChatHistory> HistoryModel::queryGroupHistory(int group_id, int count){
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "SELECT * FROM group_chat_history WHERE group_id=%d ORDER BY timestamp DESC LIMIT %d", group_id, count);

    // 2. 执行sql语句
    std::vector<GroupChatHistory> history_vec;
    MySQL mysql;
    if (mysql.connect()){
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr){
            // 把res中的数据遍历出来，放到history_vec中
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr){
                GroupChatHistory history;
                history.setGroupId(atoi(row[1]));
                history.setSenderId(atoi(row[2]));
                history.setMessage(row[3]);
                history.setSendTime(row[4]);
                history_vec.push_back(history);
            }
            mysql_free_result(res);
        }
    }
    return history_vec;
}

