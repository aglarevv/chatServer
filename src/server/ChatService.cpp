#include "ChatService.hpp"
#include "public.hpp"
#include "muduo/base/Logging.h"
#include "UserModel.hpp"
#include "User.hpp"
#include "OfflineMessageModel.hpp"
#include "FriendModel.hpp"
#include "GroupModel.hpp"
#include "ALLGroup.hpp"

#include <iostream>
//对外实例化方法
ChatService* ChatService::instance(){
    static ChatService server;
    return &server;
}
//构造
ChatService::ChatService(){
    //登录
    handleMap_.insert(
        {LOGIN, std::bind(&ChatService::login, this,
         std::placeholders::_1, 
         std::placeholders::_2, 
         std::placeholders::_3)}
    );
    //登出
    handleMap_.insert(
        {LOGINOUT, std::bind(&ChatService::loginout, this,
         std::placeholders::_1, 
         std::placeholders::_2, 
         std::placeholders::_3)}
    );
    //注册
    handleMap_.insert(
        {REGISTER, std::bind(&ChatService::regist, this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3)}
    );
    //一对一聊天
    handleMap_.insert(
        {ONE_CHAT, std::bind(&ChatService::oneChat, this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3)}
    );
    //添加好友
    handleMap_.insert(
        {FRIEDN_ADD, std::bind(&ChatService::addFriend, this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3)}
    );
    //创建群组
    handleMap_.insert(
        {GROUP_CREATE, std::bind(&ChatService::createGroup, this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3)}
    );
    //加入群组
    handleMap_.insert(
        {GROUP_ADD, std::bind(&ChatService::addGroup, this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3)}
    );
    //群组聊天
    handleMap_.insert(
        {GROUP_CHAT, std::bind(&ChatService::chatGroup, this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3)}
    );
}
//登录
void ChatService::login(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    std::string name = js["name"];
    std::string password = js["password"];
    User user = userModel_->query(name);
    if(user.getPassword() == password && (name != "")){
        //登录成功
        if(user.getState() == "online"){
            //重复登录
            json response;
            response["msgtype"] = LOGIN_ACK;
            response["errno"] = 1;
            response["errmsg"] = "该用户已登录";
            conn->send(response.dump());
        }
        else{
            //首次登录
            {
                std::unique_lock<std::mutex> lock(mutex_);
                userConnMap_.insert({name, conn});
            }
            user.setState("online");
            userModel_->updataState(user);
            json response;
            response["msgtype"] = LOGIN_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            //获取离线消息
            std::vector<std::string> offlineMessageVec = OfflineMessageModel_->query(name);
            if(!offlineMessageVec.empty()){
                response["offlinemsg"] = offlineMessageVec;
                OfflineMessageModel_->remove(name);
            }
            //获取好友信息
            auto friendVec = friendModel_->query(name);
            if(!friendVec.empty()){
                std::vector<std::string> vec;
                for(auto& user : friendVec){
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec.push_back(js.dump());
                }
                response["friends"] = vec;
            }
            //获取群组信息
            std::vector<AllGroup> groupVec = groupModel_->queryGroups(name);
            if(!groupVec.empty()){
                //群组
                std::vector<std::string> groupV;
                for(AllGroup& group : groupVec){
                    json groupjs;
                    groupjs["id"] = group.getGroupId();
                    groupjs["groupname"] = group.getGroupName();
                    groupjs["groupdesc"] = group.getGroupDesc();
                    //群员
                    std::vector<std::string> userV;
                    for(GroupUser& user : group.getUsers()){
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    groupjs["users"] = userV;
                    groupV.push_back(groupjs.dump());
                }
                response["groups"] = groupV;
            }
            conn->send(response.dump());
        }
    }
    //登录失败
    else{
        json response;
        response["msgtype"] = LOGIN_ACK;
        response["errno"] = 1;
        response["errmsg"] = "登录失败,用户名或密码错误";
        conn->send(response.dump());
    }
}
//登出
void ChatService::loginout(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    std::string name = js["name"];
     {
        std::unique_lock<std::mutex> lock(mutex_);
        auto it = userConnMap_.find(name);
        if(it != userConnMap_.end()){
            userConnMap_.erase(it);
        }
    }
    //断开服务器连接
    User user;
    user.setName(name);
    userModel_->updataState(user);
}
//注册
void ChatService::regist(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    std::string name = js["name"];
    std::string password = js["password"];
    User user;
    user.setName(name);
    user.setPassword(password);
    bool state = userModel_->insert(user);
    if(state){
        //注册成功
        json response;
        response["msgtype"] = REGISTER_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else{
        //注册失败
        json response;
        response["msgtype"] = REGISTER_ACK;
        response["errno"] = 1;
        response["errmsg"] = "注册失败";
        conn->send(response.dump());
    }

}
//一对一聊天
void ChatService::oneChat(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    std::string name = js["toname"];
    
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto it = userConnMap_.find(name);
        if(it != userConnMap_.end()){
            //在线，转发消息
            it->second->send(js.dump());
            return;
        }
        else{
            //不在线，保存到离线消息
            OfflineMessageModel_->insert(name, js.dump());
        }
    }
}
//添加好友
void ChatService::addFriend(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    int userid = js["id"].get<int>();
    std::string username = js["name"];
    int friendId = js["friendid"].get<int>();

    friendModel_->insert(userid, username, friendId);
}
//创建群组
void ChatService::createGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    int userid = js["id"].get<int>();
    std::string username = js["name"];
    std::string groupname = js["groupname"];
    std::string desc = js["groupdesc"];
    AllGroup group(-1, groupname, desc);
    if(groupModel_->create(group)){
        groupModel_->add(group.getGroupId(), userid, username ,"creator");
    }
}
//加入群组
void ChatService::addGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    int userid = js["id"].get<int>();
    std::string username = js["name"];
    int groupid = js["groupid"].get<int>();
    bool res = groupModel_->add(groupid, userid, username, "normal");
    json responsejs;
    if(!res){
        responsejs["msgtype"] = GROUP_ADD_ACK;
        responsejs["errno"] = 1;
        responsejs["id"] = userid;
        responsejs["name"] = username;
        responsejs["errmsg"] = "加入失败，该群组不存在或已加入";
        conn->send(responsejs.dump());
    }
}
//群组聊天
void ChatService::chatGroup(const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp time){
    int userid = js["id"].get<int>();
    std::string username = js["name"];
    int groupid = js["groupid"].get<int>();
    std::vector<std::string> vec = groupModel_->queryUsers(userid, groupid);
    std::unique_lock<std::mutex> lock(mutex_);
    for(auto user : vec){
        auto it = userConnMap_.find(user);
        if(it != userConnMap_.end()){
            it->second->send(js.dump());
        }
        else{
            OfflineMessageModel_->insert(user, js.dump());
        }
    }
}
//业务分发
ChatService::MsgHandle ChatService::getHandle(int msgType){
    //记录错误日志
    auto it = handleMap_.find(msgType);
    if(it == handleMap_.end()){
        return [=](const muduo::net::TcpConnectionPtr& conn, json& js, muduo::Timestamp){
            LOG_ERROR << "msgType: " << msgType << " getHandl error";
        };
    }
    else{
        //分发不同消息的处理函数
        return handleMap_[msgType];
    }
}
//客户端连接断开异常
void ChatService::clientCloseException(const muduo::net::TcpConnectionPtr& conn){
    User user;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for(auto it = userConnMap_.begin(); it != userConnMap_.end(); ++it){
            if(it->second == conn){
                user.setName(it->first);
                userConnMap_.erase(it);
                break;
            }
        }
    }
    //断开服务器连接
    if(user.getName() != ""){
        user.setState("offline");
        userModel_->updataState(user);
    }
}
//服务器连接断开异常
void ChatService::reset(){
    //重置客户端状态
    userModel_->resetState();
}