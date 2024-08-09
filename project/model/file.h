//Created by mars on 9/8/24

#ifndef FILE_H
#define FILE_H

#include <string>

class File {
public:
    

    File(int sender_id = -1, int receiver_id = -1, std::string file_name = "", std::string create_time = "")
    {
        m_senderid = sender_id;
        m_receiverid = receiver_id;
        m_filename = file_name;
        m_createtime = create_time;
    }

    int getSenderId() const { return m_senderid; }
    int getReceiverId() const { return m_receiverid; }
    std::string getFileName() const { return m_filename; }
    std::string getCreateTime() const { return m_createtime; }

    void setSenderId(int sender_id) { m_senderid = sender_id; }
    void setReceiverId(int receiver_id) { m_receiverid = receiver_id; }
    void setFileName(std::string file_name) { m_filename = file_name; }
    void setCreateTime(std::string create_time) { m_createtime = create_time; }

private:
    int m_senderid;
    int m_receiverid;
    std::string m_filename;
    std::string m_createtime;
};

#endif
