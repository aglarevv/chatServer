#include "ChatServer.hpp"
#include "json.hpp"
#include "ChatService.hpp"

#include <functional>
#include <string>

using namespace nlohmann;

ChatServer::ChatServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg)
: loop_(loop)
, server_(loop, listenAddr, nameArg){
    server_.setConnectionCallback(std::bind(&ChatServer::onConnection,
     this, std::placeholders::_1));

    server_.setMessageCallback(std::bind(&ChatServer::onMessage,
     this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    //使用4个线程
    server_.setThreadNum(4);
}
//开启
void ChatServer::start(){
    server_.start();
}
//新连接回调
void ChatServer::onConnection(const TcpConnectionPtr& conn){
    if(!conn->connected()){
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}
//业务分发回调
void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time){
    std::string buffer = buf->retrieveAllAsString();
    json js = json::parse(buffer);
    auto msgHandle = ChatService::instance()->getHandle(js["msgtype"].get<int>());
    msgHandle(conn, js, time);
}   