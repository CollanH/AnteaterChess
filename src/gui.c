//
// Created by Aurea on 04/14/26
//

#include <GL/glut.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <string.h>
#include "gui.h"

// SCREEN STATE MACHINE
typedef enum {
    SCREEN_MATCHUP,
    SCREEN_COLOR,
    SCREEN_CLOCK,
    SCREEN_DIFFICULTY,
    SCREEN_GAME
} Screen;

static Screen currentScreen = SCREEN_MATCHUP;

// GLOBAL STATE
static GameState *currentGameState = NULL; // game state pointer from chess.c
static Color     humanColor        = YELLOW;
static int       matchupChoice     = 0;    // 0=UvU, 1=UvAI, 2=AIvAI (matches chess.c)
static int       clockChoice       = 1;    // 1=5min, 2=10min, 3=15min (chess.c *300)
static int       difficultyChoice  = 1;    // 1=easy, 2=med, 3=hard (chess.c *2)

// MOUSE / MOVE TRACKING
static int    clickCount   = 0;
static Square firstClick;
static Move   pendingMove;
static int    moveReady    = 0;

// legal move highlights
static MoveList highlightMoves;
static int      hasHighlight = 0;

// DIMENSIONS
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 700
#define SQUARE_SIZE   80

// SETTERS (called by chess.c)
void setGameState(GameState *gs)  { currentGameState = gs; }
void setHumanColor(Color c)       { humanColor = c; }

// HELPERS
static void drawText(const char *str) {
    while (*str) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *str++); }
}

static void drawRect(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS);
    glVertex2f(x1, y1); glVertex2f(x2, y1);
    glVertex2f(x2, y2); glVertex2f(x1, y2);
    glEnd();
}

static void drawButton(float x1, float y1, float x2, float y2, const char *text) {
    glColor3f(0.75f, 0.75f, 0.75f);
    drawRect(x1, y1, x2, y2);
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(x1 + 20, (y1 + y2) / 2.0f - 6);
    drawText(text);
}

// MENU SCREENS
static void drawMatchupScreen(void) {
    // white panel
    glColor3f(1,1,1); drawRect(200,180,600,520);
    // title
    glColor3f(0,0,0);
    glRasterPos2f(268, 485); drawText("ANTEATER CHESS  v1.0");
    glRasterPos2f(275, 458); drawText("ChessEaters - UCI EECS 22L");
    glRasterPos2f(295, 428); drawText("Select Matchup:");
    // buttons
    drawButton(220, 375, 580, 415, "User vs User");
    drawButton(220, 320, 580, 360, "User vs AI");
    drawButton(220, 265, 580, 305, "AI vs AI");
}

static void drawColorScreen(void) {
    glColor3f(1,1,1); drawRect(200,200,600,500);
    glColor3f(0,0,0);
    glRasterPos2f(280, 462); drawText("Choose Your Color:");
    // yellow button
    glColor3f(0.9f,0.9f,0.0f); drawRect(220,370,580,420);
    glColor3f(0,0,0);
    glRasterPos2f(240,392); drawText("Yellow  (moves first)");
    // blue button
    glColor3f(0.0f,0.0f,0.85f); drawRect(220,305,580,355);
    glColor3f(1,1,1);
    glRasterPos2f(240,327); drawText("Blue");
}

static void drawClockScreen(void) {
    glColor3f(1,1,1); drawRect(200,200,600,500);
    glColor3f(0,0,0);
    glRasterPos2f(250,462); drawText("Choose Time Per Player:");
    drawButton(220,375,580,415,"5 minutes");
    drawButton(220,320,580,360,"10 minutes");
    drawButton(220,265,580,305,"15 minutes");
}

static void drawDifficultyScreen(void) {
    glColor3f(1,1,1); drawRect(200,200,600,500);
    glColor3f(0,0,0);
    glRasterPos2f(265,462); drawText("Choose AI Difficulty:");
    drawButton(220,375,580,415,"Easy");
    drawButton(220,320,580,360,"Medium");
    drawButton(220,265,580,305,"Hard");
}

