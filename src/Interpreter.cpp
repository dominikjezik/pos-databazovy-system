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
        } else if (commandName == "select") {
            this->select(words);
        }  else if (commandName == "insert") {
            this->insert(words);
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

            // TODO: ak obsahuje uvodzovky tak ich odstran a spoj do jedneho stringu

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


void Interpreter::create(std::vector<std::string> words) {
    if (words.empty()) {
        std::cout << "Neznamy prikaz!" << std::endl;
        return;
    }

    // CREATE USER username IDENTIFIED BY password
    if (words[0] == "user") {
        words.erase(words.begin());

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
    } else {
        std::cout << "Neznamy prikaz!" << std::endl;
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

        if (words[0].size() == 0) {
            std::cout << "Chyba v prikaze!" << std::endl;
            return;
        }

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








