#include "ChatServer.hpp"
#include "ChatService.hpp"
#include <iostream>
#include <string>
#include <signal.h>

void reset(int){
    ChatService::instance()->reset();
    exit(0);
}


int main(int argc, char* argv[]){
    
    std::string ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, reset);
    InetAddress addr(ip, port);
    EventLoop loop;
    ChatServer server(&loop, addr, "chatServer");
    server.start();
    loop.loop();
}