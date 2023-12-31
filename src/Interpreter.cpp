#include "Interpreter.h"

Interpreter::Interpreter() {
    this->dbms = new DBMS();
}

Interpreter::~Interpreter() {
    delete this->dbms;
}


void Interpreter::run(std::string command) {
    // TODO: Predpokladam, ze aj run bude vraciat string, ktory sa bude posielat klientovi

    // Rozdel prikaz na slova
    std::vector<std::string> words;
    this->parseCommand(command, words);

    if (words.empty()) {
        return;
    }

    // Identifikuj prikaz
    std::string commandName = words[0];
    words.erase(words.begin());

    // Predpokladame ze vsetky prikazy maju aspon dve slova
    if (words.empty()) {
        // TODO: odstranit priamy vypis na konzolu
        std::cout << "Neznamy prikaz!" << std::endl;
        return;
    }

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
        } else if (commandName == "update") {
            this->update(words);
        } else if (commandName == "delete") {
            this->deleteCommand(words);
        } else {
            // TODO: odstranit priamy vypis na konzolu
            std::cout << "Neznamy prikaz!" << std::endl;
        }
    } catch (std::exception& e) {
        // TODO: odstranit priamy vypis na konzolu
        std::cout << "Chyba v prikaze! Detaily: " << e.what() << std::endl;
    }

}


std::string Interpreter::show(std::vector<std::string>& words) {
    // TODO: nahradenie priameho vypisu na konzolu vracanim stringu pre klienta

    if (words[0] == "tables") {
        if (words.size() == 1) {
            auto tables = this->dbms->getTablesList();
            std::cout << "Tabulky:" << std::endl;
            for (auto table : tables) {
                std::cout << table << std::endl;
            }
        } else if (words.size() == 3 && words[1] == "from") {
            if (!this->dbms->userExists(words[2])) {
                return error("Pouzivatel " + words[2] + " neexistuje!");
            }

            auto table = this->dbms->getTablesListCreatedByUser(words[2]);
            std::cout << "Tabulky vytvorene pouzivatelom " << words[2] << ":" << std::endl;
            for (auto table : table) {
                std::cout << table << std::endl;
            }

        } else {
            return errorUnknownCommand();
        }
    } else if (words[0] == "users") {
        auto users = this->dbms->getUsersList();
        std::cout << "Pouzivatelia:" << std::endl;
        for (auto user : users) {
            std::cout << user << std::endl;
        }
    } else {
        return errorUnknownCommand();
    }
}


std::string Interpreter::create(std::vector<std::string>& words) {
    if (words[0] == "user") {
        words.erase(words.begin());
        return this->createUser(words);
    } else if (words[0] == "table") {
        words.erase(words.begin());
        return this->createTable(words);
    } else {
        return errorUnknownCommand();
    }
}


// CREATE USER username IDENTIFIED BY password
std::string Interpreter::createUser(std::vector<std::string>& words) {
    if (words.size() != 4) {
        return errorUnknownCommand();
    }

    if (words[1] != "identified" || words[2] != "by") {
        return error();
    }

    if (this->dbms->userExists(words[0])) {
        return error("Pouzivatel " + words[0] + " uz existuje!");
    }

    this->dbms->createUser(words[0], words[3]);

    return success("Pouzivatel " + words[0] + " bol vytvoreny!");
}


