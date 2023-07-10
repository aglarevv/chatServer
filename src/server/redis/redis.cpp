#include "redis.hpp"
#include <iostream>

Redis::Redis() : publishContext_(nullptr), subscribeContext_(nullptr){

}
Redis::~Redis(){
    if(publishContext_ != nullptr){
        redisFree(publishContext_);
    }
    if(subscribeContext_ != nullptr){
        redisFree(subscribeContext_);
    }
}
//连接redis
bool Redis::connect(){
    publishContext_= redisConnect("127.0.0.1", 6379);
    if(nullptr == publishContext_){
        std::cerr << "redis publish connect error" << std::endl;
        return false;
    }

    subscribeContext_ = redisConnect("127.0.0.1", 6379);
    if(nullptr == subscribeContext_){
        std::cerr << "redis subscibe connect error" << std::endl;
        return false;
    }

    std::thread t([&](){
        observerChannelMessage();
    });
    t.detach();

    std::cout << "connect redis-server success" << std::endl;
    return true;   
}
//发布消息
bool Redis::publish(std::string channel, std::string message){
    redisReply* reply =(redisReply*)redisCommand(this->publishContext_,"publish %s %s", channel.c_str(), message.c_str());
    if(nullptr == reply){
        std::cerr << "redis publish command error" << std::endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}   
 
//订阅消息
bool Redis::subscribe(std::string channel){
    //发送命令
    if(REDIS_ERR == redisAppendCommand(this->subscribeContext_,"subscribe %s", channel.c_str())){
        std::cerr << "redis subscribe command error" << std::endl;
        return false;
    }
    int done = 0;
    while(!done){
        if(REDIS_ERR == redisBufferWrite(this->subscribeContext_, &done)){
            std::cerr << "redis subscribe command error" << std::endl;
            return false;
        }
    }
    return true;
}

//取消订阅
bool Redis::unSubscribe(std::string channel){
    if (REDIS_ERR == redisAppendCommand(this->subscribeContext_, "unsubscribe %s", channel.c_str()))
    {
        std::cerr << "redis unsubscribe command error" << std::endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->subscribeContext_, &done))
        {
            std::cerr << "redis unsubscribe command error" << std::endl;
            return false;
        }
    }
    return true;
}
//在独立线程中接收消息
void Redis::observerChannelMessage(){
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->subscribeContext_, (void **)&reply))
    {
        // 订阅收到的消息是一个带三元素的数组
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            // 给业务层上报通道上发生的消息
            notifyMessageHandle_(reply->element[1]->str, reply->element[2]->str);
        }

        freeReplyObject(reply);
    }

    std::cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << std::endl;
}
//初始化回调
void Redis::initNotifyMessageHandle(std::function<void(std::string, std::string)> cb){
    this->notifyMessageHandle_ = cb;
}