//
// Created by clair on 4/9/2026.
//
#include "chess_types.h"
/*
 * 1. Find theoretical moves as if the board was empty
 * 2. Find moves that don't overlap "your" pieces
 * 3. Check if it puts your king in check
 *
 */


/*
 * returns true if the king of color is being attacked by an enemy piece
 * returns false otherwise
 */
bool inCheck(const GameState* gs, Color color);
void squareToMoves(const GameState *gs, Square square, MoveList* moveList);
void kingMoves(const GameState *pState, Square square, MoveList *list);
void anteaterMoves(const GameState *pState, Square square, MoveList *list);
void queenMoves(const GameState *pState, Square square, MoveList *list);
void bishopMoves(const GameState *pState, Square square, MoveList *list);
void knightMoves(const GameState *pState, Square square, MoveList *list);
void rookMoves(const GameState *pState, Square square, MoveList *list);
void antMoves(const GameState *pState, Square square, MoveList *list);

void clean_moveList(GameState *gs,MoveList* moveList) {
	int count = 0;
	int to_delete[250];
	for(int i = 0; i < moveList->count; i++) {
		Move* move = moveList_at(moveList, i);

		if(piece_at(gs, move->to)->piecetype != EMPTY && piece_at(gs, move->to)->color == gs->turn) {
			to_delete[count] = i;
			count++;
		}
		else {
			// make a move to check if it causes
			Piece occupant = *piece_at(gs, move->to);
			Piece mover = *piece_at(gs, move->from);

			gs->board[move->to.rank][move->to.file] = mover;
			gs->board[move->from.rank][move->from.file] = make_piece(EMPTY, YELLOW);
			if(inCheck(gs, gs->turn)) {
				to_delete[count] = i;
				count++;
			}
			gs->board[move->to.rank][move->to.file] = occupant;
			gs->board[move->from.rank][move->from.file] = mover;
		}


	}

	for(int i = count - 1; i >= 0; i--) {
		delete_move(moveList, to_delete[i]);
	}

}

// returns movelist of moves based on a square
MoveList findPossibleMoves(GameState *gs, Square square) {
	MoveList moveList = initialize_moveList();
	squareToMoves(gs, square, &moveList);
	clean_moveList(gs, &moveList);

	return moveList;
}
MoveList legalMoveGen(GameState *gs) {
	MoveList moveList = initialize_moveList();

	Color turn = gs->turn;
	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 10; j++){
			if((gs->board[i][j].piecetype) != EMPTY && gs->board[i][j].color == turn)
				squareToMoves(gs, make_square(i, (File)j), &moveList);
		}
	}

	clean_moveList(gs, &moveList);

	return moveList;

}


void squareToMoves(const GameState *gs, Square square, MoveList* moveList) {
	PieceType pt = gs->board[square.rank][square.file].piecetype;
	if(pt == EMPTY)
		return;

	switch (pt) {
		case KING:
			kingMoves(gs, square, moveList);
			break;
		case QUEEN:
			queenMoves(gs, square, moveList);
			break;
		case ANTEATER:
			anteaterMoves(gs, square, moveList);
			break;
		case BISHOP:
			bishopMoves(gs, square, moveList);
			break;
		case KNIGHT:
			knightMoves(gs, square, moveList);
			break;
		case ROOK:
			rookMoves(gs, square, moveList);
			break;
		case ANT:
			antMoves(gs, square, moveList);
			break;
		default:
			break;
	}





}

// true if king of color is in check
// false otherwise
Square find_king_square(const GameState* gs, Color color) {
	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 10; j++){
			if((gs->board[i][j].piecetype) == KING && gs->board[i][j].color == color)
				return make_square(i, (File)j);
		}
	}

	return make_square(-1,-1);

}

bool in_bounds(Square square){
	return square.rank < 8 && square.rank >= 0 && square.file < 10 && square.file >= 0;
}

