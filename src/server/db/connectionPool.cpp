#include "connectionPool.hpp"
#include "muduo/base/Logging.h"
#include "db.hpp"
#include <fstream>
#include <cstdlib>


ConnectionPool* ConnectionPool::getConnectionPool()
{
	static ConnectionPool pool;
	return &pool;
}

std::shared_ptr<MySql> ConnectionPool::getConnection()
{
	std::unique_lock<std::mutex> lock(queueMutex);
	while (connectionPool.empty()) {
		if (std::cv_status::timeout ==
			conditionVariable.wait_for(lock, std::chrono::milliseconds(connectionTimeout))) {
			if (connectionPool.empty()) {
				LOG_INFO << "连接超时";
				return nullptr;
			}
		}
	}
	//自定义回收方式，将连接归还到池中
	std::shared_ptr<MySql> connection(connectionPool.front(), 
		[&](MySql* pconn) {
			std::unique_lock<std::mutex> lock(queueMutex);
			pconn->updataTime();
			connectionPool.push(pconn);
		});
	connectionPool.pop();
	conditionVariable.notify_all();
	return connection;
}

ConnectionPool::ConnectionPool()
{
	if (!loadConfigFile()) {
		return;
	}
	for (int i = 0; i < initSize; ++i)
	{
		MySql* conn = new MySql();
		conn->connect();
		conn->updataTime();
		connectionPool.push(conn);
		connectionCount++;
	}
	//生产连接线程
	std::thread produceConnectionThread(std::bind(&ConnectionPool::produceConnectionTask,this));
	produceConnectionThread.detach();
	//定时检查线程
	std::thread connectionTimmingThread(std::bind(&ConnectionPool::connectionTimmingTask, this));
	connectionTimmingThread.detach();
}
//生产
void ConnectionPool::produceConnectionTask()
{
	while(true) {

		std::unique_lock<std::mutex> lock(queueMutex);
		while (!connectionPool.empty()) {
			conditionVariable.wait(lock);
		}
		if (connectionCount < maxSize) {
			MySql* conn = new MySql();
			conn->connect();
			conn->updataTime();
			connectionPool.push(conn);
			connectionCount++;
		}
		conditionVariable.notify_all();
	}
}
//定时检查
void ConnectionPool::connectionTimmingTask()
{
	while (true) {
		//定时
		std::this_thread::sleep_for(std::chrono::seconds(maxIdleTime));
		std::unique_lock<std::mutex> lock(queueMutex);
		while (connectionCount > initSize) {
			MySql* pconn = connectionPool.front();
			if (pconn->getAliveTime() >= maxIdleTime * 1000) {
				connectionPool.pop();
				connectionCount--;
				delete pconn;
			}
			else {
				break;
			}
		}
	}
}

bool ConnectionPool::loadConfigFile()
{
	FILE* fp = fopen("../mysqlconfig.ini", "r");
	if (fp == nullptr) {
		LOG_INFO << "mysqlconfig.ini file open error";
		return false;
	}
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
		else if (key == "initSize") {
			initSize = atoi(value.c_str());
		}
		else if (key == "maxSize") {
			maxSize = atoi(value.c_str());
		}
		else if (key == "maxIdleTime") {
			maxIdleTime = atoi(value.c_str());
		}
		else if (key == "connectionTimeout") {
			connectionTimeout = atoi(value.c_str());
		}
	}
	fclose(fp);
	return true;
}