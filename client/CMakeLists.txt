cmake_minimum_required(VERSION 3.18)
project(skuska10_sokety_klient)

set(CMAKE_CXX_STANDARD 17)

add_executable(skuska10_sokety_klient main.cpp
        src/sockets/my_socket.h
        src/sockets/my_socket.cpp)

target_link_libraries(skuska10_sokety_klient wsock32 ws2_32)