bool queen_bishop_rook_check(const GameState* gs, Color color){
	Square king_square = find_king_square(gs, color);

	// look UP for rooks and queens
	Square sq = make_square(king_square.rank - 1, king_square.file);

	Piece piece;
	while(in_bounds(sq)){
		piece = *piece_at(gs, sq);
		if(piece.piecetype != EMPTY){
			if(piece.color != color){
				if(piece.piecetype == ROOK || piece.piecetype == QUEEN){
					return true;
				}
			}
			break;
		} else {
			sq.rank--;
		}
	}

	// look DOWN for rooks and queens
	sq.rank = king_square.rank + 1;
	while(in_bounds(sq)){
		piece = *piece_at(gs, sq);
		if(piece.piecetype != EMPTY){
			if(piece.color != color){
				if(piece.piecetype == ROOK || piece.piecetype == QUEEN){
					return true;
				}
			}
			break;
		} else {
			sq.rank++;
		}
	}

	// look RIGHT for rooks and queens
	sq.rank = king_square.rank;
	sq.file = king_square.file + 1;
	while(in_bounds(sq)){
		piece = *piece_at(gs, sq);
		if(piece.piecetype != EMPTY){
			if(piece.color != color){
				if(piece.piecetype == ROOK || piece.piecetype == QUEEN){
					return true;
				}
			}
			break;
		} else {
			sq.file++;
		}
	}

	// look LEFT for rooks and queens
	sq.file = king_square.file - 1;
	while(in_bounds(sq)){
		piece = *piece_at(gs, sq);
		if(piece.piecetype != EMPTY){
			if(piece.color != color){
				if(piece.piecetype == ROOK || piece.piecetype == QUEEN){
					return true;
				}
			}
			break;
		} else {
			sq.file--;
		}
	}

	// look up-left for queens and bishops
	sq.rank = king_square.rank - 1;
	sq.file = king_square.file - 1;
	while(in_bounds(sq)){
		piece = *piece_at(gs, sq);
		if(piece.piecetype != EMPTY){
			if(piece.color != color){
				if(piece.piecetype == BISHOP || piece.piecetype == QUEEN){
					return true;
				}
			}
			break;
		} else {
			sq.rank--;
			sq.file--;
		}
	}
	// look up-right for queens and bishops
	sq.rank = king_square.rank - 1;
	sq.file = king_square.file + 1;
	while(in_bounds(sq)){
		piece = *piece_at(gs, sq);
		if(piece.piecetype != EMPTY){
			if(piece.color != color){
				if(piece.piecetype == BISHOP || piece.piecetype == QUEEN){
					return true;
				}
			}
			break;
		} else {
			sq.rank--;
			sq.file++;
		}
	}
	// look down-right for queens and bishops
	sq.rank = king_square.rank + 1;
	sq.file = king_square.file + 1;
	while(in_bounds(sq)){
		piece = *piece_at(gs, sq);
		if(piece.piecetype != EMPTY){
			if(piece.color != color){
				if(piece.piecetype == BISHOP || piece.piecetype == QUEEN){
					return true;
				}
			}
			break;
		} else {
			sq.rank++;
			sq.file++;
		}
	}
	// look down-left for queens and bishops
	sq.rank = king_square.rank + 1;
	sq.file = king_square.file - 1;
	while(in_bounds(sq)){
		piece = *piece_at(gs, sq);
		if(piece.piecetype != EMPTY){
			if(piece.color != color){
				if(piece.piecetype == BISHOP || piece.piecetype == QUEEN){
					return true;
				}
			}
			break;
		} else {
			sq.rank++;
			sq.file--;
		}
	}
	return false;
}

