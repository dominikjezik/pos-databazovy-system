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
void DBMS::createTable(TableScheme& tableScheme) {
    // Kontrola ci tabulka existuje v zozname tabuliek
    if (this->tableExists(tableScheme.getName())) {
        throw std::invalid_argument("Tabulka uz existuje!");
    }

    // Kontrola ci obsahuje primarny kluc
    if (tableScheme.getPrimaryKey() == "") {
        throw std::invalid_argument("Tabulka musi obsahovat primarny kluc!");
    }

    // Kontrola ci je zadany primarny kluc v tabulke a ci je nie je nullable
    bool primaryKeyExists = false;

    for (auto row : tableScheme.getRows()) {
        if (row.getName() == tableScheme.getPrimaryKey()) {
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
    auto table = new TableItem(tableScheme.getName(), tableScheme.getOwner());
    this->tables.push_back(table);
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
    this->fileManager->dropTable(tableName, tableScheme->getOwner());

    // Vymazanie tabulky zo zoznamu tabuliek
    for (int i = 0; i < this->tables.size(); i++) {
        if (this->tables[i]->getName() == tableName) {
            this->tables.erase(this->tables.begin() + i);
            break;
        }
    }

    // Dealokovanie tableScheme
    delete tableScheme;
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
    this->orderRows(rows, tableScheme, orderColumn, ascending);

    // Odstranenie nepotrebnych stlpcov
    if (!columns.empty()) {
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

    // Dealokovanie tableScheme
    delete tableScheme;

    return rows;
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

    // TODO: Kontrola opravnenia

    // Kontrola ci sa vkladaju iba stlpce ktore existuju a kontrola datovych typov
    this->validateExistingColumnsAndTypes(newRecord, tableScheme);

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

    for (auto& row : tableData) {
        if (row[tableScheme->getPrimaryKeyIndex()] == newRecord[tableScheme->getPrimaryKey()]) {
            throw std::invalid_argument("Zaznam s primarnym klucom \"" + newRecord[tableScheme->getPrimaryKey()] + "\" uz existuje!");
        }
    }

    // Zapis riadku do suboru
    this->fileManager->insertIntoTable(tableName, csvRow);

    // Dealokovanie tableScheme
    delete tableScheme;
}


/**
 * Metoda na aktualizaciu zaznamov v tabulke. (UPDATE)
 *
 * @param tableName
 * @param newValues
 * @param conditions
 * @param currentUser
 * @return
 */
size_t DBMS::updateTable(std::string tableName, std::map<std::string, std::string> newValues, std::vector<Condition> conditions, std::string currentUser) {
    // Kontrola ci tabulka existuje v zozname tabuliek
    if (!this->tableExists(tableName)) {
        throw std::invalid_argument("Tabulka neexistuje!");
    }

    // TODO: Kontrola opravnenia

    // Ziskanie schemy tabulky
    auto tableScheme = this->fileManager->loadTableScheme(tableName);

    // Ziskanie dat tabulky
    auto rows = this->fileManager->loadTableData(tableName, tableScheme->getRows().size());


    // Prefiltrovanie zaznamov podla podmienok
    std::vector<std::vector<std::string>> filteredOutRows;
    this->filterOutRows(rows, filteredOutRows, tableScheme, conditions);

    // Kontrola ci sa aktualizuju iba stlpce ktore existuju a kontrola datovych typov
    this->validateExistingColumnsAndTypes(newValues, tableScheme);

    int countOfUpdatedRows = rows.size();

    // Aktualizacia hodnot
    for (auto& row : rows) {
        for (auto& newValue : newValues) {
            for (size_t i = 0; i < tableScheme->getRows().size(); i++) {
                auto tableRow = tableScheme->getRows()[i];

                if (tableRow.getName() == newValue.first) {
                    row[i] = newValue.second;
                    break;
                }
            }
        }
    }

    // Kontrola unikatnosti primarneho kluca ak bol zmeneny
    if (newValues.find(tableScheme->getPrimaryKey()) != newValues.end()) {
        // Ak sa aktualizuje primarny kluc, pocet aktualizovanych zaznamov moze byt maximalne 1
        if (countOfUpdatedRows > 1) {
            throw std::invalid_argument("Duplicitne hodnoty v primarnom kluci!");
        }

        // Ziskanie indexu primarneho kluca
        size_t primaryKeyIndex = tableScheme->getPrimaryKeyIndex();

        // Kontrola unikatnosti primarneho kluca voci existujucim zaznamom vo filteredOutRows
        for (auto row : filteredOutRows) {
            if (row[primaryKeyIndex] == newValues[tableScheme->getPrimaryKey()]) {
                throw std::invalid_argument("Duplicitne hodnoty v primarnom kluci!");
            }
        }
    }


    // Skombinuje odfiltrovane zaznamy s aktualizovanymi zaznamami
    rows.insert(rows.end(), filteredOutRows.begin(), filteredOutRows.end());

    // Zapisanie aktualizovanych zaznamov do suboru
    this->fileManager->saveTableData(tableName, rows);

    // Dealokovanie tableScheme
    delete tableScheme;

    return countOfUpdatedRows;
}


/**
 * Metoda na vymazanie zaznamov z tabulky. (DELETE FROM)
 *
 * @param tableName
 * @param conditions
 * @param currentUser
 * @return
 */
size_t DBMS::deleteFromTable(std::string tableName, std::vector<Condition> conditions, std::string currentUser) {
    // Kontrola ci tabulka existuje v zozname tabuliek
    if (!this->tableExists(tableName)) {
        throw std::invalid_argument("Tabulka neexistuje!");
    }

    // TODO: Kontrola opravnenia

    // Ziskanie schemy tabulky
    auto tableScheme = this->fileManager->loadTableScheme(tableName);

    // Ziskanie dat tabulky
    auto rows = this->fileManager->loadTableData(tableName, tableScheme->getRows().size());

    // Prefiltrovanie zaznamov podla podmienok
    std::vector<std::vector<std::string>> filteredOutRows;
    this->filterOutRows(rows, filteredOutRows, tableScheme, conditions);

    // Vymazanie odfiltrovanych zaznamov zo suboru
    this->fileManager->saveTableData(tableName, filteredOutRows);

    // Dealokovanie tableScheme
    delete tableScheme;

    return rows.size();
}






// Metoda iba na debugovanie, bude odstranena
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


bool DBMS::dataTypeCheck(std::string value, RowDataType type, bool isNullable) {
    if (value == "" && isNullable) {
        return true;
    }

    if (value == "" && !isNullable && type != type_string) {
        throw std::invalid_argument("Hodnota nemoze byt NULL!");
    }

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
            // YYYY-MM-DD

            if (value.size() != 10) {
                throw std::invalid_argument("Zadany datovy typ nie je date!");
            }

            if (value[4] != '-' || value[7] != '-') {
                throw std::invalid_argument("Zadany datovy typ nie je date!");
            }

            for (int i = 0; i < value.size(); i++) {
                if (i == 4 || i == 7) {
                    continue;
                }

                if (value[i] < '0' || value[i] > '9') {
                    throw std::invalid_argument("Zadany datovy typ nie je date!");
                }
            }

            // Pretypovanie na int a kontrola ci je to validny datum
            try {
                auto year = std::stoi(value.substr(0, 4));
                auto month = std::stoi(value.substr(5, 2));
                auto day = std::stoi(value.substr(8, 2));

                if (month < 1 || month > 12) {
                    throw std::invalid_argument("Zadany datum nie je validny!");
                }

                if (day < 1 || day > 31) {
                    throw std::invalid_argument("Zadany datum nie je validny!");
                }

                if (month == 4 || month == 6 || month == 9 || month == 11) {
                    if (day > 30) {
                        throw std::invalid_argument("Zadany datum nie je validny!");
                    }
                }

                if (month == 2) {
                    if (year % 4 == 0) {
                        if (day > 29) {
                            throw std::invalid_argument("Zadany datum nie je validny!");
                        }
                    } else {
                        if (day > 28) {
                            throw std::invalid_argument("Zadany datum nie je validny!");
                        }
                    }
                }

            } catch (std::invalid_argument& e) {
                throw std::invalid_argument("Zadany datum nie je validny!");
            }

            break;
    }

    return true;
}


/**
 * Metoda na filtrovanie zaznamov podla podmienok, pricom zaznamy zoberie
 * zo vstupneho vektora a vystupne zaznamy ulozi do tohto vektora.
 *
 * @param rows
 * @param tableScheme
 * @param conditions
 */
void DBMS::filterRows(std::vector<std::vector<std::string>>& rows, TableScheme *tableScheme, std::vector<Condition> conditions) {
    std::vector<std::vector<std::string>> filteredOutRows;
    this->filterOutRows(rows, filteredOutRows, tableScheme, conditions);
}


/**
 * Metoda na filtrovanie zaznamov podla podmienok, pricom zaznamy zoberie
 * zo vstupneho vektora a vystupne zaznamy ulozi do tohto vektora a
 * zaznamy ktore nesplnaju podmienky ulozi do vystupneho vektora.
 *
 * @param rows
 * @param filteredOutRows
 * @param tableScheme
 * @param conditions
 */
void DBMS::filterOutRows(std::vector<std::vector<std::string>>& rows, std::vector<std::vector<std::string>>& filteredOutRows, TableScheme *tableScheme, std::vector<Condition> conditions) {
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
                    filteredOutRows.push_back(row);
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
                    filteredOutRows.push_back(row);
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
                    filteredOutRows.push_back(row);
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
                    filteredOutRows.push_back(row);
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
                    filteredOutRows.push_back(row);
                    rows.erase(rows.begin() + i);
                    i--;
                }
            }
        }
    }
}


