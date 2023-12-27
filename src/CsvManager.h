#ifndef DATABAZOVY_SYSTEM_CSVMANAGER_H
#define DATABAZOVY_SYSTEM_CSVMANAGER_H


#include <string>
#include <functional>
#include <fstream>

class CsvManager {
public:
    void loadFile(const std::string& filename, int numberOfSegments, const std::function<void(std::string*)>& action);
    void createFile(std::basic_string<char, std::char_traits<char>, std::allocator<char>> basicString, const std::function<void(std::fstream&)> &action);
    void appendToFile(const std::string& filename, const std::string& line);
    void removeLineFromFile(const std::string& filename, const std::string& lineToRemove);

private:
    std::string* parseLine(const std::string& line, int numberOfSegments);
};


#endif //DATABAZOVY_SYSTEM_CSVMANAGER_H
