#include "strategy.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "eval.h"
#include <stdlib.h>
#include <time.h>

//not exported in legalMoveGen.h but needed here
bool inCheck(const GameState *gs, Color color);

#define INF            1000000
#define TIME_LIMIT_SECS 8

//transpo table size and flag values
#define TRANSPO_SIZE  (1 << 20)
#define TRANSPO_MASK  (TRANSPO_SIZE - 1)
#define TRANSPO_EXACT 0
#define TRANSPO_ALPHA 1
#define TRANSPO_BETA  2

//one cached position
typedef struct
{
    uint64_t key;
    int      score;
    int      depth;
    int      flag;
    Move     bestMove;
} TranspoEntry;

//function declarations
int      negamax(GameState *gs, int depth, int alpha, int beta, int ply);
int      quiesce(GameState *gs, int alpha, int beta, int qDepth);
bool     timeUp(void);
uint64_t rand64(void);
void     zobristInit(void);
uint64_t computeHash(const GameState *gs);
bool     transpoProbe(uint64_t key, int depth, int alpha, int beta, int *score);
void     transpoStore(uint64_t key, int depth, int score, int flag, Move bestMove);
int      transpoGetBestMove(uint64_t key, Move *out);
int      movesEqual(Move a, Move b);
void     updateKiller(int ply, Move move);
int      scoreMove(const GameState *gs, Move m, int ply);
void     sortMoves(const GameState *gs, MoveList *moves, int ply);
GameState apply_move(const GameState *gs, Move move);

//search state
clock_t searchStartTime;
bool    searchAborted;
int     nodeCount;

//two quiet beta-cutoff moves per ply
#define MAX_KILLER_PLY 32
Move killerMoves[MAX_KILLER_PLY][2];

//tracks how often quiet moves caused beta cutoffs - deeper cutoffs count more
int historyTable[8][10][8][10];

//one random key per (rank, file, piecetype, color) combo
uint64_t zobPiece[8][10][8][2];
uint64_t zobSide;
uint64_t zobSeed;
bool     zobReady;

//the transpo table
TranspoEntry transpoTable[TRANSPO_SIZE];

//checks time limit, only reads clock every 2048 nodes to reduce overhead
bool timeUp(void)
{
    double elapsed;

    if (nodeCount++ & 2047)
    {
        return false;
    }
    elapsed = (double)(clock() - searchStartTime) / CLOCKS_PER_SEC;
    return elapsed >= TIME_LIMIT_SECS;
}

//xorshift64* - three shift-xors then multiply to spread entropy across all 64 bits
uint64_t rand64(void)
{
    zobSeed ^= zobSeed >> 12;
    zobSeed ^= zobSeed << 25;
    zobSeed ^= zobSeed >> 27;
    return zobSeed * 0x2545F4914F6CDD1DULL;
}

//fills zobrist table with random keys before first search
void zobristInit(void)
{
    int rank;
    int file;
    int piecetype;
    int color;

    //seeding with time so keys differ each run
    srand((unsigned)time(NULL));
    zobSeed = ((uint64_t)rand() << 32) | (uint64_t)rand();
    if (zobSeed == 0)
    {
        zobSeed = 1;
    }

    for (rank = 0; rank < 8; rank++)
    {
        for (file = 0; file < 10; file++)
        {
            for (piecetype = 0; piecetype < 8; piecetype++)
            {
                for (color = 0; color < 2; color++)
                {
                    zobPiece[rank][file][piecetype][color] = rand64();
                }
            }
        }
    }
    zobSide  = rand64();
    zobReady = true;
}

//XORs a key for every piece on the board to produce a unique position fingerprint
uint64_t computeHash(const GameState *gs)
{
    uint64_t hash;
    int      rank;
    int      file;
    Piece    squarePiece;

    hash = 0;
    for (rank = 0; rank < 8; rank++)
    {
        for (file = 0; file < 10; file++)
        {
            squarePiece = gs->board[rank][file];
            if (squarePiece.piecetype != EMPTY)
            {
                hash ^= zobPiece[rank][file][squarePiece.piecetype][squarePiece.color];
            }
        }
    }
    if (gs->turn == BLUE)
    {
        hash ^= zobSide;
    }
    return hash;
}

