#include "DBMS.h"


DBMS::DBMS() {
    this->fileManager = new FileManager("../data");

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
    this->tables.push_back(tableScheme->getName());

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
        throw std::invalid_argument("Tabulka neexistuje!");
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
        if (this->tables[i] == tableName) {
            this->tables.erase(this->tables.begin() + i);
            break;
        }
    }
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
            throw std::invalid_argument("Stlpec " + keyValue.first + " neexistuje!");
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

        csvRow += value->second + ";";
    }

    // Odstranenie poslednej bodkociarky
    csvRow.pop_back();

    // Kontrola unikatnosti primarneho kluca
    auto tableData = this->fileManager->loadTableData(tableName, tableScheme->getRows().size());

    for (auto row : tableData) {
        if (row[primaryKeyIndex] == primaryKeyValue) {
            throw std::invalid_argument("Zaznam s primarnym klucom " + primaryKeyValue + " uz existuje!");
        }
    }

    // Zapis riadku do suboru
    this->fileManager->insertIntoTable(tableName, csvRow);
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
        std::cout << table << std::endl;
    }
}


bool DBMS::tableExists(const std::string& tableName) {
    for (const auto& table : this->tables) {
        if (table == tableName) {
            return true;
        }
    }

    return false;
}


bool DBMS::dataTypeCheck(std::string value, RowDataType type) {
    switch (type) {
        case type_int:
            try {
                std::stoi(value);
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


