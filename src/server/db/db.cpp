#include "db.hpp"
#include "muduo/base/Logging.h"

static std::string server = "127.0.0.1";
static std::string user = "root";
static std::string password = "aglare";
static std::string dbname = "chat";

MySql::MySql(){
    conn_ = mysql_init(nullptr);
}

MySql::~MySql(){
    if(conn_ != nullptr){
        mysql_close(conn_);
    }
}
    
bool MySql::connect(){
    MYSQL* mysql = mysql_real_connect(conn_, server.c_str(), user.c_str(), password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if(mysql != nullptr){
        mysql_query(conn_, "set names utf8");
        LOG_INFO << "连接成功";
        return mysql;
    }
    LOG_INFO << "连接失败";
    return mysql;
}

bool MySql::update(std::string sql){
    if(mysql_query(conn_, sql.c_str())){
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << " 更新失败";
        return false;
    }
    return true;
}

MYSQL_RES *MySql::query(std::string sql){
    if(mysql_query(conn_, sql.c_str())){
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << " 查询失败";
        return nullptr;
    }
    return mysql_use_result(conn_);
}

MYSQL* MySql::getConnection(){
    return conn_;
}

