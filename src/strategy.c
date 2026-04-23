#include "strategy.h"
#include <stdio.h>
#include <stdint.h>
#include "eval.h"
#include <stdlib.h>
#include <time.h>

//not exported in legalMoveGen.h but needed here
bool inCheck(const GameState *gs, Color color);

//checkmate score and search time limit
#define INF             1000000
#define TIME_LIMIT_SECS 3

//transposition table size and entry flag values
#define TT_SIZE  (1 << 20)
#define TT_MASK  (TT_SIZE - 1)
#define TT_EXACT 0
#define TT_ALPHA 1
#define TT_BETA  2

//one cached position in the transposition table
typedef struct
{
    uint64_t key;
    int      score;
    int      depth;
    int      flag;
} TTEntry;

//function declarations
int      negamax(GameState *gs, int depth, int alpha, int beta, int ply);
int      quiesce(GameState *gs, int alpha, int beta, int qdepth);
bool     time_up(void);
uint64_t rand64(void);
void     zobrist_init(void);
uint64_t compute_hash(const GameState *gs);
bool     tt_probe(uint64_t key, int depth, int alpha, int beta, int *score);
void     tt_store(uint64_t key, int depth, int score, int flag);
int      score_move(const GameState *gs, Move m);
void     sort_moves(const GameState *gs, MoveList *moves);

//search state
clock_t search_start_time;
bool    search_aborted;
int     node_count;

//one random 64-bit key per (rank, file, piece type, color) combo
uint64_t zob_piece[8][10][8][2];
uint64_t zob_side;
uint64_t zob_seed;  //xorshift64* state, must be non-zero before first rand64() call
bool     zob_ready;

//the transposition table
TTEntry tt[TT_SIZE];

//checks time limit, only reads clock every 2048 nodes to reduce overhead
bool time_up(void)
{
    double elapsed;

    if (node_count++ & 2047)
    {
        return false;
    }
    elapsed = (double)(clock() - search_start_time) / CLOCKS_PER_SEC;
    return elapsed >= TIME_LIMIT_SECS;
}

//xorshift64* — shifts bits in three directions then multiplies to spread entropy
//full algorithm and shift constants from Vigna's paper (see REFERENCES.txt section 8)
uint64_t rand64(void)
{
    zob_seed ^= zob_seed >> 12;
    zob_seed ^= zob_seed << 25;
    zob_seed ^= zob_seed >> 27;
    return zob_seed * 0x2545F4914F6CDD1DULL;
}

//fills zobrist table with random keys before the first search
void zobrist_init(void)
{
    int rank;
    int file;
    int piecetype;
    int color;

    //seed xorshift64* with time so keys differ each run
    srand((unsigned)time(NULL));
    zob_seed = ((uint64_t)rand() << 32) | (uint64_t)rand();
    if (zob_seed == 0)
    {
        zob_seed = 1;
    }

    for (rank = 0; rank < 8; rank++)
    {
        for (file = 0; file < 10; file++)
        {
            for (piecetype = 0; piecetype < 8; piecetype++)
            {
                for (color = 0; color < 2; color++)
                {
                    zob_piece[rank][file][piecetype][color] = rand64();
                }
            }
        }
    }
    zob_side  = rand64();
    zob_ready = true;
}

//XORs a key for every piece on the board to produce a unique position fingerprint
uint64_t compute_hash(const GameState *gs)
{
    uint64_t hash;
    int      rank;
    int      file;
    Piece    square_piece;

    hash = 0;
    for (rank = 0; rank < 8; rank++)
    {
        for (file = 0; file < 10; file++)
        {
            square_piece = gs->board[rank][file];
            if (square_piece.piecetype != EMPTY)
            {
                hash ^= zob_piece[rank][file][square_piece.piecetype][square_piece.color];
            }
        }
    }
    if (gs->turn == BLUE)
    {
        hash ^= zob_side;
    }
    return hash;
}

