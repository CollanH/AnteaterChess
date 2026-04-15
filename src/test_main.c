#include <GL/glut.h>
#include "gui.h"

// create a fake gamestate just for testing
GameState testState;

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    displayBoard(&testState, 300, 300, YELLOW);
    glutSwapBuffers();
}

void initGL(void) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 700);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 700);
    glutCreateWindow("Anteater Chess");
    initGL();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}