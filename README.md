# chatServer
使用c++实现的聊天服务器，依赖muduo网络库，nlohmann的json库
目前只支持在同一服务器上通信

## 功能

-注册账号
-登录账号
-添加好友
-好友私聊
-创建群组
-添加群组
-群组聊天


## 环境要求

- Linux

## 项目启动

```
mkdir build
cd build
cmake ../CMakeLists.txt
make
```

## 数据库结构
数据库chat：
+----------------+
| Tables_in_chat |
+----------------+
| ALLGroup       |
| Friend         |
| GroupUser      |
| OfflineMessage |
| User           |
+----------------+

ALLGroup表：存储所有用户创建的群组
+-----------+--------------+------+-----+---------+----------------+
| Field     | Type         | Null | Key | Default | Extra          |
+-----------+--------------+------+-----+---------+----------------+
| id        | int          | NO   | PRI | NULL    | auto_increment |
| groupname | varchar(50)  | NO   |     | NULL    |                |
| groupdesc | varchar(200) | YES  |     |         |                |
+-----------+--------------+------+-----+---------+----------------+

Friend表：存储所有用户的好友
+----------+-------------+------+-----+---------+-------+
| Field    | Type        | Null | Key | Default | Extra |
+----------+-------------+------+-----+---------+-------+
| userid   | int         | NO   | PRI | NULL    |       |
| username | varchar(50) | NO   |     | NULL    |       |
| friendid | int         | NO   | PRI | NULL    |       |
+----------+-------------+------+-----+---------+-------+

GroupUser表：存储所有群组成员信息
+-----------+--------------------------+------+-----+---------+-------+
| Field     | Type                     | Null | Key | Default | Extra |
+-----------+--------------------------+------+-----+---------+-------+
| groupid   | int                      | NO   | PRI | NULL    |       |
| userid    | int                      | NO   | PRI | NULL    |       |
| username  | varchar(50)              | NO   |     | NULL    |       |
| grouprole | enum('creator','normal') | YES  |     | normal  |       |
+-----------+--------------------------+------+-----+---------+-------+

OfflineMessage表：存储所有用户的离线消息
+----------+--------------+------+-----+---------+----------------+
| Field    | Type         | Null | Key | Default | Extra          |
+----------+--------------+------+-----+---------+----------------+
| userid   | int          | NO   | PRI | NULL    | auto_increment |
| username | varchar(50)  | NO   |     | NULL    |                |
| message  | varchar(500) | NO   |     | NULL    |                |
+----------+--------------+------+-----+---------+----------------+

User表：存储所有注册用户
+----------+--------------+------+-----+---------+----------------+
| Field    | Type         | Null | Key | Default | Extra          |
+----------+--------------+------+-----+---------+----------------+
| userid   | int          | NO   | PRI | NULL    | auto_increment |
| username | varchar(50)  | NO   |     | NULL    |                |
| message  | varchar(500) | NO   |     | NULL    |                |
+----------+--------------+------+-----+---------+----------------+


## 即将添加的功能

-引入负载均衡器，提高并发量
-支持跨服务器通信