// create table NAZOV (stlpec1 typ1, stlpec2 typ2, stlpec3 typ3 not null, stlpec4 typ4, stlpec5 typ5 primary key)
std::string Interpreter::createTable(std::vector<std::string>& words) {
    if (words.empty()) {
        return error();
    }

    // Ziskaj nazov tabulky
    std::string tableName = words[0];
    words.erase(words.begin());

    // Kontrola ci zacina zatvorkou
    if (words[0].size() == 0 || words[0][0] != '(') {
        return error();
    }

    // Odstran zatvorku
    words[0].erase(words[0].begin());

    if (words[0].size() == 0) {
        words.erase(words.begin());
    }

    if (words[0].size() == 0) {
        return error();
    }

    std::vector<TableRowScheme> rows;
    std::string primaryKeyColumn = "";

    // Ziskaj stlpce
    while (words[0] != ")") {

        // Musia by minimalne dve slova: nazov_stlpca a typ
        if (words.size() < 2) {
            return error();
        }

        // Ak prve slovo konci ciarkou tak vyhod chybu
        if (words[0].back() == ',') {
            return error();
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
                return error();
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
                    return error();
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
                    return error();
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
                return error("Tabulka nemoze mat viac ako jeden primarny kluc!");
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
        return error();
    }

    if (primaryKeyColumn.empty()) {
        return error("Tabulka musi mat primarny kluc!");
    }

    // TODO: je tu hardcoded pouzivatel
    auto tableScheme = TableScheme(tableName, "admin", primaryKeyColumn);

    for (auto row : rows) {
        tableScheme.addRow(row);
    }

    this->dbms->createTable(tableScheme);

    return success("Tabulka " + tableName + " bola vytvorena!");
}


std::string Interpreter::drop(std::vector<std::string>& words) {
    if (words[0] != "table") {
        return errorUnknownCommand();
    }

    words.erase(words.begin());

    if (words.empty()) {
        return error();
    }

    std::string tableName = words[0];
    words.erase(words.begin());

    if (!words.empty()) {
        return error();
    }

    // TODO: je tu hardcoded pouzivatel
    dbms->dropTable(tableName, "admin");

    return success("Tabulka " + tableName + " bola zmazana!");
}


std::string Interpreter::select(std::vector<std::string>& words) {
    std::string tableName;
    std::vector<std::string> columns;
    std::vector<Condition> conditions;
    std::string orderColumn;
    bool ascending = true;

    // TODO: je tu hardcoded pouzivatel
    std::string currentUser = "admin";

    // ak nevyberam vsetky stlpce, ale iba konkretne
    if (words[0] != "*") {
        // ak je prikaz v tvare "select from" tak vyhoď chybu
        if (words[0] == "from") {
            return error();
        }

        while (words[0] != "from") {
            // kontrola ci v slove je ciarka, ak hej tak odstran
            if (words[0].back() == ',') {
                words[0].pop_back();
            }

            columns.push_back(words[0]);
            words.erase(words.begin());

            if (words.empty()) {
                return error();
            }
        }
    } else {
        words.erase(words.begin());
    }

    // ak prikaz nepokracuje slovom "from" tak vyhoď chybu
    if (words.empty() || words[0] != "from") {
        return error();
    }

    words.erase(words.begin());

    // ak za slovom "from" nie je meno tabulky tak vyhoď chybu
    if (words.empty()) {
        return error();
    }

    tableName = words[0];
    words.erase(words.begin());

    // ak prikaz pokracuje slovom "where"
    if (!words.empty() && words[0] == "where") {
        words.erase(words.begin());

        // ak za slovom "where" nie je podmienka tak vyhoď chybu
        if (words.empty()) {
            return error();
        }

        // Spracuj podmienky, ktoré sú oddelené slovom "and"
        while (words[0] != "order" || !words.empty()) {

            // Kontrola ci nasledujuce slova su aspo 3 (stlpec operator hodnota)
            if (words.size() < 3) {
                return error();
            }

            std::string column = words[0];
            words.erase(words.begin());

            std::string op = words[0];
            words.erase(words.begin());

            std::string value = words[0];
            words.erase(words.begin());

            // Konvertovanie operatora na enum, ak je neznamy tak vyhod chybu
            ConditionOperation operation = Condition::getOperation(op);

            conditions.push_back(Condition(column, value, operation));

            if (words.empty()) {
                break;
            }

            if (words[0] == "and") {
                words.erase(words.begin());
            } else if (words[0] == "order") {
                break;
            } else {
                return error();
            }

        }
    }

    // ak prikaz pokracuje slovom "order"
    if (!words.empty() && words[0] == "order") {
        words.erase(words.begin());

        // ak za slovom "order" nenasleduje slovo "by" tak vyhoď chybu
        if (words.empty() || words[0] != "by") {
            return error();
        }

        words.erase(words.begin());

        // ak za slovom "by" nie je stlpec tak vyhoď chybu
        if (words.empty()) {
            return error();
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
                return error();
            }
        }
    }

    // ak sa tu este nachadzaju nejake slova tak vyhoď chybu
    if (!words.empty()) {
        return error();
    }

    // ziskaj vysledky
    auto result = this->dbms->selectFromTable(tableName, columns, conditions, orderColumn, ascending, currentUser);

    return success(result);
}


