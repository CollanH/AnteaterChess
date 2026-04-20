//
// gui.h - Created by Aurea on 04/14/26
//

#ifndef GUI_H
#define GUI_H

#include "chess_types.h"

// ── Setters ──────────────────────────────────────────────────────────────────
// gives GUI the current game state to display
void setGameState(GameState *gs);
// tells GUI which color human player chose (for board flip)
void setHumanColor(Color c);

// ── State queries ─────────────────────────────────────────────────────────────
// returns 1 when player has finished all menus and game is ready
int isGameReady(void);
// returns 1 when player has clicked two squares (move is ready)
int isMoveReady(void);
// resets move tracking (call before waiting for human move)
void resetMove(void);

// ── Board display ─────────────────────────────────────────────────────────────
// redraws full board with pieces, clocks, labels
void displayBoard(GameState *gs, int yellowSecs, int blueSecs, Color humanColor);

// ── Legal move highlighting ───────────────────────────────────────────────────
// highlights legal destination squares with path projection
void dispLegalMoves(MoveList *moves);

// ── Move input ────────────────────────────────────────────────────────────────
// returns the move the player clicked (call after isMoveReady() returns 1)
Move getMove(GameState *gs);

// ── End screens ───────────────────────────────────────────────────────────────
void dispWin(Color winner);
void dispStalemate(void);
void dispTimeout(Color loser);

// ── Menu functions (return stored choices) ────────────────────────────────────
// returns 0=UvU 1=UvAI 2=AIvAI
int matchupMenu(void);
// returns YELLOW or BLUE
Color colorMenu(void);
// returns 1/2/3 (chess.c multiplies by 2 for depth)
int difficultyMenu(void);
// returns 1/2/3 (chess.c multiplies by 300 for seconds)
int clockMenu(void);

// ── Other display functions ───────────────────────────────────────────────────
void aiMove(Move move);
void printError(const char *msg);
void dispUndo(void);

// ── OpenGL callbacks (registered by chess.c) ─────────────────────────────────
void display(void);
void mouseHandler(int button, int state, int x, int y);

#endif