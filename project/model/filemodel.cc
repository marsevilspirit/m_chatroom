#include "filemodel.h"
#include "../database/database.h"


void FileModel::insertFile(int sender_id, int receiver_id, std::string file_name){
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into file_transfers(`filename`, `senderid`, `receiverid`) values('%s', %d, %d)", file_name.c_str(), sender_id, receiver_id);

    // 2. 执行sql语句
    MySQL mysql;
    if (mysql.connect()){
        mysql.update(sql);
    }
}

std::vector<File> FileModel::query(int receiver_id){
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from file_transfers where receiverid=%d", receiver_id);

    // 2. 执行sql语句
    std::vector<File> file_vec;
    MySQL mysql;
    if (mysql.connect()){
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr){
            // 把res中的数据遍历出来，放到file_vec中
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr){
                File file;
                file.setFileName(row[1]);
                file.setSenderId(atoi(row[2]));
                file.setReceiverId(atoi(row[3]));
                file.setCreateTime(row[4]);
                file_vec.push_back(file);
            }
            mysql_free_result(res);
        }
    }
    return file_vec;
}

