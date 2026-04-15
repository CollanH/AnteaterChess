#ifndef STRATEGY_H
#define STRATEGY_H

#include "chess_types.h"
#include "legalMoveGen.h"

//function declarations

//selects the best move for the AI using negamax with alpha-beta pruning
//depth: Easy=2, Medium=4, Hard=6
//returns NULL if no legal moves exist (checkmate or stalemate)
Move* SelectBestMove(GameState *gs, Color color, int depth);

#endif /* STRATEGY_H */
