#pragma once

#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"

using namespace muduo::net;
using namespace muduo;

class ConnectionPool;
class ChatServer {
    public:
        ChatServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg);
        void start();
    private:
        //注册回调函数
        void onConnection(const TcpConnectionPtr& conn);
        void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp timestamp);

        EventLoop* loop_;
        TcpServer server_;
        ConnectionPool* connectionPool_;
};