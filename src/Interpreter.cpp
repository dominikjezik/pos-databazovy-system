#include "Interpreter.h"

Interpreter::Interpreter() {
    this->dbms = new DBMS();
}

Interpreter::~Interpreter() {

}


void Interpreter::run(std::string command) {
    // TODO: vsetky vyskyty std::cout zmenit na vratenie stringu

    // Rozdel prikaz na slova
    std::vector<std::string> words;
    this->parseCommand(command, words);

    if (words.empty()) {
        return;
    }

    // Identifikuj prikaz
    std::string commandName = words[0];
    words.erase(words.begin());

    try {
        if (commandName == "show") {
            this->show(words);
        } else if (commandName == "create") {
            this->create(words);
        } else if (commandName == "drop") {
            this->drop(words);
        } else if (commandName == "select") {
            this->select(words);
        }  else if (commandName == "insert") {
            this->insert(words);
        } else if (commandName == "delete") {
            this->deleteCommand(words);
        } else {
            std::cout << "Neznamy prikaz!" << std::endl;
        }
    } catch (std::exception& e) {
        std::cout << "Chyba v prikaze! Detaily: " << e.what() << std::endl;
    }

}


void Interpreter::show(std::vector<std::string> words) {
    if (words.empty()) {
        std::cout << "Neznamy prikaz!" << std::endl;
        return;
    }

    if (words[0] == "tables") {
        if (words.size() == 1) {
            auto tables = this->dbms->getTablesList();
            std::cout << "Tabulky:" << std::endl;
            for (auto table : tables) {
                std::cout << table << std::endl;
            }
        } else if (words.size() == 3 && words[1] == "from") {
            if (!this->dbms->userExists(words[2])) {
                std::cout << "Pouzivatel " << words[2] << " neexistuje!" << std::endl;
                return;
            }

            auto table = this->dbms->getTablesListCreatedByUser(words[2]);
            std::cout << "Tabulky vytvorene pouzivatelom " << words[2] << ":" << std::endl;
            for (auto table : table) {
                std::cout << table << std::endl;
            }

        } else {
            std::cout << "Neznamy prikaz!" << std::endl;

        }
    } else if (words[0] == "users") {
        auto users = this->dbms->getUsersList();
        std::cout << "Pouzivatelia:" << std::endl;
        for (auto user : users) {
            std::cout << user << std::endl;
        }
    } else {
        std::cout << "Neznamy prikaz!" << std::endl;
    }



}


void Interpreter::create(std::vector<std::string> words) {
    if (words.empty()) {
        std::cout << "Neznamy prikaz!" << std::endl;
        return;
    }

    if (words[0] == "user") {
        words.erase(words.begin());
        this->createUser(words);
    } else if (words[0] == "table") {
        words.erase(words.begin());
        this->createTable(words);
    } else {
        std::cout << "Neznamy prikaz!" << std::endl;
    }

}


// CREATE USER username IDENTIFIED BY password
void Interpreter::createUser(std::vector<std::string> words) {
    if (words.size() != 4) {
        std::cout << "Neznamy prikaz!" << std::endl;
        return;
    }

    if (words[1] != "identified" || words[2] != "by") {
        std::cout << "Neznamy prikaz!" << std::endl;
        return;
    }

    if (this->dbms->userExists(words[0])) {
        std::cout << "Pouzivatel " << words[0] << " uz existuje!" << std::endl;
        return;
    }

    this->dbms->createUser(words[0], words[3]);

    std::cout << "Pouzivatel " << words[1] << " bol vytvoreny!" << std::endl;
}