//looks up position in transpo table, returns true if a usable result is found
bool transpoProbe(uint64_t key, int depth, int alpha, int beta, int *score)
{
    TranspoEntry *e;

    e = &transpoTable[key & TRANSPO_MASK];
    if (e->key != key || e->depth < depth)
    {
        return false;
    }
    if (e->flag == TRANSPO_EXACT)
    {
        *score = e->score;
        return true;
    }
    if (e->flag == TRANSPO_ALPHA && e->score <= alpha)
    {
        *score = alpha;
        return true;
    }
    if (e->flag == TRANSPO_BETA && e->score >= beta)
    {
        *score = beta;
        return true;
    }
    return false;
}

//stores a search result - keeps deeper entries since they have more info
void transpoStore(uint64_t key, int depth, int score, int flag, Move bestMove)
{
    TranspoEntry *e;

    e = &transpoTable[key & TRANSPO_MASK];
    if (e->key == key && e->depth > depth)
    {
        return;
    }
    e->key      = key;
    e->depth    = depth;
    e->score    = score;
    e->flag     = flag;
    e->bestMove = bestMove;
}

//fills out with the stored best move for this position, returns 1 if found
int transpoGetBestMove(uint64_t key, Move *out)
{
    TranspoEntry *e;

    e = &transpoTable[key & TRANSPO_MASK];
    if (e->key != key)
    {
        return 0;
    }
    *out = e->bestMove;
    return 1;
}

//comparing from and to squares of two moves
int movesEqual(Move a, Move b)
{
    return a.from.rank == b.from.rank && a.from.file == b.from.file &&
           a.to.rank   == b.to.rank   && a.to.file   == b.to.file;
}

//stores a quiet beta-cutoff move as a killer, shifting old killer to slot 1
void updateKiller(int ply, Move move)
{
    if (ply >= MAX_KILLER_PLY)
    {
        return;
    }
    if (!movesEqual(killerMoves[ply][0], move))
    {
        killerMoves[ply][1] = killerMoves[ply][0];
        killerMoves[ply][0] = move;
    }
}

//scores a move for ordering - captures use MVV-LVA, killers score just below min capture, quiet moves score 0
int scoreMove(const GameState *gs, Move m, int ply)
{
    PieceType victim;
    PieceType attacker;
    int       h;

    victim   = gs->board[m.to.rank  ][m.to.file  ].piecetype;
    attacker = gs->board[m.from.rank][m.from.file].piecetype;
    if (victim != EMPTY)
    {
        return PIECE_VALUE[victim] * 10 - PIECE_VALUE[attacker];
    }
    //checking killer slots before falling back to history
    if (ply < MAX_KILLER_PLY)
    {
        if (movesEqual(m, killerMoves[ply][0])) return 90;
        if (movesEqual(m, killerMoves[ply][1])) return 80;
    }
    // penalize queen moves early - searched last so eval penalty has more effect
    if(attacker == QUEEN){
        return -50;
    }
    //history score capped at 70 so it stays below killers
    h = historyTable[m.from.rank][m.from.file][m.to.rank][m.to.file];
    if (h > 0) return (h > 70) ? 70 : h;
    return 0;
}

