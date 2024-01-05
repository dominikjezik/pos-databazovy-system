#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <random>
#include <vector>
#include <string>
#include <condition_variable>
#include <Winsock2.h>

#ifdef __cplusplus
extern "C" {
#include "src/sockets/my_socket.h"
}
#endif

struct MessageBuffer {
    std::vector<char> buffer;
    std::mutex mutex;
    std::condition_variable condVar;
    bool disconnectFlag = false; // Flag pre odpojenie zo servera
};

// Funkcia pre asynchrónne čítanie dát zo soketu
void ReadSocketAsync(SOCKET socket, MessageBuffer& messageBuffer) {
    while (true) {
        char buffer[1024];
        int bytesRead = recv(socket, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            std::unique_lock<std::mutex> lock(messageBuffer.mutex);
            messageBuffer.buffer.insert(messageBuffer.buffer.end(), buffer, buffer + bytesRead);

            // Pridanie nulového znaku na koniec vektora
            //messageBuffer.buffer.push_back('*');
            //messageBuffer.buffer.push_back('X');
            //messageBuffer.buffer.push_back('X');

            std::cout << "Obsah vektora: ";
            for (size_t i = 0; i < messageBuffer.buffer.size(); ++i) {
                std::cout << messageBuffer.buffer[i] << " ";
            }


            std::cout << std::endl;
            lock.unlock();
            messageBuffer.condVar.notify_one();

            // Ak je prijatá správa ":end", okamžite oznam odpojenie
            if (strstr(buffer, ":end") != nullptr) {
                //ten sleep tu musi byt, lebo inak sa nestihnu vypisat vsetky spravy v metode PrintMessages
                Sleep(5);
                messageBuffer.disconnectFlag = true;
                messageBuffer.condVar.notify_one();
                break;
            }
        } else if (bytesRead == 0) {
            std::cout << "Pripojenie zatvorene serverom." << std::endl;
            messageBuffer.disconnectFlag = true; // Nastavenie flagu na odpojenie zo servera
            messageBuffer.condVar.notify_one();   // Oznámenie všetkým vláknami, že je čas skončiť
            break;
        } else {
            std::cerr << "Chyba pri cítani dat zo soketu." << std::endl;
            messageBuffer.disconnectFlag = true; // Nastavenie flagu na odpojenie zo servera
            messageBuffer.condVar.notify_one();   // Oznámenie všetkým vláknami, že je čas skončiť
            break;
        }
    }


//    while (true) {
//        char buffer[1024];
//        int bytesRead = recv(socket, buffer, sizeof(buffer), 0);
//        if (bytesRead > 0) {
//            std::unique_lock<std::mutex> lock(messageBuffer.mutex);
//            messageBuffer.buffer.insert(messageBuffer.buffer.end(), buffer, buffer + bytesRead);
//
//            // Pridanie nulového znaku na koniec vektora
//            messageBuffer.buffer.push_back('\0');
//
//            std::cout << "Obsah vektora: ";
//            for (size_t i = 0; i < messageBuffer.buffer.size(); ++i) {
//                std::cout << messageBuffer.buffer[i] << " ";
//            }
//            std::cout << std::endl;
//
//            lock.unlock();
//            messageBuffer.condVar.notify_one();
//
//            // Ak je prijatá správa ":end", okamžite oznam odpojenie
//            if (strstr(buffer, ":end") != nullptr) {
//                messageBuffer.disconnectFlag = true;
//                messageBuffer.condVar.notify_one();
//                break;
//            }
//        } else if (bytesRead == 0) {
//            std::cout << "Pripojenie zatvorené serverom." << std::endl;
//            messageBuffer.disconnectFlag = true;
//            messageBuffer.condVar.notify_one();
//            break;
//        } else {
//            std::cerr << "Chyba pri čítaní dát zo soketu." << std::endl;
//            messageBuffer.disconnectFlag = true;
//            messageBuffer.condVar.notify_one();
//            break;
//        }
//    }
}

