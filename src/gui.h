//
// Created by Aurea on 04/14/26
//

#ifndef GUI_H
#define GUI_H

#include "chess_types.h"

// gives the GUI the current game state to display (optional - chess.c passes gs directly)
void setGameState(GameState *gs);

// tells the GUI which color the human player chose (for board orientation)
void setHumanColor(Color c);

// called by chess.c every loop to redraw the board with pieces and clocks
void displayBoard(GameState *gs, int yellowSecs, int blueSecs, Color humanColor);

// highlights legal destination squares with path projection (called before getMove)
void dispLegalMoves(MoveList *moves);

// waits for player to click a piece then a destination, returns the move
Move getMove(GameState *gs);

// displays victory popup when checkmate detected
void dispWin(Color winner);

// displays draw popup when stalemate detected
void dispStalemate(void);

// displays timeout popup when clock hits zero
void dispTimeout(Color loser);

// returns matchup choice: 0=UvU, 1=UvAI, 2=AIvAI
int matchupMenu(void);

// returns color choice: YELLOW or BLUE
Color colorMenu(void);

// returns difficulty choice: 1=easy, 2=medium, 3=hard (chess.c multiplies by 2)
int difficultyMenu(void);

// returns clock choice: 1=5min, 2=10min, 3=15min (chess.c multiplies by 300)
int clockMenu(void);

// visually displays the AI's move in the clock strip
void aiMove(Move move);

// displays red error message for invalid actions
void printError(const char *msg);

// displays undo button in the clock strip
void dispUndo(void);

// OpenGL callbacks registered by chess.c
void display(void);
void mouseHandler(int button, int state, int x, int y);

int isGameReady(void);


#endif