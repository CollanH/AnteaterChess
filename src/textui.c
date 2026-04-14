//
// Created by Jordan  Le on 4/13/26.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "chess_types.h"
#include "textui.h"

#define BOARD_ROWS 8
#define BOARD_COLS 8



//takes file and returns matching letter to print on board
static char filetoChar(File f) {
    return (char)('A' + (int)f); //adding c bc of ascii value 65
}



//reverse of file to char -> takes character and convert to matching file to store in square struct
static int chartoFile(char c) {
    c = (char)toupper((unsigned char)c);
    if (c < 'A' || c > 'J') return -1; //if char not valid letter
    return c - 'A';
}



//takes piece struct and returns single char for printed board
static char pieceChar(Piece piece) {
    if (piece.piecetype == EMPTY) return '.';
    char base;
    switch (piece.piecetype) { //switch statement maps each pieceType to base letter
        case KING:
            base = 'K';
            break;
        case QUEEN:
            base = 'Q';
            break;
        case ROOK:
            base = 'R';
            break;
        case BISHOP:
            base = 'B';
        case KNIGHT:
            base = 'N';
        case ANT:
            base = 'P'; //using p for pawn
            break;
        case ANTEATER:
            base = 'T';
            break;
        default:
            base = '?';
            break;

    }
    //depending on if yellow (toupper) or blue (tolower)
    return (piece.color == YELLOW) ? base : (char)tolower((unsigned char)base);
}

//takes square struct to write to a string
static void squaretoString(Square sq, char *buf) {
    buf[0] = filetoChar(sq.file); //writes letter
    buf[1] = (char)('0' + sq.rank); //writes number
    buf[2] = '\0';
}

//horizontal line
static void hLine(void) {
    printf("  +");
    for (int c = 0; c < BOARD_COLS; c++) printf("---+");
    printf("\n");
}

//title screen menu
int matchupMenu(void) {
    int choice;
    printf("\n+----------------------------------+\n");
    printf("|      ANTEATER CHESS  v1.0        |\n");
    printf("|   ChessEaters -- UCI EECS 22L    |\n");
    printf("\n+----------------------------------+\n");

    //choosing matchup
    printf("  Choose your matchup:\n");
    printf("  1) User vs. User\n");
    printf("  2) User vs. AI\n");
    printf("  3) AI vs. AI\n");
    printf("  > "); //where the cursor ends

    //default choice is 1 - makes sure program does not crash & valid choice
    if (scanf("%d", &choice) != 1) choice = 1;
    while (getchar() != '\n');
    if (choice < 1 || choice > 3) choice = 1;
    return choice -1;
}

//color choice and returns enum yellow or blue
Color colorMenu(void) {
    int choice;
    printf("\n Choose your color: \n");
    printf("   1) Yellow (moves first)\n");
    printf("   2) Blue\n");
    printf("   > ");
    if (scanf("%d", &choice) != 1) choice = 1;
    while (getchar() != '\n');
    return (choice ==2) ? BLUE : YELLOW;
}

//choosing difficulty of AI : return maps to negamax search
int difficultyMenu(void) {
    int choice;
    printf("\n   Choose your AI's difficulty:\n");
    printf("   1) Easy\n");
    printf("   2) Medium\n");
    printf("   3) Hard\n");
    printf("   > ");
    if (scanf("%d", &choice) != 1) choice = 1;
    while (getchar() != '\n');
    if (choice < 1 || choice > 3) choice = 1;
    return choice;
}

//sets time limit per player and returns time in seconds
int clockMenu(void) {
    int choice;
    printf("\n Choose the time per player in minutes:\n");
    printf("   1) 5 minutes\n");
    printf("   2) 10 minutes\n");
    printf("   3) 15 minutes\n");
    printf("   > ");
    if (scanf("%d", &choice) != 1) choice = 1;
    while (getchar() != '\n');
    if (choice < 1 || choice > 3) choice = 1;
    return choice;
}


//puts full board to the terminal
void displayBoard(Gamestate *gs, int yellowSecs, int blueSecs, Color humanColor) {
    printf("\n");
    //printing labels on top of board
    for (int c = 0; c< BOARD_COLS; c++)
        printf(" %c  ", filetoChar((File)c));
    printf("\n");
    hline();

    //printing each row
    for (int row = 0; row < BOARD_ROWS; row++) {
        //row index to rank number
        int rankNum = BOARD_ROWS - row;
        printf("%d |", rankNum);

        //printing each piece in the row
        for (int col = 0; col < BOARD_COLS; col++) {
            printf(" %c |", pieceChar(gs->board[row][col]));
        }

        //print clocks next to rank 8 and rank 1

    }

}