// INSERT INTO table_name (column1 column2 column3 ...) VALUES (value1 value2 value3 ...);
std::string Interpreter::insert(std::vector<std::string>& words) {
    if (words[0] != "into") {
        return errorUnknownCommand();
    }

    words.erase(words.begin());

    if (words.size() < 4) {
        return error();
    }

    std::string tableName = words[0];
    words.erase(words.begin());

    // Kontrola ci zacina zatvorkou
    if (words[0].size() == 0 || words[0][0] != '(') {
        return error();
    }

    // Odstran zatvorku
    words[0].erase(words[0].begin());

    if (words[0].size() == 0) {
        words.erase(words.begin());
    }

    if (words[0].size() == 0) {
        return error();
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
            return error();
        }
    }

    // Odstanenie posledneho slova alebo zatvorky
    words.erase(words.begin());

    if (words[0] != "values") {
        return error();
    }

    // Odstranenie slova values
    words.erase(words.begin());

    // Kontrola ci zacina zatvorkou
    if (words[0].size() == 0 || words[0][0] != '(') {
        return error();
    }

    // Odstran zatvorku
    words[0].erase(words[0].begin());

    if (words[0].size() == 0) {
        words.erase(words.begin());
    }

    std::map<std::string, std::string> values;

    // Ziskaj hodnoty
    while (words[0] != ")") {
        // Ak je viac hodnot ako stlpcov tak vyhod chybu
        if (columns.empty()) {
            return error();
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
            return error();
        }
    }

    // Odstanenie posledneho slova alebo zatvorky
    words.erase(words.begin());

    if (!columns.empty()) {
        return error();
    }

    // TODO: je tu hardcoded pouzivatel
    dbms->insertIntoTable(tableName, values, "admin");

    return success("Zaznam bol vlozeny!");
}


std::string Interpreter::update(std::vector<std::string> &words) {
    std::string tableName = words[0];
    words.erase(words.begin());

    if (words.empty() || words[0] != "set") {
        return error();
    }

    words.erase(words.begin());

    if (words.empty()) {
        return error();
    }

    std::map<std::string, std::string> valuesForUpdate;

    while (!words.empty() && words[0] != "where") {
        // Kontrola ci nasledujuce slova su aspon 3 (stlpec = hodnota)
        if (words.size() < 3) {
            return error();
        }

        std::string column = words[0];
        words.erase(words.begin());

        std::string op = words[0];
        words.erase(words.begin());

        std::string value = words[0];
        words.erase(words.begin());

        if (op != "=") {
            return error();
        }

        // Ak je nasledujseuce slovo ciarka tak ho odstran
        if (!words.empty() && words[0] == ",") {
            words.erase(words.begin());

            // Ak slovo konci ciarkou tak ju odstran, iba ak je slovo dlhšie ako 1 znak (ak je to len ciarka tak predpokladame ze je to hodnota)
        } else if (value.length() > 1 && value.back() == ',') {
            // ciarku neodstrani ak sme na konci
            if (!words.empty() && words[0] != "where") {
                value.pop_back();
            }
        }

        valuesForUpdate[column] = value;
    }

    // Spracovanie podmienok WHERE
    std::vector<Condition> conditions;
    auto error = this->parseWhereConditions(words, conditions);

    // Ak nastala chyba pri spracovani podmienok tak ju vrat
    if (!error.empty()) {
        return error;
    }

    // TODO: je tu hardcoded pouzivatel
    size_t countOfUpdated = this->dbms->updateTable(tableName, valuesForUpdate, conditions, "admin");

    return success("Pocet aktualizovanych riadkov: " + std::to_string(countOfUpdated));
}


