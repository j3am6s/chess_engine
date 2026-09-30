#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include <cstdlib>
#include <random>
#include <chrono>

#include "pieces.h"
#include "board.h"
#include "io.h"

int main(int argc, char* argv[]){
    board* b = new board();
    b->initializeBoard();
     const std::string filename="history.csv";

     std::vector<std::string> positions=read_position(filename);
     int count = 0;
     for (const std::string& position : positions) {
         b->move2(position[0], position[1] - '0', position[2], position[3] - '0', count % 2 == 0);
         b->positionHistory.push_back(b->calculateZobristHash(Player));
         count++;
     }
     std::string next=b->getNextMove(Player);
     write_position("move.csv", next, b->moveType);

    return 0;
}

