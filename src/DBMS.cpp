#include "DBMS.h"


DBMS::DBMS() {
    this->fileManager = new FileManager("../data");

    // Vygeneruj subory ak neexistuju
    this->fileManager->createInitialFilesIfNotExists();

    // Nacitaj pouzivatelov zo suboru
    this->fileManager->loadUsers(this->users);

    // Nacitaj zoznam tabuliek zo suboru
    this->fileManager->loadTablesList(this->tables);
}


DBMS::~DBMS() {
    delete this->fileManager;

    for (auto user : this->users)
    {
        delete user;
    }

    for (auto table : this->tables)
    {
        delete table;
    }
}


/**
 * Metoda na autorizaciu uzivatela.
 *
 * Skontroluje ci sa v zozname pouzivatelov nachadza uzivatel s danym menom a heslom.
 *
 * @param username
 * @param password
 * @return logged in
 */
bool DBMS::authorize(std::string username, std::string password) {
    for (auto user : this->users) {
        if (user->getUsername() == username && user->getPassword() == password) {
            return true;
        }
    }

    return false;
}


/**
 * Metoda na vytvorenie uzivatela.
 *
 * Skontroluje ci uzivatel s danym menom uz existuje.
 * Ak nie, vytvori uzivatela a zapise ho do suboru.
 * Potom prida uzivatela do zoznamu pouzivatelov.
 *
 * @param username
 * @param password
 * @return created
 */
bool DBMS::createUser(std::string username, std::string password) {
    // Kontrola ci uzivatel uz existuje v zozname
    for (auto user : this->users) {
        if (user->getUsername() == username) {
            return false;
        }
    }

    // Vytvor uzivatela
    auto user = new User(username, password);

    // Zapis uzivatela do suboru
    this->fileManager->saveUser(user);

    // Pridaj uzivatela do zoznamu
    this->users.push_back(user);

    return true;
}


/**
 * Metoda na ziskanie zoznamu pouzivatelov.
 *
 * @return usernames
 */
std::vector<std::string> DBMS::getUsersList() {
    std::vector<std::string> usernames;

    usernames.reserve(this->users.size());
    for (auto user : this->users) {
        usernames.push_back(user->getUsername());
    }

    return usernames;
}


/**
 * Metoda na kontrolu ci uzivatel existuje.
 *
 * @param username
 * @return exists
 */
bool DBMS::userExists(std::string username) {
    for (auto user : this->users) {
        if (user->getUsername() == username) {
            return true;
        }
    }

    return false;
}


/**
 * Metoda na vytvorenie tabulky.
 *
 * Skontroluje ci tabulka uz existuje. Ak nie,
 * vytvori tabulku a k nej prislusne subory.
 * Potom prida tabulku do zoznamu tabuliek.
 *
 * @param tableScheme
 * @return created
 */
bool DBMS::createTable(TableScheme* tableScheme) {
    // Kontrola ci tabulka existuje v zozname tabuliek
    if (this->tableExists(tableScheme->getName())) {
        return false;
    }

    // Kontrola ci obsahuje primarny kluc
    if (tableScheme->getPrimaryKey() == "") {
        throw std::invalid_argument("Tabulka musi obsahovat primarny kluc!");
    }

    // Kontrola ci je zadany primarny kluc v tabulke a ci je nie je nullable
    bool primaryKeyExists = false;

    for (auto row : tableScheme->getRows()) {
        if (row.getName() == tableScheme->getPrimaryKey()) {
            primaryKeyExists = true;

            if (row.isNullable()) {
                throw std::invalid_argument("Primarny kluc nemoze byt nullable!");
            }

            break;
        }
    }

    if (!primaryKeyExists) {
        throw std::invalid_argument("Zadany primarny kluc nepatri pod ziadny stlpec!");
    }


    // Vytvor subory pre tabulku
    this->fileManager->createTable(tableScheme);

    // Pridaj tabulku do zoznamu tabuliek
    auto table = new TableItem(tableScheme->getName(), tableScheme->getOwner());
    this->tables.push_back(table);

    return true;
}


