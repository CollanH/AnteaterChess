#include <GL/glut.h>
#include "gui.h"
#include <stdio.h>

// create a test GameState (I replace this later)
GameState testState;

// initialize OpenGL settings
void initGL(void) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 700);
}

int main(int argc, char **argv) {

    // initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 700);
    glutCreateWindow("Anteater Chess");

    initGL();

    // use display + mouse from gui.c
    glutDisplayFunc(display);
    glutMouseFunc(mouseHandler);

    // start event loop
    glutMainLoop();

    return 0;
}