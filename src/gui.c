//
// gui.c - Created by Aurea on 04/14/26
//

#include <GL/glut.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <string.h>
#include "gui.h"
#include "legalMoveGen.h"

// ─── SCREEN STATE ───────────────────────────────────────────────────────────
typedef enum {
    SCREEN_MATCHUP,
    SCREEN_COLOR,
    SCREEN_CLOCK,
    SCREEN_DIFFICULTY,
    SCREEN_GAME
} Screen;

static Screen currentScreen = SCREEN_MATCHUP;

// ─── GLOBAL STATE ────────────────────────────────────────────────────────────
static GameState *currentGameState = NULL;
static Color      humanColor       = YELLOW;
static int        matchupChoice    = 0;   // 0=UvU 1=UvAI 2=AIvAI
static int        clockChoice      = 1;   // 1/2/3 => chess.c *300
static int        difficultyChoice = 1;   // 1/2/3 => chess.c *2

// ─── MOVE TRACKING ───────────────────────────────────────────────────────────
static int    clickCount  = 0;
static Square firstClick;
static Move   pendingMove;
static int    moveReady   = 0;   // set to 1 after 2nd click

// ─── LEGAL MOVE HIGHLIGHTS ───────────────────────────────────────────────────
static MoveList highlightMoves;
static int      hasHighlight = 0;

// ─── DIMENSIONS ──────────────────────────────────────────────────────────────
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 700
#define SQUARE_SIZE   80

// ─── SETTERS ─────────────────────────────────────────────────────────────────
void setGameState(GameState *gs) { currentGameState = gs; }
void setHumanColor(Color c)      { humanColor = c; }

// ─── PUBLIC STATE QUERIES ────────────────────────────────────────────────────
int isGameReady(void)  { return currentScreen == SCREEN_GAME; }
int isMoveReady(void)  { return moveReady; }
void resetMove(void)   { moveReady = 0; clickCount = 0; hasHighlight = 0; }

// ─── DRAWING HELPERS ─────────────────────────────────────────────────────────
static void drawText(const char *s) {
    while (*s) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *s++);
}

static void drawRect(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS);
    glVertex2f(x1,y1); glVertex2f(x2,y1);
    glVertex2f(x2,y2); glVertex2f(x1,y2);
    glEnd();
}

static void drawButton(float x1, float y1, float x2, float y2, const char *txt) {
    glColor3f(0.75f,0.75f,0.75f); drawRect(x1,y1,x2,y2);
    glColor3f(0,0,0);
    glRasterPos2f(x1+20, (y1+y2)/2.0f - 6);
    drawText(txt);
}

// ─── MENU SCREENS ────────────────────────────────────────────────────────────
static void drawMatchupScreen(void) {
    glColor3f(1,1,1); drawRect(200,180,600,520);
    glColor3f(0,0,0);
    glRasterPos2f(268,485); drawText("ANTEATER CHESS  v1.0");
    glRasterPos2f(275,458); drawText("ChessEaters - UCI EECS 22L");
    glRasterPos2f(295,428); drawText("Select Matchup:");
    drawButton(220,375,580,415,"User vs User");
    drawButton(220,320,580,360,"User vs AI");
    drawButton(220,265,580,305,"AI vs AI");
}

static void drawColorScreen(void) {
    glColor3f(1,1,1); drawRect(200,200,600,500);
    glColor3f(0,0,0);
    glRasterPos2f(280,462); drawText("Choose Your Color:");
    glColor3f(0.9f,0.9f,0.0f); drawRect(220,370,580,420);
    glColor3f(0,0,0); glRasterPos2f(240,392); drawText("Yellow  (moves first)");
    glColor3f(0.0f,0.0f,0.85f); drawRect(220,305,580,355);
    glColor3f(1,1,1); glRasterPos2f(240,327); drawText("Blue");
}

static void drawClockScreen(void) {
    glColor3f(1,1,1); drawRect(200,200,600,500);
    glColor3f(0,0,0); glRasterPos2f(250,462); drawText("Choose Time Per Player:");
    drawButton(220,375,580,415,"5 minutes");
    drawButton(220,320,580,360,"10 minutes");
    drawButton(220,265,580,305,"15 minutes");
}

