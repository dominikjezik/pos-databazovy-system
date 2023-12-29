#ifndef DATABAZOVY_SYSTEM_DBMS_H
#define DATABAZOVY_SYSTEM_DBMS_H


#include <string>
#include <ctime>
#include <map>
#include <algorithm>
#include "Table.h"
#include "FileManager.h"
#include "User.h"

class DBMS {
private:
    FileManager* fileManager;

    std::vector<User*> users;
    std::vector<TableItem*> tables;

    bool tableExists(const std::string& tableName);
    bool dataTypeCheck(std::string value, RowDataType type);

    void filterRows(std::vector<std::vector<std::string>>& rows, TableScheme *tableScheme, std::vector<Condition> conditions);
public:
    DBMS();
    ~DBMS();

    // User management
    bool authorize(std::string username, std::string password);
    bool createUser(std::string username, std::string password);
    std::vector<std::string> getUsersList();
    bool userExists(std::string username);

    // Permission management
    // TODO: bool grantPermission(std::string targetUser, std::string tableName, std::string permission, std::string currentUser);
    // TODO: bool revokePermission(std::string targetUser, std::string tableName, std::string permission, std::string currentUser);

    // Table management
    bool createTable(TableScheme* tableScheme);
    void dropTable(std::string tableName, std::string currentUser);
    std::vector<std::string> getTablesList();
    std::vector<std::string> getTablesListCreatedByUser(const std::string& username);
    // TODO: void getTablesWithPermissions(std::string username);

    // Data management
    void insertIntoTable(std::string tableName, std::map<std::string, std::string> newRecord, std::string currentUser);
    // TODO: Update
    // TODO: Delete

    std::vector<std::vector<std::string>> selectFromTable(std::string tableName, std::vector<std::string> columns, std::vector<Condition> conditions, std::string orderColumn, bool ascending, std::string currentUser);

    void TEST_printState();
};


#endif //DATABAZOVY_SYSTEM_DBMS_H
