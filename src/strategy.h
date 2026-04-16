#ifndef STRATEGY_H
#define STRATEGY_H

#include "chess_types.h"
#include "legalMoveGen.h"

//function declarations

//applies a move to a copy of the board and returns the new gamestate
//handles all special rules: chain captures, en passant, promotion, castling flags, turn flip
//call this in chess.c to update the real board after a move is chosen
GameState apply_move(const GameState *gs, Move move);

//selects the best move for the AI using negamax with alpha-beta pruning
//depth: Easy=2, Medium=4, Hard=6
//returns NULL if no legal moves exist (checkmate or stalemate)
Move* SelectBestMove(GameState *gs, Color color, int depth);

#endif /* STRATEGY_H */
