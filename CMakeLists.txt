cmake_minimum_required(VERSION 3.20)
project(chess)
add_executable(chess
        src/legalMoveGen.c
        src/chess_types.c
        src/legalMoveGen.h
        src/legalMoveGenTest.c
        src/gui.c
        src/test.c
        src/legalMoveGenTest.c
)

