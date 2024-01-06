#include <iostream>
#include "src/Interpreter.h"
#include "src/Decoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <cstring>
#include <pthread.h>
#include <unistd.h>
#include <bits/stdc++.h>

#ifdef __cplusplus
extern "C" {
#include "src/pos_sockets/active_socket.h"
#include "src/pos_sockets/char_buffer.h"
#include "src/pos_sockets/passive_socket.h"

}
#endif

#include "src/buffer.h"
#include "src/pos_sockets/linked_list.h"

typedef struct thread_data {
    pthread_mutex_t mutex;
    short port;
    ACTIVE_SOCKET* my_socket;
} THREAD_DATA;

void thread_data_init(struct thread_data* data, short port, ACTIVE_SOCKET* my_socket) {
    pthread_mutex_init(&data->mutex, NULL);
    data->port = port;
    data->my_socket = my_socket;
}

void thread_data_destroy(struct thread_data* data) {
    pthread_mutex_destroy(&data->mutex);

    data->port = 0;
    data->my_socket = NULL;
}

void* process_client_data(void* thread_data) {
    struct thread_data* data = (struct thread_data*)thread_data;
    PASSIVE_SOCKET sock;
    passive_socket_init(&sock);
    //mozno toto passive_socket_start_listening budem musiet dat do samostnatneho vlakna, lebo je to asi blokujuce volanie
    passive_socket_start_listening(&sock, data->port);
    //mozem spravit while cyklus, kde bude podmienka passive_socket_is_listening bude vraciat true, tak sa bude stale volat metoda passive_socket_wait_for_client
    //v tomto while cykle budem rovno vytvarat dalsie vlakna, ktore budu vykonavat metodu start_reading
    passive_socket_wait_for_client(&sock, data->my_socket);
    //tieto dve metody passive_socket_stop_listening a passive_socket_destroy presuniem niekde inde. Budem je volat, ked sa staci klavesa Q
    passive_socket_stop_listening(&sock);
    passive_socket_destroy(&sock);

    //pri tejto metode active_socket_start_reading to zastane, lebo je to blokujuce volanie. Skonci to až pri stop_reading
    //pre viacerych klientov tu nemoze byt takto to start_reading. Potrebujem to pre kazdeho klienta ako osobitne vlakno

    active_socket_start_reading(data->my_socket);

    return NULL;
}

//TODO: dat prec
void* write_to_client(void* thread_data) {
    struct thread_data *data = (struct thread_data *) thread_data;
    printf("Ide piiisat\n");

    sleep(10);
    if (data->my_socket != NULL) {
        printf("Spojenieee\n");
        CHAR_BUFFER bufferPreZapis;
        char_buffer_init(&bufferPreZapis);

        char_buffer_append(&bufferPreZapis, "Ahoj svet", strlen("Ahoj svet"));
        active_socket_write_data(data->my_socket, &bufferPreZapis);
        printf("Poslalo 1. správu\n");

        char_buffer_clear(&bufferPreZapis);
        char_buffer_append(&bufferPreZapis, "Funguuuj 111", strlen("Funguuuj 111"));
        active_socket_write_data(data->my_socket, &bufferPreZapis);
        printf("Poslalo 2. správu\n");

        char_buffer_clear(&bufferPreZapis);
        char_buffer_append(&bufferPreZapis, "Funguuuj 222", strlen("Funguuuj 222"));
        active_socket_write_data(data->my_socket, &bufferPreZapis);
        printf("Poslalo 3. správu\n");

        char_buffer_clear(&bufferPreZapis);
        char_buffer_append(&bufferPreZapis, "Funguuuj", strlen("Funguuuj"));
        active_socket_write_data(data->my_socket, &bufferPreZapis);
        printf("Poslalo 4. správu\n");

        active_socket_write_end_message(data->my_socket);
        printf("Poslalo 5. správu\n");

        // Po použití treba zničiť buffer
        char_buffer_destroy(&bufferPreZapis);

        //nemozem ihneď zničiť socket, lebo klient si tu nemusí stihnúť prečitať
        sleep(5);
        active_socket_destroy(data->my_socket);
        sleep(10);

        //active_socket_stop_reading(data->my_socket);
    } else {
        printf("Nepripojil sa klient\n");
    }

    return NULL;
}

