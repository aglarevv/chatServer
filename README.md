# chatServer
使用c++实现的聊天服务器，依赖muduo网络库，nlohmann的json库
目前只支持在同一服务器上通信

## 功能

- 注册账号
- 登录账号
- 添加好友
- 好友私聊
- 创建群组
- 添加群组
- 群组聊天


## 环境要求

- Linux
- C++11

## 项目启动

```
mkdir build
cd build
cmake ../CMakeLists.txt
make
```

## 数据库结构



## 即将添加的功能

-引入负载均衡器，提高并发量
-支持跨服务器通信
