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
#define BOARD_COLS 10



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
void displayBoard(GameState *gs, int yellowSecs, int blueSecs, Color humanColor) {
    printf("\n");
    //printing labels on top of board
    for (int c = 0; c< BOARD_COLS; c++)
        printf(" %c  ", filetoChar((File)c));
    printf("\n");
    hLine();

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
        if (rankNum == 8)
            printf("   Blue  : %d:%02d", blueSecs/60, blueSecs%60);
        if (rankNum == 1)
            printf("   Yellow: %d:%02d", yellowSecs/60, yellowSecs%60);
        printf("\n");
        hLine();
    }

    //printing labels across the bottom
    printf("    ");
    for (int c = 0; c < BOARD_COLS; c++)
        printf(" %c  ", filetoChar((File)c));
    printf("\n");

    //printing legend so that each letter is known
    printf("\n  Pieces (UPPER=Yellow / lower=Blue):\n");
    printf("  K/k=King  Q/q=Queen  R/r=Rook  B/b=Bishop\n");
    printf("  N/n=Knight  P/p=Ant  T/t=Anteater  .=Empty\n");

    //printing whose turn it is
    const char *turnName = (gs->turn == YELLOW) ? "YELLOW" : "BLUE";
    printf("\n >> %s to move.", turnName);
    if (gs->turn == humanColor)
        printf("  (your turn)");
    printf("\n");

    //printing if castling is allowed for either player
    printf("  Castling -- Yellow: %s Blue: %s\n",
        gs->yellow_castled ? "done" : "available",
        gs->blue_castled ? "done" : "available");

    //printing en passant square if one is currently active, if rank ==0: no en passant
    if (gs->en_passant_square.rank != 0) {
        char buf[4];
        squaretoString(gs->en_passant_square, buf);
        printf("  En passant square: %s\n", buf);
    }

    //printing anteater capture if the anteater just ate
    if (gs->anteater_ate)
        printf("  [Anteater chain capture in progress]\n");

    //printing if there is an undo
    if (gs->prev_state != NULL)
        printf("  u = undo available]\n");
}

//prints all legal destination for piece the player selected 
void dispLegalMoves(MoveList *moves) {
    if (moves -> count ==0) {
        printf("  No legal moves for this piece.\n");
        return;
    }

    printf("  Legal destinations: ");
    for (int i = 0; i < moves->count; i++) {
        char buf[4];
        squaretoString(moves->moves[i].to, buf);
        printf("%s", buf);
        if (i < moves->count - 1)
            printf(", ");
    }

    printf("\n");
}

//returns a Move after reading command from player
Move getMove(GameState *gs) {
    Move result;
    memset(&result, 0, sizeof(result));
    char input[64];
    char fromTok[8], toTok[8];

    const char *turnName = (gs->turn == YELLOW) ? "YELLOW" : "BLUE";
    printf("\n [%s] Enter move (e.g. F2 F4) or u = undo, h = help, q = quit: ", turnName);
    fflush(stdout);

    //fgets reads line and returns NULL if input error
    if (!fgets(input, sizeof(input), stdin)) {
        result.from.file = (File)TUI_CMD_QUIT;
        return result;
    }

    //removing newline fgets adds at end
    input[strcspn(input, "\n")] = 0;

    //skip spaces the player typed before command
    char *trimmed = input;
    while (*trimmed == ' ') trimmed++;

    //checking for single character
    if (strlen(trimmed) == 1) {
        char cmd = (char)tolower((unsigned char)trimmed[0]);
        if (cmd == 'u') {
            result.from.file = (File)TUI_CMD_UNDO;
            return result;
        }
        if (cmd == 'q') {
            result.from.file = (File)TUI_CMD_QUIT;
            return result;
        }
        if (cmd == 'h') {
            result.from.file = (File)TUI_CMD_HELP;
            return result;
        }
    }   


    //parse move in format of F2 F4
    if (sscanf(trimmed, "%7s %7s", fromTok, toTok) == 2) {
        int fromFile = chartoFile(fromTok[0]);
        int fromRank = fromTok[1] - '0';
        int toFile = chartoFile(toTok[0]);
        int toRank = toTok[1] - '0';

        if (fromFile >= 0 && fromRank >= 1 && fromRank <= 8 && toFile >= 0 && toRank >= 8) {
            result.from.file= (File)fromFile;
            result.from.rank = fromRank;
            result.to.file= (File)toFile;
            result.to.rank = toRank;
            return result;
        }
    }

    //nothing matches
    printError("Unrecognised input. Type h for help.");
    result.from.file = (File)TUI_CMD_HELP;
    return result;
}

//prints victory banner when checkmate found
void dispWin(Color winner) {
    const char *w = (winner == YELLOW) ? "YELLOW" : "BLUE";
    const char *l = (winner == YELLOW) ? "Blue" : "Yellow";
    printf("\n  +------------------------------+\n");
    printf(  "  | %s checkmated!               \n", l);
    printf(  "  | %s WINS!                    |\n", w);
    printf(  "  +------------------------------+\n\n");
}

//printing stalemate menu if draw detected
void dispStalemate(void) {
    printf("\n  +------------------------------+\n");
    printf(  "  |      STALEMATE -- DRAW!      |\n");
    printf(  "  |  No legal moves remaining.   |\n");
    printf(  "  +------------------------------+\n");
}

//prints timeout banner if player's clock hits 0
void dispTimeout(Color loser) {
    const char *l = (loser == YELLOW) ? "Yellow" : "Blue";
    const char *w = (loser == YELLOW) ? "BLUE" : "YELLOW";
    printf("\n +------------------------------+\n");
    printf(  " |  %s timed out!               \n", l);
    printf(  " |  %s WINS!                   |\n", w);
    printf(  " +------------------------------+\n\n");
}

//prints the move the ai just played
void aiMove(Move move) {
    char fromBuf[4], toBuf[4];
    squaretoString(move.from, fromBuf);
    squaretoString(move.to, toBuf);
    printf("  AI played: %s %s\n", fromBuf, toBuf);
}

//prints error message
void printError(const char *msg) {
    printf("  [!] %s\n", msg);
}

//prints command reference card for player
void printHelp(void) {
    printf("\n  --- HOW TO MOVE ----------------------------------------\n");
    printf("  Type the square your piece is on, then the square\n");
    printf("  you want to move it to, separated by a space.\n");
    printf("  Example:  F2 F4  moves the piece at F2 to F4\n");
    printf("  --------------------------------------------------------\n");
    printf("  COMMANDS:\n");
    printf("  <File><Rank> <File><Rank>   Move a piece, e.g. F2 F4\n");
    printf("  u                           Undo last move\n");
    printf("  h                           Show this help\n");
    printf("  q                           Quit the game\n");
    printf("  --------------------------------------------------------\n");
    printf("  FILE (column) : A B C D E F G H I J  (left to right)\n");
    printf("  RANK (row)    : 1-8                  (bottom to top)\n");
    printf("  --------------------------------------------------------\n");
    printf("  PIECES  (UPPER = Yellow   lower = Blue)\n");
    printf("  K/k  King        Q/q  Queen\n");
    printf("  R/r  Rook        B/b  Bishop\n");
    printf("  N/n  Knight      P/p  Ant (Pawn)\n");
    printf("  T/t  Anteater    .    Empty square\n");
    printf("  --------------------------------------------------------\n\n");
}





