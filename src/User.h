#ifndef DATABAZOVY_SYSTEM_USER_H
#define DATABAZOVY_SYSTEM_USER_H


#include <string>

class User {
private:
    std::string username;
    std::string password;
public:
    User(std::string username, std::string password): username(username), password(password) {
    }

    std::string getUsername() {
        return this->username;
    }

    std::string getPassword() {
        return this->password;
    }

    std::string serialize() {
        return this->username + ";" + this->password;
    }
};


#endif //DATABAZOVY_SYSTEM_USER_H
