#pragma once

#include <string>
#include <vector>

class AllGroup;
class ConnectionPool;
class GroupModel {
    public:
        //创建群组 
        bool create(AllGroup& allGroup);
        //添加群员
        bool add(int groupid, int userid, std::string username, std::string role);
        //查询用户所在群组信息
        std::vector<AllGroup> queryGroups(std::string username);
        //查询除userid外群员列表
        std::vector<std::string> queryUsers(int userid, int groupid);

};