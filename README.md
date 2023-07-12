# chatServer
使用c++实现的集群聊天服务器

## 功能

- 注册账号
- 登录账号
- 添加好友
- 好友私聊
- 创建群组
- 添加群组
- 群组聊天


## 环境要求/依赖项

- Linux
- C++11
- gcc/g++ 7.5
- redis
- nginx
- muduo
- nlohmann的json库

## 项目启动

```
./autobuild.sh
执行前请先更改mysqlconfig.ini文件内容
```

## 数据库结构
![image](https://github.com/aglarevv/chatServer/blob/main/image/database.png)
![image](https://github.com/aglarevv/chatServer/blob/main/image/ALLGroup.png)
![image](https://github.com/aglarevv/chatServer/blob/main/image/Friend.png)
![image](https://github.com/aglarevv/chatServer/blob/main/image/GroupUser.png)
![image](https://github.com/aglarevv/chatServer/blob/main/image/OfflineMessage.png)
![image](https://github.com/aglarevv/chatServer/blob/main/image/User.png)

## 更新内容

```
引入redis消息队列中间件，用以跨服务器通信
引入数据库连接池
修复若干问题
```

## 即将添加的功能

- 增加客户端图形界面

