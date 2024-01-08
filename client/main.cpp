#include <iostream>
#include <thread>
#include <vector>
#include "src/sockets/my_socket.h"
#include "src/Decoder.h"

#include "src/ThreadHandler.h"
#include "src/MessageBuffer.h"

void vypisLogo();

int main() {
    vypisLogo();

    MySocket* mySocket = MySocket::createConnection("frios2.fri.uniza.sk", 12730);
    ThreadHandler threadHandler;
    MessageBuffer messageBuffer;

    // Vytvorenie a spustenie vlákna pre asynchrónne čítanie dát
    std::thread readThread(&ThreadHandler::readSocketAsync, std::ref(threadHandler), mySocket->connectSocket,std::ref(messageBuffer));

    // Spustenie vlákna pre vypisovanie správ
    std::thread printThread(&ThreadHandler::printMessages, std::ref(threadHandler), std::ref(messageBuffer));

    //Spustenie vlákna, ktoré vykonáva hlavný proces
    std::thread processThread(&ThreadHandler::mainProcess, std::ref(threadHandler), mySocket, std::ref(messageBuffer));

    readThread.join();
    printThread.join();
    processThread.join();

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
