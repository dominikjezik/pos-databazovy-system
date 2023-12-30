#ifndef DATABAZOVY_SYSTEM_TABLEROW_H
#define DATABAZOVY_SYSTEM_TABLEROW_H


#include <string>
#include <vector>
#include <stdexcept>

class TableItem {
private:
    std::string name;
    std::string owner;
public:
    TableItem(std::string name, std::string owner): name(name), owner(owner) {
    }

    std::string getName() {
        return this->name;
    }

    std::string getOwner() {
        return this->owner;
    }
};

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

    TableRowScheme(std::string name, std::string dataType, bool nullable) {
        this->name = name;
        this->nullable = nullable;

        if (dataType == "string") {
            this->dataType = type_string;
        } else if (dataType == "int") {
            this->dataType = type_int;
        } else if (dataType == "double") {
            this->dataType = type_double;
        } else if (dataType == "boolean") {
            this->dataType = type_boolean;
        } else if (dataType == "date") {
            this->dataType = type_date;
        } else {
            throw std::invalid_argument("Neplatny datovy typ!");
        }
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

    void addRow(TableRowScheme newRow) {
        // Kontrola, ci sa v scheme nenachadza uz rovnaky stlpec
        for (auto row : this->rows) {
            if (row.getName() == newRow.getName()) {
                throw std::invalid_argument("Stlpec s nazvom \"" + row.getName() + "\" uz existuje!");
            }
        }

        this->rows.push_back(newRow);
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

enum ConditionOperation {
    equal,
    not_equal,
    greater_than,
    less_than,
    greater_than_or_equal,
    less_than_or_equal
};

class Condition {
private:
    std::string column;
    std::string value;
    ConditionOperation operation;
public:
    Condition(std::string column, std::string value, ConditionOperation operation): column(column), value(value), operation(operation) {
    }

    std::string getColumn() {
        return this->column;
    }

    std::string getValue() {
        return this->value;
    }

    ConditionOperation getOperation() {
        return this->operation;
    }

    static bool compareInt(int a, int b, ConditionOperation operation) {
        switch (operation) {
            case equal:
                return a == b;
            case not_equal:
                return a != b;
            case greater_than:
                return a > b;
            case less_than:
                return a < b;
            case greater_than_or_equal:
                return a >= b;
            case less_than_or_equal:
                return a <= b;
        }
    }

    static bool compareNull(std::string a, std::string b, ConditionOperation operation) {
        switch (operation) {
            case equal:
                return a == b;
            case not_equal:
                return a != b;
            case greater_than:
            case less_than:
            case greater_than_or_equal:
            case less_than_or_equal:
                throw std::invalid_argument("Nullable je mozne porovnat iba cez operaciu \"=\" alebo \"!=\"!");
        }
    }

    static bool compareDouble(double a, double b, ConditionOperation operation) {
        switch (operation) {
            case equal:
                return a == b;
            case not_equal:
                return a != b;
            case greater_than:
                return a > b;
            case less_than:
                return a < b;
            case greater_than_or_equal:
                return a >= b;
            case less_than_or_equal:
                return a <= b;
        }
    }

    static bool compareString(std::string a, std::string b, ConditionOperation operation) {
        switch (operation) {
            case equal:
                return a == b;
            case not_equal:
                return a != b;
            case greater_than:
            case less_than:
            case greater_than_or_equal:
            case less_than_or_equal:
                throw std::invalid_argument("Hodnota typu string moze byt porovnana iba cez operaciu \"=\" alebo \"!=\"!");
        }
    }

    static bool compareDate(std::string a, std::string b, ConditionOperation operation) {
        switch (operation) {
            case equal:
                return a == b;
            case not_equal:
                return a != b;
            case greater_than:
            case less_than:
            case greater_than_or_equal:
            case less_than_or_equal:
                throw std::invalid_argument("Hodnota typu date moze byt porovnana iba cez operaciu \"=\" alebo \"!=\"!");
        }
    }

    static bool compareBoolean(bool a, bool b, ConditionOperation operation) {
        switch (operation) {
            case equal:
                return a == b;
            case not_equal:
                return a != b;
            case greater_than:
            case less_than:
            case greater_than_or_equal:
            case less_than_or_equal:
                throw std::invalid_argument("Hodnota typu boolean moze byt porovnana iba cez operaciu \"=\" alebo \"!=\"!");
        }
    }


};

#endif //DATABAZOVY_SYSTEM_TABLEROW_H
