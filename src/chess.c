#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "chess_types.h"
#include "strategy.h"
#include "legalMoveGen.h"
#include "gui.h"

//function declarations
void logMove(FILE *logfile, Color color, Move move);
void init_board(GameState *gs);
bool is_legal(MoveList *moves, Move move);
bool inCheck(const GameState *gs, Color color);
static void apply_human_promotion_if_needed(GameState *gs, const GameState *before, Move move);
static int  run_ai_search(GameState *gs, Color color, int depth, Move *out);

//holds everything the AI thread needs, plus the result
typedef struct
{
    GameState    gs_copy;
    Color        color;
    int          depth;
    Move         result;
    int          found;
    volatile int done;
} AISearchJob;

static void *ai_thread_func(void *arg)
{
    AISearchJob *job = (AISearchJob *)arg;
    Move        *m;

    m = SelectBestMove(&job->gs_copy, job->color, job->depth);
    if (m != NULL)
    {
        job->result = *m;
        job->found  = 1;
    }
    job->done = 1;
    return NULL;
}

//runs SelectBestMove on a background thread so the GUI stays alive while the AI thinks
//returns 1 if a move was found, 0 if no legal moves
static int run_ai_search(GameState *gs, Color color, int depth, Move *out)
{
    AISearchJob job;
    pthread_t   thread;
    Move        dummy;

    memset(&dummy, 0, sizeof(dummy));
    job.gs_copy = *gs;
    job.color   = color;
    job.depth   = depth;
    job.found   = 0;
    job.done    = 0;

    pthread_create(&thread, NULL, ai_thread_func, &job);

    //keep rendering until the AI finishes
    while (!job.done)
        aiMove(dummy);

    pthread_join(&thread, NULL);

    if (job.found)
        *out = job.result;

    return job.found;
}

static void apply_human_promotion_if_needed(GameState *gs, const GameState *before, Move move)
{
    GameState *promoted;
    PieceType choice;

    if (!is_promotion_move(before, move))
    {
        return;
    }

    choice = dispPromotion();
    promoted = promote_pawn(gs, move.to, choice);
    if (promoted != NULL)
    {
        *gs = *promoted;
    }
}

