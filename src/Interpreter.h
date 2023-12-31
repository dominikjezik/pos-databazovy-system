#ifndef DATABAZOVY_SYSTEM_INTERPRETER_H
#define DATABAZOVY_SYSTEM_INTERPRETER_H


#include "DBMS.h"
#include <sstream>

class Interpreter {
private:
    DBMS* dbms;
    std::string show(std::vector<std::string>& words);
    std::string create(std::vector<std::string>& words);
    std::string createUser(std::vector<std::string>& words);
    std::string createTable(std::vector<std::string>& words);
    std::string drop(std::vector<std::string>& words);
    std::string select(std::vector<std::string>& words);
    std::string insert(std::vector<std::string>& words);
    std::string update(std::vector<std::string>& words);
    std::string deleteCommand(std::vector<std::string>& words);
    void parseCommand(std::string command, std::vector<std::string>& words);
    std::string parseWhereConditions(std::vector<std::string>& words, std::vector<Condition>& conditions);

    std::string success(std::string message);
    std::string success(std::vector<std::vector<std::string>>& result);
    std::string error();
    std::string errorUnknownCommand();
    std::string error(std::string message);
public:
    Interpreter();
    ~Interpreter();
    void run(std::string command);
};


#endif //DATABAZOVY_SYSTEM_INTERPRETER_H
