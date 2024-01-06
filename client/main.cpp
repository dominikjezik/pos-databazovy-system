#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <random>
#include <vector>
#include <string>
#include <condition_variable>
#include <Winsock2.h>
#include "src/sockets/my_socket.h"
#include "src/Decoder.h"
#include <limits>

struct MessageBuffer {
    std::vector<char> buffer;
    std::mutex mutex;
    std::condition_variable condVar;
    std::condition_variable cakajNaOznamOPrihlaseni;
    std::condition_variable cakajNaOznamORegistracii;
    std::condition_variable cakajNaUplneUkoncenieVlakien;
    std::condition_variable cakajNaVypisPrikazu;
    bool disconnectFlag = false; // Flag pre odpojenie zo servera
    bool iSWaitingForLogin = false;
    bool isWaitingForRegister = false;
    bool isWaitingForSQL = false;
    bool isLoggedIn = false;
    bool isEnd = false;
    bool dontShowEndMessage = false;
};

// Funkcia pre asynchrónne čítanie dát zo soketu
void ReadSocketAsync(SOCKET socket, MessageBuffer& messageBuffer) {
    while (true) {
        char buffer[3000];
        int bytesRead = recv(socket, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            std::unique_lock<std::mutex> lock(messageBuffer.mutex);
            messageBuffer.buffer.insert(messageBuffer.buffer.end(), buffer, buffer + bytesRead);

            //TODO: pojde prec
//            std::cout << "Obsah vektora: ";
//            for (size_t i = 0; i < messageBuffer.buffer.size(); ++i) {
//                std::cout << messageBuffer.buffer[i];
//            }
//            std::cout << std::endl;

            // Ak je prijatá správa ":end", okamžite oznam odpojenie
            if (strstr(buffer, ":end") != nullptr) {
                //ten sleep tu musi byt, lebo inak sa nestihnu vypisat vsetky spravy v metode PrintMessages
                Sleep(5);
                messageBuffer.disconnectFlag = true;
                break;
            }

            lock.unlock();
            messageBuffer.condVar.notify_one();
        } else if (bytesRead == 0) {
            if (!messageBuffer.dontShowEndMessage) {
                std::cout << "Pripojenie zatvorene serverom." << std::endl;
            }
            std::unique_lock<std::mutex> lock(messageBuffer.mutex);
            messageBuffer.disconnectFlag = true; // Nastavenie flagu na odpojenie zo servera
            lock.unlock();
            messageBuffer.condVar.notify_one();   // Oznámenie všetkým vláknami, že je čas skončiť
            break;
        } else {
            if (!messageBuffer.dontShowEndMessage) {
                std::cerr << "Chyba pri cítani dat zo soketu." << std::endl;
            }
            std::unique_lock<std::mutex> lock(messageBuffer.mutex);
            messageBuffer.disconnectFlag = true; // Nastavenie flagu na odpojenie zo servera
            lock.unlock();
            messageBuffer.condVar.notify_one();   // Oznámenie všetkým vláknami, že je čas skončiť
            break;
        }
    }
}

// Funkcia pre vypisovanie správ zo štruktúry
void PrintMessages(MessageBuffer& messageBuffer) {
    while (true) {
        std::unique_lock<std::mutex> lock(messageBuffer.mutex);
        messageBuffer.condVar.wait(lock, [&messageBuffer] { return !messageBuffer.buffer.empty() || messageBuffer.disconnectFlag; });

        if (messageBuffer.disconnectFlag) {
            if (!messageBuffer.dontShowEndMessage) {
                std::cout << "Odpojenie zo servera." << std::endl;
            }
            messageBuffer.cakajNaUplneUkoncenieVlakien.notify_one();
            break;
        }

        if (!messageBuffer.buffer.empty()) {
            std::vector<char>::iterator nullTerminator = std::find(messageBuffer.buffer.begin(), messageBuffer.buffer.end(), '~');
            if (nullTerminator != messageBuffer.buffer.end()) {
                // Extrahujte reťazec od začiatku po prvý nulový znak
                std::string message(messageBuffer.buffer.begin(), nullTerminator);
                // std::cout << "Prijata sprava: " << message << std::endl;

                // Preskočte výpis ":end" správy
                if (message != ":end") {
                    if (messageBuffer.iSWaitingForLogin) {
                        if (message == "0;0;TRYLOGIN$TRUE") {
                            std::cout << "Uspesne prihlasenie!" << std::endl;
                            messageBuffer.isLoggedIn = true;
                        } else {
                            std::cout << "Nespravne meno alebo heslo!" << std::endl;
                            messageBuffer.isLoggedIn = false;
                        }
                        messageBuffer.iSWaitingForLogin = false;
                        messageBuffer.cakajNaOznamOPrihlaseni.notify_one();

                    } else if (messageBuffer.isWaitingForRegister) {
                        if (message == "0;0;TRYREGISTER$TRUE") {
                            std::cout << "Uspesna registracia!" << std::endl;
                            messageBuffer.isLoggedIn = true;
                        } else {
                            std::cout << "Dany pouzivatel uz existuje!" << std::endl;
                            messageBuffer.isLoggedIn = false;
                        }
                        messageBuffer.isWaitingForRegister = false;
                        messageBuffer.cakajNaOznamORegistracii.notify_one();



                    } else if (messageBuffer.isWaitingForSQL) {
                        Decoder::decodeAndPrint(message);
                        messageBuffer.isWaitingForSQL = false;
                        messageBuffer.cakajNaVypisPrikazu.notify_one();
                    } else {
                        std::cout << "Prijata sprava: " << message << std::endl;
                    }
                }

                // Odstránenie správy z bufferu (s vrátane '\0')
                messageBuffer.buffer.erase(messageBuffer.buffer.begin(), nullTerminator + 1);
            }
        }
    }
}

