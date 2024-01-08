//
// Created by Alan on 8. 1. 2024.
//

#include "ThreadHandler.h"
#include "Decoder.h"

void ThreadHandler::printMessages(MessageBuffer &messageBuffer) {
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
                        if (message == "0;0;TRYLOGIN;TRUE") {
                            std::cout << "Uspesne prihlasenie!" << std::endl;
                            messageBuffer.isLoggedIn = true;
                        } else {
                            std::cout << "Nespravne meno alebo heslo!" << std::endl;
                            messageBuffer.isLoggedIn = false;
                        }
                        messageBuffer.iSWaitingForLogin = false;
                        messageBuffer.cakajNaOznamOPrihlaseni.notify_one();

                    } else if (messageBuffer.isWaitingForRegister) {
                        if (message == "0;0;TRYREGISTER;TRUE") {
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

                // Odstránenie správy z bufferu (s vrátane '~')
                messageBuffer.buffer.erase(messageBuffer.buffer.begin(), nullTerminator + 1);
            }
        }
    }
}

void ThreadHandler::readSocketAsync(SOCKET socket, MessageBuffer &messageBuffer) {
    while (true) {
        char buffer[3000];
        int bytesRead = recv(socket, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            std::unique_lock<std::mutex> lock(messageBuffer.mutex);
            messageBuffer.buffer.insert(messageBuffer.buffer.end(), buffer, buffer + bytesRead);

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

void ThreadHandler::mainProcess(MySocket *mySocket, MessageBuffer &messageBuffer) {
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
            std::string posielajuciString = "0;0;TRYLOGIN;"+currentUser+";"+password;
            mySocket->sendData(posielajuciString);
            std::cout << "Poslala sa ziadost o login" << std::endl;
            lock.lock();
            messageBuffer.iSWaitingForLogin = true;
            //caka pokial pokial je iSWaitingForLogin stale na true.
            messageBuffer.cakajNaOznamOPrihlaseni.wait(lock, [&messageBuffer] { return !messageBuffer.iSWaitingForLogin; });
            if (messageBuffer.iSWaitingForLogin) {
                std::cout << "NASTALA CHYBA. DO TEJTO VETVY SA NEMAL DOSTAT!!!" << std::endl;
            }

            lock.unlock();
        } else if (sposobPrihlasenia == 1) {
            std::string posielajuciString = "0;0;TRYREGISTER;"+currentUser+";"+password;
            mySocket->sendData(posielajuciString);
            std::cout << "Poslala sa ziadost o registracii" << std::endl;
            lock.lock();
            messageBuffer.isWaitingForRegister = true;
            messageBuffer.cakajNaOznamORegistracii.wait(lock, [&messageBuffer] { return !messageBuffer.isWaitingForRegister; });
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

        //tu je lock už kvoli continue
        if (prikaz.find(';') != std::string::npos) {
            std::cout << "sql prikaz nemoze obsahovat ;" << std::endl;
            continue;
        }

        if (prikaz == "exit") {
            mySocket->sendEndMessage();
            std::cout << "Koniec spojenia" << std::endl;
            messageBuffer.isEnd = true;
            messageBuffer.dontShowEndMessage = true;
        } else if (prikaz == "stop server listening") {
            std::string posielajuciString = "0;0;STOPLISTENING";
            mySocket->sendData(posielajuciString);
            std::cout << "Poslala sa ziadost o vypnuti pocuvania servera" << std::endl;
        } else {
            std::string posielajuciString = "0;0;TRYSQL;"+prikaz;
            mySocket->sendData(posielajuciString);
            std::cout << "Poslala sa ziadost o prikaz" << std::endl;
            messageBuffer.isWaitingForSQL = true;
            messageBuffer.cakajNaVypisPrikazu.wait(lock, [&messageBuffer] { return !messageBuffer.isWaitingForSQL; });

        }
    }
    lock.unlock();
}
