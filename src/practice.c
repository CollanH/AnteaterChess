//i want to use opengl and its tools:
#include <GL/glut.h>


//BLANK BLACK SCREEN OPENING FOUNDATION
//opengl calls this everytime it needs to redraw
void display(void){
glClear(GL_COLOR_BUFFER_BIT); //erase anything on it like a whiteboard with marks

glColor3f(1.0f, 1.0f, 1.0f); //rgb
glBegin(GL_QUADS); // <-- means rectangle
glVertex2f(250, 250); //left corner
glVertex2f(550,250); //right corner
glVertex2f(550,350); //top right
glVertex2f(250,350);//top left
glEnd();

//text
glColor3f(1.0f,1.0f,0.0f); //color
glRasterPos2f(375,300); //position where text starts
//store the word in a variable and loop thru it

const char *word = "YELLOW WINS!";
while (*word) 
{
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *word);
    word++;
}



glutSwapBuffers(); //pulls back the curtain to show what's been drawn. it's like flipping a page to reveal the art on the other page

}


int main(int argc, char **argv){
    /*
    gl = core OpenGL function (e.g. glColor3f, glBegin)
    glu = OpenGL Utility function (e.g. gluOrtho2D)
    glut =  GLUT toolkit function (e.g. glutInit, glutMainLoop)
    
    */
//starts up the GLUT library glut + initialize
glutInit(&argc, argv);

//initialize window with the double buffering and rgb
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

//big the windows is 800 wide, 600 height
glutInitWindowSize(800, 600);

//opens and creates window with that label:
glutCreateWindow("practice makes perfect");

//how world is mapped onto screen
glMatrixMode(GL_PROJECTION);

//like resetting phone to factory settings
glLoadIdentity();

//sets up the coordinate system
gluOrtho2D(0,800, 0, 700);

//when i need to draw smth call my "display" function, it connects your display funct to the window
glutDisplayFunc(display);

//starts the opengl and keeps window open and keeps refreshing
glutMainLoop();
}