int main()
{
    GameState  gs;
    GameState  prevGs;    //TODO: wire up undo signal from GUI with Oreo
    int        hasPrev;   //TODO: wire up undo signal from GUI with Oreo
    (void)prevGs;
    (void)hasPrev;

    int        matchup;
    Color      humanColor;
    Color      winner;
    int        depth;
    int        diff;

    int        clockSecs;
    int        yellowSecs;
    int        blueSecs;
    time_t     lastTime;
    time_t     currentTime;
    int        elapsed;

    Move       move;
    MoveList   legalMoves;

    int        running;

    FILE      *logfile;

    if (!guiInit())
    {
        fprintf(stderr, "failed to initialize GUI\n");
        return 1;
    }

    logfile = fopen("chess_log.txt", "w");

    matchup   = matchupMenu();
    clockSecs = clockMenu() * 300;  //1->5min, 2->10min, 3->15min
    //TODO: clockMenu needs a no-clock option (return 0), coordinate with Oreo

    depth      = 0;
    humanColor = YELLOW;

    switch (matchup)
    {
        case 0:
            //user vs user, ask for color so yellow always goes first
            humanColor = colorMenu();
            break;

        case 1:
            //user vs AI, need difficulty and color
            diff = difficultyMenu();
            if (diff < 3)
            {
                depth = diff * 2;  //easy=2, medium=4
            }
            else
            {
                depth = 8;  //hard=8
            }
            humanColor = colorMenu();
            break;

        case 2:
            //AI vs AI, just need difficulty
            diff = difficultyMenu();
            if (diff < 3)
            {
                depth = diff * 2;  //easy=2, medium=4
            }
            else
            {
                depth = 8;  //hard=8
            }
            break;

    }

    init_board(&gs);

    yellowSecs = clockSecs;
    blueSecs   = clockSecs;
    hasPrev    = 0;
    running    = 1;
    lastTime   = time(NULL);

    while (running)
    {
        //updating clock for current player
        currentTime = time(NULL);
        elapsed     = (int)(currentTime - lastTime);
        lastTime    = currentTime;

        //TODO: if clockSecs == 0, skip clock logic entirely (no-clock mode)
        if (gs.turn == YELLOW)
        {
            yellowSecs -= elapsed;
        }
        else
        {
            blueSecs -= elapsed;
        }

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

        legalMoves = legalMoveGen(&gs);

        //checkmate or stalemate check
        if (legalMoves.count == 0)
        {
            if (inCheck(&gs, gs.turn))
            {
                if (gs.turn == YELLOW)
                {
                    winner = BLUE;
                }
                else
                {
                    winner = YELLOW;
                }
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
            //AI vs AI
            case 2:
                if (!run_ai_search(&gs, gs.turn, depth, &move))
                {
                    running = 0;
                    break;
                }
                prevGs  = gs;
                hasPrev = 1;
                logMove(logfile, gs.turn, move);
                gs = *make_move_ai(&gs, move);
                break;

            //user vs user, always a human turn
            case 0:
                dispLegalMoves(&legalMoves);
                move = getMove(&gs);

                //TODO: coordinate undo signal from GUI with Oreo
                prevGs  = gs;
                hasPrev = 1;
                logMove(logfile, gs.turn, move);
                gs = *make_move(&gs, move);
                apply_human_promotion_if_needed(&gs, &prevGs, move);
                break;

            //user vs AI — AI moves on its turn, human moves on theirs
            case 1:
                if (gs.turn != humanColor)
                {
                    if (!run_ai_search(&gs, gs.turn, depth, &move))
                    {
                        running = 0;
                        break;
                    }
                    prevGs  = gs;
                    hasPrev = 1;
                    logMove(logfile, gs.turn, move);
                    gs = *make_move(&gs, move);
                }
                else
                {
                    dispLegalMoves(&legalMoves);
                    move = getMove(&gs);

                    //TODO: coordinate undo signal from GUI with Oreo
                    prevGs  = gs;
                    hasPrev = 1;
                    logMove(logfile, gs.turn, move);
                    gs = *make_move(&gs, move);
                    apply_human_promotion_if_needed(&gs, &prevGs, move);
                }
                break;
        }
    }

    if (logfile != NULL)
    {
        fclose(logfile);
    }

    guiQuit();

    return 0;
}

//TODO: implement move logging
void logMove(FILE *logfile, Color color, Move move)
{
    const char *colorStr = (color == YELLOW)? "YELLOW" : "BLUE"; 
    char fromFile = (char)('A'+move.from.file); 
    char toFile = (char)('A' + move.to.file); 
    int fromRank = move.from.rank+1; 
    int toRank = move.to.rank + 1; 

    //writing to log file
    if(logfile != NULL) {
        fprintf(logfile, "%s %c%d -> %c%d\n", colorStr, fromFile, fromRank, toFile, toRank); 
        fflush (logfile); 
    }
    printf("%s: %c%d -> %c%d\n", colorStr, fromFile, fromRank, toFile, toRank); 
    fflush(stdout); 

    addMoveLog(color, move); 
}

//board[0] = blue back rank (rank 8, top)
//board[7] = yellow back rank (rank 1, bottom)
void init_board(GameState *gs)
{
    int r;
    int f;

    for (r = 0; r < 8; r++)
    {
        for (f = 0; f < 10; f++)
        {
            gs->board[r][f] = (Piece){EMPTY, YELLOW};
        }
    }

    //blue back rank, anteaters between king+bishop and queen+bishop
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
    {
        gs->board[1][f] = (Piece){ANT, BLUE};
    }

    for (f = 0; f < 10; f++)
    {
        gs->board[6][f] = (Piece){ANT, YELLOW};
    }

    //yellow back rank
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

    gs->turn               = YELLOW;
    gs->yellow_kscastle    = true;
    gs->yellow_qscastle    = true;
    gs->blue_kscastle      = true;
    gs->blue_qscastle      = true;
    gs->anteater_ate       = false;
    gs->anteater_chain_square = make_square(-1, A);
    gs->en_passant_square  = make_square(-1, A);
    gs->prev_state         = NULL;
    refresh_piece_cache(gs);
}

//validates typed input against the move list, only used in textChess
bool is_legal(MoveList *moves, Move move)
{
    int i;

    for (i = 0; i < moves->count; i++)
    {
        if (moves->moves[i].from.rank == move.from.rank &&
            moves->moves[i].from.file == move.from.file &&
            moves->moves[i].to.rank   == move.to.rank   &&
            moves->moves[i].to.file   == move.to.file)
        {
            return true;
        }
    }
    return false;
}
