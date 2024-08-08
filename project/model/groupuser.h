//Created by mars on 8/8/24

#ifndef GROUPUSER_H
#define GROUPUSER_H

#include "user.h"

class GroupUser : public User
{
public:
    void setRole(std::string role) { m_role = role; }
    std::string getRole() { return m_role; }

private:
    std::string m_role;
};

#endif
