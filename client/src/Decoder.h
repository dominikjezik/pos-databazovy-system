#ifndef DATABAZOVY_SYSTEM_DECODER_H
#define DATABAZOVY_SYSTEM_DECODER_H

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>

class Decoder {
public:
    static void decodeAndPrint(std::string encodedString);
private:
    static void printTable(size_t rows, size_t columns, std::vector<std::string>& data);
};


#endif //DATABAZOVY_SYSTEM_DECODER_H
