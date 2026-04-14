//
// Created by Aurea on 04/14/26
//

#include <stdio.h>
#include "gui.h"

//visually displays the board
void displayBoard (GameState *gs, int yellowSecs, int blueSecs, Color humanColor){
//print the squares of the gameboard based on the odd/even. row+col = odd --> light row+col = even --> dark
    if ((row + col) % 2 == 0) {
        printf("dark square");
    } else {
        printf("light square");
    }

}
//visually shows the pathway of the legal moves
void dispLegalMoves(MoveList *moves){
printf("dispLegalMoves called\n");
}

//funct to move the pawn to the chosen location
Move getMove(GameState *gs){

}

//if yellow winner, display yellow, else winner is blue
void dispWin(Color winner){
if (winner == YELLOW) {
    printf("Winner is Yellow!\n");
    } else {
    printf("Winner is Blue!\n");
    }
}

void dispStalemate(void){
printf("STALEMATE! It's a draw!\n");}

void dispTimeout(Color loser){
if (loser == YELLOW){
    printf("YELLOW ran out of time. BLUE wings!\n");
    } else {
    printf("BLUE ran out of time. YELLOW wins!"\n);
    }
}

int matchupMenu(void){
    printf("Select matchup:\n");
    printf("1. User vs User\n");
    printf("2. User vs AI\n");
    printf("3. AI vs AI\n");
    int choice;
    scanf("%d", &choice);
    return choice;
}

Color colorMenu(void);

int difficultyMenu(void);

int clockMenu(void);

void aiMove(Move move);

void printError(const char *msg);

void dispUndo(void);