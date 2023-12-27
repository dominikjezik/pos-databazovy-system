#ifndef DATABAZOVY_SYSTEM_USER_H
#define DATABAZOVY_SYSTEM_USER_H


#include <string>

class User {
private:
    std::string username;
    std::string password;
public:
    User() = default;

    User(std::string username, std::string password): username(username), password(password) {
    }

    void parseData(std::string* data) {
        this->username = data[0];
        this->password = data[1];
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
