#include "GroupModel.hpp"
#include "ALLGroup.hpp"
#include "connectionPool.hpp"
#include "db.hpp"

#include <vector>
#include <iostream> 
#include <cstring>


//创建群组 
bool GroupModel::create(AllGroup& allGroup){
    char sql[1024] = {0};
    sprintf(sql, "insert into ALLGroup(groupname, groupdesc) values('%s','%s')",
     allGroup.getGroupName().c_str(), allGroup.getGroupDesc().c_str());
    ConnectionPool* pool_ = ConnectionPool::getConnectionPool();
    auto mysql = pool_->getConnection(); 
    if(mysql != nullptr){
        if(mysql->update(sql)){
            //设置真实id
            allGroup.setGroupId(mysql_insert_id(mysql->getConnection()));
            return true;
        }
    }
    return false;
}
//添加群员
bool GroupModel::add(int groupid, int userid, std::string username, std::string role){
    char sql[1024] = {0};
    //条件插入，群组存在才能插入成功
    sprintf(sql, "insert into GroupUser(groupid, userid, username, grouprole) \
        select %d, %d, '%s', '%s' from DUAL where exists (select id from ALLGroup where id = %d)", 
        groupid, userid, username.c_str(), role.c_str(), groupid);
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
//查询用户所在群组信息
std::vector<AllGroup> GroupModel::queryGroups(std::string username){
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.groupname, a.groupdesc from ALLGroup a inner \
     join GroupUser b on a.id = b.groupid where b.username = '%s'", username.c_str());
    std::vector<AllGroup> vec;
    ConnectionPool* pool_ = ConnectionPool::getConnectionPool();
    auto mysql = pool_->getConnection(); 
    //获取群信息
    if(mysql != nullptr){
        MYSQL_RES* res = mysql->query(sql);
        if(res != nullptr){
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr){
                AllGroup group;
                group.setGroupId(atoi(row[0]));
                group.setGroupName(row[1]);
                group.setGroupDesc(row[2]);
                vec.push_back(group);
            }
            mysql_free_result(res);
        }
            
    }
    bzero(sql, sizeof(sql));
    //获取群员信息
    for(auto& group : vec){
        sprintf(sql, "select a.id, a.name, a.state, b.grouprole from User a \
         inner join GroupUser b on b.userid = a.id where b.groupid = %d", group.getGroupId());
        if(mysql != nullptr){
            MYSQL_RES* res = mysql->query(sql);
            if(res != nullptr){
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)) != nullptr){
                    GroupUser user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    user.setRole(row[3]);
                    group.getUsers().push_back(user);
                }
                mysql_free_result(res);
            }               
        }
    }
    return vec;
}
//查询除userid外群员列表
std::vector<std::string> GroupModel::queryUsers(int userid, int groupid){
    char sql[1024] = {0};
    sprintf(sql, "select username from GroupUser where groupid = %d and userid != %d", groupid, userid);
    ConnectionPool* pool_ = ConnectionPool::getConnectionPool();
    auto mysql = pool_->getConnection(); 
    std::vector<std::string> vec;
    if(mysql != nullptr){
        MYSQL_RES* res = mysql->query(sql);
        if(res != nullptr){
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr ){
                vec.push_back(row[0]);
            }
        }
        mysql_free_result(res); 
    }
    return vec;
}