void vypisLogo();

int main() {
    vypisLogo();

    MySocket* mySocket = MySocket::createConnection("frios2.fri.uniza.sk", 12731);

    MessageBuffer messageBuffer;

    // Vytvorenie a spustenie vlákna pre asynchrónne čítanie dát
    std::thread readThread(ReadSocketAsync, mySocket->connectSocket, std::ref(messageBuffer));

    // Spustenie vlákna pre vypisovanie správ
    std::thread printThread(PrintMessages, std::ref(messageBuffer));

    std::string currentUser;
    std::string password;
    int sposobPrihlasenia;

    std::unique_lock<std::mutex> lock(messageBuffer.mutex);
    while(!messageBuffer.isLoggedIn) {
        lock.unlock();
        std::cout << "0 - prihlasit sa " << std::endl;
        std::cout << "1 - registrovat sa" << std::endl;
        std::cout << "2 - zrusit spojenie" << std::endl;
        std::cout << "DB> ";
        std::cin >> sposobPrihlasenia;
        std::cin.ignore();

        if (sposobPrihlasenia == 0 || sposobPrihlasenia == 1) {
            std::cout << "pouzivatelske meno> ";
            std::getline(std::cin, currentUser);

            std::cout << "heslo> ";
            std::getline(std::cin, password);
        }

        if (sposobPrihlasenia == 0) {
            std::string posielajuciString = "0;0;TRYLOGIN$"+currentUser+"$"+password;
            mySocket->sendData(posielajuciString);
            std::cout << "Poslala sa ziadost o login" << std::endl;
            lock.lock();
            messageBuffer.iSWaitingForLogin = true;
            //caka pokial pokial je iSWaitingForLogin stale na true.
            messageBuffer.cakajNaOznamOPrihlaseni.wait(lock, [&messageBuffer] { return !messageBuffer.iSWaitingForLogin; });
            //std::cout << "ISIEL DALEEEJ" << std::endl;
            if (messageBuffer.iSWaitingForLogin) {
                std::cout << "NASTALA CHYBA. DO TEJTO VETVY SA NEMAL DOSTAT!!!" << std::endl;
            }

            lock.unlock();
        } else if (sposobPrihlasenia == 1) {
            std::string posielajuciString = "0;0;TRYREGISTER$"+currentUser+"$"+password;
            mySocket->sendData(posielajuciString);
            std::cout << "Poslala sa ziadost o registracii" << std::endl;
            lock.lock();
            //messageBuffer.iSWaitingForRegister = true;
            messageBuffer.isWaitingForRegister = true;
            messageBuffer.cakajNaOznamORegistracii.wait(lock, [&messageBuffer] { return !messageBuffer.isWaitingForRegister; });
            //std::cout << "ISIEL DALEEEJ" << std::endl;
            if (messageBuffer.isWaitingForRegister) {
                std::cout << "NASTALA CHYBA. DO TEJTO VETVY SA NEMAL DOSTAT!!!" << std::endl;
            }

            lock.unlock();
        } else {
            std::cout << "Koniec spojenia" << std::endl;
            lock.lock();
            mySocket->sendEndMessage();
            messageBuffer.isLoggedIn = true;
            messageBuffer.isEnd = true;
            messageBuffer.dontShowEndMessage = true;
            lock.unlock();

        }
        lock.lock();
    }
    lock.unlock();

    std::string prikaz;
    lock.lock();
    while(!messageBuffer.isEnd) {
        lock.unlock();

        std::cout << "DB> ";
        std::getline(std::cin, prikaz);

        lock.lock();
        if (prikaz.find(';') != std::string::npos) {
            std::cout << "sql prikaz nemoze obsahovat bodkociarku" << std::endl;
            continue;
        }

        if (prikaz == "exit") {
            mySocket->sendEndMessage();
            std::cout << "Koniec spojenia" << std::endl;
            messageBuffer.isEnd = true;
            messageBuffer.dontShowEndMessage = true;
        } else {
            //TODO: zmenit ciarky za nieco ine lebo v prikazoch sa pouzivaju
            //TODO: na KLIENTOVI kontrolovat, či je tam podkočiarka. Inak vratit, že sa v priakze nemôže nachádzať ;
            std::string posielajuciString = "0;0;TRYSQL$"+prikaz;
            mySocket->sendData(posielajuciString);
            std::cout << "Poslala sa ziadost o prikaz" << std::endl;
            messageBuffer.isWaitingForSQL = true;
            messageBuffer.cakajNaVypisPrikazu.wait(lock, [&messageBuffer] { return !messageBuffer.isWaitingForSQL; });

        }
    }
    lock.unlock();

    readThread.join();
    printThread.join();

    delete mySocket;
    mySocket = nullptr;

    return 0;
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