//insertion sort descending by score - captures first, then killers, then quiet moves
void sortMoves(const GameState *gs, MoveList *moves, int ply)
{
    int  scores[250];
    int  i;
    int  j;
    Move tmp;
    int  ts;

    for (i = 0; i < moves->count; i++)
    {
        scores[i] = scoreMove(gs, moves->moves[i], ply);
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

//returns board state after a move without modifying the original - used by textui
GameState apply_move(const GameState *gs, Move move)
{
    GameState nextState;
    UndoData  undo;

    nextState            = *gs;
    nextState.prev_state = NULL;
    make_move_in_place(&nextState, move, &undo);

    return nextState;
}

//searches captures only at leaf nodes to avoid stopping in the middle of an exchange
int quiesce(GameState *gs, int alpha, int beta, int qDepth)
{
    int      standPat;
    int      score;
    int      i;
    MoveList moves;
    UndoData undo;

    if (searchAborted)
    {
        return 0;
    }
    if (timeUp())
    {
        searchAborted = true;
        return 0;
    }

    //scoring position as-is, if already good enough stop here
    standPat = evaluate(gs);

    if (standPat >= beta)
    {
        return beta;
    }
    if (standPat > alpha)
    {
        alpha = standPat;
    }
    if (qDepth == 0)
    {
        return alpha;
    }

    moves = legalMoveGen(gs);
    sortMoves(gs, &moves, MAX_KILLER_PLY);

    for (i = 0; i < moves.count; i++)
    {
        //skipping non-captures
        if (gs->board[moves.moves[i].to.rank][moves.moves[i].to.file].piecetype == EMPTY)
        {
            continue;
        }

        make_move_in_place(gs, moves.moves[i], &undo);
        score = -quiesce(gs, -beta, -alpha, qDepth - 1);
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

//recursive negamax search with alpha-beta pruning
//score is from the perspective of whoever is moving - positive means winning
//ply = distance from root, used to prefer shorter mates
int negamax(GameState *gs, int depth, int alpha, int beta, int ply)
{
    uint64_t key;
    int      transpoScore;
    MoveList moves;
    int      best;
    int      i;
    int      score;
    int      flag;
    Color    sideToMove;
    UndoData undo;
    Move     transpoMove;
    int      transpoMoveFound;
    Move     bestMoveLocal;

    if (searchAborted)
    {
        return 0;
    }
    if (timeUp())
    {
        searchAborted = true;
        return 0;
    }

    //checking if this position was already searched at this depth
    key = computeHash(gs);
    if (transpoProbe(key, depth, alpha, beta, &transpoScore))
    {
        return transpoScore;
    }

    if (depth == 0)
    {
        return quiesce(gs, alpha, beta, 1);
    }

    moves = legalMoveGen(gs);
    sortMoves(gs, &moves, ply);

    if (moves.count == 0)
    {
        if (inCheck(gs, gs->turn))
        {
            //shorter mate scores higher than longer mate
            return -(INF - ply);
        }
        return 0;
    }

    //swapping the transpo best move to index 0 so it gets searched first
    transpoMoveFound = transpoGetBestMove(key, &transpoMove);
    if (transpoMoveFound)
    {
        for (i = 0; i < moves.count; i++)
        {
            if (movesEqual(moves.moves[i], transpoMove))
            {
                Move tmp       = moves.moves[0];
                moves.moves[0] = moves.moves[i];
                moves.moves[i] = tmp;
                break;
            }
        }
    }
    bestMoveLocal = moves.moves[0];

    //null move pruning - passing the turn still beats beta so prune
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
    flag = TRANSPO_ALPHA;

    for (i = 0; i < moves.count; i++)
    {
        int reduction;
        int isCapture;
        int isKiller;
        int h;

        sideToMove = gs->turn;
        isCapture  = gs->board[moves.moves[i].to.rank][moves.moves[i].to.file].piecetype != EMPTY;

        //late move reductions - quiet moves searched late are likely bad, search them shallower
        //killers and high-history moves are exempt since they have evidence of being strong
        //if reduced score still raises alpha, re-search at full depth to confirm
        reduction = 0;
        if (depth >= 3 && i >= 3 && !isCapture && !inCheck(gs, gs->turn))
        {
            isKiller = (ply < MAX_KILLER_PLY &&
                       (movesEqual(moves.moves[i], killerMoves[ply][0]) ||
                        movesEqual(moves.moves[i], killerMoves[ply][1])));
            h = historyTable[moves.moves[i].from.rank][moves.moves[i].from.file]
                            [moves.moves[i].to.rank  ][moves.moves[i].to.file  ];
            if (!isKiller && h < 50)
            {
                reduction = (i >= 6) ? 2 : 1;
            }
        }

        make_move_in_place(gs, moves.moves[i], &undo);

        if (gs->turn == sideToMove)
        {
            score = negamax(gs, depth - 1 - reduction, alpha, beta, ply + 1);
            if (reduction > 0 && score > alpha)
            {
                score = negamax(gs, depth - 1, alpha, beta, ply + 1);
            }
        }
        else
        {
            score = -negamax(gs, depth - 1 - reduction, -beta, -alpha, ply + 1);
            if (reduction > 0 && score > alpha)
            {
                score = -negamax(gs, depth - 1, -beta, -alpha, ply + 1);
            }
        }

        undo_move_in_place(gs, moves.moves[i], &undo);

        if (score > best)
        {
            best = score;
        }
        if (score > alpha)
        {
            alpha         = score;
            flag          = TRANSPO_EXACT;
            bestMoveLocal = moves.moves[i];
        }
        if (alpha >= beta)
        {
            if (gs->board[moves.moves[i].to.rank][moves.moves[i].to.file].piecetype == EMPTY)
            {
                //quiet cutoff - store as killer and bump history so this move sorts higher next time
                updateKiller(ply, moves.moves[i]);
                historyTable[moves.moves[i].from.rank][moves.moves[i].from.file]
                            [moves.moves[i].to.rank  ][moves.moves[i].to.file  ] += depth * depth;
            }
            transpoStore(key, depth, best, TRANSPO_BETA, bestMoveLocal);
            return best;
        }
    }

    transpoStore(key, depth, best, flag, bestMoveLocal);
    return best;
}

Move* SelectBestMove(GameState *gs, Color color, int depth)
{
    static Move bestMove;
    Move        bestAtDepth;
    MoveList    moves;
    int         bestScoreAtDepth;
    int         alpha;
    int         beta;
    int         i;
    int         currentDepth;
    int         score;
    clock_t     start;
    double      elapsed;
    Color       sideToMove;
    UndoData    undo;

    (void)color;

    if (!zobReady)
    {
        zobristInit();
    }

    searchStartTime = clock();
    searchAborted   = false;
    nodeCount       = 0;
    start           = searchStartTime;
    memset(killerMoves,  0, sizeof(killerMoves));
    memset(historyTable, 0, sizeof(historyTable));

    moves = legalMoveGen(gs);
    sortMoves(gs, &moves, 0);

    if (moves.count == 0)
    {
        return NULL;
    }

    bestMove = moves.moves[0];

    //searching depth 2, then 4, then 6... stopping when time runs out
    for (currentDepth = 2; currentDepth <= depth; currentDepth += 2)
    {
        elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
        if (elapsed >= TIME_LIMIT_SECS)
        {
            break;
        }

        bestAtDepth      = bestMove;
        bestScoreAtDepth = -INF;
        alpha            = -INF;
        beta             = INF;

        for (i = 0; i < moves.count; i++)
        {
            sideToMove = gs->turn;

            make_move_in_place(gs, moves.moves[i], &undo);

            if (gs->turn == sideToMove)
            {
                score = negamax(gs, currentDepth - 1, alpha, beta, 1);
            }
            else
            {
                score = -negamax(gs, currentDepth - 1, -beta, -alpha, 1);
            }

            undo_move_in_place(gs, moves.moves[i], &undo);

            if (searchAborted)
            {
                break;
            }

            if (i == 0 || score > bestScoreAtDepth)
            {
                bestScoreAtDepth = score;
                bestAtDepth      = moves.moves[i];
            }

            if (score > alpha)
            {
                alpha = score;
            }
        }

        if (!searchAborted)
        {
            bestMove = bestAtDepth;
            elapsed  = (double)(clock() - start) / CLOCKS_PER_SEC;
            printf("depth %d done in %.2fs  best: %c%d -> %c%d\n",
                   currentDepth,
                   elapsed,
                   'A' + bestAtDepth.from.file, bestAtDepth.from.rank + 1,
                   'A' + bestAtDepth.to.file,   bestAtDepth.to.rank   + 1);
            fflush(stdout);
        }
        else
        {
            break;
        }
    }

    return &bestMove;
}