// DISPLAY BOARD
void displayBoard(GameState *gs, int yellowSecs, int blueSecs, Color hColor) {
    int row, col;

    for (row = 0; row < 8; row++) {
        for (col = 0; col < 10; col++) {
            // flip board if human is blue so their pieces are at bottom
            int boardRow = (hColor == YELLOW) ? (7 - row) : row;
            int boardCol = (hColor == YELLOW) ? (9 - col) : col;

            // alternating squares
            if ((row + col) % 2 == 0)
                glColor3f(1.0f, 1.0f, 0.0f); // yellow square
            else
                glColor3f(0.0f, 0.0f, 1.0f); // blue square

            float x = col * SQUARE_SIZE;
            float y = row * SQUARE_SIZE;
            drawRect(x, y, x + SQUARE_SIZE, y + SQUARE_SIZE);

            // draw piece letter if not empty
            Piece p = gs->board[boardRow][boardCol];
            if (p.piecetype != EMPTY) {
                if (p.color == YELLOW)
                    glColor3f(1.0f, 0.85f, 0.0f); // gold
                else
                    glColor3f(0.9f, 0.9f, 1.0f);  // light blue/white

                char letter;
                switch (p.piecetype) {
                    case KING:     letter = 'K'; break;
                    case QUEEN:    letter = 'Q'; break;
                    case BISHOP:   letter = 'B'; break;
                    case KNIGHT:   letter = 'N'; break;
                    case ROOK:     letter = 'R'; break;
                    case ANT:      letter = 'A'; break;
                    case ANTEATER: letter = 'T'; break;
                    default:       letter = '?'; break;
                }
                glRasterPos2f(x + 32, y + 28);
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, letter);
            }
        }
    }

    // column labels A-J in clock strip
    glColor3f(1,1,1);
    for (col = 0; col < 10; col++) {
        char lbl[2];
        lbl[0] = (hColor == BLUE) ? ('J' - col) : ('A' + col);
        lbl[1] = '\0';
        glRasterPos2f(col * SQUARE_SIZE + 34, 643);
        drawText(lbl);
    }

    // yellow clock left
    glColor3f(1.0f, 0.9f, 0.0f);
    glRasterPos2f(5, 670);
    char yt[32];
    sprintf(yt, "Yellow: %02d:%02d", yellowSecs/60, yellowSecs%60);
    drawText(yt);

    // blue clock right
    glColor3f(0.4f, 0.7f, 1.0f);
    glRasterPos2f(490, 670);
    char bt[32];
    sprintf(bt, "Blue: %02d:%02d", blueSecs/60, blueSecs%60);
    drawText(bt);

    // undo button
    dispUndo();

    // legal move highlights
    if (hasHighlight)
        dispLegalMoves(&highlightMoves);
}

// DISPLAY CALLBACK
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    
    printf("display called, screen=%d, gs=%p\n", currentScreen, (void*)currentGameState);
    
    switch (currentScreen) {
        case SCREEN_MATCHUP:    drawMatchupScreen();    break;
        case SCREEN_COLOR:      drawColorScreen();      break;
        case SCREEN_CLOCK:      drawClockScreen();      break;
        case SCREEN_DIFFICULTY: drawDifficultyScreen(); break;
        case SCREEN_GAME:
            if (currentGameState != NULL)
                displayBoard(currentGameState, 300, 300, humanColor);
            break;
    }

    glutSwapBuffers();
    
    if (currentScreen == SCREEN_GAME)
        glutPostRedisplay();
}

