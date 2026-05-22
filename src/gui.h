#ifndef GUI_H
#define GUI_H

#include "chess_types.h"

int guiInit(void);
void guiQuit(void);

void setGameState(GameState *gs);
void setHumanColor(Color c);

int gameModeMenu(void);
int matchupMenu(void);
int colorMenu(void);
int difficultyMenu(void);
int clockMenu(void);
void addMoveLog(Color color, Move move, Color humanColor); 

void displayBoard(GameState *gs, int yellowSecs, int blueSecs, Color humanColor);
void dispLegalMoves(MoveList *moves);
PieceType dispPromotion(void); 


Move getMove(GameState *gs);

void dispWin(Color winner);
void dispStalemate(void);
void dispTimeout(Color loser);


void aiMove(Move move);
void printError(const char *msg);
void dispUndo(void);
// polling
typedef struct {
    int undo;
    int stopChain;
    int ret;
} GuiInput;
GuiInput pollGuiInput(void);
int wasReturnPressed(void);


extern int stopChainPressed; 
#endif
