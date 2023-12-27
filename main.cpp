#include <iostream>
#include "src/DBMS.h"

void vytvorTestovaciuTabulku(DBMS &dbms);

int main() {
    DBMS dbms;

    dbms.TEST_printState();

    std::cout << "Hello, World!" << std::endl;
    return 0;
}

void vytvorTestovaciuTabulku(DBMS& dbms) {
    auto tableScheme = new TableScheme("users2", "admin", "id");

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

