//
// Created by Aurea on 04/14/26
//

#include <GL/glut.h>
#include <stdio.h>
#include "gui.h"
#include <string.h>

// global variables to track mouse clicks
static int clickCount = 0;
static Square firstClick;
static Move pendingMove;
static int moveReady = 0;

// window dimensions in pixels
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 700

// size of each square on the board in pixels
#define SQUARE_SIZE 80

// called automatically when user clicks
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // flip y coordinate since OpenGL starts from bottom
        y = WINDOW_HEIGHT - y;
        
        // convert pixel coordinates to board square
        int col = x / SQUARE_SIZE;
        int row = y / SQUARE_SIZE;
        
        // make sure click is within board boundaries
        if (col >= 0 && col < 10 && row >= 0 && row < 8) {
            if (clickCount == 0) {
                // first click - select piece
                firstClick.file = (File)col;
                firstClick.rank = row;
                clickCount = 1;
            } else {
                // second click - select destination
                pendingMove.from = firstClick;
                pendingMove.to.file = (File)col;
                pendingMove.to.rank = row;
                moveReady = 1;
                clickCount = 0;
            }
        }
    }
}


void displayBoard(GameState *gs, int yellowSecs, int blueSecs, Color humanColor) {
    
    // loop through all 8 rows
    for (int row = 0; row < 8; row++) {
        // loop through all 10 columns
        for (int col = 0; col < 10; col++) {
            
            // draw light or dark square based on row+col
            if ((row + col) % 2 == 0) {
                glColor3f(1.0f, 1.0f, 0.0f); // cream/light square
            } else {
                glColor3f(0.0f, 0.0f, 1.0f); // brown/dark square
            }
            
            // calculate where this square goes on screen
            // multiply by SQUARE_SIZE (80) to convert grid position to pixels
            float x = col * SQUARE_SIZE;
            float y = row * SQUARE_SIZE;
            
            // draw the square using 4 corners
            glBegin(GL_QUADS);
                glVertex2f(x, y);                              // bottom left
                glVertex2f(x + SQUARE_SIZE, y);               // bottom right
                glVertex2f(x + SQUARE_SIZE, y + SQUARE_SIZE); // top right
                glVertex2f(x, y + SQUARE_SIZE);               // top left
            glEnd();
            
            // get the piece on this square
            Piece p = gs->board[row][col];
            
            // only draw something if square is not empty
            if (p.piecetype != EMPTY) {
                
                // set piece color - yellow pieces are gold, blue pieces are blue
                if (p.color == YELLOW) {
                    glColor3f(1.0f, 0.85f, 0.0f); // gold color for yellow
                } else {
                    glColor3f(0.0f, 0.5f, 1.0f);  // blue color for blue
                }
                
                // pick the letter to represent each piece type
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
                
                // position the letter in the center of the square
                glRasterPos2f(x + 30, y + 30);
                
                // draw the letter on screen
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, letter);
            }
        }
    }
    
    // draw the clocks at the bottom of the screen (in the 60px leftover space)
    // yellow clock on the left
    glColor3f(1.0f, 0.85f, 0.0f); // gold color
    glRasterPos2f(50, 655);
    char yellowTime[30];
    sprintf(yellowTime, "Yellow: %02d:%02d", yellowSecs / 60, yellowSecs % 60);
    char *yt = yellowTime;
    while (*yt) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *yt);
        yt++;
    }
    
    // blue clock on the right
    glColor3f(0.0f, 0.5f, 1.0f); // blue color
    glRasterPos2f(550, 655);
    char blueTime[30];
    sprintf(blueTime, "Blue: %02d:%02d", blueSecs / 60, blueSecs % 60);
    char *bt = blueTime;
    while (*bt) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *bt);
        bt++;
    }
}


void dispWin(Color winner) {
    // white rectangle
    glColor3f(1.0f, 1.0f, 1.0f); //white
    glBegin(GL_QUADS); //for objs
    glVertex2f(250, 270); //bot left
    glVertex2f(550, 270); //bot right
    glVertex2f(550, 370); //top right
    glVertex2f(250, 370); //top left
    glEnd();

    // first line - who got checkmated
    const char *l = (winner == YELLOW) ? "Blue checkmated!" : "Yellow checkmated!";
    glColor3f(0.0f, 0.0f, 0.0f); // black text
    glRasterPos2f(310, 340); //where text is
    while (*l) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *l);
        l++;
    }

    // second line - who wins
    const char *w = (winner == YELLOW) ? "YELLOW WINS!" : "BLUE WINS!";
    if (winner == YELLOW) {
        glColor3f(1.0f, 1.0f, 0.0f); // yellow
    } else {
        glColor3f(0.0f, 0.0f, 1.0f); // blue
    }
    glRasterPos2f(310, 300); //where text is written
    while (*w) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *w);
        w++;
    }
}


//printing stalemate menu if draw detected
void dispStalemate(void) {
     // white rectangle
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(150, 270);
    glVertex2f(650, 270);
    glVertex2f(650, 370);
    glVertex2f(150, 370);
    glEnd();

    // first line
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(310, 340);
    const char *stalemate = "Stalemate!";
    while (*stalemate) { 
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *stalemate); 
        stalemate++; }

    // second line
    glRasterPos2f(220, 300);
    const char *draw = "DRAW! No legal moves remaining.";
    while (*draw) {
         glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *draw); 
         draw++; }
}