//looks up position in transposition table, returns true if a usable result is found
bool tt_probe(uint64_t key, int depth, int alpha, int beta, int *score)
{
    TTEntry *e;

    e = &tt[key & TT_MASK];
    if (e->key != key || e->depth < depth)
    {
        return false;
    }
    if (e->flag == TT_EXACT)
    {
        *score = e->score;
        return true;
    }
    if (e->flag == TT_ALPHA && e->score <= alpha)
    {
        *score = alpha;
        return true;
    }
    if (e->flag == TT_BETA && e->score >= beta)
    {
        *score = beta;
        return true;
    }
    return false;
}

//stores a search result in the transposition table
void tt_store(uint64_t key, int depth, int score, int flag)
{
    TTEntry *e;

    e = &tt[key & TT_MASK];
    //keep deeper entries, they contain more information
    if (e->key == key && e->depth > depth)
    {
        return;
    }
    e->key   = key;
    e->depth = depth;
    e->score = score;
    e->flag  = flag;
}

//scores a capture: big victim taken by small attacker scores highest (MVV-LVA)
int score_move(const GameState *gs, Move m)
{
    PieceType victim;
    PieceType attacker;

    victim   = gs->board[m.to.rank  ][m.to.file  ].piecetype;
    attacker = gs->board[m.from.rank][m.from.file].piecetype;
    if (victim != EMPTY)
    {
        return PIECE_VALUE[victim] * 10 - PIECE_VALUE[attacker];
    }
    return 0;
}

//sorts moves so captures come first, quiet moves last
void sort_moves(const GameState *gs, MoveList *moves)
{
    int  scores[250];
    int  i;
    int  j;
    Move tmp;
    int  ts;

    for (i = 0; i < moves->count; i++)
    {
        scores[i] = score_move(gs, moves->moves[i]);
    }

    for (i = 1; i < moves->count; i++)
    {
        tmp = moves->moves[i];
        ts  = scores[i];
        j   = i - 1;
        while (j >= 0 && scores[j] < ts)
        {
            moves->moves[j+1] = moves->moves[j];
            scores[j+1]       = scores[j];
            j--;
        }
        moves->moves[j+1] = tmp;
        scores[j+1]       = ts;
    }
}

//searches captures only at leaf nodes to avoid stopping in the middle of an exchange
int quiesce(GameState *gs, int alpha, int beta, int qdepth)
{
    int      stand_pat;
    int      score;
    int      i;
    MoveList moves;
    UndoData undo;

    if (search_aborted)
    {
        return 0;
    }
    if (time_up())
    {
        search_aborted = true;
        return 0;
    }

    //score position as-is, if already good enough stop here
    stand_pat = evaluate(gs);

    if (stand_pat >= beta)
    {
        return beta;
    }
    if (stand_pat > alpha)
    {
        alpha = stand_pat;
    }
    if (qdepth == 0)
    {
        return alpha;
    }

    moves = legalMoveGen(gs);
    sort_moves(gs, &moves);

    for (i = 0; i < moves.count; i++)
    {
        //skip non-captures
        if (gs->board[moves.moves[i].to.rank][moves.moves[i].to.file].piecetype == EMPTY)
        {
            continue;
        }

        make_move_in_place(gs, moves.moves[i], &undo);
        score = -quiesce(gs, -beta, -alpha, qdepth - 1);
        undo_move_in_place(gs, moves.moves[i], &undo);

        if (score >= beta)
        {
            return beta;
        }
        if (score > alpha)
        {
            alpha = score;
        }
    }

    return alpha;
}

//returns board state after a move without modifying the original
GameState apply_move(const GameState *gs, Move move)
{
    GameState nextState;
    UndoData  undo;

    nextState            = *gs;
    nextState.prev_state = NULL;
    make_move_in_place(&nextState, move, &undo);

    return nextState;
}

