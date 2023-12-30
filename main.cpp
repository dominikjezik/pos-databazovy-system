#include <iostream>
#include <iomanip>
#include "src/DBMS.h"
#include "src/Interpreter.h"

void vypisLogo();
void vytvorTestovaciuTabulkuPostov(DBMS &dbms);
void naplnTestovaciuTabulkuPostov(Interpreter& interpreter);
void vytvorTestovaciuTabulkuDatovychTypov(DBMS& dbms);
void vytvorTestovaciuTabulkuJednoduchu(DBMS& dbms);
void vytvorTestovaciuTabulkuIntov(DBMS& dbms);
void insert(DBMS& dbms);
void select(DBMS& dbms);
void vypisTabulky(DBMS& dbms);

int main() {
    vypisLogo();

    Interpreter interpreter;

    std::string command;

    // nacitanie cez cin
    while (true) {
        std::cout << "DB> ";
        std::getline(std::cin, command);

        if (command == "exit") {
            break;
        }

        interpreter.run(command);
        std::cout << std::endl;
    }


    return 0;

    DBMS dbms;

    vytvorTestovaciuTabulkuIntov(dbms);

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
    newRecord["id"] = "6";
    newRecord["first_name"] = "Nejake meno";
    newRecord["last_name"] = "Test";
    newRecord["date_of_birth"] = "2004-03-03";

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

void vytvorTestovaciuTabulkuPostov(DBMS& dbms) {
    auto tableScheme = new TableScheme("posts", "admin", "id");

    auto row0 = new TableRowScheme("id", type_int, false);
    auto row1 = new TableRowScheme("title", type_string, false);
    auto row2 = new TableRowScheme("content", type_string, true);
    auto row3 = new TableRowScheme("date", type_date, true);

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

void vytvorTestovaciuTabulkuJednoduchu(DBMS& dbms) {
    auto tableScheme = new TableScheme("sample", "admin", "key");

    auto row0 = new TableRowScheme("key", type_string, false);

    tableScheme->addRow(*row0);

    auto result = dbms.createTable(tableScheme);

    if (result) {
        std::cout << "Tabulka bola vytvorena" << std::endl;
    } else {
        std::cout << "Tabulka nebola vytvorena" << std::endl;
    }
}

void vytvorTestovaciuTabulkuIntov(DBMS& dbms) {
    auto tableScheme = new TableScheme("int", "admin", "key");

    auto row0 = new TableRowScheme("key", type_int, false);
    auto row1 = new TableRowScheme("int", type_int, true);

    tableScheme->addRow(*row0);
    tableScheme->addRow(*row1);

    auto result = dbms.createTable(tableScheme);

    if (result) {
        std::cout << "Tabulka bola vytvorena" << std::endl;
    } else {
        std::cout << "Tabulka nebola vytvorena" << std::endl;
    }
}

void vytvorTestovaciuTabulkuDatovychTypov(DBMS& dbms) {
    auto tableScheme = new TableScheme("data", "admin", "int");

    auto row0 = new TableRowScheme("int", type_int, false);
    auto row1 = new TableRowScheme("string", type_string, true);
    auto row2 = new TableRowScheme("double", type_double, true);
    auto row3 = new TableRowScheme("boolean", type_boolean, true);
    auto row4 = new TableRowScheme("date", type_date, true);

    tableScheme->addRow(*row0);
    tableScheme->addRow(*row1);
    tableScheme->addRow(*row2);
    tableScheme->addRow(*row3);
    tableScheme->addRow(*row4);

    auto result = dbms.createTable(tableScheme);

    if (result) {
        std::cout << "Tabulka bola vytvorena" << std::endl;
    } else {
        std::cout << "Tabulka nebola vytvorena" << std::endl;
    }
}

void naplnTestovaciuTabulkuPostov(Interpreter& interpreter) {
    interpreter.run("insert into posts (id title content date) values (1 \"Prvy post\" \"Toto je obsah prveho postu\" \"2020-03-03\")");
    interpreter.run("insert into posts (id title content date) values (2 \"Druhy post\" \"Toto je obsah druheho postu\" \"2020-03-04\")");
    interpreter.run("insert into posts (id title content date) values (3 \"Treti post\" \"Toto je obsah tretieho postu\" \"2020-03-05\")");
    interpreter.run("insert into posts (id title content date) values (4 \"Stvrty post\" \"Toto je obsah stvrteho postu\" \"2020-03-06\")");
    interpreter.run("insert into posts (id title content date) values (5 \"Piaty post\" \"Toto je obsah piateho postu\" \"2020-03-07\")");
}

void vypisLogo() {
    std::cout << std::endl;
    std::cout << "         ______________       ____        _        _                               " << std::endl;
    std::cout << "        /             /|     |  _ \\  __ _| |_ __ _| |__   __ _ _________   ___   _ " << std::endl;
    std::cout << "       /             / |     | | | |/ _` | __/ _` | '_ \\ / _` |_  / _ \\ \\ / / | | |" << std::endl;
    std::cout << "      /____________ /  |     | |_| | (_| | || (_| | |_) | (_| |/ / (_) \\ V /| |_| |" << std::endl;
    std::cout << "     | ___________ |   |     |____/ \\__,_|\\__\\__,_|_.__/ \\__,_/___\\___/ \\_/  \\__, |" << std::endl;
    std::cout << "     ||           ||   |                                                     |___/ " << std::endl;
    std::cout << "     ||           ||   |      ____            _                                    " << std::endl;
    std::cout << "     ||           ||   |     / ___| _   _ ___| |_ ___ _ __ ___                     " << std::endl;
    std::cout << "     ||___________||   |     \\___ \\| | | / __| __/ _ \\ '_ ` _ \\                    " << std::endl;
    std::cout << "     |   _______   |  /       ___) | |_| \\__ \\ ||  __/ | | | | |                   " << std::endl;
    std::cout << "     |  (_______)  | /       |____/ \\__, |___/\\__\\___|_| |_| |_|                   " << std::endl;
    std::cout << "     |_____________|/               |___/                                          " << std::endl;
    std::cout << std::endl;
}