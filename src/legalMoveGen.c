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
bool inCheck(GameState* gs, Color color);
void squareToMoves(const GameState *gs, Square square, MoveList* moveList);
void kingMoves(const GameState *pState, Square square, MoveList *list);
void anteaterMoves(const GameState *pState, Square square, MoveList *list);
void queenMoves(const GameState *pState, Square square, MoveList *list);
void bishopMoves(const GameState *pState, Square square, MoveList *list);
void knightMoves(const GameState *pState, Square square, MoveList *list);
void rookMoves(const GameState *pState, Square square, MoveList *list);
void antMoves(const GameState *pState, Square square, MoveList *list);

// returns movelist of moves based on a square
MoveList findPossibleMoves(const GameState *gs, Square square) {
	MoveList moveList = initialize_moveList();
	squareToMoves(gs, square, &moveList);

	return moveList;
}
MoveList legalMoveGen(const GameState *gs) {
	MoveList moveList = initialize_moveList();

	Color turn = gs->turn;
	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 10; j++){
			if((gs->board[i][j].piecetype) != EMPTY && gs->board[i][j].color == turn)
				squareToMoves(gs, make_square(i, (File)j), &moveList);
		}
	}

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
bool inCheck(GameState* gs, Color color){
	bool check = queen_bishop_rook_check(gs, color)
			|| pawn_check(gs, color)
			|| knight_check(gs, color)
			|| king_check(gs, color) ;


	return check;


}

void kingMoves(const GameState *gs, Square square, MoveList* moveList){

}


void antMoves(const GameState *pState, Square square, MoveList *list) {

}

void rookMoves(const GameState *pState, Square square, MoveList *list) {

}

void knightMoves(const GameState *pState, Square square, MoveList *list) {

}

void bishopMoves(const GameState *pState, Square square, MoveList *list) {

}

void queenMoves(const GameState *pState, Square square, MoveList *list) {

}

void anteaterMoves(const GameState *pState, Square square, MoveList *list) {

}
