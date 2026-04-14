// 
//Created by Aurea on 04/14/26
//

#ifndef GUI_H
#define GUI_H
#include "chess_types.h"

void displayBoard(GameState *gs, int yellowSecs, int BlueSecs, Color humanColor);

void dispLegalMoves(MoveList *moves);

Move getMove(GameState *gs);

void dispWin(Color winner);

void dispStalemate(void);

void dispTimeout(Color loser);

int matchupMenu(void);

Color colorMenu(void);

int difficultyMenu(void);

int clockMenu(void);

void aiMove(Move move);

void printError(const char *msg);

void dispUndo(void);


#endif
