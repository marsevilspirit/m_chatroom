//Created by mars on 7/8/24

#ifndef USR_H
#define USR_H

#include <string>

class User {
public:
    User(int id = -1, std::string name = "", std::string pwd = "", std::string state = "offline")
    {
        m_id = id;
        m_name = name;
        m_password = pwd;
        m_state = state;
    }

    void setId(int id) { m_id = id; }
    void setName(std::string name) { m_name = name; }
    void setPwd(std::string pwd) { m_password = pwd; }
    void setState(std::string state) { m_state = state; }

    int getId() const { return m_id; }
    std::string getName() const { return m_name; }
    std::string getPwd() { return m_password; }
    std::string getState() const { return m_state; }

protected:
    int m_id;
    std::string m_name;
    std::string m_password;
    std::string m_state;
};

#endif //USR_H