// create table NAZOV (stlpec1 typ1, stlpec2 typ2, stlpec3 typ3 not null, stlpec4 typ4, stlpec5 typ5 primary key)
void Interpreter::createTable(std::vector<std::string> words) {

    // Ziskaj nazov tabulky
    if (words.empty()) {
        std::cout << "Chyba v prikaze!" << std::endl;
        return;
    }

    std::string tableName = words[0];
    words.erase(words.begin());

    // Kontrola ci zacina zatvorkou
    if (words[0].size() == 0 || words[0][0] != '(') {
        std::cout << "Chyba v prikaze!" << std::endl;
        return;
    }

    // Odstran zatvorku
    words[0].erase(words[0].begin());

    if (words[0].size() == 0) {
        words.erase(words.begin());
    }

    if (words[0].size() == 0) {
        std::cout << "Chyba v prikaze!" << std::endl;
        return;
    }

    std::vector<TableRowScheme> rows;
    std::string primaryKeyColumn = "";

    // Ziskaj stlpce
    while (words[0] != ")") {

        // Musia by minimalne dve slova: nazov_stlpca a typ
        if (words.size() < 2) {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

        // Ak prve slovo konci ciarkou tak vyhod chybu
        if (words[0].back() == ',') {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

        std::string columnName = words[0];
        words.erase(words.begin());

        std::string columnType;
        bool notNull = false;
        bool primaryKey = false;

        bool koniec = false;

        // Ak druhe slovo konci ciarkou tak ju odstran
        if (words[0].back() == ',' || words[0].back() == ')') {
            if (words[0].back() == ')') {
                koniec = true;
            }

            words[0].pop_back();
            columnType = words[0];
            words.erase(words.begin());

        } else {
            columnType = words[0];
            words.erase(words.begin());

            // Ak nekonci ciarkou tak musi byt slovo "not" a "null" alebo "primary" a "key" alebo ")"
            if (words[0] != "not" && words[0] != "primary" && words[0] != ")") {
                std::cout << "Chyba v prikaze!" << std::endl;
                return;
            }

            if (words[0] == "not") {
                words.erase(words.begin());

                // ak obsahuje na konci ciarku alebo zatvorku tak ju odstran
                if (words[0].back() == ',' || words[0].back() == ')') {
                    if (words[0].back() == ')') {
                        koniec = true;
                    }

                    words[0].pop_back();
                }

                if (words[0] != "null") {
                    std::cout << "Chyba v prikaze!" << std::endl;
                    return;
                }

                words.erase(words.begin());
                notNull = true;
            } else if (words[0] == "primary") {
                words.erase(words.begin());

                // ak obsahuje na konci ciarku alebo zatvorku tak ju odstran
                if (words[0].back() == ',' || words[0].back() == ')') {
                    if (words[0].back() == ')') {
                        koniec = true;
                    }

                    words[0].pop_back();
                }

                if (words[0] != "key") {
                    std::cout << "Chyba v prikaze!" << std::endl;
                    return;
                }

                words.erase(words.begin());
                primaryKey = true;
            }
        }

        bool nullable = !notNull;

        if (primaryKey) {
            nullable = false;
        }

        auto row = TableRowScheme(columnName, columnType, nullable);

        rows.push_back(row);

        if (primaryKey) {
            if (primaryKeyColumn.empty()) {
                primaryKeyColumn = columnName;
            } else {
                std::cout << "Tabulka nemoze mat viac ako jeden primarny kluc!" << std::endl;
                return;
            }
        }

        if (koniec) {
            break;
        }
    }

    // ak je posledne slovo zatvorka tak ho odstran
    if (words[0].back() == ')') {
        words.erase(words.begin());
    }

    // ak este nieco ostalo tak vyhod chybu
    if (!words.empty()) {
        std::cout << "Chyba v prikaze!" << std::endl;
        return;
    }

    if (primaryKeyColumn.empty()) {
        std::cout << "Tabulka musi mat primarny kluc!" << std::endl;
        return;
    }

    // TODO: je tu hardcode pouzivatel
    auto tableScheme = TableScheme(tableName, "admin", primaryKeyColumn);

    for (auto row : rows) {
        tableScheme.addRow(row);
    }

    this->dbms->createTable(tableScheme);

}


void Interpreter::drop(std::vector<std::string> words) {
    if (words.empty()) {
        std::cout << "Neznamy prikaz!" << std::endl;
        return;
    }

    if (words[0] == "table") {
        words.erase(words.begin());

        if (words.empty()) {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

        std::string tableName = words[0];
        words.erase(words.begin());

        if (!words.empty()) {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

        // TODO: je tu hardcode pouzivatel
        dbms->dropTable(tableName, "admin");
    }
}


void Interpreter::select(std::vector<std::string> words) {
    if (words.empty()) {
        std::cout << "Chyba v prikaze!" << std::endl;
        return;
    }

    std::string tableName;
    std::vector<std::string> columns;
    std::vector<Condition> conditions;
    std::string orderColumn;
    bool ascending = true;

    // TODO: odstranit hardcode pouzivatela
    std::string currentUser = "admin";

    // ak nevyberam vsetky stlpce, ale iba konkretne
    if (words[0] != "*") {
        // ak je prikaz v tvare "select from" tak vyhoď chybu
        if (words[0] == "from") {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

        while (words[0] != "from") {
            // kontrola ci v slove je ciarka, ak hej tak odstran
            if (words[0].back() == ',') {
                words[0].pop_back();
            }

            columns.push_back(words[0]);
            words.erase(words.begin());

            if (words.empty()) {
                std::cout << "Chyba v prikaze!" << std::endl;
                return;
            }
        }
    } else {
        words.erase(words.begin());
    }

    // ak prikaz nepokracuje slovom "from" tak vyhoď chybu
    if (words.empty() || words[0] != "from") {
        std::cout << "Chyba v prikaze!" << std::endl;
        return;
    }

    words.erase(words.begin());

    // ak za slovom "from" nie je meno tabulky tak vyhoď chybu
    if (words.empty()) {
        std::cout << "Chyba v prikaze!" << std::endl;
        return;
    }

    tableName = words[0];
    words.erase(words.begin());

    // ak prikaz pokracuje slovom "where"
    if (!words.empty() && words[0] == "where") {
        words.erase(words.begin());

        // ak za slovom "where" nie je podmienka tak vyhoď chybu
        if (words.empty()) {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

        // Spracuj podmienky, ktoré sú oddelené slovom "and"
        while (words[0] != "order" || words.empty()) {

            // Kontrola ci nasledujuce slova su aspo 3 (stlpec operator hodnota)
            if (words.size() < 3) {
                std::cout << "Chyba v prikaze!" << std::endl;
                return;
            }

            std::string column = words[0];
            words.erase(words.begin());

            std::string op = words[0];
            words.erase(words.begin());

            std::string value = words[0];
            words.erase(words.begin());

            // TODO: refactor do funkcie
            // konvertovanie operatora na enum
            ConditionOperation operation;
            if (op == "=") {
                operation = equal;
            } else if (op == "!=") {
                operation = not_equal;
            } else if (op == ">") {
                operation = greater_than;
            } else if (op == "<") {
                operation = less_than;
            } else if (op == ">=") {
                operation = greater_than_or_equal;
            } else if (op == "<=") {
                operation = less_than_or_equal;
            } else {
                std::cout << "Chyba v prikaze!" << std::endl;
                return;
            }

            conditions.push_back(Condition(column, value, operation));

            if (words.empty()) {
                break;
            }

            if (words[0] == "and") {
                words.erase(words.begin());
            } else if (words[0] == "order") {
                break;
            } else {
                std::cout << "Chyba v prikaze!" << std::endl;
                return;
            }

        }
    }

    // ak prikaz pokracuje slovom "order"
    if (!words.empty() && words[0] == "order") {
        words.erase(words.begin());

        // ak za slovom "order" nenasleduje slovo "by" tak vyhoď chybu
        if (words.empty() || words[0] != "by") {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

        words.erase(words.begin());

        // ak za slovom "by" nie je stlpec tak vyhoď chybu
        if (words.empty()) {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

        orderColumn = words[0];
        words.erase(words.begin());

        // slovo asc desc je nepovinne
        if (!words.empty()) {
            if (words[0] == "asc") {
                words.erase(words.begin());
            } else if (words[0] == "desc") {
                words.erase(words.begin());
                ascending = false;
            } else {
                std::cout << "Chyba v prikaze!" << std::endl;
                return;
            }
        }
    }

    // ak sa tu este nachadzaju nejake slova tak vyhoď chybu
    if (!words.empty()) {
        std::cout << "Chyba v prikaze!" << std::endl;
        return;
    }


    // ziskaj vysledky
    auto result = this->dbms->selectFromTable(tableName, columns, conditions, orderColumn, ascending, currentUser);
    for (auto row : result) {
        for (auto item : row) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }

}


void Interpreter::insert(std::vector<std::string> words) {
    if (words.empty()) {
        std::cout << "Neznamy prikaz!" << std::endl;
        return;
    }

    // INSERT INTO table_name (column1 column2 column3 ...) VALUES (value1 value2 value3 ...);
    if (words[0] == "into") {
        words.erase(words.begin());

        if (words.size() < 4) {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

        std::string tableName = words[0];
        words.erase(words.begin());

        // Kontrola ci zacina zatvorkou
        if (words[0].size() == 0 || words[0][0] != '(') {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

        // Odstran zatvorku
        words[0].erase(words[0].begin());

        if (words[0].size() == 0) {
            words.erase(words.begin());
        }

        if (words[0].size() == 0) {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

        std::vector<std::string> columns;

        // Ziskaj nazvy stlpcov
        while (words[0] != ")") {
            // Ak slovo konci zatvorkou tak ju odstran
            if (words[0].back() == ')') {
                words[0].pop_back();
                columns.push_back(words[0]);
                break;
            }

            columns.push_back(words[0]);
            words.erase(words.begin());

            if (words.empty()) {
                std::cout << "Chyba v prikaze!" << std::endl;
                return;
            }
        }

        // Odstanenie posledneho slova alebo zatvorky
        words.erase(words.begin());

        if (words[0] != "values") {
            std::cout << "Chyba v prikaze" << std::endl;
            return;
        }

        // Odstranenie slova values
        words.erase(words.begin());

        // TODO kontrola si ma slovo dlzku viac ako 0
        // Kontrola ci zacina zatvorkou
        if (words[0].size() == 0 || words[0][0] != '(') {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

        // Odstran zatvorku
        words[0].erase(words[0].begin());

        if (words[0].size() == 0) {
            words.erase(words.begin());
        }

        /*//vypnute kvoli vkladaniu null hodnot
        if (words[0].size() == 0) {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }
         */

        std::map<std::string, std::string> values;

        // Ziskaj hodnoty
        while (words[0] != ")") {
            // Ak je viac hodnot ako stlpcov tak vyhod chybu
            if (columns.empty()) {
                std::cout << "Chyba v prikaze!" << std::endl;
                return;
            }

            // Ak slovo konci zatvorkou tak ju odstran
            if (words[0].back() == ')') {
                words[0].pop_back();

                auto column = columns[0];
                columns.erase(columns.begin());
                values[column] = words[0];

                break;
            }

            auto column = columns[0];
            columns.erase(columns.begin());
            values[column] = words[0];

            words.erase(words.begin());

            if (words.empty()) {
                std::cout << "Chyba v prikaze!" << std::endl;
                return;
            }
        }

        // Odstanenie posledneho slova alebo zatvorky
        words.erase(words.begin());

        if (!columns.empty()) {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

        // TODO: je tu hardcode pouzivatel
        dbms->insertIntoTable(tableName, values, "admin");
    }

    // TODO: ak nie je iny insert tak obrat if

}


void Interpreter::deleteCommand(std::vector<std::string> words) {
    if (words.empty()) {
        std::cout << "Neznamy prikaz!" << std::endl;
        return;
    }

    // DELETE FROM table_name WHERE column_name = value
    if (words[0] == "from") {
        words.erase(words.begin());

        if (words.empty()) {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

        std::string tableName = words[0];
        words.erase(words.begin());

        std::vector<Condition> conditions;

        if (words.size() != 0) {
            if (words[0] != "where") {
                std::cout << "Chyba v prikaze!" << std::endl;
                return;
            }

            words.erase(words.begin());
        }

        // Spracuj podmienky, ktoré sú oddelené slovom "and"
        while (!words.empty()) {

            // Kontrola ci nasledujuce slova su aspon 3 (stlpec operator hodnota)
            if (words.size() < 3) {
                std::cout << "Chyba v prikaze!" << std::endl;
                return;
            }

            std::string column = words[0];
            words.erase(words.begin());

            std::string op = words[0];
            words.erase(words.begin());

            std::string value = words[0];
            words.erase(words.begin());

            // TODO: refactor do funkcie
            // konvertovanie operatora na enum
            ConditionOperation operation;
            if (op == "=") {
                operation = equal;
            } else if (op == "!=") {
                operation = not_equal;
            } else if (op == ">") {
                operation = greater_than;
            } else if (op == "<") {
                operation = less_than;
            } else if (op == ">=") {
                operation = greater_than_or_equal;
            } else if (op == "<=") {
                operation = less_than_or_equal;
            } else {
                std::cout << "Chyba v prikaze!" << std::endl;
                return;
            }

            conditions.emplace_back(column, value, operation);

            if (words.empty()) {
                break;
            }

            if (words[0] == "and") {
                words.erase(words.begin());
            } else {
                std::cout << "Chyba v prikaze!" << std::endl;
                return;
            }

        }

        // TODO: je tu hardcode pouzivatel
        int countOfDeleted = this->dbms->deleteFromTable(tableName, conditions, "admin");

        std::cout << "Pocet zmazanych riadkov: " << countOfDeleted << std::endl;

    }

    // TODO: ak nie je iny delete tak obrat if
}


/**
 * Rozdeli prikaz na slova, ktore su oddelene medzerou ale pri tomto rozdeleni ignoruje medzery v uvodzovkach
 *
 * @param command
 * @param words
 */
void Interpreter::parseCommand(std::string command, std::vector<std::string> &words) {
    bool uvodzovky = false;

    int indexKoncaPoslednehoSlova = 0;

    for (int i = 0; i < command.length(); i++) {
        auto aktualnyZnak = command[i];

        if (aktualnyZnak == '"' && !uvodzovky) {
            uvodzovky = true;
            auto slovo = command.substr(indexKoncaPoslednehoSlova, i - indexKoncaPoslednehoSlova);
            if (!slovo.empty()) {
                words.push_back(slovo);
            }
            indexKoncaPoslednehoSlova = i + 1;
            continue;
        }

        if (aktualnyZnak == ' ' && !uvodzovky) {
            auto slovo = command.substr(indexKoncaPoslednehoSlova, i - indexKoncaPoslednehoSlova);
            if (!slovo.empty()) {
                words.push_back(slovo);
            }
            indexKoncaPoslednehoSlova = i + 1;
        }

        if (aktualnyZnak == '"') {
            uvodzovky = false;

            auto slovo = command.substr(indexKoncaPoslednehoSlova, i - indexKoncaPoslednehoSlova);
            words.push_back(slovo);
            indexKoncaPoslednehoSlova = i + 1;
        }
    }

    // pridaj posledne slovo
    auto slovo = command.substr(indexKoncaPoslednehoSlova, command.length() - indexKoncaPoslednehoSlova);
    if (!slovo.empty()) {
        words.push_back(slovo);
    }
}


