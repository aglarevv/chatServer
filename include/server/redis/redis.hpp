#pragma once

#include <hiredis/hiredis.h>
#include <functional>
#include <string>
#include <thread>

class Redis {
    public:
        Redis();
        ~Redis();
        //连接redis
        bool connect();
        //发布消息
        bool publish(std::string channel, std::string message);
        //订阅消息
        bool subscribe(std::string channel);
        //取消订阅
        bool unSubscribe(std::string channel);
        //在独立线程中接收消息
        void observerChannelMessage();
        //初始化回调
        void initNotifyMessageHandle(std::function<void(std::string, std::string)> cb);
    private:
        //上下文对象，负责publish
        redisContext* publishContext_;
        //负责subcribe
        redisContext* subscribeContext_;
        //回调，给server层上报消息
        std::function<void(std::string, std::string)> notifyMessageHandle_;
};  



