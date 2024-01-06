#include "Interpreter.h"

Interpreter::Interpreter() {
    this->dbms = new DBMS();
}

Interpreter::~Interpreter() {
    delete this->dbms;
}


std::string Interpreter::run(std::string command, std::string currentUser) {
    // Rozdel prikaz na slova
    std::vector<std::string> words;
    auto error = this->parseCommand(command, words);

    if (error != "") {
        return error;
    }

    if (words.empty()) {
        return "";
    }

    // Identifikuj prikaz
    std::string commandName = words[0];
    words.erase(words.begin());

    // Predpokladame ze vsetky prikazy maju aspon dve slova
    if (words.empty()) {
        return Encoder::errorUnknownCommand();
    }

    try {
        if (commandName == "show") {
            return this->show(words);
        } else if (commandName == "create") {
            return this->create(words, currentUser);
        } else if (commandName == "drop") {
            return this->drop(words, currentUser);
        } else if (commandName == "select") {
            return this->select(words, currentUser);
        }  else if (commandName == "insert") {
            return this->insert(words, currentUser);
        } else if (commandName == "update") {
            return this->update(words, currentUser);
        } else if (commandName == "delete") {
            return this->deleteCommand(words, currentUser);
        } else if (commandName == "grant") {
            return this->grant(words, currentUser);
        } else if (commandName == "revoke") {
            return this->revoke(words, currentUser);
        }

        return Encoder::errorUnknownCommand();
    } catch (std::exception& e) {
        return Encoder::error(std::string(e.what()));
    }
}


bool Interpreter::tryLogin(std::string username, std::string password) {
    return this->dbms->authorize(username, password);
}

bool Interpreter::tryRegister(std::string username, std::string password) {
    return this->dbms->createUser(username, password);
}


std::string Interpreter::show(std::vector<std::string>& words) {
    if (words[0] == "tables") {
        if (words.size() == 1) {
            auto tables = this->dbms->getTablesList();

            // Do tabulky vloz na zaciatok hlavicku
            tables.insert(tables.begin(), std::vector<std::string>{"Tabulka", "Vlastnik" });

            return Encoder::success(tables);
        } else if (words.size() == 3 && words[1] == "from") {
            if (!this->dbms->userExists(words[2])) {
                return Encoder::error("Pouzivatel " + words[2] + " neexistuje!");
            }

            auto table = this->dbms->getTablesListCreatedByUser(words[2]);

            // Do tabulky vloz na zaciatok hlavicku
            table.insert(table.begin(), "Tabulka");

            return Encoder::success("Tabulky vytvorene pouzivatelom " + words[2] + ":", table);

            // show tables accessible for
        } else if (words.size() == 4 && words[1] == "accessible" && words[2] == "for") {
            if (!this->dbms->userExists(words[3])) {
                return Encoder::error("Pouzivatel " + words[3] + " neexistuje!");
            }

            auto tables = this->dbms->getTablesWithPermissions(words[3]);

            auto result = std::vector<std::vector<std::string>>();

            // Do tabulky vloz na zaciatok hlavicku
            result.push_back(std::vector<std::string>{"Tabulka", "Prava"});

            for (auto table : tables) {
                auto row = std::vector<std::string>();
                row.push_back(table.first);
                row.push_back(table.second);
                result.push_back(row);
            }

            return Encoder::success("Tabulky pristupne pouzivatelovi " + words[3] + ":", result);
        } else {
            return Encoder::errorUnknownCommand();
        }
    } else if (words[0] == "users") {
        auto users = this->dbms->getUsersList();

        // Do tabulky vloz na zaciatok hlavicku
        users.insert(users.begin(), "Pouzivatel");

        return Encoder::success(users);
    }

    return Encoder::errorUnknownCommand();
}


std::string Interpreter::create(std::vector<std::string>& words, std::string currentUser) {
    if (words[0] == "user") {
        words.erase(words.begin());
        return this->createUser(words);
    } else if (words[0] == "table") {
        words.erase(words.begin());
        return this->createTable(words, currentUser);
    } else {
        return Encoder::errorUnknownCommand();
    }
}


