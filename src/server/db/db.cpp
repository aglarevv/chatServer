#include "db.hpp"
#include <mysql/mysql.h>
#include "muduo/base/Logging.h"

#include <fstream>

MySql::MySql(){
    conn_ = mysql_init(nullptr);
}

MySql::~MySql(){
    if(conn_ != nullptr){
        mysql_close(conn_);
    }
}
    
bool MySql::connect(){
    FILE* fp = fopen("../mysqlconfig.ini", "r");
	if (fp == nullptr) {
		LOG_INFO << "mysqlconfig.ini file open error" ;
		return false;
	}
    std::string ip;
    uint16_t port;
    std::string username;
    std::string password;
    std::string dbname;
	while (!feof(fp)) {
		char buf[1024] = { 0 };
		fgets(buf, 1024, fp);
		std::string str = buf;
		int index = str.find('=', 0);
		if (index == -1) {
			continue;
		}
		int endindex = str.find('\n', index);
		std::string key = str.substr(0, index);
		std::string value = str.substr(index + 1, endindex - index - 1);
		if (key == "ip") {
			ip = value;
		}
		else if (key == "port") {
			port = atoi(value.c_str());
		}
		else if (key == "username") {
			username = value;
		}
		else if (key == "password") {
			password = value;
		}
		else if (key == "dbname") {
			dbname = value;
		}
		
	}
	fclose(fp);
    MYSQL* mysql = MySql::connection(ip, username, password, dbname, port);
    if(mysql != nullptr){
		return true;
	}
	else{
		return false;
	}
}

MYSQL* MySql::connection(std::string& ip, std::string& username,
	std::string& password, std::string& dbname, unsigned short port)
{
	MYSQL* mysql = mysql_real_connect(conn_, ip.c_str(), username.c_str(), 
		password.c_str(), dbname.c_str(), port, nullptr, 0);
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

