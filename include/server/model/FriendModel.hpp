#pragma once

#include "User.hpp"

#include <string>
#include <vector>


class FriendModel {
    public:
        //添加好友
        void insert(int userid, std::string username, int friendid);
        //查询好友
        std::vector<User> query(std::string username);
};