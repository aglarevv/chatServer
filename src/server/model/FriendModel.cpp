#include "FriendModel.hpp"
#include "db.hpp"

#include <iostream>

void FriendModel::insert(int userid, std::string username, int friendid){
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend values(%d, '%s', %d)",userid, username.c_str(), friendid);
    MySql mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}

std::vector<User> FriendModel::query(std::string username){
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name, a.state from User a inner \
     join Friend b on b.friendid = a.id where b.username = '%s'", username.c_str());
    std::vector<User> vec;
    MySql mysql;
    if(mysql.connect()){
        //获取sql语句执行结果
        MYSQL_RES* res = mysql.query(sql);
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