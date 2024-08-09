//Created by mars on 10/8/24

#ifndef PRIVATECHATHISTORY_H
#define PRIVATECHATHISTORY_H

#include <string>

class PrivateChatHistory {
public:
    PrivateChatHistory(int sender_id = -1, int receiver_id = -1, std::string message = "", std::string send_time = "")
            : m_sender_id(sender_id), m_receiver_id(receiver_id), m_message(message), m_send_time(send_time) {}

    int getSenderId() const {
        return m_sender_id;
    }

    void setSenderId(int sender_id) {
        m_sender_id = sender_id;
    }

    int getReceiverId() const {
        return m_receiver_id;
    }

    void setReceiverId(int receiver_id) {
        m_receiver_id = receiver_id;
    }

    const std::string &getMessage() const {
        return m_message;
    }

    void setMessage(const std::string &message) {
        m_message = message;
    }

    const std::string &getSendTime() const {
        return m_send_time;
    }

    void setSendTime(const std::string &send_time) {
        m_send_time = send_time;
    }

private:
    int m_sender_id;
    int m_receiver_id;
    std::string m_message;
    std::string m_send_time;
};

#endif //PRIVATECHATHISTORY_H