// CREATE USER username IDENTIFIED BY password
std::string Interpreter::createUser(std::vector<std::string>& words) {
    if (words.size() != 4) {
        return Encoder::errorUnknownCommand();
    }

    if (words[1] != "identified" || words[2] != "by") {
        return Encoder::error();
    }

    if (this->dbms->userExists(words[0])) {
        return Encoder::error("Pouzivatel " + words[0] + " uz existuje!");
    }

    this->dbms->createUser(words[0], words[3]);

    return Encoder::success("Pouzivatel " + words[0] + " bol vytvoreny!");
}


// create table NAZOV (stlpec1 typ1, stlpec2 typ2, stlpec3 typ3 not null, stlpec4 typ4, stlpec5 typ5 primary key)
std::string Interpreter::createTable(std::vector<std::string>& words, std::string currentUser) {
    if (words.empty()) {
        return Encoder::error();
    }

    // Ziskaj nazov tabulky
    std::string tableName = words[0];
    words.erase(words.begin());

    // Kontrola ci zacina zatvorkou
    if (words[0].size() == 0 || words[0][0] != '(') {
        return Encoder::error();
    }

    // Odstran zatvorku
    words[0].erase(words[0].begin());

    if (words[0].size() == 0) {
        words.erase(words.begin());
    }

    if (words[0].size() == 0) {
        return Encoder::error();
    }

    std::vector<TableRowScheme> rows;
    std::string primaryKeyColumn = "";

    // Ziskaj stlpce
    while (words[0] != ")") {

        // Musia by minimalne dve slova: nazov_stlpca a typ
        if (words.size() < 2) {
            return Encoder::error();
        }

        // Ak prve slovo konci ciarkou tak vyhod chybu
        if (words[0].back() == ',') {
            return Encoder::error();
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
                return Encoder::error();
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
                    return Encoder::error();
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
                    return Encoder::error();
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
                return Encoder::error("Tabulka nemoze mat viac ako jeden primarny kluc!");
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
        return Encoder::error();
    }

    if (primaryKeyColumn.empty()) {
        return Encoder::error("Tabulka musi mat primarny kluc!");
    }

    auto tableScheme = TableScheme(tableName, currentUser, primaryKeyColumn);

    for (auto row : rows) {
        tableScheme.addRow(row);
    }

    this->dbms->createTable(tableScheme);

    return Encoder::success("Tabulka " + tableName + " bola vytvorena!");
}


std::string Interpreter::drop(std::vector<std::string>& words, std::string currentUser) {
    if (words[0] != "table") {
        return Encoder::errorUnknownCommand();
    }

    words.erase(words.begin());

    if (words.empty()) {
        return Encoder::error();
    }

    std::string tableName = words[0];
    words.erase(words.begin());

    if (!words.empty()) {
        return Encoder::error();
    }

    dbms->dropTable(tableName, currentUser);

    return Encoder::success("Tabulka " + tableName + " bola zmazana!");
}


std::string Interpreter::select(std::vector<std::string>& words, std::string currentUser) {
    std::string tableName;
    std::vector<std::string> columns;
    std::vector<Condition> conditions;
    std::string orderColumn;
    bool ascending = true;

    // ak nevyberam vsetky stlpce, ale iba konkretne
    if (words[0] != "*") {
        // ak je prikaz v tvare "select from" tak vyhoď chybu
        if (words[0] == "from") {
            return Encoder::error();
        }

        while (words[0] != "from") {
            // kontrola ci v slove je ciarka, ak hej tak odstran
            if (words[0].back() == ',') {
                words[0].pop_back();
            }

            columns.push_back(words[0]);
            words.erase(words.begin());

            if (words.empty()) {
                return Encoder::error();
            }
        }
    } else {
        words.erase(words.begin());
    }

    // ak prikaz nepokracuje slovom "from" tak vyhoď chybu
    if (words.empty() || words[0] != "from") {
        return Encoder::error();
    }

    words.erase(words.begin());

    // ak za slovom "from" nie je meno tabulky tak vyhoď chybu
    if (words.empty()) {
        return Encoder::error();
    }

    tableName = words[0];
    words.erase(words.begin());

    // ak prikaz pokracuje slovom "where"
    if (!words.empty() && words[0] == "where") {
        words.erase(words.begin());

        // ak za slovom "where" nie je podmienka tak vyhoď chybu
        if (words.empty()) {
            return Encoder::error();
        }

        // Spracuj podmienky, ktoré sú oddelené slovom "and"
        while (words[0] != "order" || !words.empty()) {

            // Kontrola ci nasledujuce slova su aspo 3 (stlpec operator hodnota)
            if (words.size() < 3) {
                return Encoder::error();
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
                return Encoder::error();
            }

        }
    }

    // ak prikaz pokracuje slovom "order"
    if (!words.empty() && words[0] == "order") {
        words.erase(words.begin());

        // ak za slovom "order" nenasleduje slovo "by" tak vyhoď chybu
        if (words.empty() || words[0] != "by") {
            return Encoder::error();
        }

        words.erase(words.begin());

        // ak za slovom "by" nie je stlpec tak vyhoď chybu
        if (words.empty()) {
            return Encoder::error();
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
                return Encoder::error();
            }
        }
    }

    // ak sa tu este nachadzaju nejake slova tak vyhoď chybu
    if (!words.empty()) {
        return Encoder::error();
    }

    // ziskaj vysledky
    auto result = this->dbms->selectFromTable(tableName, columns, conditions, orderColumn, ascending, currentUser);

    return Encoder::success(result);
}


