#include "FileManager.h"

#include <utility>

FileManager::FileManager(std::string basePath) : basePath(std::move(basePath)) {
}


void FileManager::loadUsers(std::vector<User*>& users) {
    this->createFileIfNotExists(this->basePath + "/users.csv");

    csvManager.loadFile(this->basePath + "/users.csv", 2, [&](std::string* line) {
        User* user = new User();
        user->parseData(line);
        users.push_back(user);
    });

//    User* userA = new User("admin", "admin");
//    users.push_back(userA);
}


void FileManager::saveUser(User *user) {
    csvManager.appendToFile(this->basePath + "/users.csv", user->serialize());
}


void FileManager::loadTablesList(std::vector<std::string>& tables) {
    this->createFileIfNotExists(this->basePath + "/tables.csv");

    csvManager.loadFile(this->basePath + "/tables.csv", 1, [&](std::string* line) {
        tables.push_back(line[0]);
    });

//    tables.push_back("posts");
//    tables.push_back("comments");
}


TableScheme *FileManager::loadTableScheme(std::string tableName) {
    TableScheme* tableScheme = new TableScheme();
    tableScheme->setName(tableName);
    bool firstLine = true;

    csvManager.loadFile(this->basePath + "/tables/" + tableName + "_scheme.csv", 3, [&](std::string* line) {
        if (firstLine) {
            firstLine = false;
            tableScheme->setOwner(line[0]);
            tableScheme->setPrimaryKey(line[1]);
            return;
        }

        TableRowScheme row(line[0], (RowDataType) std::stoi(line[1]), std::stoi(line[2]));
        tableScheme->addRow(row);
    });

    return tableScheme;
}


void FileManager::createTable(TableScheme *tableScheme) {
    // zapis nazov tabulky do suboru s tabulkami
    csvManager.appendToFile(this->basePath + "/tables.csv", tableScheme->getName());

    // vytvor subor kde je schéma tabulky
    csvManager.createFile(this->basePath + "/tables/" + tableScheme->getName() + "_scheme.csv", [&](std::fstream& file){
        file << tableScheme->getOwner() << ";" << tableScheme->getPrimaryKey() << std::endl;

        for (auto row : tableScheme->getRows()) {
            file << row.getName() << ";" << row.getDataType() << ";" << row.isNullable() << std::endl;
        }
    });

    // vytvor subor kde su data tabulky
    csvManager.createFile(this->basePath + "/tables/" + tableScheme->getName() + "_data.csv", [&](std::fstream& file){ });
}


void FileManager::dropTable(std::string tableName) {
    // Vymaz nazov tabulky zo suboru s tabulkami
    csvManager.removeLineFromFile(this->basePath + "/tables.csv", tableName);

    // Vymaz subor so schemou tabulky
    if(std::remove((this->basePath + "/tables/" + tableName + "_scheme.csv").c_str()) != 0) {
        throw std::invalid_argument("Nastala chyba pri vymazavani suboru so schemou tabulky!");
    }

    // Vymaz subor s datami tabulky
    if(std::remove((this->basePath + "/tables/" + tableName + "_data.csv").c_str()) != 0) {
        throw std::invalid_argument("Nastala chyba pri vymazavani suboru s datami tabulky!");
    }

    // TODO: odstranit permisie na tabulku
}




void FileManager::createFileIfNotExists(const std::string& filename) {
    std::fstream file(filename, std::ios::in);

    if (!file.is_open()) {
        std::fstream file(filename, std::ios::out);
        file.close();
    }
}

void FileManager::insertIntoTable(const std::string& tableName, const std::string& row) {
    csvManager.appendToFile(this->basePath + "/tables/" + tableName + "_data.csv", row);
}

std::vector<std::vector<std::string>> FileManager::loadTableData(std::string tableName, int numberOfColumns) {
    std::vector<std::vector<std::string>> data;

    csvManager.loadFile(this->basePath + "/tables/" + tableName + "_data.csv", numberOfColumns, [&](std::string* line) {
        std::vector<std::string> row;

        // Predalokovanie pamate pre vektor
        row.reserve(numberOfColumns);

        for (int i = 0; i < numberOfColumns; i++) {
            row.push_back(line[i]);
        }

        data.push_back(row);
    });

    return data;
}