// displays timeout screen
void dispTimeout(Color loser) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(250, 270);
    glVertex2f(550, 270);
    glVertex2f(550, 370);
    glVertex2f(250, 370);
    glEnd();

    const char *l = (loser == YELLOW) ? "Yellow timed out!" : "Blue timed out!";
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(280, 340);
    while (*l) {
         glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *l); 
         l++; }

    const char *w = (loser == YELLOW) ? "BLUE WINS!" : "YELLOW WINS!";
    if (loser == YELLOW) {
        glColor3f(0.0f, 0.0f, 1.0f);
    } else {
        glColor3f(1.0f, 1.0f, 0.0f);
    }
    glRasterPos2f(310, 300);
    while (*w) { 
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *w);
         w++;
         }
}
// displays matchup menu
int matchupMenu(void) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(200, 200);
    glVertex2f(600, 200);
    glVertex2f(600, 500);
    glVertex2f(200, 500);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(300, 450);
    const char *title = "ANTEATER CHESS";
    while (*title) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *title); title++; }

    glRasterPos2f(220, 400);
    const char *op1 = "1. User vs User";
    while (*op1) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op1); op1++; }

    glRasterPos2f(220, 360);
    const char *op2 = "2. User vs AI";
    while (*op2) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op2); op2++; }

    glRasterPos2f(220, 320);
    const char *op3 = "3. AI vs AI";
    while (*op3) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op3); op3++; }

    glutSwapBuffers();
    int choice;
    scanf("%d", &choice);
    if (choice < 1 || choice > 3) choice = 1;
    return choice;
}

// displays color selection menu
Color colorMenu(void) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(200, 250);
    glVertex2f(600, 250);
    glVertex2f(600, 450);
    glVertex2f(200, 450);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(300, 400);
    const char *title = "Choose your color:";
    while (*title) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *title); title++; }

    glColor3f(1.0f, 1.0f, 0.0f);
    glRasterPos2f(220, 350);
    const char *op1 = "1. Yellow (moves first)";
    while (*op1) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op1); op1++; }

    glColor3f(0.0f, 0.0f, 1.0f);
    glRasterPos2f(220, 300);
    const char *op2 = "2. Blue";
    while (*op2) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op2); op2++; }

    glutSwapBuffers();
    int choice;
    scanf("%d", &choice);
    return (choice == 2) ? BLUE : YELLOW;
}

// displays difficulty menu
int difficultyMenu(void) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(200, 200);
    glVertex2f(600, 200);
    glVertex2f(600, 500);
    glVertex2f(200, 500);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(280, 450);
    const char *title = "Choose difficulty:";
    while (*title) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *title); title++; }

    glRasterPos2f(220, 400);
    const char *op1 = "1. Easy";
    while (*op1) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op1); op1++; }

    glRasterPos2f(220, 360);
    const char *op2 = "2. Medium";
    while (*op2) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op2); op2++; }

    glRasterPos2f(220, 320);
    const char *op3 = "3. Hard";
    while (*op3) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op3); op3++; }

    glutSwapBuffers();
    int choice;
    scanf("%d", &choice);
    if (choice < 1 || choice > 3) choice = 1;
    return choice;
}

// displays clock menu
int clockMenu(void) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(200, 200);
    glVertex2f(600, 200);
    glVertex2f(600, 500);
    glVertex2f(200, 500);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(250, 450);
    const char *title = "Choose time per player:";
    while (*title) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *title); title++; }

    glRasterPos2f(220, 400);
    const char *op1 = "1. 5 minutes";
    while (*op1) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op1); op1++; }

    glRasterPos2f(220, 360);
    const char *op2 = "2. 10 minutes";
    while (*op2) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op2); op2++; }

    glRasterPos2f(220, 320);
    const char *op3 = "3. 15 minutes";
    while (*op3) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *op3); op3++; }

    glutSwapBuffers();
    int choice;
    scanf("%d", &choice);
    if (choice < 1 || choice > 3) choice = 1;
    return choice * 300;
}

// displays AI move
void aiMove(Move move) {
    char from[3], to[3];
    from[0] = 'A' + move.from.file;
    from[1] = '0' + move.from.rank;
    from[2] = '\0';
    to[0] = 'A' + move.to.file;
    to[1] = '0' + move.to.rank;
    to[2] = '\0';

    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(50, 670);
    const char *msg = "AI played: ";
    while (*msg) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *msg); msg++; }
    char *f = from;
    while (*f) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *f); f++; }
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ' ');
    char *t = to;
    while (*t) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *t); t++; }
}

// displays error message
void printError(const char *msg) {
    glColor3f(1.0f, 0.0f, 0.0f);
    glRasterPos2f(50, 675);
    while (*msg) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *msg); msg++; }
}

// displays undo button
void dispUndo(void) {
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(650, 10);
    glVertex2f(790, 10);
    glVertex2f(790, 50);
    glVertex2f(650, 50);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(670, 25);
    const char *btn = "UNDO";
    while (*btn) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *btn); btn++; }
}

// highlights legal move squares
void dispLegalMoves(MoveList *moves) {
    for (int i = 0; i < moves->count; i++) {
        float x = moves->moves[i].to.file * SQUARE_SIZE; //col of destination sq
        float y = moves->moves[i].to.rank * SQUARE_SIZE; //row of destination sq

        glColor3f(0.0f, 1.0f, 0.0f); // green highlight
        glBegin(GL_QUADS); //green rectangle
        glVertex2f(x, y);
        glVertex2f(x + SQUARE_SIZE, y);
        glVertex2f(x + SQUARE_SIZE, y + SQUARE_SIZE);
        glVertex2f(x, y + SQUARE_SIZE);
        glEnd();
    }
}

// gets move from player mouse click
Move getMove(GameState *gs) {
    moveReady = 0;
    glutMouseFunc(mouse);
    return pendingMove;
}

