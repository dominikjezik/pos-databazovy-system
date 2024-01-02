#ifndef DATABAZOVY_SYSTEM_INTERPRETER_H
#define DATABAZOVY_SYSTEM_INTERPRETER_H


#include "DBMS.h"
#include "Encoder.h"
#include <sstream>

class Interpreter {
private:
    DBMS* dbms;
    std::string show(std::vector<std::string>& words);
    std::string create(std::vector<std::string>& words, std::string currentUser);
    std::string createUser(std::vector<std::string>& words);
    std::string createTable(std::vector<std::string>& words, std::string currentUser);
    std::string drop(std::vector<std::string>& words, std::string currentUser);
    std::string select(std::vector<std::string>& words, std::string currentUser);
    std::string insert(std::vector<std::string>& words, std::string currentUser);
    std::string update(std::vector<std::string>& words, std::string currentUser);
    std::string deleteCommand(std::vector<std::string>& words, std::string currentUser);
    std::string grant(std::vector<std::string>& words, std::string currentUser);
    std::string revoke(std::vector<std::string>& words, std::string currentUser);

    void parseCommand(std::string command, std::vector<std::string>& words);
    std::string parseWhereConditions(std::vector<std::string>& words, std::vector<Condition>& conditions);
public:
    Interpreter();
    ~Interpreter();
    std::string run(std::string command, std::string currentUser);
};


#endif //DATABAZOVY_SYSTEM_INTERPRETER_H
