#include <iostream>
#include <iomanip>
#include "src/DBMS.h"
#include "src/Interpreter.h"

void vypisLogo();
void seedUsersTable(Interpreter& interpreter);
void seedPostsTable(Interpreter& interpreter);
void seedDatatypesTable(Interpreter& interpreter);

int main() {

    vypisLogo();

    Interpreter interpreter;

    //seedUsersTable(interpreter);
    //seedPostsTable(interpreter);
    //seedDatatypesTable(interpreter);

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

    dbms.TEST_printState();

    std::cout << "Hello, World!" << std::endl;
    return 0;
}

void seedUsersTable(Interpreter& interpreter) {
    interpreter.run("create table users (id int primary key, username string not null, first_name string, last_name string, created_at date not null)");

    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (1 "john_doe" "John" "Doe" "2023-01-01"))");
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (2 "jane_smith" "Jane" "Smith" "2023-02-15"))");
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (3 "mike_jones" "Mike" "Jones" "2023-03-22"))");
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (4 "sara_williams" "Sara" "Williams" "2023-04-10"))");
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (5 "peter_brown" "Peter" "Brown" "2023-05-05"))");
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (6 "emily_white" "Emily" "White" "2023-06-18"))");
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (7 "alex_garcia" "Alex" "Garcia" "2023-07-30"))");
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (8 "olivia_martin" "Olivia" "Martin" "2023-08-12"))");
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (9 "david_clark" "David" "Clark" "2023-09-24"))");
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (10 "linda_king" "Linda" "King" "2023-10-08"))");
    interpreter.run(R"(insert into users (id username created_at) values (11 "user_11" "2023-11-22"))");
    interpreter.run(R"(insert into users (id username first_name created_at) values (12 "user_12" "John" "2023-12-05"))");
    interpreter.run(R"(insert into users (id username last_name created_at) values (13 "user_13" "Jones" "2024-01-15"))");
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (14 "user_14" "Sara" "Williams" "2024-02-28"))");
    interpreter.run(R"(insert into users (id username created_at) values (15 "user_15" "2024-03-10"))");
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (16 "user_16" "Emily" "White" "2024-04-18"))");
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (17 "user_17" "Alex" "Garcia" "2024-05-30"))");
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (18 "user_18" "Olivia" "Martin" "2024-06-12"))");
    interpreter.run(R"(insert into users (id username created_at) values (19 "user_19" "2024-07-24"))");
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (20 "user_20" "Linda" "King" "2024-08-08"))");
}

void seedPostsTable(Interpreter& interpreter) {
    interpreter.run("create table posts (id int primary key, title string not null, content string, created_at date)");

    interpreter.run(R"(insert into posts (id title content created_at) values (1 "Lorem ipsum dolor sit amet" "Content of post 1" "2023-01-01"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (2 "Consectetur adipiscing elit" "Content of post 2" "2023-02-15"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (3 "Sed do eiusmod tempor incididunt" "Content of post 3" "2023-03-22"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (4 "Ut labore et dolore magna aliqua" "Content of post 4" "2023-04-10"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (5 "Quis nostrud exercitation ullamco" "Content of post 5" "2023-05-05"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (6 "Duis aute irure dolor in reprehenderit" "Content of post 6" "2023-06-18"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (7 "Excepteur sint occaecat cupidatat non proident" "Content of post 7" "2023-07-30"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (8 "Sunt in culpa qui officia deserunt mollit" "Content of post 8" "2023-08-12"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (9 "Laboris nisi ut aliquip ex ea commodo" "Content of post 9" "2023-09-24"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (10 "Duis aute irure dolor in reprehenderit" "Content of post 10" "2023-10-08"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (11 "Lorem ipsum dolor sit amet" "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat." "2023-11-22"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (12 "Consectetur adipiscing elit" "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur." "2023-12-05"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (13 "Sed do eiusmod tempor incididunt" "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." "2024-01-15"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (14 "Ut labore et dolore magna aliqua" "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium." "2024-02-28"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (15 "Quis nostrud exercitation ullamco" "Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit." "2024-03-10"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (16 "Duis aute irure dolor in reprehenderit" "Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit." "2024-04-18"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (17 "Excepteur sint occaecat cupidatat non proident" "At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum." "2024-05-30"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (18 "Sunt in culpa qui officia deserunt mollit" "Similique sunt in culpa qui officia deserunt mollitia animi, id est laborum et dolorum fuga." "2024-06-12"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (19 "Laboris nisi ut aliquip ex ea commodo" "Eum autem quod aut officiis debitis aut rerum necessitatibus saepe eveniet ut." "2024-07-24"))");
    interpreter.run(R"(insert into posts (id title content created_at) values (20 "Duis aute irure dolor in reprehenderit" "Itaque earum rerum hic tenetur a sapiente delectus, ut aut reiciendis voluptatibus.." "2024-08-08"))");
}

void seedDatatypesTable(Interpreter& interpreter) {
    interpreter.run(R"(create table data (int int primary key, string string, double double, boolean boolean, date date))");

    interpreter.run(R"(insert into data (int string double boolean date) values (8 "adipiscing elit" 45.3 true "2001-08-17"))");
    interpreter.run(R"(insert into data (int string double boolean date) values (17 "sed do eiusmod" 69.8 false "2014-03-02"))");
    interpreter.run(R"(insert into data (int double boolean date) values (23 "" true "2009-12-05"))");
    interpreter.run(R"(insert into data (int string double boolean date) values (9 "tempor incididunt ut" 52.6 false "2017-06-19"))");
    interpreter.run(R"(insert into data (int string double) values (31 "labore et dolore" 93.2))");
    interpreter.run(R"(insert into data (int string double) values (14 "consectetur adipiscing" 60.7))");
    interpreter.run(R"(insert into data (int string double date) values (19 "quis nostrud exercitation" 75.4 "2016-09-28"))");
    interpreter.run(R"(insert into data (int double boolean date) values (26 49.0 false "2011-11-14"))");
    interpreter.run(R"(insert into data (int string date) values (11 "in voluptate velit" "2003-04-30"))");
    interpreter.run(R"(insert into data (int string date) values (16 "aliquip ex ea commodo" "2018-07-22"))");
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