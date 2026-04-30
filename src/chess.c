#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "chess_types.h"
#include "strategy.h"
#include "legalMoveGen.h"
#include "gui.h"

//function declarations
void logMove(FILE *logfile, Color color, Move move, int isUndo, Color humanColor);
void init_board(GameState *gs);
bool is_legal(MoveList *moves, Move move);
bool inCheck(const GameState *gs, Color color);
static void apply_human_promotion_if_needed(GameState *gs, const GameState *before, Move move);
static int  run_ai_search(GameState *gs, Color color, int depth, Move *out);

//gui signals
extern int stopChainPressed;
extern int logCount;

// this struct bundles everything the AI thread needs to do its search.
// we copy the game state in here so the AI works on its own snapshot
// and doesn't touch the real board while the human could be clicking around.
// 'done' is volatile because the main thread reads it in a loop — volatile
// tells the compiler not to cache the value in a register.
typedef struct
{
    GameState    gs_copy;
    Color        color;
    int          depth;
    Move         result;
    int          found;
    volatile int done;
} AISearchJob;

// this is the function that runs on the background thread.
// it just calls SelectBestMove on the copied game state and stores the result.
static int ai_thread_func(void *arg)
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
    return 0;
}

// kicks off the AI search on a background thread so the window doesn't freeze
// while the AI is thinking. the main thread spins in a loop calling aiMove()
// which keeps rendering frames until job.done flips to 1.
// returns 1 if the AI found a move, 0 if there were no legal moves.
static int run_ai_search(GameState *gs, Color color, int depth, Move *out)
{
    AISearchJob  job;
    SDL_Thread  *thread;
    Move         dummy;

    memset(&dummy, 0, sizeof(dummy));
    job.gs_copy = *gs;
    job.color   = color;
    job.depth   = depth;
    job.found   = 0;
    job.done    = 0;

    thread = SDL_CreateThread(ai_thread_func, "AISearch", &job);

    //keep rendering until the AI finishes
    while (!job.done)
        aiMove(dummy);

    SDL_WaitThread(thread, NULL);

    if (job.found)
        *out = job.result;

    return job.found;
}

// after a human moves an ant to the back rank, we need to ask what piece they want.
// we check if it was actually a promotion move first — if not, we just return.
// dispPromotion() opens a dialog and returns the chosen piece type.
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