//recursive search — score from the perspective of whoever is moving
//positive = winning, negative = losing
//ply = distance from root, used to prefer shorter mates over longer ones
int negamax(GameState *gs, int depth, int alpha, int beta, int ply)
{
    uint64_t key;
    int      tt_score;
    MoveList moves;
    int      best;
    int      i;
    int      score;
    int      flag;
    Color    side_to_move;
    UndoData undo;

    if (search_aborted)
    {
        return 0;
    }
    if (time_up())
    {
        search_aborted = true;
        return 0;
    }

    //check if this position was already searched at this depth
    key = compute_hash(gs);
    if (tt_probe(key, depth, alpha, beta, &tt_score))
    {
        return tt_score;
    }

    if (depth == 0)
    {
        return quiesce(gs, alpha, beta, 1);
    }

    moves = legalMoveGen(gs);
    sort_moves(gs, &moves);

    if (moves.count == 0)
    {
        if (inCheck(gs, gs->turn))
        {
            //shorter mate scores higher than longer mate
            return -(INF - ply);
        }
        return 0;
    }

    //null move pruning: passing the turn still beats beta, so prune
    if (depth >= 3 && !inCheck(gs, gs->turn))
    {
        if (gs->turn == YELLOW)
        {
            gs->turn = BLUE;
        }
        else
        {
            gs->turn = YELLOW;
        }
        score = -negamax(gs, depth - 3, -beta, -beta + 1, ply + 1);
        if (gs->turn == YELLOW)
        {
            gs->turn = BLUE;
        }
        else
        {
            gs->turn = YELLOW;
        }
        if (score >= beta)
        {
            return beta;
        }
    }

    best = -(INF - ply);
    flag = TT_ALPHA;

    for (i = 0; i < moves.count; i++)
    {
        side_to_move = gs->turn;

        make_move_in_place(gs, moves.moves[i], &undo);

        if (gs->turn == side_to_move)
        {
            score = negamax(gs, depth - 1, alpha, beta, ply + 1);
        }
        else
        {
            score = -negamax(gs, depth - 1, -beta, -alpha, ply + 1);
        }

        undo_move_in_place(gs, moves.moves[i], &undo);

        if (score > best)
        {
            best = score;
        }
        if (score > alpha)
        {
            alpha = score;
            flag  = TT_EXACT;
        }
        if (alpha >= beta)
        {
            tt_store(key, depth, best, TT_BETA);
            return best;
        }
    }

    tt_store(key, depth, best, flag);
    return best;
}

Move* SelectBestMove(GameState *gs, Color color, int depth)
{
    static Move best_move;
    Move        best_at_depth;
    MoveList    moves;
    int         best_score_at_depth;
    int         alpha;
    int         beta;
    int         i;
    int         current_depth;
    int         score;
    clock_t     start;
    double      elapsed;
    Color       side_to_move;
    UndoData    undo;

    (void)color;

    if (!zob_ready)
    {
        zobrist_init();
    }

    search_start_time = clock();
    search_aborted    = false;
    node_count        = 0;
    start             = search_start_time;

    moves = legalMoveGen(gs);
    sort_moves(gs, &moves);

    if (moves.count == 0)
    {
        return NULL;
    }

    best_move = moves.moves[0];

    //search depth 2, then 4, then 6... stopping when time runs out
    for (current_depth = 2; current_depth <= depth; current_depth += 2)
    {
        elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
        if (elapsed >= TIME_LIMIT_SECS)
        {
            break;
        }

        best_at_depth       = best_move;
        best_score_at_depth = -INF;
        alpha               = -INF;
        beta                = INF;

        for (i = 0; i < moves.count; i++)
        {
            side_to_move = gs->turn;

            make_move_in_place(gs, moves.moves[i], &undo);

            if (gs->turn == side_to_move)
            {
                score = negamax(gs, current_depth - 1, alpha, beta, 1);
            }
            else
            {
                score = -negamax(gs, current_depth - 1, -beta, -alpha, 1);
            }

            undo_move_in_place(gs, moves.moves[i], &undo);

            if (search_aborted)
            {
                break;
            }

            if (i == 0 || score > best_score_at_depth)
            {
                best_score_at_depth = score;
                best_at_depth       = moves.moves[i];
            }

            if (score > alpha)
            {
                alpha = score;
            }
        }

        if (!search_aborted)
        {
            best_move = best_at_depth;
        }
        else
        {
            break;
        }
    }

    return &best_move;
}