// Funkcia pre vypisovanie správ zo štruktúry
void PrintMessages(MessageBuffer& messageBuffer) {
//    while (true) {
//        std::unique_lock<std::mutex> lock(messageBuffer.mutex);
//        messageBuffer.condVar.wait(lock, [&messageBuffer] { return !messageBuffer.buffer.empty() || messageBuffer.disconnectFlag; });
//
//        if (messageBuffer.disconnectFlag) {
//            std::cout << "Odpojenie zo servera." << std::endl;
//            break;
//        }
//
////        //std::size_t pos = messageBuffer.buffer.find('\0');
////        std::size_t pos = std::distance(messageBuffer.buffer.begin(), std::find(messageBuffer.buffer.begin(), messageBuffer.buffer.end(), '\0'));
////        if (pos != std::string::npos) {
////            std::string message = std::string(messageBuffer.buffer.begin(), messageBuffer.buffer.begin() + pos);
////            std::cout << "Prijatá správa: " << message << std::endl;
////
////            // Odstránenie správy z bufferu (s vrátane '\0')
////            messageBuffer.buffer.erase(messageBuffer.buffer.begin(), messageBuffer.buffer.begin() + pos + 1);
////        }
//
//        if (!messageBuffer.buffer.empty()) {
//            //std::cout << "Ide citat." << std::endl;
//            std::size_t pos = std::distance(messageBuffer.buffer.begin(), std::find(messageBuffer.buffer.begin(), messageBuffer.buffer.end(), '\0'));
//            if (pos != messageBuffer.buffer.size()) {
//                std::string message = std::string(messageBuffer.buffer.begin(), messageBuffer.buffer.begin() + pos);
//                std::cout << "Prijatá správa: " << message << std::endl;
//
//                // Odstránenie správy z bufferu (s vrátane '\0')
//                if (messageBuffer.buffer.begin() + pos + 1 != messageBuffer.buffer.end()) {
//                    messageBuffer.buffer.erase(messageBuffer.buffer.begin(), messageBuffer.buffer.begin() + pos + 1);
//                } else {
//                    messageBuffer.buffer.clear();  // Buffer je prázdny
//                }
//            }
//        }
//    }


    while (true) {
        std::unique_lock<std::mutex> lock(messageBuffer.mutex);
        messageBuffer.condVar.wait(lock, [&messageBuffer] { return !messageBuffer.buffer.empty() || messageBuffer.disconnectFlag; });

        if (messageBuffer.disconnectFlag) {
            std::cout << "Odpojenie zo servera." << std::endl;
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
                    std::cout << "Prijata sprava: " << message << std::endl;
                }

                // Odstránenie správy z bufferu (s vrátane '\0')
                messageBuffer.buffer.erase(messageBuffer.buffer.begin(), nullTerminator + 1);
            }
        }
    }


//    while (true) {
//        std::unique_lock<std::mutex> lock(messageBuffer.mutex);
//        messageBuffer.condVar.wait(lock, [&messageBuffer] { return !messageBuffer.buffer.empty() || messageBuffer.disconnectFlag; });
//
//        if (messageBuffer.disconnectFlag) {
//            std::cout << "Odpojenie zo servera." << std::endl;
//            break;
//        }
//
//        if (!messageBuffer.buffer.empty()) {
//            std::vector<char>::iterator nullTerminator = std::find(messageBuffer.buffer.begin(), messageBuffer.buffer.end(), '\0');
//            if (nullTerminator != messageBuffer.buffer.end()) {
//                // Extrahujte reťazec od začiatku po prvý nulový znak
//                std::string message(messageBuffer.buffer.begin(), nullTerminator);
//
//                // Preskočte výpis ":end" správy
//                if (message != ":end") {
//                    std::cout << "Prijatá správa: " << message << std::endl;
//                }
//
//                // Odstránenie správy z bufferu (s vrátane '\0')
//                messageBuffer.buffer.erase(messageBuffer.buffer.begin(), nullTerminator + 1);
//            }
//        }
//    }
}

class ThreadData {
public:
    ThreadData(long long replicationsCount, int bufferCapacity, MySocket* serverSocket);
private:
    MySocket* serverSocket;
};

ThreadData::ThreadData(long long replicationsCount, int bufferCapacity, MySocket* serverSocket) :
        serverSocket(serverSocket) {

}

int main() {
    MySocket* mySocket = MySocket::createConnection("frios2.fri.uniza.sk", 12735);

    //ThreadData data(3000, 10, mySocket);
    //std::thread thProduce(produce, std::ref(data));

    MessageBuffer messageBuffer;

    // Vytvorenie a spustenie vlákna pre asynchrónne čítanie dát
    std::thread readThread(ReadSocketAsync, mySocket->connectSocket, std::ref(messageBuffer));


    // Spustenie vlákna pre vypisovanie správ
    std::thread printThread(PrintMessages, std::ref(messageBuffer));

    std::string naServer1 = "POSIELANIE NA SERVER 1";
    mySocket->sendData(naServer1);
    std::cout << "Poslalo 1. spravu na server" << std::endl;

    std::string naServer2 = "POSIELANIE NA SERVER 2";
    mySocket->sendData(naServer2);
    std::cout << "Poslalo 2. spravu na server" << std::endl;

    std::string naServer3 = "POSIELANIE NA SERVER 3";
    mySocket->sendData(naServer3);
    std::cout << "Poslalo 3. spravu na server" << std::endl;

    mySocket->sendEndMessage();

    //consume(data);
    //thProduce.join();

    readThread.join();
    printThread.join();

    delete mySocket;
    mySocket = nullptr;

    return 0;
}
