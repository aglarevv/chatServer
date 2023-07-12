#include "FriendModel.hpp"
#include "connectionPool.hpp"
#include "db.hpp"

#include <iostream>


bool FriendModel::insert(int userid, std::string username, int friendid){
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend(userid, username, friendid) \
     select %d, '%s', %d from DUAL where exists(select id from User where id = %d and id != %d)",
     userid, username.c_str(), friendid, friendid, userid);
    ConnectionPool* pool_ = ConnectionPool::getConnectionPool();
    auto mysql = pool_->getConnection();    
    if(mysql != nullptr){
       //等于调用mysql.update(sql)
        MYSQL_RES* res = mysql->query(sql);

        if(mysql_affected_rows(mysql->getConnection()) > 0){
            mysql_free_result(res);
            return true;
        }
        //插入失败
        mysql_free_result(res);
        return false;
    }
}

std::vector<User> FriendModel::query(std::string username){
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name, a.state from User a inner \
     join Friend b on b.friendid = a.id where b.username = '%s'", username.c_str());
    std::vector<User> vec;
    ConnectionPool* pool_ = ConnectionPool::getConnectionPool();
    auto mysql = pool_->getConnection();   
    if(mysql != nullptr){
        //获取sql语句执行结果
        MYSQL_RES* res = mysql->query(sql);
        if(res != nullptr){
            MYSQL_ROW row;
            //获取一行数据
            while ((row = mysql_fetch_row(res)) != nullptr){
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }            
        }
        //释放资源
        mysql_free_result(res);
    }
    return vec;
}