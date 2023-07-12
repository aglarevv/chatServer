#pragma once

enum MsgType {
    LOGIN = 1,//登录
    LOGIN_ACK,//登录响应
    LOGINOUT,//登出
    REGISTER,//注册
    REGISTER_ACK,//注册响应
    ONE_CHAT,//一对一聊天
    FRIEDN_ADD,//添加好友
    FRIEND_ADD_ACK,//添加好友响应
    GROUP_CREATE,//创建群组
    GROUP_CREATE_ACK,//创建群组响应
    GROUP_ADD,//加入群组
    GROUP_ADD_ACK,//加入群组响应
    GROUP_CHAT,//群组聊天
};