static void drawDifficultyScreen(void) {
    glColor3f(1,1,1); drawRect(200,200,600,500);
    glColor3f(0,0,0); glRasterPos2f(265,462); drawText("Choose AI Difficulty:");
    drawButton(220,375,580,415,"Easy");
    drawButton(220,320,580,360,"Medium");
    drawButton(220,265,580,305,"Hard");
}

// ─── DISPLAY BOARD ───────────────────────────────────────────────────────────
void displayBoard(GameState *gs, int yellowSecs, int blueSecs, Color hColor) {
    int row, col;

    for (row = 0; row < 8; row++) {
        for (col = 0; col < 10; col++) {
            // flip board so human's pieces always at bottom
            int boardRow = (hColor == YELLOW) ? (7 - row) : row;
            int boardCol = (hColor == YELLOW) ? (9 - col) : col;

            // draw square
            glColor3f((row+col)%2==0 ? 1.0f:0.0f,
                      (row+col)%2==0 ? 1.0f:0.0f,
                      (row+col)%2==0 ? 0.0f:1.0f);
            float x = col * SQUARE_SIZE;
            float y = row * SQUARE_SIZE;
            drawRect(x, y, x+SQUARE_SIZE, y+SQUARE_SIZE);

            // draw piece letter
            Piece p = gs->board[boardRow][boardCol];
            if (p.piecetype != EMPTY) {
                glColor3f(p.color==YELLOW ? 1.0f:0.9f,
                          p.color==YELLOW ? 0.85f:0.9f,
                          p.color==YELLOW ? 0.0f:1.0f);
                char letter;
                switch(p.piecetype){
                    case KING:     letter='K'; break;
                    case QUEEN:    letter='Q'; break;
                    case BISHOP:   letter='B'; break;
                    case KNIGHT:   letter='N'; break;
                    case ROOK:     letter='R'; break;
                    case ANT:      letter='A'; break;
                    case ANTEATER: letter='T'; break;
                    default:       letter='?'; break;
                }
                glRasterPos2f(x+32, y+28);
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, letter);
            }
        }
    }

    // column labels
    glColor3f(1,1,1);
    for (col = 0; col < 10; col++) {
        char lbl[2];
        lbl[0] = (hColor==BLUE) ? ('J'-col) : ('A'+col);
        lbl[1] = '\0';
        glRasterPos2f(col*SQUARE_SIZE+34, 643);
        drawText(lbl);
    }

    // clocks
    char yt[32], bt[32];
    glColor3f(1.0f,0.9f,0.0f);
    glRasterPos2f(5,670);
    sprintf(yt,"Yellow: %02d:%02d", yellowSecs/60, yellowSecs%60);
    drawText(yt);

    glColor3f(0.4f,0.7f,1.0f);
    glRasterPos2f(490,670);
    sprintf(bt,"Blue: %02d:%02d", blueSecs/60, blueSecs%60);
    drawText(bt);

    // undo button
    dispUndo();

    // legal move highlights
    if (hasHighlight) dispLegalMoves(&highlightMoves);
}

// ─── DISPLAY CALLBACK ────────────────────────────────────────────────────────
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    switch (currentScreen) {
        case SCREEN_MATCHUP:    drawMatchupScreen();    break;
        case SCREEN_COLOR:      drawColorScreen();      break;
        case SCREEN_CLOCK:      drawClockScreen();      break;
        case SCREEN_DIFFICULTY: drawDifficultyScreen(); break;
        case SCREEN_GAME:
            if (currentGameState)
                displayBoard(currentGameState, 300, 300, humanColor);
            break;
    }
    glutSwapBuffers();
}

