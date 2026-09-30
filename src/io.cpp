#include "io.h"
#include <fstream>
#include <stdexcept>
#include <vector>
#include <iostream>

bool Player;

std::vector<std::string> read_position(const std::string& filename) {
    std::vector<std::string> positions;
    std::ifstream file(filename);
    std::string l;
    while (std::getline(file, l)) {
        if (l.size() >= 4) {
            positions.push_back(l.substr(0, 4));
        }
    }
    file.close();

    Player = (positions.size() % 2 == 0);

    return positions;
}

void write_position(const std::string& filename, const std::string& position, const std::string& moveType) {
    std::ofstream file(filename);
    file << position << "\n";
    file.close();
}
