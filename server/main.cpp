#include <iostream>
#include "src/Interpreter.h"
#include "src/Decoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
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

void vypisLogo();
void seedUser(Interpreter& interpreter, std::string user, std::string password);
void seedUsersTable(Interpreter& interpreter, std::string user);
void seedPostsTable(Interpreter& interpreter, std::string user);
void seedDatatypesTable(Interpreter& interpreter, std::string user);

typedef struct thread_data {
    //pthread_mutex_t mutex;
    std::mutex mutex;
    short port;
} THREAD_DATA;

void thread_data_init(struct thread_data* data, short port) {
    //pthread_mutex_init(&data->mutex, NULL);
    data->port = port;
}

void thread_data_destroy(struct thread_data* data) {
    //pthread_mutex_destroy(&data->mutex);
    data->port = 0;
}

void* process_client_data(void* thread_data);
void* read_from_one_client(ACTIVE_SOCKET* my_socket);
void* process_one_client(void* thread_data, ACTIVE_SOCKET* my_socket, PASSIVE_SOCKET* passiveSocket);

int main() {
    vypisLogo();

    pthread_t th_receive;
    struct thread_data data;
    thread_data_init(&data, 12732);

    pthread_create(&th_receive, NULL, process_client_data, &data);

    pthread_join(th_receive, NULL);
    thread_data_destroy(&data);

    return 0;
}

void* process_client_data(void* thread_data) {
    struct thread_data* data = (struct thread_data*)thread_data;
    PASSIVE_SOCKET sock;
    passive_socket_init(&sock);
    passive_socket_start_listening(&sock, data->port);

    std::vector<std::thread*> threadsForClients;
    std::vector<active_socket*> activeSocketsForClients;

    while (passive_socket_is_listening(&sock)) {
        struct active_socket* my_socket = new struct active_socket();
        active_socket_init(my_socket);
        activeSocketsForClients.push_back(my_socket);
        passive_socket_wait_for_client(&sock, my_socket);
        if (!passive_socket_is_listening(&sock)) {
            break;
        }
        std::thread* th_read = new std::thread(read_from_one_client, my_socket);
        std::thread* th_process = new std::thread(process_one_client, thread_data, my_socket, &sock);
        threadsForClients.push_back(th_read);
        threadsForClients.push_back(th_process);
    }

    for (std::thread* threadForClient : threadsForClients) {
        threadForClient->join();
    }

    for (int i = 0; i < threadsForClients.size(); i++) {
        delete threadsForClients[i];
        threadsForClients[i] = nullptr;
    }

    for (int i = 0; i < activeSocketsForClients.size(); i++) {
        delete activeSocketsForClients[i];
        activeSocketsForClients[i] = nullptr;
    }

    return NULL;
}

void* read_from_one_client(ACTIVE_SOCKET* my_socket) {
    std::cout << "Cita data z klienta s aktivnym socketom: " << my_socket->socket_descriptor << std::endl;
    //tu stoji program, pokial sa z aktivneho socketu cita
    active_socket_start_reading(my_socket);
    std::cout << "Skoncil citanie dat z klienta s aktivnym socketom: " << my_socket->socket_descriptor << std::endl;

    return NULL;
}