// ─── MOUSE HANDLER ───────────────────────────────────────────────────────────
void mouseHandler(int button, int state, int x, int y) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;
    y = WINDOW_HEIGHT - y;  // flip y

    switch (currentScreen) {

        case SCREEN_MATCHUP:
            if      (x>=220&&x<=580&&y>=375&&y<=415){matchupChoice=0;currentScreen=SCREEN_COLOR;}
            else if (x>=220&&x<=580&&y>=320&&y<=360){matchupChoice=1;currentScreen=SCREEN_COLOR;}
            else if (x>=220&&x<=580&&y>=265&&y<=305){matchupChoice=2;currentScreen=SCREEN_COLOR;}
            break;

        case SCREEN_COLOR:
            if      (x>=220&&x<=580&&y>=370&&y<=420){humanColor=YELLOW;currentScreen=SCREEN_CLOCK;}
            else if (x>=220&&x<=580&&y>=305&&y<=355){humanColor=BLUE;  currentScreen=SCREEN_CLOCK;}
            break;

        case SCREEN_CLOCK:
            if      (x>=220&&x<=580&&y>=375&&y<=415) clockChoice=1;
            else if (x>=220&&x<=580&&y>=320&&y<=360) clockChoice=2;
            else if (x>=220&&x<=580&&y>=265&&y<=305) clockChoice=3;
            else break;
            currentScreen = (matchupChoice>0) ? SCREEN_DIFFICULTY : SCREEN_GAME;
            break;

        case SCREEN_DIFFICULTY:
            if      (x>=220&&x<=580&&y>=375&&y<=415){difficultyChoice=1;currentScreen=SCREEN_GAME;}
            else if (x>=220&&x<=580&&y>=320&&y<=360){difficultyChoice=2;currentScreen=SCREEN_GAME;}
            else if (x>=220&&x<=580&&y>=265&&y<=305){difficultyChoice=3;currentScreen=SCREEN_GAME;}
            break;

        case SCREEN_GAME: {
            int col = x / SQUARE_SIZE;
            int row = y / SQUARE_SIZE;
            printf("click col=%d row=%d count=%d\n", col, row, clickCount);
            if (col>=0 && col<10 && row>=0 && row<8) {
                // convert screen coords to board coords
                int boardRow = (humanColor==YELLOW) ? (7-row) : row;
                int boardCol = (humanColor==YELLOW) ? (9-col) : col;

                if (clickCount == 0) {
                    // first click: select piece
                    firstClick.file = (File)boardCol;
                    firstClick.rank = boardRow;
                    clickCount = 1;
                    hasHighlight = 0;
                    // show legal moves for this piece
                    if (currentGameState) {
                        MoveList possible = findPossibleMoves(currentGameState, firstClick);
                        if (possible.count > 0) {
                            highlightMoves = possible;
                            hasHighlight = 1;
                        } else {
                            // clicked empty square or enemy piece, reset
                            clickCount = 0;
                        }
                    }
                } else {
                    // second click: select destination
                    pendingMove.from    = firstClick;
                    pendingMove.to.file = (File)boardCol;
                    pendingMove.to.rank = boardRow;
                    moveReady  = 1;
                    clickCount = 0;
                    hasHighlight = 0;
                }
            }
            break;
        }
    }
    glutPostRedisplay();
}

// ─── PUBLIC GAME FUNCTIONS ───────────────────────────────────────────────────
int   matchupMenu(void)    { return matchupChoice; }
Color colorMenu(void)      { return humanColor; }
int   difficultyMenu(void) { return difficultyChoice; }
int   clockMenu(void)      { return clockChoice; }

void dispWin(Color winner) {
    glClear(GL_COLOR_BUFFER_BIT);
    if (currentGameState)
        displayBoard(currentGameState, 0, 0, humanColor);
    glColor3f(1,1,1); drawRect(180,260,620,390);
    const char *l = (winner==YELLOW)?"Blue checkmated!":"Yellow checkmated!";
    glColor3f(0,0,0); glRasterPos2f(270,355); drawText(l);
    const char *w = (winner==YELLOW)?"YELLOW WINS!":"BLUE WINS!";
    if(winner==YELLOW) glColor3f(0.8f,0.8f,0.0f);
    else               glColor3f(0.0f,0.0f,1.0f);
    glRasterPos2f(290,295); drawText(w);
    glutSwapBuffers();
    // keep showing until window closed
    while(1) glutMainLoopEvent();
}

