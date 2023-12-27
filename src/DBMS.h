#ifndef DATABAZOVY_SYSTEM_DBMS_H
#define DATABAZOVY_SYSTEM_DBMS_H


#include <string>
#include <ctime>
#include <map>
#include "Table.h"
#include "FileManager.h"
#include "User.h"

class DBMS {
private:
    FileManager* fileManager;

    std::vector<User*> users;
    std::vector<std::string> tables;

    bool tableExists(const std::string& tableName);
    bool dataTypeCheck(std::string value, RowDataType type);
public:
    DBMS();
    ~DBMS();

    // User management
    bool authorize(std::string username, std::string password);
    bool createUser(std::string username, std::string password);

    // Permission management
    // TODO: bool grantPermission(std::string targetUser, std::string tableName, std::string permission, std::string currentUser);
    // TODO: bool revokePermission(std::string targetUser, std::string tableName, std::string permission, std::string currentUser);

    // Table management
    bool createTable(TableScheme* tableScheme);
    void dropTable(std::string tableName, std::string currentUser);
    // TODO: void getTablesCreatedByUser(std::string username);
    // TODO: void getTablesWithPermissions(std::string username);

    // Data management
    void insertIntoTable(std::string tableName, std::map<std::string, std::string> newRecord, std::string currentUser);
    // TODO: Update
    // TODO: Delete
    // TODO: Select

    void TEST_printState();
};


#endif //DATABAZOVY_SYSTEM_DBMS_H
