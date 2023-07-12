#include "UserModel.hpp"
#include "User.hpp"
#include "connectionPool.hpp"
#include "db.hpp"

#include <iostream>


bool UserModel::insert(User& user){
    char sql[1024] = {0};
    sprintf(sql, "insert into User(name, password, state) values('%s', '%s', '%s')",
     user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
    ConnectionPool* pool_ = ConnectionPool::getConnectionPool();
    auto mysql = pool_->getConnection();
    if(mysql != nullptr){
        if(mysql->update(sql)){
            //设置真实userid
            user.setId(mysql_insert_id(mysql->getConnection()));
            return true;
        }
    }
     return false;
}

User UserModel::query(std::string& name){
    char sql[1024] = {0};
    sprintf(sql, "select * from User where name = '%s'",name.c_str());
    ConnectionPool* pool_ = ConnectionPool::getConnectionPool();
    auto mysql = pool_->getConnection();
    if(mysql != nullptr){
        MYSQL_RES* res = mysql->query(sql);
        if(res != nullptr){
            //获取用户信息
            MYSQL_ROW row = mysql_fetch_row(res);
            if(row != nullptr){
                //设置用户信息
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}

bool UserModel::updataState(User& user){
    char sql[1024] = {0};
    sprintf(sql, "update User set state = '%s' where name = '%s'",
        user.getState().c_str(), user.getName().c_str());
    ConnectionPool* pool_ = ConnectionPool::getConnectionPool();
    auto mysql = pool_->getConnection();
    if(mysql != nullptr){
        if(mysql->update(sql)){
            return true;
        }
    }
    return false;
}

void UserModel::resetState(){
    char sql[1024] = {"update User set state = 'offline' where state = 'online'"};
    ConnectionPool* pool_ = ConnectionPool::getConnectionPool();
    auto mysql = pool_->getConnection();
    if(mysql != nullptr){
        mysql->update(sql);
    }
}