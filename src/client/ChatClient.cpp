#include "public.hpp"
#include "User.hpp"
#include "UserModel.hpp"
#include "ALLGroup.hpp"
#include "json.hpp"

#include <iostream>
#include <vector>
#include <ctime>
#include <chrono>
#include <string>
#include <cstring>
#include <thread>
#include <unordered_map>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <atomic>

using namespace std;
using json = nlohmann::json;

//当前登录用户
User currentUser;
//当前登录用户操作
UserModel userModel;
//当前登录用户好友列表
vector<User> c_UserFriendList;
//当前登录用户群组列表信息
vector<AllGroup> c_UserGroupList;
//信号量
sem_t sem;
//登录是否成功
atomic_bool isLoginSuccess;

//接收线程
void readTaskHandle(int clientfd);
//获取系统时间
string getCurrentTime();
//主页面聊天程序
void mainMenu(int clientfd);
//显示当前登录用户的基本信息
void showCurrentUserData();

//帮助命令列表
void help(int fd = 0, string str = "");
//聊天
void chat(int, string);
//添加好友
void addfriend(int, string);
//创建群组
void creategroup(int, string);
//加入群组
void addgroup(int, string);
//群组聊天
void groupchat(int, string);
//登出
void loginout(int, string);

//系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令,格式help"},
    {"chat", "一对一聊天,格式chat:friendname:message"},
    {"addfriend", "添加好友,格式addfriend:friendid"},
    {"creategroup", "创建群组,格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组,格式addgroup:groupid"},
    {"groupchat", "群聊,格式groupchat:groupid:message"},
    {"loginout", "注销,格式loginout"}};

//注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};


int main(int argc, char* argv[]){
    if(argc < 3){
        cerr << "command invalid ! example: ./chatClient 127.0.0.1 8888" << endl;
        exit(-1);
    }
    //解析ip和port
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);
    //创建client的socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd == -1){
        cerr << "client socket create error" << endl;
        exit(-1);
    }
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    //连接
    if( -1 == connect(clientfd, (sockaddr*)&addr, sizeof(sockaddr_in))){
        cerr << "socket connect error" << endl;
        close(clientfd);
        exit(-1);
    }
    sem_init(&sem, 0, 0);
    //启动接收子线程
    thread readTask(readTaskHandle, clientfd);
    readTask.detach();


    while(1){
        
        // 显示首页面菜单 登录、注册、退出
        cout << "========================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "========================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice;
        cin.get();
        
        switch (choice) {
            case 1://登录
            {
                string name;
                string password;
                cout << "username: " << endl;
                getline(cin, name);
                cout << "password: " << endl;
                getline(cin, password);
                
                json js;
                js["msgtype"] = LOGIN;
                js["name"] = name;
                js["password"] = password;
                string request = js.dump();

                isLoginSuccess = false;
                //向服务器发送
                ssize_t writeBytes = write(clientfd, request.c_str(), strlen(request.c_str())+1);
                if(writeBytes == -1){
                    cerr << "write login error: " << request << endl;
                }
                sem_wait(&sem);
                if(isLoginSuccess){
                    //显示函数
                    showCurrentUserData();
                    //聊天主界面
                    mainMenu(clientfd);     
                }
                else{
                    continue;
                }
            }

                break;

            case 2://注册
            {
                string name;
                string password;
                cout << "username: " << endl;
                getline(cin, name);
                cout << "password: " << endl;
                getline(cin, password);

                json js;
                js["msgtype"] = REGISTER;
                js["name"] = name;
                js["password"] = password;
                string request = js.dump();
                //向发送服务器
                ssize_t writeBytes = write(clientfd, request.c_str(), strlen(request.c_str())+1);
                if(writeBytes == -1){
                     cerr << "write register error: " << request << endl;
                }
            
                sem_wait(&sem);
            }   
                break;

            case 3: 
                close(clientfd);
                sem_destroy(&sem);
                exit(0);
            
            default:
                cout << "invalid input" << endl;           
                break;
        }
    }
}
//主页面聊天程序
void mainMenu(int clientfd){

    help();

    char buf[1024] = {0};
    for(;;){
        cin.getline(buf, 1024);
        string commandBuf(buf);
        string command;
        int index = commandBuf.find(":");
        if(index == -1){
            //命令是本身
            command = commandBuf;          
        }
        else{
            //:前的是命令
            command = commandBuf.substr(0, index);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            //无效命令
            cerr << "invalid input command!" << endl;
            continue;
        }
        // 调用相应命令的事件处理回调
        it->second(clientfd, commandBuf.substr(index + 1, commandBuf.size() - index)); 
        //登出命令，结束聊天界面程序
        if(command == "loginout"){
            return;
        }
    }

}
void loginResponse(json& responsejs){               
    //登录失败
    if(0 != responsejs["errno"].get<int>()){
        cerr << responsejs["errmsg"] << endl;
        return;
    }
    else {
        //保存当前用户id和name
        currentUser.setId(responsejs["id"].get<int>());
        currentUser.setName(responsejs["name"]);
        //检查好友信息
        if(responsejs.contains("friends")){
            c_UserFriendList.clear();
            vector<string> vec = responsejs["friends"];
            for(string& str : vec){
                json js = json::parse(str);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                c_UserFriendList.push_back(user);
            }
        }
        isLoginSuccess = true;
    }
    //检查离线消息
    if(responsejs.contains("offlinemsg")){
        vector<string> vec = responsejs["offlinemsg"];
        for(string& msg : vec){
            json js = json::parse(msg);
            if(ONE_CHAT == js["msgtype"].get<int>()){
                cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                    << " said: " << js["msg"].get<string>() << endl;
            }
            else {
                cout << "群消息[" << js["groupid"].get<int>() << "]: " << js["time"].get<string>() 
                    << " [" << js["id"] << "]" << js["name"].get<string>()
                    << " said: " << js["msg"].get<string>() << endl;                               
            }                                    
        }
    }
    //检查群组信息
    if(responsejs.contains("groups")){
        c_UserGroupList.clear();
        vector<string> vec = responsejs["groups"];
        for(string& group : vec){
            json groupjs = json::parse(group);
            AllGroup allGroup;
            allGroup.setGroupId(groupjs["id"].get<int>());
            allGroup.setGroupName(groupjs["groupname"]);
            allGroup.setGroupDesc(groupjs["groupdesc"]);
            
            vector<string> vec2 = groupjs["users"];
            for(string& user : vec2){
                json userjs = json::parse(user);
                GroupUser groupUser;
                groupUser.setId(userjs["id"].get<int>());
                groupUser.setName(userjs["name"]);
                groupUser.setState(userjs["state"]);
                groupUser.setRole(userjs["role"]);
                allGroup.getUsers().push_back(groupUser);
            }
            c_UserGroupList.push_back(allGroup);
        }                            
    }                                                              
                               
}

