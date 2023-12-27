#ifndef DATABAZOVY_SYSTEM_TABLEROW_H
#define DATABAZOVY_SYSTEM_TABLEROW_H


#include <string>
#include <vector>

enum RowDataType {
    type_string,
    type_int,
    type_double,
    type_boolean,
    type_date
};

class TableRowScheme {
private:
    std::string name;
    RowDataType dataType;
    bool nullable;

public:
    TableRowScheme(std::string name, RowDataType dataType, bool nullable): name(name), dataType(dataType), nullable(nullable) {
    }
    std::string getName() {
        return this->name;
    }
    RowDataType getDataType() {
        return this->dataType;
    }
    bool isNullable() {
        return this->nullable;
    }
};


class TableScheme {
private:
    std::string name;
    std::string owner;
    std::string primaryKey;
    std::vector<TableRowScheme> rows;

public:
    TableScheme() = default;

    TableScheme(std::string name, std::string owner, std::string primaryKey): name(name), owner(owner), primaryKey(primaryKey) {
    }

    void addRow(const TableRowScheme& row) {
        this->rows.push_back(row);
    }

    std::string getName() {
        return this->name;
    }

    std::string getOwner() {
        return this->owner;
    }

    std::string getPrimaryKey() {
        return this->primaryKey;
    }

    std::vector<TableRowScheme> getRows() {
        return this->rows;
    }

    void setName(std::string name) {
        this->name = name;
    }

    void setOwner(std::string owner) {
        this->owner = owner;
    }

    void setPrimaryKey(std::string primaryKey) {
        this->primaryKey = primaryKey;
    }
};


#endif //DATABAZOVY_SYSTEM_TABLEROW_H
