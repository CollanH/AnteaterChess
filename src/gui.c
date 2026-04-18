//
// Created by Aurea on 04/14/26
//

#include <GL/glut.h>
#include <stdio.h>
#include "gui.h"

// window dimensions in pixels
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 700

// size of each square on the board in pixels
#define SQUARE_SIZE 80


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
    char yellowTime[20];
    sprintf(yellowTime, "Yellow: %02d:%02d", yellowSecs / 60, yellowSecs % 60);
    char *yt = yellowTime;
    while (*yt) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *yt);
        yt++;
    }
    
    // blue clock on the right
    glColor3f(0.0f, 0.5f, 1.0f); // blue color
    glRasterPos2f(550, 655);
    char blueTime[20];
    sprintf(blueTime, "Blue: %02d:%02d", blueSecs / 60, blueSecs % 60);
    char *bt = blueTime;
    while (*bt) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *bt);
        bt++;
    }
}
