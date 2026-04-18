#include "strategy.h"
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

//TODO: MVV-LVA move ordering
//  score_move(gs, move) -> victim_value - attacker_value/10, captures score high, quiet moves score 0
//  sort_moves(gs, moves) -> selection sort using score_move, search best captures first

//TODO: quiescence search
//  at depth 0, keep searching captures until position is quiet before scoring
//  prevents horizon effect (AI walking into a recapture it cant see)

//takes the current board and a move, returns what the board looks like after that move
//never touches the original board - negamax needs it unchanged to keep searching
GameState apply_move(const GameState *gs, Move move)
{
    GameState nextState;
    Square from;
    Square to;
    PieceType movedPiece;
    Color currentTurn;
    int rankDistance;
    int fileDistance;
    int direction;
    int fileIdx;
    int rankIdx;

    //stack copy for search tree, no malloc - make_move in chess_types is heap based (for GUI)
    nextState              = *gs;
    nextState.prev_state   = NULL;
    replace_piece(&nextState, *piece_at(gs, move.from), move.to);
    replace_piece(&nextState, make_piece(EMPTY, YELLOW), move.from);

    from = move.from;
    to   = move.to;

    //figure out what piece moved and whose turn it was
    movedPiece  = gs->board[from.rank][from.file].piecetype;
    currentTurn = gs->turn;

    //anteater special rule, if it moved more than one square along a rank or file
    //all the ants it ran through in between get eaten too
    if (movedPiece == ANTEATER)
    {
        rankDistance = to.rank - from.rank;
        fileDistance = to.file - from.file;

        //moved horizontally more than one square, eat ants along the rank
        if (rankDistance == 0 && (fileDistance > 1 || fileDistance < -1))
        {
            if (fileDistance > 0)
            {
                direction = 1;
            }
            else
            {
                direction = -1;
            }

            for (fileIdx = from.file + direction; fileIdx != to.file; fileIdx += direction)
            {
                nextState.board[from.rank][fileIdx] = (Piece){EMPTY, YELLOW};
            }
        }

        //moved vertically more than one square, eat ants along the file
        if (fileDistance == 0 && (rankDistance > 1 || rankDistance < -1))
        {
            if (rankDistance > 0)
            {
                direction = 1;
            }
            else
            {
                direction = -1;
            }

            for (rankIdx = from.rank + direction; rankIdx != to.rank; rankIdx += direction)
            {
                nextState.board[rankIdx][from.file] = (Piece){EMPTY, YELLOW};
            }
        }

        nextState.anteater_ate = true;
    }
    else
    {
        nextState.anteater_ate = false;
    }

    //en passant, ant moved diagonally to an empty square
    //the captured ant isnt at the destination, its sitting beside the attacker
    //so we have to manually remove it
    if (movedPiece == ANT)
    {
        fileDistance = to.file - from.file;

        //moved diagonally to empty square = en passant, remove the ant that got bypassed
        if (fileDistance != 0 && gs->board[to.rank][to.file].piecetype == EMPTY)
        {
            nextState.board[from.rank][to.file] = (Piece){EMPTY, YELLOW};
        }
    }

    //reset en passant square every turn, its only valid for one turn
    nextState.en_passant_square.rank = -1;
    nextState.en_passant_square.file = A;

    //if an ant just double pushed, set the square it skipped over
    //so legalMoveGen knows en passant is available next turn
    if (movedPiece == ANT)
    {
        rankDistance = to.rank - from.rank;

        if (rankDistance == -2 || rankDistance == 2)
        {
            nextState.en_passant_square.rank = (from.rank + to.rank) / 2;
            nextState.en_passant_square.file = from.file;
        }
    }

    //ant reached the back rank, promote it to queen
    if (movedPiece == ANT && (to.rank == 0 || to.rank == 7))
    {
        nextState.board[to.rank][to.file].piecetype = QUEEN;
    }

    //king moved, revoke both castling rights for that side
    if (movedPiece == KING)
    {
        if (currentTurn == YELLOW)
        {
            nextState.yellow_kscastle = false;
            nextState.yellow_qscastle = false;
        }
        else
        {
            nextState.blue_kscastle = false;
            nextState.blue_qscastle = false;
        }
    }

    //move is done, flip whose turn it is
    if (currentTurn == YELLOW)
    {
        nextState.turn = BLUE;
    }
    else
    {
        nextState.turn = YELLOW;
    }

    return nextState;
}