void vypisLogo();
void seedUser(Interpreter& interpreter, std::string user, std::string password);
void seedUsersTable(Interpreter& interpreter, std::string user);
void seedPostsTable(Interpreter& interpreter, std::string user);
void seedDatatypesTable(Interpreter& interpreter, std::string user);

int main() {
    vypisLogo();

    Interpreter interpreter;

    std::string currentUser;
    std::string password;

    pthread_t th_receive, th_write;
    struct thread_data data;
    //TODO: do štruktúry data musím dať vector active_sockets
    struct active_socket my_socket;

    //TODO: tento konstruktor pre aktiviny soket budem volat vo vlakne
    //TODO: vsetky tie aktivne sokety budem musiet zabalit do struktury, ktora bude este v sebe mat ID toho clienta, napr. jeho IP
    //TODO: ??????? treba si toto este poriadne premysliet, lebo uplne netusim ????????????
    //TODO: ALEBO kazde vlakno bude mat osobitne pridelene svoje thread_data, cize tie data nebudu zdielane medzi viacerymi vlaknami
    //TODO: jedno vlakno bude mat teda svoje vlastna data, kde bude aktivny socket PLUS budu mat aj zdielane data, v ktorych bude prikaz pre ukoncenie vsetkych vlakien, cize znicenie aktivneho socketu. To nastane, ked pride :end sprava
    active_socket_init(&my_socket);
    //TODO: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! vyskúšať najskôr. Danovi sa nedali pripojiť dvaja klienti na jeden port !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    thread_data_init(&data, 12731, &my_socket);

    pthread_create(&th_receive, NULL, process_client_data, &data);
    //TODO: th_write pojde prec
    //pthread_create(&th_write, NULL, write_to_client, &data);

    if (&my_socket != NULL) {
        printf("Soket je nastaveny\n");
        //pokial aktivny soket cita este data (nebola priajata end message od klienta), tak stale bude vytahovat data do client_pi_estimaton
        //pre viacerych klientov to musim spravit tak, ze budem vytvarat vlakna, ktore budu vykonavat to try_get_client_pi_estimation
        //budú mať nad sebou, hento že while (active_socket_is_reading(data->my_socket))
        //tieto vlakna ASI budu mat zdielane data, v ktorych bude buffer pre aktivne sockety, cize vtedy tam budem musiet riesit MUTEX pre pristup a zapis do toho buffera
        //ALEBO kazde vlakno bude mat osobitne pridelene svoje thread_data, cize tie data nebudu zdielane medzi viacerymi vlaknami
        bool pokracujCitanie = true;
        while (pokracujCitanie) {
            while (active_socket_is_reading(&my_socket)) {
                CHAR_BUFFER buf;
                char_buffer_init(&buf);
                if (active_socket_try_get_read_data(&my_socket, &buf)) {
                    printf("Precital zo socktu\n");






//                    char vypisSpravy[10000];
//                    //sscanf(buf.data, "%s", vypisSpravy);
//                    sscanf(buf.data, "%[^\t\n]", vypisSpravy); //precita vsetko okrem tabs a newlines
//                    size_t dlzkaVypisuSpravy = strlen(vypisSpravy);
//                    std::vector<char> filtrovaneData;
//                    //odstranenie prazdnych znakov na konci spravy pomocou vektora a metody strlen
//                    for (size_t i = 0; i < dlzkaVypisuSpravy; ++i) {
//                        filtrovaneData.push_back(vypisSpravy[i]);
//                    }
//
//                    printf("Sprava od klienta: ");
//                    for (char znak : filtrovaneData) {
//                        std::cout << znak;
//                    }




                    if (active_socket_is_end_message(&my_socket, &buf)) {
                        printf("UKONCOVACIA SPRAVA\n");
                        active_socket_stop_reading(&my_socket);
                        pokracujCitanie = false;
                        //TODO: tento cely while asi budu samostatne vlakna pre jednotlivych klientov
                        //TODO: asi urcite to tak bude, lebo su sa preberaju vsetky prijate spravy a aj posielaju dalsie naspat
                        active_socket_destroy(data.my_socket);
                    } else {
                        //TODO: preco mu nevadi take male pole charov???? aj tak sa tam zmesti hocico
                        //char vypisSpravy[100000];
                        //sscanf(buf.data, "%[^\t\n]", vypisSpravy); //precita vsetko okrem tabs a newlines
                        //size_t dlzkaVypisuSpravy = strlen(buf.data);
                        std::vector<char> filtrovaneData;
                        //odstranenie prazdnych znakov na konci spravy pomocou vektora a metody strlen
                        for (size_t i = 0; i < buf.size; ++i) {
                            filtrovaneData.push_back(buf.data[i]);
                        }

                        //TODO: pojde prec
//                        printf("Sprava od klienta: ");
//                        for (char znak : filtrovaneData) {
//                            std::cout << znak;
//                        }

                        std::string filtrovanyString;
                        for (size_t i = 0; i < filtrovaneData.size(); ++i) {
                            filtrovanyString += filtrovaneData[i];
                        }

                        // Use find function to find 1st position of delimiter.
                        int end = filtrovanyString.find(";");
                        while (end != -1) { // Loop until no delimiter is left in the string.
                            //Tu su tie prvotne kody ako 0;0; alebo 0;1;
                            //std::cout << filtrovanyString.substr(0, end) << std::endl;
                            filtrovanyString.erase(filtrovanyString.begin(), filtrovanyString.begin() + end + 1);
                            end = filtrovanyString.find(";");
                        }

                        //NASLEDUJE STRING bez uvodných 0;0;
                        std::string stringBezZaciatku = filtrovanyString.substr(0, end);
                        std::vector<std::string> vectorBezZaciatku;
                        end = stringBezZaciatku.find("$");
                        while (end != -1) { // Loop until no delimiter is left in the string.
                            vectorBezZaciatku.push_back(stringBezZaciatku.substr(0, end));
                            stringBezZaciatku.erase(stringBezZaciatku.begin(), stringBezZaciatku.begin() + end + 1);
                            end = stringBezZaciatku.find("$");
                        }
                        vectorBezZaciatku.push_back(stringBezZaciatku.substr(0, end));


//                        //TODO: pojde prec
//                        std::cout << "Poslpitovanyyyy vectoooor" << std::endl;
//                        for (std::string znak : vectorBezZaciatku) {
//                            std::cout << znak << std::endl;
//                        }

                        //spracovavanie spravy, ked klient NIE je prihlaseny
                        if (currentUser.empty()) {
                            if (vectorBezZaciatku[0] == "TRYLOGIN") {
                                std::cout << "Klient ziada o prihlásenie sa" << std::endl;
                                bool isLoggedIn = interpreter.tryLogin(vectorBezZaciatku[1], vectorBezZaciatku[2]);
                                sleep(1);

                                CHAR_BUFFER bufferPreZapis;
                                char_buffer_init(&bufferPreZapis);
                                if (isLoggedIn) {
                                    char_buffer_append(&bufferPreZapis, "0;0;TRYLOGIN$TRUE", strlen("0;0;TRYLOGIN$TRUE"));
                                    //POMOCOU TOHTO currentUser SA POSIELAJU SQL PRIKAZY
                                    currentUser = vectorBezZaciatku[1];
                                    password = vectorBezZaciatku[2];
                                } else {
                                    char_buffer_append(&bufferPreZapis, "0;0;TRYLOGIN$FALSE", strlen("0;0;TRYLOGIN$FALSE"));
                                }

                                active_socket_write_data(data.my_socket, &bufferPreZapis);
                            } else if (vectorBezZaciatku[0] == "TRYREGISTER") {
                                std::cout << "Klient ziada o registraciu" << std::endl;
                                bool successOnRegistration = interpreter.tryRegister(vectorBezZaciatku[1], vectorBezZaciatku[2]);
                                sleep(1);

                                CHAR_BUFFER bufferPreZapis;
                                char_buffer_init(&bufferPreZapis);
                                if (successOnRegistration) {
                                    char_buffer_append(&bufferPreZapis, "0;0;TRYREGISTER$TRUE", strlen("0;0;TRYREGISTER$TRUE"));
                                    //POMOCOU TOHTO currentUser SA POSIELAJU SQL PRIKAZY
                                    currentUser = vectorBezZaciatku[1];
                                    password = vectorBezZaciatku[2];
                                } else {
                                    char_buffer_append(&bufferPreZapis, "0;0;TRYREGISTER$FALSE", strlen("0;0;TRYREGISTER$FALSE"));
                                }

                                active_socket_write_data(data.my_socket, &bufferPreZapis);
                            } else {
                                std::cout << "Klient poslal nezmysuplnu spravu" << std::endl;
                            }
                            //spracovavanie spravy, ked klient JE prihlaseny
                        } else {
                            if (vectorBezZaciatku[0] == "TRYSQL") {
                                std::cout << "Klient ziada aktivovat SQL prikaz" << std::endl;
                                std::cout << vectorBezZaciatku[0] << std::endl;
                                std::cout << vectorBezZaciatku[1] << std::endl;
                                std::string vysledokSQLString = interpreter.run(vectorBezZaciatku[1], currentUser);

                                CHAR_BUFFER bufferPreZapis;
                                char_buffer_init(&bufferPreZapis);
                                const char* vysledokSQL = vysledokSQLString.c_str();
                                char_buffer_append(&bufferPreZapis, vysledokSQL, strlen(vysledokSQL));
                                active_socket_write_data(data.my_socket, &bufferPreZapis);
                            } else {
                                std::cout << "Klient poslal nezmysuplnu spravu" << std::endl;
                            }
                        }


                        printf("\n");
                    }
                }
            }
        }
    }

    pthread_join(th_receive, NULL);
    //TODO: th_write pojde prec
    //pthread_join(th_write, NULL);

    thread_data_destroy(&data);





//    Interpreter interpreter;
//
//    std::string currentUser;
//    std::string password;
//
//    bool isLoggedIn = false;
//
//    while(!isLoggedIn) {
//        std::cout << "username> ";
//        std::getline(std::cin, currentUser);
//
//        std::cout << currentUser << "'s password> ";
//        std::getline(std::cin, password);
//
//        //seedUser(interpreter, currentUser, password);
//
//        isLoggedIn = interpreter.tryLogin(currentUser, password);
//
//        if (!isLoggedIn) {
//            std::cout << "Invalid username or password!" << std::endl;
//        }
//    }
//
//    //seedUsersTable(interpreter, currentUser);
//    //seedPostsTable(interpreter, currentUser);
//    //seedDatatypesTable(interpreter, currentUser);
//
//    std::string command;
//
//    while (true) {
//        std::cout << "DB> ";
//        std::getline(std::cin, command);
//
//        if (command == "exit") {
//            break;
//        }
//
//        std::string result = interpreter.run(command, currentUser);
//        Decoder::decodeAndPrint(result);
//        std::cout << std::endl;
//    }

    return 0;
}