// this is the entry point for the whole program.
// the structure is two nested loops:
//   outer loop = one full game (menu -> play -> back to menu)
//   inner loop = one move per iteration
int main()
{
    GameState  gs;
    GameState  prevGs;    // snapshot of the board from the previous move, used for undo
    int        hasPrev;   // whether prevGs is valid (0 at game start or after an undo)


    int        matchup;   // 0=HvH, 1=HvAI, 2=AIvAI
    Color      humanColor;
    Color      winner;
    int        depth;     // how many plies deep the AI searches
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

    // outer loop — each iteration is one full game.
    // when a game ends (checkmate, timeout, return key) we break the inner loop
    // and land back here to show the menu again.
    while (1)
    {
        matchup   = matchupMenu();
        clockSecs = clockMenu() * 300;  //1->5min, 2->10min, 3->15min
        //TODO: clockMenu needs a no-clock option (return 0), coordinate with Oreo

        depth      = 0;
        humanColor = YELLOW;

        // based on what matchup the player picked, we ask the right questions.
        // difficulty maps directly to search depth: easy=2 plies, medium=4, hard=8.
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

        // inner loop — each iteration is one move.
        // we tick the clock, render the board, generate legal moves,
        // check for game-over, then handle input based on the matchup type.
        while (running)
        {
            // tick the clock for whoever's turn it is
            currentTime = time(NULL);
            elapsed     = (int)(currentTime - lastTime);
            lastTime    = currentTime;
            GuiInput in = pollGuiInput();

            // return key from the menu screen exits back to matchup selection
            if (in.ret)
            {
                running = 0;
                hasPrev  = 0;
                break;  // exits to matchup
            }

            //TODO: if clockSecs == 0, skip clock logic entirely (no-clock mode)
            if (gs.turn == YELLOW)
            {
                yellowSecs -= elapsed;
            }
            else
            {
                blueSecs -= elapsed;
            }

            // if either clock hits zero, show the timeout screen and end the game
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

            // generate every legal move for whoever's turn it is
            legalMoves = legalMoveGen(&gs);

            // if there are zero legal moves, the game is over.
            // if the current player is in check it's checkmate, otherwise stalemate.
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
                // AI vs AI — both sides call run_ai_search each turn.
                // if the return key is pressed we bail back to the menu.
                case 2:
                    if (!run_ai_search(&gs, gs.turn, depth, &move)|| wasReturnPressed())
                    {
                        hasPrev = 0;
                        running = 0;
                        break;
                    }
                    prevGs  = gs;
                    hasPrev = 1;
                    logMove(logfile, gs.turn, move, 0, humanColor);
                    gs = *make_move_ai(&gs, move);
                    break;

                // Human vs Human — both sides go through getMove() which waits for a click.
                // supports undo (restores prevGs), anteater chain stopping, and promotion.
                case 0:
                    dispLegalMoves(&legalMoves);
                    move = getMove(&gs);

                    in = pollGuiInput();
                    if (in.ret) {
                        running = 0;
                        hasPrev = 0;
                        break;
                    }

                    // anteater chain — if the player pressed stop chain, end the anteater's
                    // multi-capture turn early and hand control to the other side
                    if (in.stopChain)
                    {
                        gs.anteater_ate = false;
                        gs.anteater_chain_square = make_square(-1, A);
                        gs.turn = (gs.turn == YELLOW) ? BLUE : YELLOW;
                        stopChainPressed = 0;
                        break;
                    }

                    //TODO: coordinate undo signal from GUI with Oreo
                    // undo — if we saved a previous state, snap back to it
                    if (in.undo)
                    {
                        if (hasPrev)
                        {
                            logMove(logfile, prevGs.turn, move, 1, humanColor);
                            gs = prevGs;
                            hasPrev = 0;
                        }
                        break;
                    }

                    // normal move — save the current state for undo, apply the move,
                    // then check if a promotion dialog needs to pop up
                    prevGs  = gs;
                    hasPrev = 1;
                    logMove(logfile, gs.turn, move, 0, humanColor);
                    gs = *make_move(&gs, move);
                    apply_human_promotion_if_needed(&gs, &prevGs, move);
                    break;

                // Human vs AI — if it's the AI's turn we run the search,
                // otherwise the human clicks a move just like in HvH.
                case 1:
                    if (gs.turn != humanColor)
                    {
                        if (!run_ai_search(&gs, gs.turn, depth, &move)|| wasReturnPressed())
                        {
                            hasPrev = 0;
                            running = 0;
                            break;
                        }
                        logMove(logfile, gs.turn, move, 0, humanColor);
                        gs = *make_move(&gs, move);
                    }
                    else
                    {
                        dispLegalMoves(&legalMoves);
                        move = getMove(&gs);

                        in = pollGuiInput();
                        if (in.ret) {
                            running = 0;
                            hasPrev = 0;
                            break;
                        }

                        if (in.stopChain)
                    {
                        gs.anteater_ate = false;
                        gs.anteater_chain_square = make_square(-1, A);
                        gs.turn = (gs.turn == YELLOW) ? BLUE : YELLOW;
                        stopChainPressed = 0;
                        break;
                    }

                        //TODO: coordinate undo signal from GUI with Oreo
                        if (in.undo)
                        {
                            if (hasPrev)
                            {
                                logMove(logfile, prevGs.turn, move, 1, humanColor);
                                gs = prevGs;
                                hasPrev = 0;
                            }
                            break;
                        }
                        prevGs  = gs;
                        hasPrev = 1;
                        logMove(logfile, gs.turn, move, 0, humanColor);
                        gs = *make_move(&gs, move);
                        apply_human_promotion_if_needed(&gs, &prevGs, move);
                    }
                    break;
            }
        }

    } //end outer menu loop

    if (logfile != NULL)
    {
        fclose(logfile);
    }

    guiQuit();

    return 0;
}

//TODO: implement move logging
// writes a move to both the log file and stdout in the format "Yellow A2 -> A4".
// if it's an undo we flip from/to so the log shows the move being reversed.
// addMoveLog also sends it to the on-screen move history panel.
void logMove(FILE *logfile, Color color, Move move, int isUndo, Color humanColor)
{
    const char *colorStr = (color == YELLOW) ? "Yellow" : "Blue";
    char fromFile = (char)('A'+move.from.file);
    char toFile   = (char)('A' + move.to.file);
    int  fromRank = 8 - move.from.rank;
    int  toRank   = 8 - move.to.rank;

    //writing to log file
    if(logfile != NULL) {
        fprintf(logfile, "%s %c%d -> %c%d\n", colorStr, fromFile, fromRank, toFile, toRank);
        fflush (logfile);
    }
    printf("%s: %c%d -> %c%d\n", colorStr, fromFile, fromRank, toFile, toRank);
    fflush(stdout);

    if (isUndo)
    addMoveLog(color, (Move){move.to, move.from}, humanColor);
    else
    addMoveLog(color, move, humanColor);

}

// sets up the starting board position for our custom 8x10 variant.
// board[0] is blue's back rank (top of the screen), board[7] is yellow's (bottom).
// anteaters sit at columns D and G, between the bishops and the queen/king.
// all castling rights start as true and get revoked when pieces move.
void init_board(GameState *gs)
{
    int r;
    int f;

    // fill every square with empty first, then place pieces on top
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

// checks whether a given move appears in the legal move list.
// used in text mode to reject illegal input from the player.
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
