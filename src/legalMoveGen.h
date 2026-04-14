//
// Created by clair on 4/14/2026.
//
#include "chess_types.h"
#ifndef CHESS22L_LEGALMOVEGEN_H
#define CHESS22L_LEGALMOVEGEN_H
	MoveList legalMoveGen(const GameState *gs);
	bool inCheck(GameState* gs, Color color);
#endif //CHESS22L_LEGALMOVEGEN_H
