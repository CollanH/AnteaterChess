#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include "chess_types.h"
#include "strategy.h"
#include "legalMoveGen.h"
#include "gui.h"

// function declarations
void logMove(FILE *logfile, Color color, Move move);
void init_board(GameState *gs);
bool is_legal(MoveList *moves, Move move);
bool inCheck(const GameState *gs, Color color);

// OpenGL init
void initGL(void) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 700);
}

int main(int argc, char **argv)
{
    GameState  gs;
    GameState  prevGs;
    int        hasPrev;
    int        matchup;
    Color      humanColor;
    Color      winner;
    int        depth;
    int        clockSecs;
    int        yellowSecs;
    int        blueSecs;
    time_t     lastTime;
    time_t     currentTime;
    int        elapsed;
    Move       move;
    Move      *aiMoveResult;
    MoveList   legalMoves;
    int        running;
    FILE      *logfile;

    // initialize GLUT and create window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 700);
    glutCreateWindow("Anteater Chess");
    initGL();
    glutDisplayFunc(display);
    glutMouseFunc(mouseHandler);

    logfile = fopen("chess_log.txt", "w");

    // wait for GUI menus to finish before reading values
    while (!isGameReady()) {
        glutMainLoopEvent();
        glutPostRedisplay();
    }

    // now read menu selections
    matchup   = matchupMenu();
    clockSecs = clockMenu() * 300;
    humanColor = colorMenu();
    depth = 0;

    switch (matchup)
    {
        case 1:
        case 2:
            depth = difficultyMenu() * 2;
            break;
        default:
            break;
    }

    init_board(&gs);
    setGameState(&gs);

    yellowSecs = clockSecs;
    blueSecs   = clockSecs;
    hasPrev    = 0;
    running    = 1;
    lastTime   = time(NULL);

    while (running)
    {
        glutMainLoopEvent();
        glutPostRedisplay();

        currentTime = time(NULL);
        elapsed     = (int)(currentTime - lastTime);
        lastTime    = currentTime;

        if (gs.turn == YELLOW)
            yellowSecs -= elapsed;
        else
            blueSecs -= elapsed;

        if (yellowSecs <= 0)
        {
            displayBoard(&gs, 0, blueSecs, humanColor);
            dispTimeout(YELLOW);
            break;
        }
        if (blueSecs <= 0)
        {
            displayBoard(&gs, yellowSecs, 0, humanColor);
            dispTimeout(BLUE);
            break;
        }

        displayBoard(&gs, yellowSecs, blueSecs, humanColor);
        glutSwapBuffers();

        legalMoves = legalMoveGen(&gs);

        if (legalMoves.count == 0)
        {
            if (inCheck(&gs, gs.turn))
            {
                winner = (gs.turn == YELLOW) ? BLUE : YELLOW;
                dispWin(winner);
            }
            else
            {
                dispStalemate();
            }
            break;
        }

        switch (matchup)
        {
            case 2:
                aiMoveResult = SelectBestMove(&gs, gs.turn, depth);
                if (!aiMoveResult) { running = 0; break; }
                move = *aiMoveResult;
                prevGs = gs;
                hasPrev = 1;
                aiMove(move);
                logMove(logfile, gs.turn, move);
                gs = apply_move(&gs, move);
                break;

            case 0:
            case 1:
                if (matchup == 1 && gs.turn != humanColor)
                {
                    aiMoveResult = SelectBestMove(&gs, gs.turn, depth);
                    if (!aiMoveResult) { running = 0; break; }
                    move = *aiMoveResult;
                    prevGs = gs;
                    hasPrev = 1;
                    aiMove(move);
                    logMove(logfile, gs.turn, move);
                    gs = apply_move(&gs, move);
                }
                else
                {
                    dispLegalMoves(&legalMoves);
                    move = getMove(&gs);
                    prevGs = gs;
                    hasPrev = 1;
                    logMove(logfile, gs.turn, move);
                    gs = apply_move(&gs, move);
                }
                break;
        }
    }

    if (logfile)
        fclose(logfile);

    return 0;
}

// writes one move to the log file
void logMove(FILE *logfile, Color color, Move move)
{
    (void)logfile;
    (void)color;
    (void)move;
}

// initializes the board with all pieces in starting positions
void init_board(GameState *gs)
{
    int r, f;

    for (r = 0; r < 8; r++)
        for (f = 0; f < 10; f++)
            gs->board[r][f] = (Piece){EMPTY, YELLOW};

    // blue back rank
    gs->board[0][A] = (Piece){ROOK,     BLUE};
    gs->board[0][B] = (Piece){KNIGHT,   BLUE};
    gs->board[0][C] = (Piece){BISHOP,   BLUE};
    gs->board[0][D] = (Piece){ANTEATER, BLUE};
    gs->board[0][E] = (Piece){QUEEN,    BLUE};
    gs->board[0][F] = (Piece){KING,     BLUE};
    gs->board[0][G] = (Piece){ANTEATER, BLUE};
    gs->board[0][H] = (Piece){BISHOP,   BLUE};
    gs->board[0][I] = (Piece){KNIGHT,   BLUE};
    gs->board[0][J] = (Piece){ROOK,     BLUE};

    for (f = 0; f < 10; f++)
        gs->board[1][f] = (Piece){ANT, BLUE};

    for (f = 0; f < 10; f++)
        gs->board[6][f] = (Piece){ANT, YELLOW};

    // yellow back rank
    gs->board[7][A] = (Piece){ROOK,     YELLOW};
    gs->board[7][B] = (Piece){KNIGHT,   YELLOW};
    gs->board[7][C] = (Piece){BISHOP,   YELLOW};
    gs->board[7][D] = (Piece){ANTEATER, YELLOW};
    gs->board[7][E] = (Piece){QUEEN,    YELLOW};
    gs->board[7][F] = (Piece){KING,     YELLOW};
    gs->board[7][G] = (Piece){ANTEATER, YELLOW};
    gs->board[7][H] = (Piece){BISHOP,   YELLOW};
    gs->board[7][I] = (Piece){KNIGHT,   YELLOW};
    gs->board[7][J] = (Piece){ROOK,     YELLOW};

    gs->turn              = YELLOW;
    gs->yellow_kscastle   = true;
    gs->yellow_qscastle   = true;
    gs->blue_kscastle     = true;
    gs->blue_qscastle     = true;
    gs->anteater_ate      = false;
    gs->en_passant_square = make_square(-1, A);
    gs->prev_state        = NULL;
}

// checks if a move is in the legal move list
bool is_legal(MoveList *moves, Move move)
{
    int i;
    for (i = 0; i < moves->count; i++)
    {
        if (moves->moves[i].from.rank == move.from.rank &&
            moves->moves[i].from.file == move.from.file &&
            moves->moves[i].to.rank   == move.to.rank   &&
            moves->moves[i].to.file   == move.to.file)
            return true;
    }
    return false;
}