#include "OfflineMessageModel.hpp"
#include "connectionPool.hpp"
#include "db.hpp"


// 存储离线消息
bool OfflineMessageModel::insert(std::string name, std::string message){
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage(username, message) values('%s', '%s')",
     name.c_str(), message.c_str());
    ConnectionPool* pool_ = ConnectionPool::getConnectionPool();
    auto mysql = pool_->getConnection(); 
    if (mysql != nullptr){
        mysql->update(sql);
        return true;
    }
    return false;
}
// 删除离线消息
bool OfflineMessageModel::remove(const std::string name){
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where username='%s'", name.c_str());
    ConnectionPool* pool_ = ConnectionPool::getConnectionPool();
    auto mysql = pool_->getConnection();
    if (mysql != nullptr){
        mysql->update(sql);
        return true;
    }
    return false;
}
// 查询离线消息
std::vector<std::string> OfflineMessageModel::query(std::string name){
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where username = '%s'",name.c_str());
    ConnectionPool* pool_ = ConnectionPool::getConnectionPool();
    auto mysql = pool_->getConnection();
    std::vector<std::string> vec;
    if(mysql != nullptr){
        MYSQL_RES* res = mysql->query(sql);
        if(res != nullptr){
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr){
                vec.push_back(row[0]);
            }
        }
        mysql_free_result(res);  
    }
    return vec;
}
