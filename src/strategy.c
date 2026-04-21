#include "strategy.h"

#include <stdio.h>

#include "eval.h"
#include <stdlib.h>
#include <time.h>

//legalMoveGen.c gives us this, not exported in legalMoveGen.h
bool inCheck(const GameState *gs, Color color);

//big number to represent checkmate (basically infinity)
#define INF 1000000

//time limit for iterative deepening, stop searching at 5 seconds
#define TIME_LIMIT_SECS 5

//internal function declarations
static int negamax(GameState *gs, int depth, int alpha, int beta);
static int piece_value(PieceType piece);
static int score_move(const GameState *gs, Move move);
static clock_t search_start_time;
static bool search_aborted;
static bool time_up(void)
{
    double elapsed = (double)(clock() - search_start_time) / CLOCKS_PER_SEC;
    return elapsed >= TIME_LIMIT_SECS;
}
static int piece_value(PieceType piece)
{
    switch (piece)
    {
        case KING:     return 10000;
        case QUEEN:    return 900;
        case ROOK:     return 500;
        case BISHOP:   return 330;
        case KNIGHT:   return 320;
        case ANT:      return 100;
        case ANTEATER: return 200;   // adjust if you want
        case EMPTY:    return 0;
        default:       return 0;
    }
}

static int score_move(const GameState *gs, Move move)
{
    Piece attacker;
    Piece victim;
    int attacker_value;
    int victim_value;

    attacker = gs->board[move.from.rank][move.from.file];
    victim   = gs->board[move.to.rank][move.to.file];

    attacker_value = piece_value(attacker.piecetype);
    victim_value   = piece_value(victim.piecetype);

    // normal capture
    if (victim.piecetype != EMPTY)
    {
        return 10000 + victim_value - attacker_value / 10;
    }

    // en passant capture:
    // ant moves diagonally to empty square, so destination is empty
    if (attacker.piecetype == ANT && move.from.file != move.to.file)
    {
        return 10000 + piece_value(ANT) - attacker_value / 10;
    }

    // quiet move
    return 0;
}

static void sort_moves(const GameState *gs, MoveList *moves)
{
    int i;
    int j;
    int best_index;
    int best_score;
    int current_score;
    Move temp;

    for (i = 0; i < moves->count - 1; i++)
    {
        best_index = i;
        best_score = score_move(gs, moves->moves[i]);

        for (j = i + 1; j < moves->count; j++)
        {
            current_score = score_move(gs, moves->moves[j]);

            if (current_score > best_score)
            {
                best_score = current_score;
                best_index = j;
            }
        }

        if (best_index != i)
        {
            temp = moves->moves[i];
            moves->moves[i] = moves->moves[best_index];
            moves->moves[best_index] = temp;
        }
    }
}
//TODO: MVV-LVA move ordering
//  score_move(gs, move) -> victim_value - attacker_value/10, captures score high, quiet moves score 0
//  sort_moves(gs, moves) -> selection sort using score_move, search best captures first

//TODO: quiescence search
//  at depth 0, keep searching captures until position is quiet before scoring
//  prevents horizon effect (AI walking into a recapture it cant see)

//takes the current board and a move, returns what the board looks like after that move
//never touches the original board - negamax needs it unchanged to keep searching

//the actual AI search, tries every move and picks the best one
//score is always from the perspective of whoever is moving right now
//positive = winning, negative = losing
static int negamax(GameState *gs, int depth, int alpha, int beta)
{
    if (search_aborted)
    {
        return 0;
    }

    if (time_up())
    {
        search_aborted = true;
        return 0;
    }
    MoveList moves;
    int best;
    int i;
    int score;

    if (depth == 0)
    {
        return evaluate(gs);
    }

    moves = legalMoveGen(gs);
    sort_moves(gs, &moves);

    if (moves.count == 0)
    {
        if (inCheck(gs, gs->turn))
        {
            return -INF;
        }
        return 0;
    }

    best = -INF;

    for (i = 0; i < moves.count; i++)
    {

        UndoData undo;
        make_move_in_place(gs, moves.moves[i], &undo);

        score = -negamax(gs, depth - 1, -beta, -alpha);

        undo_move_in_place(gs, moves.moves[i], &undo);

        if (score > best)
        {
            best = score;
        }

        if (score > alpha)
        {
            alpha = score;
        }

        if (alpha >= beta)
        {
            break;
        }
    }


    return best;
}


Move* SelectBestMove(GameState *gs, Color color, int depth)
{
    search_start_time = clock();
    search_aborted = false;

    MoveList moves;
    static Move best_move;
    Move best_at_depth;
    int best_score;
    int best_score_at_depth;
    int alpha;
    int beta;
    int i;
    int current_depth;
    int score;
    clock_t start;
    double elapsed;

    (void)color;

    moves = legalMoveGen(gs);
    sort_moves(gs, &moves);

    if (moves.count == 0)
    {
        return NULL;
    }

    start = search_start_time;
    best_move = moves.moves[0];

    for (current_depth = 2; current_depth <= depth; current_depth += 2)
    {
        elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
        if (elapsed >= TIME_LIMIT_SECS)
        {
            break;
        }

        best_at_depth = best_move;
        best_score_at_depth = -INF;
        alpha = -INF;
        beta = INF;

        for (i = 0; i < moves.count; i++)
        {


            UndoData undo;
            make_move_in_place(gs, moves.moves[i], &undo);

            score = -negamax(gs, current_depth - 1, -beta, -alpha);

            undo_move_in_place(gs, moves.moves[i], &undo);


            if (search_aborted)
            {
                break;
            }

            if (i == 0 || score > best_score_at_depth)
            {
                best_score_at_depth = score;
                best_at_depth = moves.moves[i];
            }

            if (score > alpha)
            {
                alpha = score;
            }
        }

        if (!search_aborted)
        {
            best_score = best_score_at_depth;
            best_move = best_at_depth;
        }
        else
        {
            break;
        }
    }


    return &best_move;
}