void dispStalemate(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    if (currentGameState)
        displayBoard(currentGameState, 0, 0, humanColor);
    glColor3f(1,1,1); drawRect(130,260,670,390);
    glColor3f(0,0,0);
    glRasterPos2f(330,355); drawText("Stalemate!");
    glRasterPos2f(185,295); drawText("DRAW! No legal moves remaining.");
    glutSwapBuffers();
    while(1) glutMainLoopEvent();
}

void dispTimeout(Color loser) {
    glClear(GL_COLOR_BUFFER_BIT);
    if (currentGameState)
        displayBoard(currentGameState, 0, 0, humanColor);
    glColor3f(1,1,1); drawRect(180,260,620,390);
    const char *l = (loser==YELLOW)?"Yellow timed out!":"Blue timed out!";
    glColor3f(0,0,0); glRasterPos2f(260,355); drawText(l);
    const char *w = (loser==YELLOW)?"BLUE WINS!":"YELLOW WINS!";
    if(loser==YELLOW) glColor3f(0.0f,0.0f,1.0f);
    else              glColor3f(0.8f,0.8f,0.0f);
    glRasterPos2f(290,295); drawText(w);
    glutSwapBuffers();
    while(1) glutMainLoopEvent();
}

void aiMove(Move move) {
    char from[3],to[3];
    from[0]='A'+move.from.file; from[1]='0'+move.from.rank; from[2]='\0';
    to[0]  ='A'+move.to.file;   to[1]  ='0'+move.to.rank;   to[2]  ='\0';
    glColor3f(0,0,0); glRasterPos2f(320,657);
    char msg[24]; sprintf(msg,"AI: %s->%s",from,to);
    drawText(msg);
}

void printError(const char *msg) {
    glColor3f(1,0,0); glRasterPos2f(5,675); drawText(msg);
    glutSwapBuffers();
}

void dispUndo(void) {
    glColor3f(0.4f,0.4f,0.4f); drawRect(658,641,795,669);
    glColor3f(1,1,1); glRasterPos2f(692,651); drawText("UNDO");
}

void dispLegalMoves(MoveList *moves) {
    int i;
    if (!moves || moves->count==0) return;
    highlightMoves = *moves;
    hasHighlight   = 1;

    for (i=0; i<moves->count; i++) {
        int toFile   = moves->moves[i].to.file;
        int toRank   = moves->moves[i].to.rank;
        int fromFile = moves->moves[i].from.file;
        int fromRank = moves->moves[i].from.rank;

        int dFile=0, dRank=0;
        if      (toFile>fromFile) dFile= 1;
        else if (toFile<fromFile) dFile=-1;
        if      (toRank>fromRank) dRank= 1;
        else if (toRank<fromRank) dRank=-1;

        // path squares
        int cf=fromFile+dFile, cr=fromRank+dRank;
        while (cf!=toFile || cr!=toRank) {
            int sc=(humanColor==BLUE)?(9-cf):cf;
            int sr=(humanColor==BLUE)?(7-cr):cr;
            glColor3f(0.0f,0.7f,0.0f);
            drawRect(sc*SQUARE_SIZE+10, sr*SQUARE_SIZE+10,
                     sc*SQUARE_SIZE+SQUARE_SIZE-10, sr*SQUARE_SIZE+SQUARE_SIZE-10);
            cf+=dFile; cr+=dRank;
        }

        // destination square
        int sc=(humanColor==BLUE)?(9-toFile):toFile;
        int sr=(humanColor==BLUE)?(7-toRank):toRank;
        glColor3f(0.0f,1.0f,0.0f);
        drawRect(sc*SQUARE_SIZE+4, sr*SQUARE_SIZE+4,
                 sc*SQUARE_SIZE+SQUARE_SIZE-4, sr*SQUARE_SIZE+SQUARE_SIZE-4);
    }
}

// getMove: returns immediately — chess_test loop uses isMoveReady()
Move getMove(GameState *gs) {
    setGameState(gs);
    return pendingMove;
}