// MOUSE HANDLER
void mouseHandler(int button, int state, int x, int y) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;
    y = WINDOW_HEIGHT - y; // flip y

    switch (currentScreen) {

        case SCREEN_MATCHUP:
            if      (x>=220&&x<=580&&y>=375&&y<=415){ matchupChoice=0; currentScreen=SCREEN_COLOR; }
            else if (x>=220&&x<=580&&y>=320&&y<=360){ matchupChoice=1; currentScreen=SCREEN_COLOR; }
            else if (x>=220&&x<=580&&y>=265&&y<=305){ matchupChoice=2; currentScreen=SCREEN_COLOR; }
            break;

        case SCREEN_COLOR:
            if      (x>=220&&x<=580&&y>=370&&y<=420){ humanColor=YELLOW; currentScreen=SCREEN_CLOCK; }
            else if (x>=220&&x<=580&&y>=305&&y<=355){ humanColor=BLUE;   currentScreen=SCREEN_CLOCK; }
            break;

        case SCREEN_CLOCK:
            if      (x>=220&&x<=580&&y>=375&&y<=415){ clockChoice=1; }
            else if (x>=220&&x<=580&&y>=320&&y<=360){ clockChoice=2; }
            else if (x>=220&&x<=580&&y>=265&&y<=305){ clockChoice=3; }
            else break;
            // go to difficulty if AI matchup, else start game
            currentScreen = (matchupChoice > 0) ? SCREEN_DIFFICULTY : SCREEN_GAME;
            break;

        case SCREEN_DIFFICULTY:
            if      (x>=220&&x<=580&&y>=375&&y<=415){ difficultyChoice=1; currentScreen=SCREEN_GAME; }
            else if (x>=220&&x<=580&&y>=320&&y<=360){ difficultyChoice=2; currentScreen=SCREEN_GAME; }
            else if (x>=220&&x<=580&&y>=265&&y<=305){ difficultyChoice=3; currentScreen=SCREEN_GAME; }
            break;

        case SCREEN_GAME: {
            int col = x / SQUARE_SIZE;
            int row = y / SQUARE_SIZE;
                printf("clicked col=%d row=%d clickCount=%d\n", col, row, clickCount);

            if (col >= 0 && col < 10 && row >= 0 && row < 8) {
                // flip if blue
                int boardRow = (humanColor == BLUE) ? (7 - row) : row;
                int boardCol = (humanColor == BLUE) ? (9 - col) : col;
                if (clickCount == 0) {
                    firstClick.file = (File)boardCol;
                    firstClick.rank = boardRow;
                    clickCount  = 1;
                    hasHighlight = 0;
                } else {
                    pendingMove.from     = firstClick;
                    pendingMove.to.file  = (File)boardCol;
                    pendingMove.to.rank  = boardRow;
                    moveReady   = 1;
                    clickCount  = 0;
                    hasHighlight = 0;
                }
            }
            break;
        }
    }

    glutPostRedisplay();
}

// PUBLIC FUNCTIONS (called by chess.c)

// returns 0=UvU, 1=UvAI, 2=AIvAI  (matches chess.c switch cases)
int matchupMenu(void) { return matchupChoice; }

// returns YELLOW or BLUE
Color colorMenu(void) { return humanColor; }

// returns 1, 2, or 3  (chess.c multiplies by 2 for depth)
int difficultyMenu(void) { return difficultyChoice; }

// returns 1, 2, or 3  (chess.c multiplies by 300 for seconds)
int clockMenu(void) { return clockChoice; }

// displays victory popup
void dispWin(Color winner) {
    glColor3f(1,1,1); drawRect(180,260,620,390);
    const char *l = (winner==YELLOW) ? "Blue checkmated!" : "Yellow checkmated!";
    glColor3f(0,0,0);
    glRasterPos2f(270, 355); drawText(l);
    const char *w = (winner==YELLOW) ? "YELLOW WINS!" : "BLUE WINS!";
    if (winner==YELLOW) glColor3f(0.8f,0.8f,0.0f);
    else                glColor3f(0.0f,0.0f,1.0f);
    glRasterPos2f(290, 295); drawText(w);
    glutSwapBuffers();
}

// displays stalemate popup
void dispStalemate(void) {
    glColor3f(1,1,1); drawRect(130,260,670,390);
    glColor3f(0,0,0);
    glRasterPos2f(330,355); drawText("Stalemate!");
    glRasterPos2f(185,295); drawText("DRAW! No legal moves remaining.");
    glutSwapBuffers();
}

