cmake_minimum_required(VERSION 3.18)
project(client)

set(CMAKE_CXX_STANDARD 17)

add_executable(client main.cpp
        src/sockets/my_socket.h
        src/sockets/my_socket.cpp)

target_link_libraries(client wsock32 ws2_32)