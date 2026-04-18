//
// Created by clair on 4/14/2026.
//
#include "chess_types.h"
#ifndef CHESS22L_LEGALMOVEGEN_H
#define CHESS22L_LEGALMOVEGEN_H
	MoveList legalMoveGen(GameState *gs);
	MoveList findPossibleMoves(GameState *gs, Square square);
#endif //CHESS22L_LEGALMOVEGEN_H
