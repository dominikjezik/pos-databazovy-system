#include <sstream>
#include <vector>
#include "CsvManager.h"


void CsvManager::loadFile(const std::string& filename, int numberOfSegments, const std::function<void(std::string*)> &action) {
    std::fstream file (filename, std::ios::in);

    if (!file.is_open()) {
        throw std::runtime_error("Subor " + filename + " sa nepodarilo otvorit.");
    }

    std::string line;

    while (getline(file, line)) {
        auto segments = parseLine(line, numberOfSegments);

        action(segments);

        delete[] segments;
    }

    file.close();
}


void CsvManager::createFile(std::basic_string<char, std::char_traits<char>, std::allocator<char>> basicString, const std::function<void(std::fstream&)> &action) {
    std::fstream file(basicString, std::ios::out);

    action(file);

    file.close();
}


void CsvManager::appendToFile(const std::string& filename, const std::string& line) {
    std::ofstream file(filename, std::ios::app);
    file << line << std::endl;
    file.close();
}

void CsvManager::removeLineFromFile(const std::string &filename, const std::string &lineToRemove) {
    // Nacitaj subor do pamate
    std::vector<std::string> lines;

    std::fstream file(filename, std::ios::in);

    if (!file.is_open()) {
        throw std::runtime_error("Subor " + filename + " sa nepodarilo otvorit.");
    }

    std::string line;

    while (getline(file, line)) {
        lines.push_back(line);
    }

    file.close();

    // Vymazanie riadku
    for (int i = 0; i < lines.size(); i++) {
        if (lines[i] == lineToRemove) {
            lines.erase(lines.begin() + i);
            break;
        }
    }

    // Zapísanie riadkov späť do súboru
    file.open(filename, std::ios::out);

    if (!file.is_open()) {
        throw std::runtime_error("Subor " + filename + " sa nepodarilo otvorit.");
    }

    for (auto line : lines) {
        file << line << std::endl;
    }

    file.close();
}

std::string* CsvManager::parseLine(const std::string& line, int numberOfSegments) {
    std::stringstream streamLine(line);

    std::string segment;

    auto segments = new std::string[6];

    int i = 0;
    while (i < numberOfSegments && std::getline(streamLine, segment, ';'))
    {
        segments[i] = segment;
        ++i;
    }

    return segments;
}
