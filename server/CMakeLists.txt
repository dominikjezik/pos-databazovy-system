cmake_minimum_required(VERSION 3.0)
project(databazovy_system)

set(CMAKE_CXX_STANDARD 17)

add_executable(databazovy_system main.cpp
        src/FileManager.cpp
        src/FileManager.h
        src/DBMS.cpp
        src/DBMS.h
        src/Table.h
        src/User.h
        src/Interpreter.h
        src/Interpreter.cpp
        src/CsvManager.cpp
        src/CsvManager.h
        src/Permission.h
        src/Encoder.h
        src/Encoder.cpp
        src/Decoder.h
        src/Decoder.cpp)

target_link_libraries(databazovy_system pthread)