// DELETE FROM table_name WHERE column_name = value
std::string Interpreter::deleteCommand(std::vector<std::string>& words) {
    if (words[0] != "from") {
        return errorUnknownCommand();
    }

    words.erase(words.begin());

    if (words.empty()) {
        return error();
    }

    std::string tableName = words[0];
    words.erase(words.begin());

    // Spracovanie podmienok WHERE
    std::vector<Condition> conditions;
    auto error = this->parseWhereConditions(words, conditions);

    // Ak nastala chyba pri spracovani podmienok tak ju vrat
    if (!error.empty()) {
        return error;
    }

    // TODO: je tu hardcoded pouzivatel
    size_t countOfDeleted = this->dbms->deleteFromTable(tableName, conditions, "admin");

    return success("Pocet zmazanych riadkov: " + std::to_string(countOfDeleted));
}


/**
 * Rozdeli prikaz na slova, ktore su oddelene medzerou ale pri tomto rozdeleni ignoruje medzery v uvodzovkach
 *
 * @param command
 * @param words
 */
void Interpreter::parseCommand(std::string command, std::vector<std::string>& words) {
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


std::string Interpreter::parseWhereConditions(std::vector<std::string>& words, std::vector<Condition>& conditions) {
    // Odstranenie slova where
    if (!words.empty()) {
        if (words[0] != "where") {
            return error();
        }

        words.erase(words.begin());
    }

    // Za slovom where musi nasledovat aspon jedna podmienka
    if (words.empty()) {
        return error();
    }

    // Spracuj podmienky, ktoré sú oddelené slovom "and"
    while (!words.empty()) {

        // Kontrola ci nasledujuce slova su aspon 3 (stlpec operator hodnota)
        if (words.size() < 3) {
            return error();
        }

        std::string column = words[0];
        words.erase(words.begin());

        std::string op = words[0];
        words.erase(words.begin());

        std::string value = words[0];
        words.erase(words.begin());

        // Konvertovanie operatora na enum, ak je neznamy tak vyhod chybu
        ConditionOperation operation = Condition::getOperation(op);

        conditions.emplace_back(column, value, operation);

        if (words.empty()) {
            break;
        }

        if (words[0] == "and") {
            words.erase(words.begin());
        } else {
            return error();
        }
    }

    return "";
}


std::string Interpreter::success(std::string message) {
    // TODO: odstranit priamy vypis na konzolu
    std::cout << message << std::endl;

    // TODO: vratit spravu vo formate pre klienta
    return "";
}


std::string Interpreter::success(std::vector<std::vector<std::string>> &result) {
    // TODO: odstranit priamy vypis na konzolu
    for (auto row : result) {
        for (auto item : row) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }

    // TODO: vratit spravu vo formate pre klienta
    return "";
}


std::string Interpreter::error() {
    // TODO: odstranit priamy vypis na konzolu
    std::cout << "Chyba v prikaze!" << std::endl;

    // TODO: vratit spravu vo formate pre klienta
    // pozn. vratena sprava musi byt urcite rozna od prazdenho stringu! (vysledok parseWhereConditions sa kontroluje ci je rozny od "")
    return "error";
}


std::string Interpreter::errorUnknownCommand() {
    // TODO: odstranit priamy vypis na konzolu
    std::cout << "Neznamy prikaz!" << std::endl;

    // TODO: vratit spravu vo formate pre klienta
    return "error";
}


std::string Interpreter::error(std::string message) {
    // TODO: odstranit priamy vypis na konzolu
    std::cout << message << std::endl;

    // TODO: vratit spravu vo formate pre klienta
    return "error";
}


