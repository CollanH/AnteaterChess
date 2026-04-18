#include "chess_types.h"
#include "string.h"
#include "stdlib.h"

Square make_square(int rank, File file){
	Square sq;
	sq.file = file;
	sq.rank = rank;

	return sq;
}

MoveList initialize_moveList(){
	MoveList mv;
	memset(&mv.moves, 0, sizeof(mv.moves));
	mv.count = 0;

	return mv;
}

void append_move(MoveList* moveList, Move move){
	*moveList_at(moveList, moveList->count) = move;
	moveList->count++;
}

void delete_move(MoveList* moveList, int index){
	*moveList_at(moveList, index) = *moveList_at(moveList, moveList->count - 1);
	moveList->count--;
}

Move* moveList_at(MoveList* moveList, int index){
	if(index > moveList->count)
		return NULL;
	return &(moveList->moves[index]);
}

const Piece* piece_at(const GameState* gs, Square square){
	return &(gs->board[square.rank][square.file]);
}

void replace_piece(GameState* gs, Piece piece, Square square){
	gs->board[square.rank][square.file] = piece;

}

Piece make_piece(PieceType pt, Color color) {
	Piece p;
	p.piecetype = pt;
	p.color = color;

	return p;
}

bool square_equals(Square a, Square b) {
	return a.file == b.file && a.rank == b.rank;
}

GameState* undo(GameState* gs) {
	if (gs == NULL) return NULL;

	GameState* prev = gs->prev_state;
	free(gs);
	return prev;

}


GameState* make_move(const GameState* gs, Move move) {
	GameState* new_gs = malloc(sizeof(GameState));
	*new_gs = *gs;

	new_gs->prev_state = (GameState*)gs;
	Piece piece = *piece_at(gs, move.from);
	if(piece.piecetype!=ANTEATER) {
		new_gs->anteater_ate == false;
	}
	else if(piece_at(gs,move.to)->piecetype == ANT) {
		new_gs->anteater_ate == true;
	}

	if (square_equals(move.to, make_square(7, A))) {
		new_gs->yellow_qscastle = false;
	}
	if (square_equals(move.to, make_square(7, J))) {
		new_gs->yellow_kscastle = false;
	}
	if (square_equals(move.to, make_square(0, A))) {
		new_gs->blue_qscastle = false;
	}
	if (square_equals(move.to, make_square(0, J))) {
		new_gs->blue_kscastle = false;
	}

	if (piece.piecetype == ROOK) {
		if (square_equals(move.from, make_square(7, A))) {
			new_gs->yellow_qscastle = false;
		}
		else if (square_equals(move.from, make_square(7, J))) {
			new_gs->yellow_kscastle = false;
		}
		else if (square_equals(move.from, make_square(0, A))) {
			new_gs->blue_qscastle = false;
		}
		else if (square_equals(move.from, make_square(0, J))) {
			new_gs->blue_kscastle = false;
		}
	}
	else if (piece.piecetype == ANT) {
		if (move.from.rank - move.to.rank == 2) {
			new_gs->en_passant_square = make_square(move.to.rank + 1, move.to.file);
		}
		else if (move.from.rank - move.to.rank == -2) {
			new_gs->en_passant_square = make_square(move.to.rank - 1, move.to.file);
		}
	}
	else if (piece.piecetype == KING) {
		if (piece.color == YELLOW) {
			new_gs->yellow_qscastle = false;
			new_gs->yellow_kscastle = false;
		}
		else {
			new_gs->blue_qscastle = false;
			new_gs->blue_kscastle = false;
		}
	}

	if (piece.piecetype == ANT && square_equals(move.to, gs->en_passant_square)) {
		if (piece.color == YELLOW) {
			replace_piece(new_gs, make_piece(EMPTY, YELLOW),
				make_square(gs->en_passant_square.rank + 1, gs->en_passant_square.file));
		}
		else {
			replace_piece(new_gs, make_piece(EMPTY, YELLOW),
				make_square(gs->en_passant_square.rank - 1, gs->en_passant_square.file));
		}
	}

	if (piece.piecetype == KING && move.from.file - move.to.file == 2) {
		if (piece.color == YELLOW) {
			replace_piece(new_gs, make_piece(ROOK, YELLOW), make_square(7, E));
			replace_piece(new_gs, make_piece(EMPTY, YELLOW), make_square(7, A));
		}
		else {
			replace_piece(new_gs, make_piece(ROOK, BLUE), make_square(0, E));
			replace_piece(new_gs, make_piece(EMPTY, YELLOW), make_square(0, A));
		}
	}
	else if (piece.piecetype == KING && move.from.file - move.to.file == -2) {
		if (piece.color == YELLOW) {
			replace_piece(new_gs, make_piece(ROOK, YELLOW), make_square(7, G));
			replace_piece(new_gs, make_piece(EMPTY, YELLOW), make_square(7, J));
		}
		else {
			replace_piece(new_gs, make_piece(ROOK, BLUE), make_square(0, G));
			replace_piece(new_gs, make_piece(EMPTY, YELLOW), make_square(0, J));
		}
	}

	replace_piece(new_gs, piece, move.to);
	replace_piece(new_gs, make_piece(EMPTY, YELLOW), move.from);

	new_gs->turn = !new_gs->turn;

	return new_gs;
}

GameState initalize_empty_GameState() {
	GameState gs;
	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 10; j++) {
			gs.board[i][j] = make_piece(EMPTY, YELLOW);


		}
	}

	gs.turn = YELLOW;
	gs.anteater_ate = false;
	gs.yellow_kscastle = true;
	gs.yellow_qscastle = true;
	gs.blue_kscastle = true;
	gs.blue_qscastle = true;
	gs.en_passant_square = make_square(-2,-1);
	gs.prev_state = NULL;

	return gs;
}