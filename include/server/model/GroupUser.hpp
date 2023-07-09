#pragma once

#include "User.hpp"

class GroupUser : public User{
    public:
        void setRole(std::string role) { this->role = role; }
        std::string getRole() { return this->role; }
    private:
        std::string role;
};