/**
 * Metoda na vymazanie tabulky.
 *
 * Skontroluje ci tabulka existuje a ci je uzivatel vlastnikom tabulky.
 * Ak ano, vymaze subory tabulky a vymaze tabulku zo zoznamu tabuliek.
 *
 * @param tableName
 * @param username
 */
void DBMS::dropTable(std::string tableName, std::string username) {
    // Kontrola ci tabulka existuje v zozname tabuliek
    if (!this->tableExists(tableName)) {
        throw std::invalid_argument("Tabulka " + tableName +" neexistuje!");
    }

    // Ziskanie schemy tabulky
    auto tableScheme = this->fileManager->loadTableScheme(tableName);

    // Kontrola ci uzivatel je vlastnikom tabulky
    if (tableScheme->getOwner() != username) {
        throw std::invalid_argument("Tabulku moze vymazat iba vlastnik!");
    }

    // Vymazanie tabulky zo suboru
    this->fileManager->dropTable(tableName);

    // Vymazanie tabulky zo zoznamu tabuliek
    for (int i = 0; i < this->tables.size(); i++) {
        if (this->tables[i]->getName() == tableName) {
            this->tables.erase(this->tables.begin() + i);
            break;
        }
    }
}


/**
 * Metoda na ziskanie zoznamu tabuliek.
 *
 * @return tables
 */
std::vector<std::string> DBMS::getTablesList() {
    std::vector<std::string> tableNames;

    tables.reserve(this->tables.size());
    for (auto table : this->tables) {
        tableNames.push_back(table->getName());
    }

    return tableNames;
}


/**
 * Metoda na ziskanie zoznamu tabuliek vytvorenych danym uzivatelom.
 *
 * @param username
 * @return tables
 */
std::vector<std::string> DBMS::getTablesListCreatedByUser(const std::string& username) {
    std::vector<std::string> tableNames;

    for (auto table : this->tables) {
        if (table->getOwner() == username) {
            tableNames.push_back(table->getName());
        }
    }

    return tableNames;
}


/**
 * Metoda na vlozenie zaznamu do tabulky.
 *
 * Skontroluje ci tabulka existuje, ci uzivatel ma opravnenie vkladat do tabulky,
 * ci sa vkladaju iba stlpce ktore existuju a kontroluje datove typy.
 *
 * @param tableName
 * @param newRecord
 * @param currentUser
 */
void DBMS::insertIntoTable(std::string tableName, std::map<std::string, std::string> newRecord, std::string currentUser) {
    // Kontrola ci tabulka existuje v zozname tabuliek
    if (!this->tableExists(tableName)) {
        throw std::invalid_argument("Tabulka neexistuje!");
    }

    // Ziskanie schemy tabulky
    auto tableScheme = this->fileManager->loadTableScheme(tableName);

    // TODO: Kontrola ci uzivatel ma opravnenie vkladat do tabulky

    // Ulozenie hodnoty primarneho kluca
    std::string primaryKeyValue;
    size_t primaryKeyIndex = 0;

    // Kontrola ci sa vkladaju iba stlpce ktore existuju a kontrola datovych typov
    for (const auto& keyValue : newRecord) {
        bool columnExists = false;

        for (size_t i = 0; i < tableScheme->getRows().size(); i++) {
            auto row = tableScheme->getRows()[i];

            if (row.getName() == keyValue.first) {
                columnExists = true;

                this->dataTypeCheck(keyValue.second, row.getDataType());

                if (row.getName() == tableScheme->getPrimaryKey()) {
                    primaryKeyValue = keyValue.second;
                    primaryKeyIndex = i;
                }
            }
        }

        if (!columnExists) {
            throw std::invalid_argument("Stlpec \"" + keyValue.first + "\" neexistuje!");
        }
    }

    // Vytvorenie csv riadku
    std::string csvRow;

    // Kontrola ci sa vkladaju vsetky povinne stlpce a vytvorenie csv riadku
    for (auto row : tableScheme->getRows()) {
        auto value = newRecord.find(row.getName());

        if (!row.isNullable() && value == newRecord.end()) {
            throw std::invalid_argument(row.getName() + " nie je nullable!");
        }

        // Ak nebol zadany stlpec, pridaj do riadku prazdny retazec
        if (value == newRecord.end()) {
            csvRow += ";";
            continue;
        }

        csvRow += value->second + ";";
    }

    // Odstranenie poslednej bodkociarky
    csvRow.pop_back();

    // Kontrola unikatnosti primarneho kluca
    auto tableData = this->fileManager->loadTableData(tableName, tableScheme->getRows().size());

    for (auto row : tableData) {
        if (row[primaryKeyIndex] == primaryKeyValue) {
            throw std::invalid_argument("Zaznam s primarnym klucom \"" + primaryKeyValue + "\" uz existuje!");
        }
    }

    // Zapis riadku do suboru
    this->fileManager->insertIntoTable(tableName, csvRow);
}