// displays timeout popup
void dispTimeout(Color loser) {
    glColor3f(1,1,1); drawRect(180,260,620,390);
    const char *l = (loser==YELLOW) ? "Yellow timed out!" : "Blue timed out!";
    glColor3f(0,0,0);
    glRasterPos2f(260,355); drawText(l);
    const char *w = (loser==YELLOW) ? "BLUE WINS!" : "YELLOW WINS!";
    if (loser==YELLOW) glColor3f(0.0f,0.0f,1.0f);
    else               glColor3f(0.8f,0.8f,0.0f);
    glRasterPos2f(290,295); drawText(w);
    glutSwapBuffers();
}

// displays AI move text in clock strip
void aiMove(Move move) {
    char from[3], to[3];
    from[0]='A'+move.from.file; from[1]='0'+move.from.rank; from[2]='\0';
    to[0]  ='A'+move.to.file;   to[1]  ='0'+move.to.rank;   to[2]  ='\0';
    glColor3f(0,0,0);
    glRasterPos2f(320,657);
    char msg[24];
    sprintf(msg,"AI: %s -> %s", from, to);
    drawText(msg);
    glutSwapBuffers();
}

// displays red error message
void printError(const char *msg) {
    glColor3f(1,0,0);
    glRasterPos2f(5,675);
    drawText(msg);
    glutSwapBuffers();
}

// displays undo button in clock strip
void dispUndo(void) {
    glColor3f(0.4f,0.4f,0.4f); drawRect(658,641,795,669);
    glColor3f(1,1,1);
    glRasterPos2f(692,651); drawText("UNDO");
}

// highlights legal moves with path projection for sliding pieces
void dispLegalMoves(MoveList *moves) {
    int i;
    if (!moves || moves->count == 0) return;

    // store for display loop
    highlightMoves = *moves;
    hasHighlight   = 1;

    for (i = 0; i < moves->count; i++) {
        int toFile   = moves->moves[i].to.file;
        int toRank   = moves->moves[i].to.rank;
        int fromFile = moves->moves[i].from.file;
        int fromRank = moves->moves[i].from.rank;

        // direction of movement
        int dFile = 0, dRank = 0;
        if      (toFile > fromFile) dFile =  1;
        else if (toFile < fromFile) dFile = -1;
        if      (toRank > fromRank) dRank =  1;
        else if (toRank < fromRank) dRank = -1;

        // highlight path squares (not including source)
        int curFile = fromFile + dFile;
        int curRank = fromRank + dRank;
        while (curFile != toFile || curRank != toRank) {
            int sc = (humanColor==BLUE) ? (9-curFile) : curFile;
            int sr = (humanColor==BLUE) ? (7-curRank) : curRank;
            float px = sc * SQUARE_SIZE;
            float py = sr * SQUARE_SIZE;
            glColor3f(0.0f, 0.7f, 0.0f);
            drawRect(px+10, py+10, px+SQUARE_SIZE-10, py+SQUARE_SIZE-10);
            curFile += dFile;
            curRank += dRank;
        }

        // highlight destination square (brighter)
        int sc = (humanColor==BLUE) ? (9-toFile) : toFile;
        int sr = (humanColor==BLUE) ? (7-toRank) : toRank;
        float px = sc * SQUARE_SIZE;
        float py = sr * SQUARE_SIZE;
        glColor3f(0.0f, 1.0f, 0.0f);
        drawRect(px+4, py+4, px+SQUARE_SIZE-4, py+SQUARE_SIZE-4);
    }
}
// returns 1 when player has finished all menus and game is ready to start
int isGameReady(void) {
    return currentScreen == SCREEN_GAME;
}

// waits for player to click two squares, returns move
Move getMove(GameState *gs) {
  moveReady = 0;
    clickCount = 0;
    while (!moveReady) {
        glutMainLoopEvent();
        glutPostRedisplay();
    }
    return pendingMove;
}