#include <iostream>
#include "src/DBMS.h"

void vytvorTestovaciuTabulku(DBMS &dbms);
void insert(DBMS& dbms);
void select(DBMS& dbms);
void vypisTabulky(DBMS& dbms);

int main() {
    DBMS dbms;

    dbms.TEST_printState();

    std::cout << "Hello, World!" << std::endl;
    return 0;
}

void vypisTabulky(DBMS& dbms) {
    std::cout << "Tabulky v databaze:" << std::endl;
    // vypis vsetky tabulky
    auto tables = dbms.getTablesList();
    for (auto table : tables) {
        std::cout << table << std::endl;
    }
}


void insert(DBMS& dbms) {
    std::map<std::string, std::string> newRecord;
    newRecord["id"] = "1";
    newRecord["first_name"] = "Dominik";
    newRecord["last_name"] = "Ježík";
    newRecord["date_of_birth"] = "2002-14-04";

    dbms.insertIntoTable("users", newRecord, "jezik");
}

void select(DBMS& dbms) {
    auto podmienka = Condition("id", "1", greater_than);

    auto results = dbms.selectFromTable("users", {"date_of_birth"}, { podmienka }, "first_name", true, "admin");
    //auto results = dbms.selectFromTable("users", {}, { podmienka }, "first_name", true, "admin");

    // vypis vysledky selectu v peknej tabulke s nazvami stlpcov, pricom kazdy stlpec ma rovnaku sirku
    for (int i = 0; i < results.size(); i++) {
        for (int j = 0; j < results[i].size(); j++) {
            std::cout << std::setw(20) << results[i][j];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void vytvorTestovaciuTabulku(DBMS& dbms) {
    auto tableScheme = new TableScheme("users", "admin", "id");

    auto row0 = new TableRowScheme("id", type_int, false);
    auto row1 = new TableRowScheme("first_name", type_string, false);
    auto row2 = new TableRowScheme("last_name", type_string, true);
    auto row3 = new TableRowScheme("date_of_birth", type_date, true);

    tableScheme->addRow(*row0);
    tableScheme->addRow(*row1);
    tableScheme->addRow(*row2);
    tableScheme->addRow(*row3);

    auto result = dbms.createTable(tableScheme);

    if (result) {
        std::cout << "Tabulka bola vytvorena" << std::endl;
    } else {
        std::cout << "Tabulka nebola vytvorena" << std::endl;
    }
}