/**
 * Metoda na vyber zaznamov z tabulky. (SELECT)
 *
 * @param tableName
 * @param columns
 * @param conditions
 * @param currentUser
 * @return
 */
std::vector<std::vector<std::string>> DBMS::selectFromTable(std::string tableName, std::vector<std::string> columns, std::vector<Condition> conditions, std::string orderColumn, bool ascending, std::string currentUser) {
    // Kontrola ci tabulka existuje v zozname tabuliek
    if (!this->tableExists(tableName)) {
        throw std::invalid_argument("Tabulka neexistuje!");
    }

    // TODO: Kontrola opravnenia

    // Ziskanie schemy tabulky
    auto tableScheme = this->fileManager->loadTableScheme(tableName);

    // Kontrolora ci vybrane stlpce existuju
    for (auto column : columns) {
        bool columnExists = false;

        for (auto row : tableScheme->getRows()) {
            if (row.getName() == column) {
                columnExists = true;
                break;
            }
        }

        if (!columnExists) {
            throw std::invalid_argument("Stlpec " + column + " neexistuje!");
        }
    }

    // Ziskanie dat tabulky
    auto rows = this->fileManager->loadTableData(tableName, tableScheme->getRows().size());

    // Prefiltrovanie zaznamov podla podmienok
    this->filterRows(rows, tableScheme, conditions);

    // Zoradenie zaznamov
    // TODO: brat do uvahy datove typy
    if (!orderColumn.empty()) {
        // Kontrola ci stlpec z podmienky existuje
        bool columnExists = false;
        int columnIndex = 0;

        for (int i = 0; i < tableScheme->getRows().size(); i++) {
            auto row = tableScheme->getRows()[i];

            if (row.getName() == orderColumn) {
                columnExists = true;
                columnIndex = i;
                break;
            }
        }

        if (!columnExists) {
            throw std::invalid_argument("Stlpec " + orderColumn + " neexistuje!");
        }

        // Zoradenie zaznamov
        std::sort(rows.begin(), rows.end(), [&](const std::vector<std::string>& a, const std::vector<std::string>& b) {
            if (ascending) {
                return a[columnIndex] < b[columnIndex];
            } else {
                return a[columnIndex] > b[columnIndex];
            }
        });
    }

    // Odstranenie nepotrebnych stlpcov
    if (columns.size() != 0) {

        std::vector<int> columnsToRemove;

        for (int i = 0; i < tableScheme->getRows().size(); i++) {
            auto row = tableScheme->getRows()[i];

            if (std::find(columns.begin(), columns.end(), row.getName()) == columns.end()) {
                columnsToRemove.push_back(i);
            }
        }

        for (int i = 0; i < rows.size(); i++) {
            for (int j = 0; j < columnsToRemove.size(); j++) {
                rows[i].erase(rows[i].begin() + columnsToRemove[j] - j);
            }
        }
    }


    // Pridanie hlavicky tabulky
    std::vector<std::string> header;

    if (columns.empty()) {
        for (auto row : tableScheme->getRows()) {
            header.push_back(row.getName());
        }
    } else {
        // Zoradenie stlpcov headerov podla poradia v table scheme
        for (auto row : tableScheme->getRows()) {
            for (auto column : columns){
                if (row.getName() == column) {
                    header.push_back(row.getName());
                    break;
                }
            }
        }
    }

    rows.insert(rows.begin(), header);

    return rows;
}





