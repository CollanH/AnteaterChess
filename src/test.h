//
// Created by clair on 4/14/2026.
//
#include "chess_types.h"

#ifndef CHESS_TEST_H
#define CHESS_TEST_H


/*
 * HOW TO USE:
 * K king
 * Q queen
 * A anteater
 * B bishop
 * N Knight
 * R rook
 * P ant
 * . is empty
 * uppercase is YELLOW
 * lowercase is BLUE
 *
 * starts with rank 0 (top of the board from yellows side) and goes left to right
 * from yellows perspective
 *
 * you can pass through strings like this:
 *
 * char str[81] =
    ".........."
    ".........."
    ".........."
    ".........."
    ".........."
    "R...K....."
    ".........."
    "..........";
 * NOTE: GameState values like castling rights will default to beginning of game defaults
 * to revoke castling rights or note en passant you must edit the game state after this function
 */
GameState string_to_gs(char board_string[81]);

#endif //CHESS_TEST_H
