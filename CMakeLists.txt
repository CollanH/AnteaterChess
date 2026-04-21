cmake_minimum_required(VERSION 3.16)
project(chess C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(chess
        src/chess.c
        src/strategy.c
        src/eval.c
        src/legalMoveGen.c
        src/chess_types.c
        src/gui.c
)

target_include_directories(chess PRIVATE src)

target_compile_options(chess PRIVATE -Wall -g)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

target_link_libraries(chess PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf m)