void* process_one_client(void* thread_data, ACTIVE_SOCKET* my_socket, PASSIVE_SOCKET* passiveSocket) {
    struct thread_data *data = (struct thread_data *) thread_data;

    Interpreter interpreter;
    std::string currentUser;
    std::string password;
    if (&my_socket != NULL) {
        printf("Soket je nastaveny\n");
        bool pokracujCitanie = true;
        while (pokracujCitanie) {
            while (active_socket_is_reading(my_socket)) {
                CHAR_BUFFER buf;
                char_buffer_init(&buf);
                if (active_socket_try_get_read_data(my_socket, &buf)) {
                    printf("Precital zo socktu\n");
                    if (active_socket_is_end_message(my_socket, &buf)) {
                        printf("UKONCOVACIA SPRAVA\n");
                        active_socket_stop_reading(my_socket);
                        pokracujCitanie = false;
                        active_socket_destroy(my_socket);
                    } else {
                        std::vector<char> filtrovaneData;
                        for (size_t i = 0; i < buf.size; ++i) {
                            filtrovaneData.push_back(buf.data[i]);
                        }

                        std::string filtrovanyString;
                        for (size_t i = 0; i < filtrovaneData.size(); ++i) {
                            filtrovanyString += filtrovaneData[i];
                        }

                        std::vector<std::string> vyslednyVektorSprav;

                        int end = filtrovanyString.find(";");
                        while (end != -1) { // Loop until no delimiter is left in the string.
                            vyslednyVektorSprav.push_back(filtrovanyString.substr(0, end));
                            filtrovanyString.erase(filtrovanyString.begin(), filtrovanyString.begin() + end + 1);
                            end = filtrovanyString.find(";");
                        }
                        vyslednyVektorSprav.push_back(filtrovanyString.substr(0, end));

                        //spracovavanie spravy, ked klient NIE je prihlaseny
                        if (currentUser.empty()) {
                            if (vyslednyVektorSprav[2] == "TRYLOGIN") {
                                std::cout << "Klient ziada o prihlásenie sa" << std::endl;
                                std::unique_lock<std::mutex> lock(data->mutex);
                                bool isLoggedIn = interpreter.tryLogin(vyslednyVektorSprav[3], vyslednyVektorSprav[4]);
                                lock.unlock();
                                sleep(1);

                                CHAR_BUFFER bufferPreZapis;
                                char_buffer_init(&bufferPreZapis);
                                if (isLoggedIn) {
                                    char_buffer_append(&bufferPreZapis, "0;0;TRYLOGIN;TRUE", strlen("0;0;TRYLOGIN;TRUE"));
                                    //POMOCOU TOHTO currentUser SA POSIELAJU SQL PRIKAZY
                                    currentUser = vyslednyVektorSprav[3];
                                    password = vyslednyVektorSprav[4];
                                } else {
                                    char_buffer_append(&bufferPreZapis, "0;0;TRYLOGIN;FALSE", strlen("0;0;TRYLOGIN;FALSE"));
                                }

//                                 seedUsersTable(interpreter, currentUser);
//                                 seedPostsTable(interpreter, currentUser);
//                                 seedDatatypesTable(interpreter, currentUser);

                                active_socket_write_data(my_socket, &bufferPreZapis);
                                char_buffer_destroy(&bufferPreZapis);
                            } else if (vyslednyVektorSprav[2] == "TRYREGISTER") {
                                std::cout << "Klient ziada o registraciu" << std::endl;
                                std::unique_lock<std::mutex> lock(data->mutex);
                                bool successOnRegistration = interpreter.tryRegister(vyslednyVektorSprav[3], vyslednyVektorSprav[4]);
                                lock.unlock();
                                sleep(1);

                                CHAR_BUFFER bufferPreZapis;
                                char_buffer_init(&bufferPreZapis);
                                if (successOnRegistration) {
                                    char_buffer_append(&bufferPreZapis, "0;0;TRYREGISTER;TRUE", strlen("0;0;TRYREGISTER;TRUE"));
                                    //POMOCOU TOHTO currentUser SA POSIELAJU SQL PRIKAZY
                                    currentUser = vyslednyVektorSprav[3];
                                    password = vyslednyVektorSprav[4];
                                } else {
                                    char_buffer_append(&bufferPreZapis, "0;0;TRYREGISTER;FALSE", strlen("0;0;TRYREGISTER;FALSE"));
                                }

                                active_socket_write_data(my_socket, &bufferPreZapis);
                                char_buffer_destroy(&bufferPreZapis);
                            } else {
                                std::cout << "Klient poslal nezmysuplnu spravu" << std::endl;
                            }
                            //spracovavanie spravy, ked klient JE prihlaseny
                        } else {
                            if (vyslednyVektorSprav[2] == "TRYSQL") {
                                std::cout << "Klient ziada aktivovat SQL prikaz" << std::endl;
                                std::cout << vyslednyVektorSprav[3] << std::endl;
                                std::unique_lock<std::mutex> lock(data->mutex);
                                std::string vysledokSQLString = interpreter.run(vyslednyVektorSprav[3], currentUser);
                                lock.unlock();

                                CHAR_BUFFER bufferPreZapis;
                                char_buffer_init(&bufferPreZapis);
                                const char* vysledokSQL = vysledokSQLString.c_str();
                                char_buffer_append(&bufferPreZapis, vysledokSQL, strlen(vysledokSQL));
                                active_socket_write_data(my_socket, &bufferPreZapis);
                                char_buffer_destroy(&bufferPreZapis);
                            } else if (vyslednyVektorSprav[2] == "STOPLISTENING") {
                                if (passive_socket_is_listening(passiveSocket)) {
                                    passive_socket_stop_listening(passiveSocket);
                                    passive_socket_destroy(passiveSocket);
                                    std::cout << "Server prestal prijimat dalsich klientov" << std::endl;
                                    std::cout << "Server skonci, ked sa vsetci postupne odpoja" << std::endl;
                                } else {
                                    std::cout << "Server uz nejaku dobu neprijima dalsich klientov" << std::endl;
                                }
                            } else {
                                std::cout << "Klient poslal nezmysuplnu spravu" << std::endl;
                            }
                        }
                        printf("\n");
                    }
                }
                char_buffer_destroy(&buf);
            }
        }
    }

    return NULL;
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