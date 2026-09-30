cmake_minimum_required(VERSION 3.15)
project(michelangelo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(michelangelo
    src/main.cpp
    src/io.cpp
    src/board.h
    src/pieces.h
    src/io.h
)
