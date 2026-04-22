//
// chess_test.c - test integration file (copy of chess.c with GUI wiring)
// DO NOT push as chess.c - show Collan the changes needed
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include "chess_types.h"
#include "strategy.h"
#include "legalMoveGen.h"
#include "gui.h"

void logMove(FILE *logfile, Color color, Move move);
void init_board(GameState *gs);
bool is_legal(MoveList *moves, Move move);
bool inCheck(const GameState *gs, Color color);

void initGL(void) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 700);
}

int main(int argc, char **argv)
{
    GameState gs;
    int       matchup;
    Color     humanColor;
    Color     winner;
    int       depth;
    int       clockSecs;
    int       yellowSecs;
    int       blueSecs;
    time_t    lastTime;
    time_t    currentTime;
    int       elapsed;
    Move      move;
    Move     *aiMoveResult;
    MoveList  legalMoves;
    int       running;
    FILE     *logfile;

    // ── 1. Init GLUT window ──────────────────────────────────────────────────
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 700);
    glutCreateWindow("Anteater Chess");
    initGL();
    glutDisplayFunc(display);
    glutMouseFunc(mouseHandler);

    logfile = fopen("chess_log.txt", "w");

    // ── 2. Run menu loop until player finishes all menus ─────────────────────
    while (!isGameReady()) {
        glutMainLoopEvent();
        glutPostRedisplay();
    }

    // ── 3. Read menu selections ───────────────────────────────────────────────
    matchup    = matchupMenu();
    clockSecs  = clockMenu() * 300;
    humanColor = colorMenu();
    depth      = 0;

    if (matchup == 1 || matchup == 2)
        depth = difficultyMenu() * 2;

    // ── 4. Init board ─────────────────────────────────────────────────────────
    init_board(&gs);
    setGameState(&gs);

    yellowSecs = clockSecs;
    blueSecs   = clockSecs;
    running    = 1;
    lastTime   = time(NULL);

    // ── 5. Main game loop ─────────────────────────────────────────────────────
    while (running)
    {
        // process GL events and redraw every iteration
        glutMainLoopEvent();
        glutPostRedisplay();

        // update clock
        currentTime = time(NULL);
        elapsed     = (int)(currentTime - lastTime);
        lastTime    = currentTime;

        if (gs.turn == YELLOW) yellowSecs -= elapsed;
        else                   blueSecs   -= elapsed;

        // timeout check
        if (yellowSecs <= 0) {
            setGameState(&gs);
            displayBoard(&gs, 0, blueSecs, humanColor);
            glutSwapBuffers();
            dispTimeout(YELLOW);
            break;
        }
        if (blueSecs <= 0) {
            setGameState(&gs);
            displayBoard(&gs, yellowSecs, 0, humanColor);
            glutSwapBuffers();
            dispTimeout(BLUE);
            break;
        }

        // update game state pointer and redraw board
        setGameState(&gs);
        displayBoard(&gs, yellowSecs, blueSecs, humanColor);
        glutSwapBuffers();

        // generate legal moves
        legalMoves = legalMoveGen(&gs);

        // checkmate / stalemate
        if (legalMoves.count == 0) {
            winner = inCheck(&gs, gs.turn)
                   ? ((gs.turn==YELLOW) ? BLUE : YELLOW)
                   : gs.turn; // stalemate - winner unused
            if (inCheck(&gs, gs.turn))
                dispWin(winner);
            else
                dispStalemate();
            break;
        }

        // ── AI vs AI ──────────────────────────────────────────────────────────
        if (matchup == 2) {
            aiMoveResult = SelectBestMove(&gs, gs.turn, depth);
            if (!aiMoveResult) { running=0; break; }
            move = *aiMoveResult;
            aiMove(move);
            logMove(logfile, gs.turn, move);
            gs = apply_move(&gs, move);
            setGameState(&gs);
            continue;
        }

        // ── User vs User or User vs AI ────────────────────────────────────────
        if (matchup == 1 && gs.turn != humanColor) {
            // AI turn
            aiMoveResult = SelectBestMove(&gs, gs.turn, depth);
            if (!aiMoveResult) { running=0; break; }
            move = *aiMoveResult;
            aiMove(move);
            logMove(logfile, gs.turn, move);
            gs = apply_move(&gs, move);
            setGameState(&gs);
            continue;
        }

        // ── Human turn ────────────────────────────────────────────────────────
        // show legal move highlights for ALL pieces (highlights update on click)
        dispLegalMoves(&legalMoves);

        // reset and wait for player to click two squares
        resetMove();
        while (!isMoveReady()) {
            glutMainLoopEvent();
            glutPostRedisplay();
        }

        // get the move the player clicked
        move = getMove(&gs);

        // validate move
        if (!is_legal(&legalMoves, move)) {
            printError("Illegal move! Try again.");
            continue;
        }

        logMove(logfile, gs.turn, move);
        gs = apply_move(&gs, move);
        setGameState(&gs);
    }

    if (logfile) fclose(logfile);
    return 0;
}

void logMove(FILE *logfile, Color color, Move move) {
    (void)logfile; (void)color; (void)move;
}

void init_board(GameState *gs) {
    int r, f;
    for (r=0; r<8; r++)
        for (f=0; f<10; f++)
            gs->board[r][f] = (Piece){EMPTY, YELLOW};

    gs->board[0][A]=(Piece){ROOK,BLUE};     gs->board[0][B]=(Piece){KNIGHT,BLUE};
    gs->board[0][C]=(Piece){BISHOP,BLUE};   gs->board[0][D]=(Piece){ANTEATER,BLUE};
    gs->board[0][E]=(Piece){QUEEN,BLUE};    gs->board[0][F]=(Piece){KING,BLUE};
    gs->board[0][G]=(Piece){ANTEATER,BLUE}; gs->board[0][H]=(Piece){BISHOP,BLUE};
    gs->board[0][I]=(Piece){KNIGHT,BLUE};   gs->board[0][J]=(Piece){ROOK,BLUE};
    for (f=0; f<10; f++) gs->board[1][f]=(Piece){ANT,BLUE};
    for (f=0; f<10; f++) gs->board[6][f]=(Piece){ANT,YELLOW};
    gs->board[7][A]=(Piece){ROOK,YELLOW};     gs->board[7][B]=(Piece){KNIGHT,YELLOW};
    gs->board[7][C]=(Piece){BISHOP,YELLOW};   gs->board[7][D]=(Piece){ANTEATER,YELLOW};
    gs->board[7][E]=(Piece){QUEEN,YELLOW};    gs->board[7][F]=(Piece){KING,YELLOW};
    gs->board[7][G]=(Piece){ANTEATER,YELLOW}; gs->board[7][H]=(Piece){BISHOP,YELLOW};
    gs->board[7][I]=(Piece){KNIGHT,YELLOW};   gs->board[7][J]=(Piece){ROOK,YELLOW};

    gs->turn=YELLOW; gs->yellow_kscastle=true; gs->yellow_qscastle=true;
    gs->blue_kscastle=true; gs->blue_qscastle=true;
    gs->anteater_ate=false; gs->en_passant_square=make_square(-1,A);
    gs->anteater_chain_square = make_square(-1, A);
    gs->prev_state=NULL;
    refresh_piece_cache(gs);
}

bool is_legal(MoveList *moves, Move move) {
    int i;
    for (i=0; i<moves->count; i++)
        if (moves->moves[i].from.rank==move.from.rank &&
            moves->moves[i].from.file==move.from.file &&
            moves->moves[i].to.rank  ==move.to.rank   &&
            moves->moves[i].to.file  ==move.to.file)
            return true;
    return false;
}
