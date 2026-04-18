#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chess_types.h"
#include "strategy.h"
#include "legalMoveGen.h"
#include "textui.h"

//function declarations
void logMove(FILE *logfile, Color color, Move move);
void init_board(GameState *gs);
bool is_legal(MoveList *moves, Move move);
bool inCheck(GameState *gs, Color color);

int main(void)
{
    GameState  gs;
    GameState  prevGs;     //saved before each move for undo
    int        hasPrev;    //1 if prevGs is valid

    int        matchup;    //0=user vs user, 1=user vs AI, 2=AI vs AI
    Color      humanColor;
    Color      winner;
    int        depth;      //negamax search depth, set by difficulty

    int        clockSecs;  //starting time per player in seconds
    int        yellowSecs;
    int        blueSecs;
    time_t     lastTime;
    time_t     currentTime;
    int        elapsed;

    Move       move;
    Move      *aiMoveResult;
    MoveList   legalMoves;

    int        running;
    int        validMove;

    FILE      *logfile;

    logfile = fopen("chess_log.txt", "w");

    //menus: matchup first, then difficulty if AI involved, color if human playing
    matchup   = matchupMenu();
    clockSecs = clockMenu() * 300;  //1->5min, 2->10min, 3->15min

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
            depth      = difficultyMenu() * 2;  //1->2, 2->4, 3->6
            humanColor = colorMenu();
            break;

        case 2:
            //AI vs AI, just need difficulty
            depth = difficultyMenu() * 2;
            break;

        default:
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

        if (gs.turn == YELLOW)
        {
            yellowSecs -= elapsed;
        }
        else
        {
            blueSecs -= elapsed;
        }

        //timeout check
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
            //AI vs AI
            case 2:
                aiMoveResult = SelectBestMove(&gs, gs.turn, depth);
                if (aiMoveResult == NULL)
                {
                    running = 0;
                    break;
                }
                move    = *aiMoveResult;
                prevGs  = gs;
                hasPrev = 1;
                aiMove(move);
                logMove(logfile, gs.turn, move);
                gs = apply_move(&gs, move);
                break;

            //user vs user or human side in user vs AI
            case 0:
            case 1:
                if (matchup == 1 && gs.turn != humanColor)
                {
                    //AI turn in user vs AI
                    aiMoveResult = SelectBestMove(&gs, gs.turn, depth);
                    if (aiMoveResult == NULL)
                    {
                        running = 0;
                        break;
                    }
                    move    = *aiMoveResult;
                    prevGs  = gs;
                    hasPrev = 1;
                    aiMove(move);
                    logMove(logfile, gs.turn, move);
                    gs = apply_move(&gs, move);
                }
                else
                {
                    //human turn, show legal squares then read command
                    dispLegalMoves(&legalMoves);

                    validMove = 0;
                    while (!validMove)
                    {
                        move = getMove(&gs);

                        if (move.from.file == (File)TUI_CMD_QUIT)
                        {
                            running   = 0;
                            validMove = 1;
                        }
                        else if (move.from.file == (File)TUI_CMD_HELP)
                        {
                            printHelp();
                        }
                        else if (move.from.file == (File)TUI_CMD_UNDO)
                        {
                            if (hasPrev)
                            {
                                gs         = prevGs;
                                hasPrev    = 0;
                                legalMoves = legalMoveGen(&gs);
                                displayBoard(&gs, yellowSecs, blueSecs, humanColor);
                                validMove  = 1;
                            }
                            else
                            {
                                printError("Nothing to undo.");
                            }
                        }
                        else
                        {
                            if (is_legal(&legalMoves, move))
                            {
                                prevGs  = gs;
                                hasPrev = 1;
                                logMove(logfile, gs.turn, move);
                                gs        = apply_move(&gs, move);
                                validMove = 1;
                            }
                            else
                            {
                                printError("Illegal move. Try again.");
                            }
                        }
                    }
                }
                break;

            default:
                break;
        }
    }

    if (logfile != NULL)
    {
        fclose(logfile);
    }

    return 0;
}

//TODO: implement move logging
void logMove(FILE *logfile, Color color, Move move)
{
    (void)logfile;
    (void)color;
    (void)move;
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

    gs->turn                   = YELLOW;
    gs->yellow_castled         = false;
    gs->blue_castled           = false;
    gs->anteater_ate           = false;
    gs->en_passant_square.rank = -1;
    gs->en_passant_square.file = A;
    gs->prev_state             = NULL;
}

//checks if a move is in the legal moves list
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
