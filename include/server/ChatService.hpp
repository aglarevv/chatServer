#pragma once

#include "muduo/net/TcpServer.h"
#include "json.hpp"
#include "redis.hpp"
using json = nlohmann::json;

#include <functional>
#include <mutex>
#include <unordered_map>
#include <map>
#include <string>


class GroupModel;
class FriendModel;
class OfflineMessageModel;
class UserModel;
class ChatService {
    public:
        //消息处理回调
        using MsgHandle = std::function<void(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp)>;
        //单例对象的接口
        static ChatService* instance();
        //登录业务处理方法
        void login(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);
        //登出
        void loginout(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);
        //注册
        void regist(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);
        //一对一聊天
        void oneChat(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);
        //添加好友
        void addFriend(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);
        //创建群组
        void createGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);
        //加入群组
        void addGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);
        //群组聊天
        void chatGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time);
        //redis回调
        void redisHandleMessage(std::string channel, std::string message);
        //获取对应的业务处理方法
        MsgHandle getHandle(int msgType); 
        //客户端连接异常断开处理
        void clientCloseException(const muduo::net::TcpConnectionPtr& conn);
        //服务器异常断开
        void reset();
    private:
        ChatService();
        //消息类型及其对应的业务处理方法
        std::unordered_map<int, MsgHandle> handleMap_;

        std::mutex mutex_;
        //保存用户及对应的连接
        std::map<std::string, muduo::net::TcpConnectionPtr> userConnMap_;
        //保存用户id和name
        std::map<int, std::string> userIdName_;

        //数据库操作
        UserModel* userModel_;
        OfflineMessageModel* OfflineMessageModel_;
        FriendModel* friendModel_;
        GroupModel* groupModel_;

        Redis redis_;
};

