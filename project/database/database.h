//Created by mars on 7/8/24

#ifndef DATABASE_H
#define DATABASE_H

#include <mysql/mysql.h>
#include <string>

class MySQL {
public:
    MySQL();
    ~MySQL();


    bool update(const std::string &sql);

    MYSQL_RES *query(const std::string &sql);

    MYSQL* getConnection();

    bool connect();
private:

    MYSQL *m_conn;

};

#endif //DATABASE_H