bool pawn_check(const GameState* gs, Color color) {
	Square king_square = find_king_square(gs, color);
	if(color == YELLOW) {
		Square sq1 = make_square(king_square.rank - 1, king_square.file - 1);
		Square sq2 = make_square(king_square.rank - 1, king_square.file + 1);

		if(in_bounds(sq1)){
			Piece piece = *piece_at(gs, sq1);
			if(piece.piecetype == ANT && piece.color == BLUE){
				return true;
			}
		}
		if(in_bounds(sq2)){
			Piece piece = *piece_at(gs, sq2);
			if(piece.piecetype == ANT && piece.color == BLUE){
				return true;
			}
		}
	}
	if(color == BLUE) {
		Square sq1 = make_square(king_square.rank + 1, king_square.file - 1);
		Square sq2 = make_square(king_square.rank + 1, king_square.file + 1);
		if(in_bounds(sq1)){
			Piece piece = *piece_at(gs, sq1);
			if(piece.piecetype == ANT && piece.color == YELLOW){
				return true;
			}
		}
		if(in_bounds(sq2)){
			Piece piece = *piece_at(gs, sq2);
			if(piece.piecetype == ANT && piece.color == YELLOW){
				return true;
			}
		}
	}

	return false;


}
bool knight_check(const GameState* gs, Color color) {
	Square king_square = find_king_square(gs, color);
	Color enemy = (color == YELLOW) ? BLUE : YELLOW;

	int dr[8] = {-2, -2, -1, -1, 1, 1, 2, 2};
	int df[8] = {-1,  1, -2,  2, -2, 2, -1, 1};

	for (int i = 0; i < 8; i++) {
		Square sq = make_square(king_square.rank + dr[i],
								king_square.file + df[i]);

		if (in_bounds(sq)) {
			Piece piece = *piece_at(gs, sq);
			if (piece.piecetype == KNIGHT && piece.color == enemy) {
				return true;
			}
		}
	}
	return false;
}

