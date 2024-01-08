#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <string>
#include <condition_variable>
#include <Winsock2.h>

#ifndef CLIENT_MESSAGEBUFFER_H
#define CLIENT_MESSAGEBUFFER_H

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

#endif //CLIENT_MESSAGEBUFFER_H
