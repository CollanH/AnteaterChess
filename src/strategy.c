#include "strategy.h"
#include <stdlib.h>

//provided by eval.c - scores position relative to side to move
int evaluate(GameState *gs);

//provided by legalMoveGen.c - checks if color's king is under attack
bool inCheck(GameState *gs, Color color);

//large value used to represent checkmate score
#define INF 1000000

//internal helper declarations
static GameState apply_move(const GameState *gs, Move move);
static int negamax(GameState *gs, int depth, int alpha, int beta);

//applies move to a copy of gs and returns the new gamestate
//does not modify the original gs - negamax needs it intact for backtracking
static GameState apply_move(const GameState *gs, Move move)
{
    //copy gs and perform the basic piece movement
    //make_move handles: place piece at to, clear from, capture whatever is at to
    GameState next = make_move(gs, move);

    Square from = move.from;
    Square to   = move.to;

    //identifying what piece just moved
    PieceType moved = gs->board[from.rank][from.file].piecetype;
    Color     side  = gs->turn;

    //handling anteater chain capture
    //if anteater moved more than one step along a rank or file,
    //all enemy ants between from and to must be cleared
    if (moved == ANTEATER)
    {
        int dr = to.rank - from.rank;
        int df = to.file - from.file;

        //chain along same rank (horizontal move, more than one step)
        if (dr == 0 && (df > 1 || df < -1))
        {
            int step = (df > 0) ? 1 : -1;
            for (int f = from.file + step; f != to.file; f += step)
                next.board[from.rank][f] = (Piece){EMPTY, YELLOW};
        }

        //chain along same file (vertical move, more than one step)
        if (df == 0 && (dr > 1 || dr < -1))
        {
            int step = (dr > 0) ? 1 : -1;
            for (int r = from.rank + step; r != to.rank; r += step)
                next.board[r][from.file] = (Piece){EMPTY, YELLOW};
        }

        next.anteater_ate = true;
    }
    else
    {
        next.anteater_ate = false;
    }

    //handling en passant capture
    //an ant captures diagonally to an empty square matching en_passant_square
    //the captured ant sits on the same rank as the attacker, same file as destination
    if (moved == ANT)
    {
        int df = to.file - from.file;

        //diagonal move to empty square means en passant
        if (df != 0 && gs->board[to.rank][to.file].piecetype == EMPTY)
            next.board[from.rank][to.file] = (Piece){EMPTY, YELLOW};
    }

    //updating en passant square for next turn
    //only set if an ant just double pushed, otherwise clear it
    next.en_passant_square.rank = -1;
    next.en_passant_square.file = A;

    if (moved == ANT)
    {
        int dr = to.rank - from.rank;

        //yellow ants double push from rank 6 downward (rank decreases)
        //blue ants double push from rank 1 upward (rank increases)
        if (dr == -2 || dr == 2)
        {
            next.en_passant_square.rank = (from.rank + to.rank) / 2;
            next.en_passant_square.file = from.file;
        }
    }

    //handling ant promotion
    //yellow ants promote at rank 0, blue ants promote at rank 7
    if (moved == ANT && (to.rank == 0 || to.rank == 7))
    {
        next.board[to.rank][to.file].piecetype = QUEEN;
    }
        
    //updating castling flags if king moved
    //once the king moves, castling is no longer available for that side
    if (moved == KING)
    {
        if (side == YELLOW)
        {
            next.yellow_castled = true;
        }
        else
        {
            next.blue_castled   = true;
        }                
    }
    //flipping turn - after a move it is the other player's turn
    next.turn = (side == YELLOW) ? BLUE : YELLOW;

    return next;
}

//recursive negamax search with alpha-beta pruning
//returns a score relative to the side to move
//positive = side to move is winning, negative = losing
static int negamax(GameState *gs, int depth, int alpha, int beta)
{
    //base case - depth reached, score the position as-is
    if (depth == 0)
    {
        return evaluate(gs);
    }
        
    //generating all legal moves for the current side
    MoveList moves = legalMoveGen(gs);

    //no legal moves means checkmate or stalemate
    if (moves.count == 0)
    {
        //checkmate - current side has no moves and is in check, very bad
        if (inCheck(gs, gs->turn)) return -INF;

        //stalemate - no moves but not in check, draw
        return 0;
    }

    int best = -INF;

    for (int i = 0; i < moves.count; i++)
    {
        //applying the move to get the resulting board state
        GameState next = apply_move(gs, moves.moves[i]);

        //recursing - negate the result because after the move
        //it is the opponent's turn and their best is our worst
        int score = -negamax(&next, depth - 1, -beta, -alpha);

        //tracking the best score found so far
        if (score > best)
        {
            best = score;
        }
        
        //updating alpha - best score we can guarantee from here
        if (score > alpha)
        {
            alpha = score;
        }
            
        //beta cutoff - opponent already has a better option elsewhere
        //no point searching further, they will never let us reach this branch
        if (alpha >= beta)
        {
            break;
        }
    }

    return best;
}

//selects the best move for the AI player
//iterates over all legal moves, scores each with negamax, returns the best
Move* SelectBestMove(GameState *gs, Color color, int depth)
{
    (void)color;

    //generating all legal moves for the current position
    MoveList moves = legalMoveGen(gs);

    //no legal moves means checkmate or stalemate, nothing to return
    if (moves.count == 0) return NULL;

    static Move best_move;
    int best_score = -INF;
    int alpha      = -INF;
    int beta       =  INF;

    for (int i = 0; i < moves.count; i++)
    {
        //applying each move to get the resulting board state
        GameState next = apply_move(gs, moves.moves[i]);

        //scoring the resulting position from the opponent's perspective
        //negated because negamax returns score for the side to move (opponent)
        int score = -negamax(&next, depth - 1, -beta, -alpha);

        //keeping track of the highest scoring move seen so far
        if (score > best_score)
        {
            best_score = score;
            best_move  = moves.moves[i];

            //updating alpha so negamax can prune in subsequent calls
            if (score > alpha)
                alpha = score;
        }
    }

    return &best_move;
}
