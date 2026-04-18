//
// Created by Jordan  Le on 4/13/26.
//

#ifndef CHESS_TEXTUI_H
#define CHESS_TEXTUI_H

#include "chess_types.h"

#define TUI_CMD_UNDO -1
#define TUI_CMD_QUIT -2
#define TUI_CMD_HELP -3

void displayBoard(GameState *gs, int yellowSecs, int blueSecs, Color humanColor);  //displays full board to terminal
void dispLegalMoves(MoveList *moves); //prints legal destination squares for piece selected
Move getMove(GameState *gs); //returns a move after reading command from player
void dispWin (Color winner); //prints win banner when checkmated
void dispStalemate(void); //prints stalemate banner if there is a draw detected
void dispTimeout(Color loser); //prints timeout banner if player's clock is 0
int matchupMenu(void); //title screen menu for matchup choice from player
Color colorMenu(void); //color choice that returns enum yellow/blue 
int difficultyMenu(void); //ai difficulty choice: return maps to negamax
int clockMenu(void); //timer choice by player : returns time in seconds
void aiMove(Move move); //prints the move that ai just did 
void printError(const char *msg); //error message used in getMove
void printHelp(void); //prints reference for player 
#endif //CHESS_TEXTUI_H