// insert into table_name (column1 column2 column3 ...) values (value1 value2 value3 ...)
std::string Interpreter::insert(std::vector<std::string>& words, std::string currentUser) {
    if (words[0] != "into") {
        return Encoder::errorUnknownCommand();
    }

    words.erase(words.begin());

    if (words.size() < 4) {
        return Encoder::error();
    }

    std::string tableName = words[0];
    words.erase(words.begin());

    // Kontrola ci zacina zatvorkou
    if (words[0].size() == 0 || words[0][0] != '(') {
        return Encoder::error();
    }

    // Odstran zatvorku
    words[0].erase(words[0].begin());

    if (words[0].size() == 0) {
        words.erase(words.begin());
    }

    if (words[0].size() == 0) {
        return Encoder::error();
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
            return Encoder::error();
        }
    }

    // Odstanenie posledneho slova alebo zatvorky
    words.erase(words.begin());

    if (words[0] != "values") {
        return Encoder::error();
    }

    // Odstranenie slova values
    words.erase(words.begin());

    // Kontrola ci zacina zatvorkou
    if (words[0].size() == 0 || words[0][0] != '(') {
        return Encoder::error();
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
            return Encoder::error();
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
            return Encoder::error();
        }
    }

    // Odstanenie posledneho slova alebo zatvorky
    words.erase(words.begin());

    if (!columns.empty()) {
        return Encoder::error();
    }

    dbms->insertIntoTable(tableName, values, currentUser);

    return Encoder::success("Zaznam bol vlozeny!");
}