//接收服务器消息线程
void readTaskHandle(int clientfd){
    std::vector<char> buffer;
    for(;;){
        buffer.clear();
        while(true){
            char buf[1024] = {0};
            ssize_t readBytes = read(clientfd, buf, sizeof(buf));
            if(readBytes == -1 || readBytes == 0){
                close(clientfd);
                exit(-1);
            }
            buffer.insert(buffer.end(), buf, buf+readBytes);
            if(readBytes < sizeof(buf)){
                break;
            }
        }      
              
        json js = json::parse(buffer);
        int msgType = js["msgtype"].get<int>();
        //登录
        if(LOGIN_ACK == msgType){
            loginResponse(js);
            sem_post(&sem);
            continue;
        }
        //注册
        if(REGISTER_ACK == msgType){
            if(0 != js["errno"].get<int>()){
                cerr << "register error errmsg: " <<  js["errmsg"] << endl;
            }
            else{
                cout << "register success, userid is: " << js["id"] << endl;
            }
            sem_post(&sem);
            continue;
        }
        //创建群组
        if(GROUP_CREATE_ACK == msgType){
           cout << "errno: " << js["errno"].get<int>() << js["errmsg"].get<string>() << endl;
        }
        //加入群组
        if(GROUP_ADD_ACK == msgType){
            cout << "errno: " << js["errno"].get<int>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << js["errmsg"].get<string>() << endl;
            continue; 
        }
        //添加好友
        if(FRIEND_ADD_ACK == msgType){
           cout << "errno: " << js["errno"].get<int>() << js["errmsg"].get<string>() << endl;
        }
        //单聊
        if(ONE_CHAT == msgType){
            cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
            continue; 
        }
        //群组聊天
        if(GROUP_CHAT == msgType){
            cout << "群消息[" << js["groupid"].get<int>() << "]: " << js["time"].get<string>() 
                 << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
            continue; 
        }
        
        
    }
}

