#ifndef DATABAZOVY_SYSTEM_DBMS_H
#define DATABAZOVY_SYSTEM_DBMS_H


#include <string>
#include <ctime>
#include <map>
#include <algorithm>
#include "Table.h"
#include "FileManager.h"
#include "User.h"
#include "Permission.h"

class DBMS {
private:
    FileManager* fileManager;

    std::vector<User*> users;
    std::vector<TableItem*> tables;
    std::map<std::string, std::map<std::string, std::vector<PermissionType>>> permissions;

    bool tableExists(const std::string& tableName);
    bool dataTypeCheck(std::string value, RowDataType type, bool isNullable);

    void filterRows(std::vector<std::vector<std::string>>& rows, TableScheme& tableScheme, std::vector<Condition> conditions);
    void filterOutRows(std::vector<std::vector<std::string>>& rows, std::vector<std::vector<std::string>>& filteredOutRows, TableScheme& tableScheme, std::vector<Condition> conditions);
    void validateExistingColumnsAndTypes(std::map<std::string, std::string>& record, TableScheme& tableScheme);
    void orderRows(std::vector<std::vector<std::string>>& rows, TableScheme& tableScheme, std::string orderColumn, bool ascending);

    void authorizeAccess(std::string tableName, std::string currentUser, PermissionType permissionType);
public:
    DBMS();
    ~DBMS();

    // User management
    bool authorize(std::string username, std::string password);
    bool createUser(std::string username, std::string password);
    std::vector<std::string> getUsersList();
    bool userExists(std::string username);

    // Permission management
    void grantPermission(std::string targetUser, std::string tableName, PermissionType permissionType, std::string currentUser);
    void revokePermission(std::string targetUser, std::string tableName, PermissionType permissionType, std::string currentUser);

    // Table management
    void createTable(TableScheme& tableScheme);
    void dropTable(std::string tableName, std::string currentUser);
    std::vector<std::string> getTablesList();
    std::vector<std::string> getTablesListCreatedByUser(const std::string& username);
    std::map<std::string, std::string> getTablesWithPermissions(std::string username);

    // Data management
    std::vector<std::vector<std::string>> selectFromTable(std::string tableName, std::vector<std::string> columns, std::vector<Condition> conditions, std::string orderColumn, bool ascending, std::string currentUser);
    void insertIntoTable(std::string tableName, std::map<std::string, std::string> newRecord, std::string currentUser);
    size_t updateTable(std::string tableName, std::map<std::string, std::string> newValues, std::vector<Condition> conditions, std::string currentUser);
    size_t deleteFromTable(std::string tableName, std::vector<Condition> conditions, std::string currentUser);


    void TEST_printState();
};


#endif //DATABAZOVY_SYSTEM_DBMS_H
