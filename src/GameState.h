//
// Created by clair on 4/9/2026.
//

#ifndef CHESS22L_GAMESTATE_H
#define CHESS22L_GAMESTATE_H

#include "Piece.h"
#include "Square.h"
#include "stdlib.h"





typedef struct GameState {
	Piece board[8][10];

	// -1,-1 if not available, points to square if yes en passant available
	Square enpassant;

	// 1 has already castled, 0 has not
	int whiteCastled;
	int blackCastled;

	//
	int turn;
	struct GameState* lastState;


} GameState;




#endif //CHESS22L_GAMESTATE_H
