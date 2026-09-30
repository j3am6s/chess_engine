#ifndef IO_H
#define IO_H

#include <string>
#include <vector>

//Determines if we are white (true) or black (false) according to prof's file
extern bool Player;

//Reads prof's file
std::vector<std::string> read_position(const std::string& filename);

//Writes the next position in the prof's file
void write_position(const std::string& filename, const std::string& position, const std::string& moveType);

#endif