void seedUser(Interpreter& interpreter, std::string user, std::string password) {
    interpreter.run("create user " + user + " identified by " + password, "");
}

void seedUsersTable(Interpreter& interpreter, std::string user) {
    interpreter.run("create table users (id int primary key, username string not null, first_name string, last_name string, created_at date not null)", user);

    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (1 "john_doe" "John" "Doe" "2023-01-01"))", user);
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (2 "jane_smith" "Jane" "Smith" "2023-02-15"))", user);
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (3 "mike_jones" "Mike" "Jones" "2023-03-22"))", user);
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (4 "sara_williams" "Sara" "Williams" "2023-04-10"))", user);
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (5 "peter_brown" "Peter" "Brown" "2023-05-05"))", user);
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (6 "emily_white" "Emily" "White" "2023-06-18"))", user);
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (7 "alex_garcia" "Alex" "Garcia" "2023-07-30"))", user);
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (8 "olivia_martin" "Olivia" "Martin" "2023-08-12"))", user);
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (9 "david_clark" "David" "Clark" "2023-09-24"))", user);
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (10 "linda_king" "Linda" "King" "2023-10-08"))", user);
    interpreter.run(R"(insert into users (id username created_at) values (11 "user_11" "2023-11-22"))", user);
    interpreter.run(R"(insert into users (id username first_name created_at) values (12 "user_12" "John" "2023-12-05"))", user);
    interpreter.run(R"(insert into users (id username last_name created_at) values (13 "user_13" "Jones" "2024-01-15"))", user);
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (14 "user_14" "Sara" "Williams" "2024-02-28"))", user);
    interpreter.run(R"(insert into users (id username created_at) values (15 "user_15" "2024-03-10"))", user);
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (16 "user_16" "Emily" "White" "2024-04-18"))", user);
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (17 "user_17" "Alex" "Garcia" "2024-05-30"))", user);
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (18 "user_18" "Olivia" "Martin" "2024-06-12"))", user);
    interpreter.run(R"(insert into users (id username created_at) values (19 "user_19" "2024-07-24"))", user);
    interpreter.run(R"(insert into users (id username first_name last_name created_at) values (20 "user_20" "Linda" "King" "2024-08-08"))", user);
}

