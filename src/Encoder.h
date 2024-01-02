#ifndef DATABAZOVY_SYSTEM_ENCODER_H
#define DATABAZOVY_SYSTEM_ENCODER_H


#include <iostream>
#include <string>
#include <vector>

class Encoder {
public:
    static std::string success(std::string message);
    static std::string success(std::vector<std::string>& result);
    static std::string success(std::vector<std::vector<std::string>>& result);
    static std::string success(std::string message, std::vector<std::string>& result);
    static std::string success(std::string message, std::vector<std::vector<std::string>>& result);
    static std::string error();
    static std::string errorUnknownCommand();
    static std::string error(std::string message);
};


#endif //DATABAZOVY_SYSTEM_ENCODER_H
