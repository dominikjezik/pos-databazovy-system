#include "Decoder.h"

/*
 * Nech sa vratia vysledky v tomto formate:
 * status_kod;data
 * - v pripade success:
 * 0;typ_vysledku;data
 * 0;0;sprava - v pripade odoslania spravy klientovi
 * 0;1;pocet_riadkov;pocet_stlpcov;bunka1;bunka2;bunka3;... - v pripade tabulky bez spravy
 * 0;2;pocet_riadkov;pocet_stlpcov;sprava;bunka1;bunka2;bunka3;... - v pripade tabulky so spravou
 *
 * - v pripade error:
 * 1;chybova sprava
 */

void Decoder::decodeAndPrint(std::string encodedString) {
    std::stringstream streamLine(encodedString);

    std::string segment;
    std::vector<std::string> segments;

    while(std::getline(streamLine, segment, ';')) {
        segments.push_back(segment);
    }

    if (segments.size() < 1) {
        std::cout << "Chyba pri dekodovani spravy!" << std::endl;
        return;
    }

    if (segments[0] == "0") {
        if (segments.size() < 3) {
            std::cout << "Chyba pri dekodovani spravy!" << std::endl;
            return;
        }

        if (segments[1] == "0") {
            std::cout << segments[2] << std::endl;
        } else if (segments[1] == "1") {
            if (segments.size() < 5) {
                std::cout << "Chyba pri dekodovani spravy!" << std::endl;
                return;
            }

            size_t rows = std::stoi(segments[2]);
            size_t columns = std::stoi(segments[3]);

            // odstranenie prvych 4 prvkov
            segments.erase(segments.begin(), segments.begin() + 4);

            printTable(rows, columns, segments);
        } else if (segments[1] == "2") {
            if (segments.size() < 6) {
                std::cout << "Chyba pri dekodovani spravy!" << std::endl;
                return;
            }

            // Vypis spravy
            std::cout << segments[4] << std::endl;

            size_t rows = std::stoi(segments[2]);
            size_t columns = std::stoi(segments[3]);

            // odstranenie prvych 5 prvkov
            segments.erase(segments.begin(), segments.begin() + 5);

            printTable(rows, columns, segments);
        }
    } else if (segments[0] == "1") {
        std::cout << segments[1] << std::endl;
    }

}

void Decoder::printTable(size_t rows, size_t columns, std::vector<std::string> &data) {
    // TODO: Kontrola ci sedia rozmery tabulky s datami

    // Vypocet sirky kazdeho stlpca
    std::vector<size_t> columnWidths(columns, 0);

    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < columns; j++) {
            if (data[i * columns + j].length() > columnWidths[j]) {
                columnWidths[j] = data[i * columns + j].length();
            }
        }
    }

    // Vypis horneho okraja tabulky
    std::cout << "+";
    for (size_t i = 0; i < columns; i++) {
        std::cout << std::string(columnWidths[i] + 2, '-') << "+";
    }

    std::cout << std::endl;

    // Vypis hlavicky tabulky
    for (size_t i = 0; i < columns; i++) {
        std::cout << "| " << std::setw(columnWidths[i]) << std::left << data[i] << " ";
    }

    std::cout << "|" << std::endl;

    // Vypis oddelovaca hlavicky a dat
    std::cout << "+";
    for (size_t i = 0; i < columns; i++) {
        std::cout << std::string(columnWidths[i] + 2, '-') << "+";
    }

    std::cout << std::endl;

    for (size_t i = 1; i < rows; i++) {
        for (size_t j = 0; j < columns; j++) {
            std::cout << "| " << std::setw(columnWidths[j]) << std::left << data[i * columns + j] << " ";
        }

        std::cout << "|" << std::endl;
    }

    // Vypis spodneho okraja tabulky
    std::cout << "+";
    for (size_t i = 0; i < columns; i++) {
        std::cout << std::string(columnWidths[i] + 2, '-') << "+";
    }

    std::cout << std::endl;
}
