#ifndef DATABAZOVY_SYSTEM_INTERPRETER_H
#define DATABAZOVY_SYSTEM_INTERPRETER_H


#include "DBMS.h"

class Interpreter {
private:
    DBMS* dbms;
    void show(std::vector<std::string> words);
    void create(std::vector<std::string> words);
    void createUser(std::vector<std::string> words);
    void createTable(std::vector<std::string> words);
    void drop(std::vector<std::string> words);
    void select(std::vector<std::string> words);
    void insert(std::vector<std::string> words);
    void deleteCommand(std::vector<std::string> words);
    void parseCommand(std::string command, std::vector<std::string> &words);
public:
    Interpreter();
    ~Interpreter();
    void run(std::string command);


};


#endif //DATABAZOVY_SYSTEM_INTERPRETER_H
