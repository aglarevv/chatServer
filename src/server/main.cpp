#include "ChatServer.hpp"
#include "ChatService.hpp"
#include <iostream>
#include <signal.h>

void reset(int){
    ChatService::instance()->reset();
    exit(0);
}


int main(){
    signal(SIGINT, reset);
    InetAddress addr("127.0.0.1",8888);
    EventLoop loop;
    ChatServer server(&loop, addr, "chatServer");
    server.start();
    loop.loop();
}