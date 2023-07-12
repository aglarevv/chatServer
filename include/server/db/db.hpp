#pragma once 

#include "mysql/mysql.h"

#include <string>
#include <ctime>

class MySql
{
    public:
        MySql();
        ~MySql();
        bool connect();
        bool update(std::string sql);
        MYSQL_RES *query(std::string sql);
        MYSQL* getConnection();

        void updataTime() { aliveTime = clock(); }
        std::clock_t getAliveTime() const { return difftime(clock(), aliveTime); }
    private:
        MYSQL *conn_;
        std::clock_t aliveTime;

        MYSQL* connection(std::string& ip, std::string& username, std::string& password,
            std::string& dbname, unsigned short port);
};