/**
 * Pomocna metoda na kontrolu ci sa vkladaju iba stlpce ktore existuju a kontrola datovych typov.
 * Tiez sa pouziva na kontrolu datovych typov pri aktualizacii zaznamov.
 *
 * @param record
 * @param tableScheme
 */
void DBMS::validateExistingColumnsAndTypes(std::map<std::string, std::string> &record, TableScheme *tableScheme) {
    // Kontrola ci sa vkladaju iba stlpce ktore existuju a kontrola datovych typov
    for (const auto& keyValue : record) {
        bool columnExists = false;

        for (size_t i = 0; i < tableScheme->getRows().size(); i++) {
            auto row = tableScheme->getRows()[i];

            if (row.getName() == keyValue.first) {
                columnExists = true;

                this->dataTypeCheck(keyValue.second, row.getDataType(), row.isNullable());
            }
        }

        if (!columnExists) {
            throw std::invalid_argument("Stlpec \"" + keyValue.first + "\" neexistuje!");
        }
    }
}


void DBMS::orderRows(std::vector<std::vector<std::string>> &rows, TableScheme *tableScheme, std::string orderColumn, bool ascending) {
    if (orderColumn.empty())
    {
        return;
    }

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

    // Zoradenie zaznamov podla cisla
    if (tableScheme->getRows()[columnIndex].getDataType() == type_int || tableScheme->getRows()[columnIndex].getDataType() == type_double) {
        // Ak su pripustne aj null hodnoty,
        if (tableScheme->getRows()[columnIndex].isNullable()) {
            std::sort(rows.begin(), rows.end(), [&](const std::vector<std::string>& a, const std::vector<std::string>& b) {
                if (a[columnIndex].empty()) {
                    // Zabezpeci ze null hodnoty su "najmensie" v zoradeni
                    return ascending;
                }

                if (b[columnIndex].empty()) {
                    // Zabezpeci ze null hodnoty su "najmensie" v zoradeni
                    return ascending;
                }

                if (ascending) {
                    return std::stod(a[columnIndex]) < std::stod(b[columnIndex]);
                } else {
                    return std::stod(a[columnIndex]) > std::stod(b[columnIndex]);
                }
            });
        } else {
            // Zoradenie zaznamov bez null hodnot
            std::sort(rows.begin(), rows.end(), [&](const std::vector<std::string>& a, const std::vector<std::string>& b) {
                if (ascending) {
                    return std::stod(a[columnIndex]) < std::stod(b[columnIndex]);
                } else {
                    return std::stod(a[columnIndex]) > std::stod(b[columnIndex]);
                }
            });
        }
    } else {
        // Zoradenie zaznamov podla retazca
        std::sort(rows.begin(), rows.end(), [&](const std::vector<std::string>& a, const std::vector<std::string>& b) {
            if (ascending) {
                return a[columnIndex] < b[columnIndex];
            } else {
                return a[columnIndex] > b[columnIndex];
            }
        });
    }
}