//the actual AI search, tries every move and picks the best one
//score is always from the perspective of whoever is moving right now
//positive = winning, negative = losing
static int negamax(GameState *gs, int depth, int alpha, int beta)
{
    MoveList moves;
    int best;
    int i;
    GameState next;
    int score;

    //hit the depth limit, score the board as is
    if (depth == 0)
    {
        return evaluate(gs);
    }

    //get all legal moves for whoever's turn it is
    moves = legalMoveGen(gs);

    //no moves means its either checkmate or stalemate
    if (moves.count == 0)
    {
        //in check with no moves = checkmate, worst possible outcome
        if (inCheck(gs, gs->turn))
        {
            return -INF;
        }

        //not in check with no moves = stalemate, its a draw
        return 0;
    }

    best = -INF;

    for (i = 0; i < moves.count; i++)
    {
        //apply this move to get the resulting board
        next = apply_move(gs, moves.moves[i]);

        //recurse but negate the score because its the opponents turn now
        //what is good for them is bad for us
        score = -negamax(&next, depth - 1, -beta, -alpha);

        //keep track of the best score we found so far
        if (score > best)
        {
            best = score;
        }

        //alpha is the best we are guaranteed to get from here
        if (score > alpha)
        {
            alpha = score;
        }

        //alpha >= beta means the opponent already has a better option elsewhere
        //they would never let us reach this position so stop searching it
        if (alpha >= beta)
        {
            break;
        }
    }

    return best;
}

//entry point, finds and returns the best move for the AI to play
//uses iterative deepening, searches depth 2, 4, 6, 8... until 50 seconds
//always has a valid move saved from the last completed depth
Move* SelectBestMove(GameState *gs, Color color, int depth)
{
    MoveList moves;
    static Move best_move;
    Move best_at_depth;
    int best_score;
    int best_score_at_depth;
    int alpha;
    int beta;
    int i;
    int current_depth;
    GameState next;
    int score;
    clock_t start;
    double elapsed;

    (void)color;

    //get all legal moves for the current position
    moves = legalMoveGen(gs);

    //no moves means checkmate or stalemate - nothing to return
    if (moves.count == 0)
    {
        return NULL;
    }

    start     = clock();
    best_move = moves.moves[0];

    //TODO: call sort_moves here once implemented

    //iterative deepening up to the requested depth, stops early if time runs out
    for (current_depth = 2; current_depth <= depth; current_depth += 2)
    {
        //check time before starting a new depth
        elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
        if (elapsed >= TIME_LIMIT_SECS)
        {
            break;
        }

        best_score_at_depth = -INF;
        alpha               = -INF;
        beta                =  INF;

        for (i = 0; i < moves.count; i++)
        {
            //apply the move and score the resulting position
            next = apply_move(gs, moves.moves[i]);

            //negated because negamax scores from the opponents perspective after our move
            score = -negamax(&next, current_depth - 1, -beta, -alpha);

            //if this move scored higher than anything at this depth, save it
            if (score > best_score_at_depth)
            {
                best_score_at_depth = score;
                best_at_depth       = moves.moves[i];

                if (score > alpha)
                {
                    alpha = score;
                }
            }
        }

        //completed this depth fully, save as our best result
        best_score = best_score_at_depth;
        best_move  = best_at_depth;

        (void)best_score;
    }

    return &best_move;
}
