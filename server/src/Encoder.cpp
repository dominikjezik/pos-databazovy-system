#include "Encoder.h"

std::string Encoder::success(std::string message) {
    return "0;0;" + message;
}

std::string Encoder::success(std::vector<std::string> &result) {
    std::string message = "0;1;" + std::to_string(result.size()) + ";1;";
    for (const auto& item : result) {
        message += item + ";";
    }
    message.pop_back();

    return message;
}


std::string Encoder::success(std::vector<std::vector<std::string>> &result) {
    std::string message = "0;1;" + std::to_string(result.size()) + ";" + std::to_string(result[0].size()) + ";";
    for (const auto& row : result) {
        for (const auto& item : row) {
            message += item + ";";
        }
    }
    message.pop_back();

    return message;
}


std::string Encoder::success(std::string message, std::vector<std::string> &result) {
    std::string encoded = "0;2;" + std::to_string(result.size()) + ";1;" + message + ";";
    for (const auto& item : result) {
        encoded += item + ";";
    }
    encoded.pop_back();

    return encoded;
}


std::string Encoder::success(std::string message, std::vector<std::vector<std::string>> &result) {
    std::string encoded = "0;2;" + std::to_string(result.size()) + ";" + std::to_string(result[0].size()) + ";" + message + ";";
    for (const auto& row : result) {
        for (const auto& item : row) {
            encoded += item + ";";
        }
    }
    encoded.pop_back();

    return encoded;
}


std::string Encoder::error() {
    // pozn. vratena sprava musi byt urcite rozna od prazdenho stringu! (vysledok parseWhereConditions sa kontroluje ci je rozny od "")
    return "1;Chyba v prikaze!";
}


std::string Encoder::errorUnknownCommand() {
    return "1;Neznamy prikaz!";
}


std::string Encoder::error(std::string message) {
    return "1;" + message;
}