void seedPostsTable(Interpreter& interpreter, std::string user) {
    interpreter.run("create table posts (id int primary key, title string not null, content string, created_at date)", user);

    interpreter.run(R"(insert into posts (id title content created_at) values (1 "Lorem ipsum dolor sit amet" "Content of post 1" "2023-01-01"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (2 "Consectetur adipiscing elit" "Content of post 2" "2023-02-15"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (3 "Sed do eiusmod tempor incididunt" "Content of post 3" "2023-03-22"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (4 "Ut labore et dolore magna aliqua" "Content of post 4" "2023-04-10"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (5 "Quis nostrud exercitation ullamco" "Content of post 5" "2023-05-05"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (6 "Duis aute irure dolor in reprehenderit" "Content of post 6" "2023-06-18"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (7 "Excepteur sint occaecat cupidatat non proident" "Content of post 7" "2023-07-30"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (8 "Sunt in culpa qui officia deserunt mollit" "Content of post 8" "2023-08-12"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (9 "Laboris nisi ut aliquip ex ea commodo" "Content of post 9" "2023-09-24"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (10 "Duis aute irure dolor in reprehenderit" "Content of post 10" "2023-10-08"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (11 "Lorem ipsum dolor sit amet" "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat." "2023-11-22"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (12 "Consectetur adipiscing elit" "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur." "2023-12-05"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (13 "Sed do eiusmod tempor incididunt" "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." "2024-01-15"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (14 "Ut labore et dolore magna aliqua" "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium." "2024-02-28"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (15 "Quis nostrud exercitation ullamco" "Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit." "2024-03-10"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (16 "Duis aute irure dolor in reprehenderit" "Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit." "2024-04-18"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (17 "Excepteur sint occaecat cupidatat non proident" "At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum." "2024-05-30"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (18 "Sunt in culpa qui officia deserunt mollit" "Similique sunt in culpa qui officia deserunt mollitia animi, id est laborum et dolorum fuga." "2024-06-12"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (19 "Laboris nisi ut aliquip ex ea commodo" "Eum autem quod aut officiis debitis aut rerum necessitatibus saepe eveniet ut." "2024-07-24"))", user);
    interpreter.run(R"(insert into posts (id title content created_at) values (20 "Duis aute irure dolor in reprehenderit" "Itaque earum rerum hic tenetur a sapiente delectus, ut aut reiciendis voluptatibus.." "2024-08-08"))", user);
}