std::string Interpreter::update(std::vector<std::string> &words, std::string currentUser) {
    std::string tableName = words[0];
    words.erase(words.begin());

    if (words.empty() || words[0] != "set") {
        return Encoder::error();
    }

    words.erase(words.begin());

    if (words.empty()) {
        return Encoder::error();
    }

    std::map<std::string, std::string> valuesForUpdate;

    while (!words.empty() && words[0] != "where") {
        // Kontrola ci nasledujuce slova su aspon 3 (stlpec = hodnota)
        if (words.size() < 3) {
            return Encoder::error();
        }

        std::string column = words[0];
        words.erase(words.begin());

        std::string op = words[0];
        words.erase(words.begin());

        std::string value = words[0];
        words.erase(words.begin());

        if (op != "=") {
            return Encoder::error();
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

    size_t countOfUpdated = this->dbms->updateTable(tableName, valuesForUpdate, conditions, currentUser);

    return Encoder::success("Pocet aktualizovanych riadkov: " + std::to_string(countOfUpdated));
}


// delete from table_name where stlpec = hodnota
std::string Interpreter::deleteCommand(std::vector<std::string>& words, std::string currentUser) {
    if (words[0] != "from") {
        return Encoder::errorUnknownCommand();
    }

    words.erase(words.begin());

    if (words.empty()) {
        return Encoder::error();
    }

    std::string tableName = words[0];
    words.erase(words.begin());

    // Spracovanie podmienok where
    std::vector<Condition> conditions;
    auto error = this->parseWhereConditions(words, conditions);

    // Ak nastala chyba pri spracovani podmienok tak ju vrat
    if (!error.empty()) {
        return error;
    }

    size_t countOfDeleted = this->dbms->deleteFromTable(tableName, conditions, currentUser);

    return Encoder::success("Pocet zmazanych riadkov: " + std::to_string(countOfDeleted));
}


// GRANT pravo ON tabulka TO pouzivatel
std::string Interpreter::grant(std::vector<std::string> &words, std::string currentUser) {
    std::string permission = words[0];
    words.erase(words.begin());

    if (words.size() < 4 || words[0] != "on") {
        return Encoder::error();
    }

    std::string table = words[1];

    if (words[2] != "to") {
        return Encoder::error();
    }

    std::string user = words[3];

    PermissionType permissionType = Permission::stringToPermissionType(permission);

    this->dbms->grantPermission(user, table, permissionType, currentUser);

    return Encoder::success("Pouzivatelovi " + user + " bolo pridelene pravo " + permission + " na tabulku " + table);
}


// REVOKE pravo ON tabulka FROM pouzivatel
std::string Interpreter::revoke(std::vector<std::string> &words, std::string currentUser) {
    std::string permission = words[0];
    words.erase(words.begin());

    if (words.size() < 4 || words[0] != "on") {
        return Encoder::error();
    }

    std::string table = words[1];

    if (words[2] != "from") {
        return Encoder::error();
    }

    std::string user = words[3];

    PermissionType permissionType = Permission::stringToPermissionType(permission);

    this->dbms->revokePermission(user, table, permissionType, currentUser);

    return Encoder::success("Pouzivatelovi " + user + " bolo odobrate pravo " + permission + " na tabulku " + table);
}


/**
 * Rozdeli prikaz na slova, ktore su oddelene medzerou ale pri tomto rozdeleni ignoruje medzery v uvodzovkach
 *
 * @param command
 * @param words
 */
std::string Interpreter::parseCommand(std::string command, std::vector<std::string>& words) {
    bool uvodzovky = false;

    int indexKoncaPoslednehoSlova = 0;

    for (int i = 0; i < command.length(); i++) {
        auto aktualnyZnak = command[i];

        if (aktualnyZnak == ';') {
            return Encoder::error("Nepovoleny znak bodkociarka");
        }

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

    return "";
}


std::string Interpreter::parseWhereConditions(std::vector<std::string>& words, std::vector<Condition>& conditions) {
    // Odstranenie slova where
    if (!words.empty()) {
        if (words[0] != "where") {
            return Encoder::error();
        }

        words.erase(words.begin());
    } else {
        return "";
    }

    // Za slovom where musi nasledovat aspon jedna podmienka
    if (words.empty()) {
        return Encoder::error();
    }

    // Spracuj podmienky, ktoré sú oddelené slovom "and"
    while (!words.empty()) {

        // Kontrola ci nasledujuce slova su aspon 3 (stlpec operator hodnota)
        if (words.size() < 3) {
            return Encoder::error();
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
            return Encoder::error();
        }
    }

    return "";
}