void DBMS::TEST_printState() {
    // Vypis pouzivatelov
    std::cout << "*** Pouzivatelia ***" << std::endl;
    for (auto user : this->users) {
        std::cout << user->getUsername() << std::endl;
    }

    // Vypis tabuliek
    std::cout << "*** Tabulky ***" << std::endl;
    for (auto table : this->tables) {
        std::cout << table->getName() << " : " << table->getOwner() << std::endl;
    }
}


bool DBMS::tableExists(const std::string& tableName) {
    for (const auto& table : this->tables) {
        if (table->getName() == tableName) {
            return true;
        }
    }

    return false;
}


bool DBMS::dataTypeCheck(std::string value, RowDataType type) {
    switch (type) {
        case type_int:
            try {
                auto converted = std::stoi(value);

                // Kontrola ci je to naozaj int a nie double s odrezanou desatinou ciarkou
                if (std::to_string(converted) != value) {
                    throw std::invalid_argument("Zadany datovy typ nie je int!");
                }
            } catch (std::invalid_argument& e) {
                throw std::invalid_argument("Zadany datovy typ nie je int!");
            }
            break;
        case type_string:
            break;
        case type_boolean:
            if (value != "true" && value != "false") {
                throw std::invalid_argument("Zadany datovy typ nie je bool!");
            }
            break;
        case type_double:
            try {
                std::stod(value);
            } catch (std::invalid_argument& e) {
                throw std::invalid_argument("Zadany datovy typ nie je double!");
            }
            break;
        case type_date:
            // TODO: Kontrola datumu

            break;
    }

    return true;
}


/**
 * Metoda na filtrovanie zaznamov podla podmienok.
 *
 * @param rows
 * @param tableScheme
 * @param conditions
 */