void seedDatatypesTable(Interpreter& interpreter, std::string user) {
    interpreter.run(R"(create table data (int int primary key, string string, double double, boolean boolean, date date))", user);

    interpreter.run(R"(insert into data (int string double boolean date) values (8 "adipiscing elit" 45.3 true "2001-08-17"))", user);
    interpreter.run(R"(insert into data (int string double boolean date) values (17 "sed do eiusmod" 69.8 false "2014-03-02"))", user);
    interpreter.run(R"(insert into data (int double boolean date) values (23 "" true "2009-12-05"))", user);
    interpreter.run(R"(insert into data (int string double boolean date) values (9 "tempor incididunt ut" 52.6 false "2017-06-19"))", user);
    interpreter.run(R"(insert into data (int string double) values (31 "labore et dolore" 93.2))", user);
    interpreter.run(R"(insert into data (int string double) values (14 "consectetur adipiscing" 60.7))", user);
    interpreter.run(R"(insert into data (int string double date) values (19 "quis nostrud exercitation" 75.4 "2016-09-28"))", user);
    interpreter.run(R"(insert into data (int double boolean date) values (26 49.0 false "2011-11-14"))", user);
    interpreter.run(R"(insert into data (int string date) values (11 "in voluptate velit" "2003-04-30"))", user);
    interpreter.run(R"(insert into data (int string date) values (16 "aliquip ex ea commodo" "2018-07-22"))", user);
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