bool king_check(const GameState* gs, Color color){
	Square king_square = find_king_square(gs, color);
	Color enemy = (color == YELLOW) ? BLUE : YELLOW;

	int dr[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
	int df[8] = {-1, 0, 1, 1, 1, 0, -1, -1};

	for (int i = 0; i < 8; i++) {
		Square sq = make_square(king_square.rank + dr[i],
								king_square.file + df[i]);

		if (in_bounds(sq)) {
			Piece piece = *piece_at(gs, sq);
			if (piece.piecetype == KING && piece.color == enemy) {
				return true;
			}
		}
	}

	return false;
}



bool inCheck(const GameState* gs, Color color){
	bool check = queen_bishop_rook_check(gs, color)
			|| pawn_check(gs, color)
			|| knight_check(gs, color)
			|| king_check(gs, color) ;


	return check;


}
bool move_in_check(const GameState *gs, const Move* move) {
	GameState game = *gs;
	Piece mover = *piece_at(&game, move->from);

	game.board[move->to.rank][move->to.file] = mover;
	game.board[move->from.rank][move->from.file] = make_piece(EMPTY, YELLOW);
	if(inCheck(&game, mover.color)) {
		return true;
	}

	return false;

}

void kingMoves(const GameState *gs, Square square, MoveList* moveList){
	int dr[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
	int df[8] = {-1,  0,  1, 1, 1, 0, -1, -1};

	for (int i = 0; i < 8; i++) {
		Square to = make_square(square.rank + dr[i], square.file + df[i]);
		if(in_bounds(to)) {
			Move move;
			move.from = square;
			move.to = to;
			append_move(moveList, move);
		}
	}

	// castling logic

	if (piece_at(gs, square)->color == YELLOW) {
	    if (gs->yellow_qscastle && square.rank == 7 && square.file == F) {
	        bool can = true;

	        if (inCheck(gs, YELLOW)) {
	            can = false;
	        }

	        // squares between rook and king must be empty
	        if (piece_at(gs, make_square(7, B))->piecetype != EMPTY ||
	            piece_at(gs, make_square(7, C))->piecetype != EMPTY ||
	            piece_at(gs, make_square(7, D))->piecetype != EMPTY ||
	            piece_at(gs, make_square(7, E))->piecetype != EMPTY) {
	            can = false;
	        }

	        // king path safety: E, D
	        if (can) {
	            Move move;
	            move.from = square;

	            move.to = make_square(7, E);
	            if (move_in_check(gs, &move)) can = false;

	            move.to = make_square(7, D);
	            if (move_in_check(gs, &move)) can = false;
	        }

	        if (can) {
	            Move move;
	            move.from = square;
	            move.to = make_square(7, D);
	            append_move(moveList, move);
	        }
	    }

	    if (gs->yellow_kscastle && square.rank == 7 && square.file == F) {
	        bool can = true;

	        if (inCheck(gs, YELLOW)) {
	            can = false;
	        }

	        // squares between king and rook must be empty
	        if (piece_at(gs, make_square(7, G))->piecetype != EMPTY ||
	            piece_at(gs, make_square(7, H))->piecetype != EMPTY ||
	            piece_at(gs, make_square(7, I))->piecetype != EMPTY) {
	            can = false;
	        }

	        // king path safety: G, H
	        if (can) {
	            Move move;
	            move.from = square;

	            move.to = make_square(7, G);
	            if (move_in_check(gs, &move)) can = false;

	            move.to = make_square(7, H);
	            if (move_in_check(gs, &move)) can = false;
	        }

	        if (can) {
	            Move move;
	            move.from = square;
	            move.to = make_square(7, H);
	            append_move(moveList, move);
	        }
	    }
	}
	else {
	    if (gs->blue_qscastle && square.rank == 0 && square.file == F) {
	        bool can = true;

	        if (inCheck(gs, BLUE)) {
	            can = false;
	        }

	        // squares between rook and king must be empty
	        if (piece_at(gs, make_square(0, B))->piecetype != EMPTY ||
	            piece_at(gs, make_square(0, C))->piecetype != EMPTY ||
	            piece_at(gs, make_square(0, D))->piecetype != EMPTY ||
	            piece_at(gs, make_square(0, E))->piecetype != EMPTY) {
	            can = false;
	        }

	        // king path safety: E, D
	        if (can) {
	            Move move;
	            move.from = square;

	            move.to = make_square(0, E);
	            if (move_in_check(gs, &move)) can = false;

	            move.to = make_square(0, D);
	            if (move_in_check(gs, &move)) can = false;
	        }

	        if (can) {
	            Move move;
	            move.from = square;
	            move.to = make_square(0, D);
	            append_move(moveList, move);
	        }
	    }

	    if (gs->blue_kscastle && square.rank == 0 && square.file == F) {
	        bool can = true;

	        if (inCheck(gs, BLUE)) {
	            can = false;
	        }

	        // squares between king and rook must be empty
	        if (piece_at(gs, make_square(0, G))->piecetype != EMPTY ||
	            piece_at(gs, make_square(0, H))->piecetype != EMPTY ||
	            piece_at(gs, make_square(0, I))->piecetype != EMPTY) {
	            can = false;
	        }

	        // king path safety: G, H
	        if (can) {
	            Move move;
	            move.from = square;

	            move.to = make_square(0, G);
	            if (move_in_check(gs, &move)) can = false;

	            move.to = make_square(0, H);
	            if (move_in_check(gs, &move)) can = false;
	        }

	        if (can) {
	            Move move;
	            move.from = square;
	            move.to = make_square(0, H);
	            append_move(moveList, move);
	        }
	    }
	}

}




void anteaterMoves(const GameState *gs, Square square, MoveList *list) {
	Color myColor = piece_at(gs, square)->color;

	int dr[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
	int df[8] = {-1,  0,  1, 1, 1, 0, -1, -1};

	for (int i = 0; i < 8; i++) {
		// if chaining, skip diagonal moves
		if (gs->anteater_ate && dr[i] != 0 && df[i] != 0) {
			continue;
		}

		Square to = make_square(square.rank + dr[i], square.file + df[i]);
		if (!in_bounds(to)) {
			continue;
		}

		Piece dest = *piece_at(gs, to);

		if (gs->anteater_ate) {
			// chain move: only capture enemy ants, no empty-square moves
			if (dest.piecetype == ANT && dest.color != myColor) {
				Move move;
				move.from = square;
				move.to = to;
				append_move(list, move);
			}
		} else {
			// normal move: can move to empty square
			if (dest.piecetype == EMPTY) {
				Move move;
				move.from = square;
				move.to = to;
				append_move(list, move);
			}
			// or capture enemy ant
			else if (dest.piecetype == ANT && dest.color != myColor) {
				Move move;
				move.from = square;
				move.to = to;
				append_move(list, move);
			}
		}
	}
}
void rookMoves(const GameState *pState, Square square, MoveList *list) {
	int dr[4] = {-1, 1, 0, 0};
	int df[4] = {0, 0, -1, 1};

	for (int dir = 0; dir < 4; dir++) {
		Square to = make_square(square.rank + dr[dir], square.file + df[dir]);

		while (in_bounds(to)) {
			Move move;
			move.from = square;
			move.to = to;
			append_move(list, move);

			if (piece_at(pState, to)->piecetype != EMPTY) {
				break;
			}

			to.rank += dr[dir];
			to.file += df[dir];
		}
	}
}

void knightMoves(const GameState *pState, Square square, MoveList *list) {
	int dr[8] = {-2, -2, -1, -1, 1, 1, 2, 2};
	int df[8] = {-1,  1, -2,  2, -2, 2, -1, 1};

	for (int i = 0; i < 8; i++) {
		Square to = make_square(square.rank + dr[i], square.file + df[i]);
		if (in_bounds(to)) {
			Move move;
			move.from = square;
			move.to = to;
			append_move(list, move);
		}
	}
}

void bishopMoves(const GameState *pState, Square square, MoveList *list) {
	int dr[4] = {-1, -1, 1, 1};
	int df[4] = {-1, 1, -1, 1};

	for (int dir = 0; dir < 4; dir++) {
		Square to = make_square(square.rank + dr[dir], square.file + df[dir]);

		while (in_bounds(to)) {
			Move move;
			move.from = square;
			move.to = to;
			append_move(list, move);

			if (piece_at(pState, to)->piecetype != EMPTY) {
				break;
			}

			to.rank += dr[dir];
			to.file += df[dir];
		}
	}
}

void queenMoves(const GameState *pState, Square square, MoveList *list) {
	int dr[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
	int df[8] = {-1,  0,  1, 1, 1, 0, -1, -1};

	for (int dir = 0; dir < 8; dir++) {
		Square to = make_square(square.rank + dr[dir], square.file + df[dir]);

		while (in_bounds(to)) {
			Move move;
			move.from = square;
			move.to = to;
			append_move(list, move);

			if (piece_at(pState, to)->piecetype != EMPTY) {
				break;
			}

			to.rank += dr[dir];
			to.file += df[dir];
		}
	}
}

void antMoves(const GameState *gs, Square square, MoveList *list) {

	Color myColor = piece_at(gs, square)->color;
	int dir = (myColor == YELLOW) ? -1 : 1;
	int start_rank = (myColor == YELLOW) ? 6 : 1;

	Square forward = make_square(square.rank + dir, square.file);
	if (in_bounds(forward) && piece_at(gs, forward)->piecetype == EMPTY) {
		Move move = {square, forward};
		append_move(list, move);

		Square forward2 = make_square(square.rank + 2 * dir, square.file);
		if (square.rank == start_rank &&
			in_bounds(forward2) &&
			piece_at(gs, forward2)->piecetype == EMPTY) {
			Move move2 = {square, forward2};
			append_move(list, move2);
		}
	}

	Square diagL = make_square(square.rank + dir, square.file - 1);
	Square diagR = make_square(square.rank + dir, square.file + 1);

	if (in_bounds(diagL)) {
		Piece p = *piece_at(gs, diagL);
		if (p.piecetype != EMPTY && p.color != myColor) {
			Move move = {square, diagL};
			append_move(list, move);
		}
	}

	if (in_bounds(diagR)) {
		Piece p = *piece_at(gs, diagR);
		if (p.piecetype != EMPTY && p.color != myColor) {
			Move move = {square, diagR};
			append_move(list, move);
		}
	}

	if (in_bounds(gs->en_passant_square)) {
		if (gs->en_passant_square.rank == square.rank + dir &&
			(gs->en_passant_square.file == square.file - 1 ||
			 gs->en_passant_square.file == square.file + 1)) {
			Move move = {square, gs->en_passant_square};
			append_move(list, move);
			 }
	}
}