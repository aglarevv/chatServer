#pragma once

#include "GroupUser.hpp"

#include <vector>
#include <string>


class AllGroup {
    public:
        AllGroup(int id = -1, std::string groupname = "", std::string groupdesc = ""){
            this->id = id;
            this->groupname = groupname;
            this->groupdesc = groupdesc;
        }
        void setGroupId(int id) { this->id = id; }
        void setGroupName(std::string groupname) { this->groupname = groupname; }
        void setGroupDesc(std::string groupdesc) { this->groupdesc = groupdesc; }

        int getGroupId() { return this->id; }
        std::string getGroupName() { return this->groupname; }
        std::string getGroupDesc() { return this->groupdesc; }
        //获取群组成员
        std::vector<GroupUser>& getUsers() { return this->users;}

    private:
        int id;
        std::string groupname;
        std::string groupdesc;
        //群组成员
        std::vector<GroupUser> users;
};