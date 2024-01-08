//
// Created by Alan on 8. 1. 2024.
//
#include "MessageBuffer.h"
#include "sockets/my_socket.h"

#ifndef CLIENT_THREADHANDLER_H
#define CLIENT_THREADHANDLER_H


class ThreadHandler {
public:
    void readSocketAsync(SOCKET socket, MessageBuffer& messageBuffer);
    void printMessages(MessageBuffer& messageBuffer);
    void mainProcess(MySocket* socket, MessageBuffer& messageBuffer);
};


#endif //CLIENT_THREADHANDLER_H

