#include "strategy.h"
#include <stdlib.h>

//internal helper declarations
static int negamax(GameState *gs, int depth, int alpha, int beta);
static GameState apply_move(const GameState *gs, Move move);

//recursive negamax search with alpha-beta pruning
//returns a score relative to the side to move
static int negamax(GameState *gs, int depth, int alpha, int beta)
{
    //TODO: base case - if depth == 0, return evaluate(gs)
    //TODO: generate moves via legalMoveGen(gs)
    //TODO: for each move, apply it, recurse, track best score
    //TODO: prune branches using alpha and beta bounds
    (void)gs; (void)depth; (void)alpha; (void)beta;
    return 0;
}

//returns a new gamestate with the given move applied
//does not modify the original gs
static GameState apply_move(const GameState *gs, Move move)
{
    //TODO: copy gs into next
    //TODO: update board for the move
    //TODO: flip turn
    //TODO: handle special moves (en passant, castling, anteater capture)
    (void)move;
    GameState next = *gs;
    return next;
}

//selects the best move for the AI player
//iterates over all legal moves, scores each with negamax, returns the best
Move* SelectBestMove(GameState *gs, Color color, int depth)
{
    //TODO: generate all legal moves
    //TODO: if no moves, return NULL (checkmate or stalemate)
    //TODO: for each move, call negamax on the resulting position
    //TODO: track and return pointer to highest scoring move
    (void)color; (void)depth;
    MoveList moves = legalMoveGen(gs);
    if (moves.count == 0) return NULL;
    return NULL;
}
