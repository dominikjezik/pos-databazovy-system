#ifndef DATABAZOVY_SYSTEM_FILEMANAGER_H
#define DATABAZOVY_SYSTEM_FILEMANAGER_H


#include "Table.h"
#include "User.h"
#include "CsvManager.h"
#include <iostream>
#include <filesystem>
#include <utility>

class FileManager {
private:
    std::string basePath;
    CsvManager csvManager;

    void createFileIfNotExists(const std::string& filename);
public:
    FileManager(std::string basePath);

    void loadUsers(std::vector<User*>& users);
    void saveUser(User *user);

    void loadTablesList(std::vector<TableItem*>& table);
    TableScheme* loadTableScheme(std::string tableName);

    void createTable(TableScheme& tableScheme);
    void dropTable(std::string tableName, std::string owner);

    void insertIntoTable(const std::string& tableName, const std::string& row);
    std::vector<std::vector<std::string>> loadTableData(std::string tableName, int numberOfColumns);
    void saveTableData(std::string tableName, std::vector<std::vector<std::string>> data);

    void createInitialFilesIfNotExists();
};


#endif //DATABAZOVY_SYSTEM_FILEMANAGER_H