//获取系统时间
string getCurrentTime(){
    auto time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    struct tm* ptm = localtime(&time);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
        (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
        (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return string(date);
}
// 帮助命令列表
void help(int fd, string str){
    cout << "命令列表 >>> " << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}
//聊天
void chat(int clientfd, string command){
    int index = command.find(":");
    if(-1 == index){
        cerr << "无效的命令" << endl;
        return;
    }
    string toname = command.substr(0, index);
    string message = command.substr(index+1, command.size()-index);
    json js;
    js["msgtype"] = ONE_CHAT;
    js["id"] = currentUser.getId();
    js["name"] = currentUser.getName();
    js["toname"] = toname;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buf = js.dump();
    ssize_t writeBytes = write(clientfd, buf.c_str(), strlen(buf.c_str())+1);
    if(writeBytes == -1){
        cerr << "write chat msg error -> " << buf<< endl;
    }

}
// 添加好友
void addfriend(int clientfd, string friendid){
    int friendId = atoi(friendid.c_str());
    json js;
    js["msgtype"] = FRIEDN_ADD;
    js["id"] = currentUser.getId();
    js["name"] = currentUser.getName();
    js["friendid"] = friendId;
    string buf = js.dump();
    ssize_t writeBytes = write(clientfd, buf.c_str(), strlen(buf.c_str())+1);
    if(writeBytes == -1){
        cerr << "write addfriend msg error -> " << buf<< endl;
    }
}
// 创建群组
void creategroup(int clientfd, string groupname){
    int index =groupname.find(":");
    if(-1 == index){
        cerr << "无效的命令" << endl;
        return;
    }
    string groupName = groupname.substr(0, index);
    string groupdesc = groupname.substr(index+1, groupname.size()-index);
    json js;
    js["msgtype"] = GROUP_CREATE;
    js["id"] = currentUser.getId();
    js["name"] = currentUser.getName();
    js["groupname"] = groupName;
    js["groupdesc"] = groupdesc;
    string buf = js.dump();
    ssize_t writeBytes = write(clientfd, buf.c_str(), strlen(buf.c_str())+1);
    if(writeBytes == -1){
        cerr << "write creategroup msg error -> " << buf<< endl;
    }
}
// 加入群组
void addgroup(int clientfd, string groupid){
    int groupId = atoi(groupid.c_str());  
    json js;
    js["msgtype"] = GROUP_ADD;
    js["id"] = currentUser.getId();
    js["name"] = currentUser.getName();
    js["groupid"] = groupId;
    string buf = js.dump();
    ssize_t writeBytes = write(clientfd, buf.c_str(), strlen(buf.c_str())+1);
    if(writeBytes == -1){
        cerr << "write groupid error -> " << buf<< endl;
    }
}
// 群组聊天
void groupchat(int clientfd, string groupid){
    int index = groupid.find(":");
    if(-1 == index){
        cerr << "无效的命令" << endl;
        return;
    }
    int groupId = atoi(groupid.substr(0, index).c_str());
    string message = groupid.substr(index+1, groupid.size()-index);
    json js;
    js["msgtype"] = GROUP_CHAT;
    js["id"] = currentUser.getId();
    js["name"] = currentUser.getName();
    js["groupid"] = groupId;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buf = js.dump();
    ssize_t writeBytes = write(clientfd, buf.c_str(), strlen(buf.c_str())+1);
    if(writeBytes == -1){
        cerr << "write groupchat error -> " << buf<< endl;
    }
}
// 登出
void loginout(int clientfd, string str){
    json js;
    js["msgtype"] = LOGINOUT;
    js["id"] = currentUser.getId();
    js["name"] = currentUser.getName();
    string buf = js.dump();
    ssize_t writeBytes = write(clientfd, buf.c_str(), strlen(buf.c_str())+1);
    if(writeBytes == -1){
        cerr << "write loginout error -> " << buf<< endl;
    }
}
//显示当前登录用户的基本信息
void showCurrentUserData(){
    cout << "======================login user======================" << endl;
    cout << "current login user => id:" << currentUser.getId() << " name:" << currentUser.getName() << endl;
    cout << "----------------------friend list---------------------" << endl;
    if (!c_UserFriendList.empty())
    {
        for (User &user : c_UserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "----------------------group list----------------------" << endl;
    if (!c_UserGroupList.empty())
    {
        for (AllGroup &group : c_UserGroupList)
        {
            cout << group.getGroupId() << " " << group.getGroupName() << " " << group.getGroupDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState()
                     << " " << user.getRole() << endl;
            }
        }
    }
    cout << "======================================================" << endl;
}