void DBMS::filterRows(std::vector<std::vector<std::string>>& rows, TableScheme *tableScheme, std::vector<Condition> conditions) {
    // Cez cyklus prejdeme vsetky zadane podmienky
    for (auto condition : conditions) {
        // Kontrola ci stlpec z podmienky existuje
        bool columnExists = false;
        int columnIndex = 0;

        for (int i = 0; i < tableScheme->getRows().size(); i++) {
            auto row = tableScheme->getRows()[i];

            if (row.getName() == condition.getColumn()) {
                columnExists = true;
                columnIndex = i;
                break;
            }
        }

        if (!columnExists) {
            throw std::invalid_argument("Stlpec " + condition.getColumn() + " neexistuje!");
        }


        if (tableScheme->getRows()[columnIndex].getDataType() == type_int) {
            // Pretypovana hodnota s ktorou budeme zaznamy porovnavat
            int conditionValue;

            // Informacia ci budeme porovnavat s NULL hodnotou
            bool compareNull = false;

            try {
                // Ak je nullable a hodnota v podmienke je NULL
                if (condition.getValue().size() == 0) {
                    compareNull = true;
                } else {
                    conditionValue = std::stoi(condition.getValue());
                }
            } catch (std::invalid_argument& e) {
                throw std::invalid_argument("Hodnota zadaná v podmienke nie je typu int!");
            }

            // Filtracia zaznamov
            for (int i = 0; i < rows.size(); i++) {
                auto row = rows[i];
                bool isOk;

                // Ak porovnavame s NULL hodnotou
                if (compareNull) {
                    isOk = Condition::compareNull(row[columnIndex], "", condition.getOperation());

                    // Ak porovnavame s hodnotou
                } else {
                    try {
                        auto rowValue = std::stoi(row[columnIndex]);
                        isOk = Condition::compareInt(rowValue, conditionValue, condition.getOperation());
                    } catch (std::invalid_argument& e) {
                        isOk = false;
                    }
                }

                if (!isOk) {
                    rows.erase(rows.begin() + i);
                    i--;
                }
            }
        } else if (tableScheme->getRows()[columnIndex].getDataType() == type_double) {
            // Pretypovana hodnota s ktorou budeme zaznamy porovnavat
            double conditionValue;

            // Informacia ci budeme porovnavat s NULL hodnotou
            bool compareNull = false;

            try {
                // Ak je nullable a hodnota v podmienke je NULL
                if (condition.getValue().size() == 0) {
                    compareNull = true;
                } else {
                    conditionValue = std::stod(condition.getValue());
                }
            } catch (std::invalid_argument& e) {
                throw std::invalid_argument("Hodnota zadaná v podmienke nie je typu double!");
            }

            // Filtracia zaznamov
            for (int i = 0; i < rows.size(); i++) {
                auto row = rows[i];
                bool isOk;

                // Ak porovnavame s NULL hodnotou
                if (compareNull) {
                    isOk = Condition::compareNull(row[columnIndex], "", condition.getOperation());

                    // Ak porovnavame s hodnotou
                } else {
                    try {
                        auto rowValue = std::stod(row[columnIndex]);
                        isOk = Condition::compareDouble(rowValue, conditionValue, condition.getOperation());
                    } catch (std::invalid_argument& e) {
                        isOk = false;
                    }
                }

                if (!isOk) {
                    rows.erase(rows.begin() + i);
                    i--;
                }
            }
        } else if (tableScheme->getRows()[columnIndex].getDataType() == type_string) {
            // Filtracia zaznamov
            for (int i = 0; i < rows.size(); i++) {
                auto row = rows[i];

                auto isOk = Condition::compareString(row[columnIndex], condition.getValue(), condition.getOperation());

                if (!isOk) {
                    rows.erase(rows.begin() + i);
                    i--;
                }
            }
        } else if (tableScheme->getRows()[columnIndex].getDataType() == type_date) {
            // Filtracia zaznamov
            for (int i = 0; i < rows.size(); i++) {
                auto row = rows[i];

                auto isOk = Condition::compareDate(row[columnIndex], condition.getValue(), condition.getOperation());

                if (!isOk) {
                    rows.erase(rows.begin() + i);
                    i--;
                }
            }
        } else if (tableScheme->getRows()[columnIndex].getDataType() == type_boolean) {
            // Pretypovana hodnota s ktorou budeme zaznamy porovnavat
            bool conditionValue;

            // Informacia ci budeme porovnavat s NULL hodnotou
            bool compareNull = false;


            // Ak je nullable a hodnota v podmienke je NULL
            if (condition.getValue().size() == 0) {
                compareNull = true;
            } else if (condition.getValue() == "true") {
                conditionValue = true;
            } else if (condition.getValue() == "false") {
                conditionValue = false;
            } else {
                throw std::invalid_argument("Hodnota zadaná v podmienke nie je typu bool!");
            }

            // Filtracia zaznamov
            for (int i = 0; i < rows.size(); i++) {
                auto row = rows[i];
                bool isOk;

                // Ak porovnavame s NULL hodnotou
                if (compareNull) {
                    isOk = Condition::compareNull(row[columnIndex], "", condition.getOperation());

                    // Ak porovnavame s hodnotou
                } else {
                    if (row[columnIndex] == "true" || row[columnIndex] == "false") {
                        auto rowValue = row[columnIndex] == "true";
                        isOk = Condition::compareBoolean(rowValue, conditionValue, condition.getOperation());
                    } else {
                        isOk = false;
                    }
                }

                if (!isOk) {
                    rows.erase(rows.begin() + i);
                    i--;
                }
            }
        }
    }


}


