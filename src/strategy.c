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
#define TIME_LIMIT_SECS 10

//internal function declarations
static int negamax(GameState *gs, int depth, int alpha, int beta);
static clock_t search_start_time;
static bool search_aborted;
static bool time_up(void)
{
    double elapsed = (double)(clock() - search_start_time) / CLOCKS_PER_SEC;
    return elapsed >= TIME_LIMIT_SECS;
}

//TODO: quiescence search
//  at depth 0, keep searching captures until position is quiet before scoring
//  prevents horizon effect (AI walking into a recapture it cant see)

//MVV-LVA: big victim captured by small attacker scores highest
//uses eval.h PIECE_VALUE so ordering is consistent with the board evaluator
static int score_move(const GameState *gs, Move m)
{
    PieceType victim   = gs->board[m.to.rank  ][m.to.file  ].piecetype;
    PieceType attacker = gs->board[m.from.rank][m.from.file].piecetype;
    if (victim != EMPTY)
        return PIECE_VALUE[victim] * 10 - PIECE_VALUE[attacker];
    return 0;
}

//insertion sort descending — captures first, quiet moves last
static void sort_moves(const GameState *gs, MoveList *moves)
{
    int scores[250];
    int i, j;
    Move tmp;
    int  ts;

    for (i = 0; i < moves->count; i++)
        scores[i] = score_move(gs, moves->moves[i]);

    for (i = 1; i < moves->count; i++) {
        tmp = moves->moves[i];
        ts  = scores[i];
        j   = i - 1;
        while (j >= 0 && scores[j] < ts) {
            moves->moves[j+1] = moves->moves[j];
            scores[j+1]       = scores[j];
            j--;
        }
        moves->moves[j+1] = tmp;
        scores[j+1]       = ts;
    }
}

//takes the current board and a move, returns what the board looks like after that move
//never touches the original board - negamax needs it unchanged to keep searching
GameState apply_move(const GameState *gs, Move move)
{
    GameState nextState;
    UndoData undo;

    nextState              = *gs;
    nextState.prev_state   = NULL;
    make_move_in_place(&nextState, move, &undo);

    return nextState;
}

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

    //search captures before quiet moves — alpha-beta prunes much more with good ordering
    sort_moves(gs, &moves);

    best = -INF;

    for (i = 0; i < moves.count; i++)
    {
        Color side_to_move = gs->turn;

        UndoData undo;
        make_move_in_place(gs, moves.moves[i], &undo);

        if (gs->turn == side_to_move)
        {
            score = negamax(gs, depth - 1, alpha, beta);
        }
        else
        {
            score = -negamax(gs, depth - 1, -beta, -alpha);
        }

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

    //sort root moves once — captures searched first at every depth
    sort_moves(gs, &moves);

    //iterative deepening up to the requested depth, stops early if time runs out
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
            Color side_to_move = gs->turn;

            UndoData undo;
            make_move_in_place(gs, moves.moves[i], &undo);

            if (gs->turn == side_to_move)
            {
                score = negamax(gs, current_depth - 1, alpha, beta);
            }
            else
            {
                score = -negamax(gs, current_depth - 1, -beta, -alpha);
            }

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

    fprintf(stderr,
            "search %s (elapsed=%.2f sec, target_depth=%d)\n",
            search_aborted ? "aborted" : "completed",
            (double)(clock() - start) / CLOCKS_PER_SEC,
            depth);

    return &best_move;
}
