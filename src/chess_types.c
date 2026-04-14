#include "chess_types.h"
#include "string.h"

Square make_square(int rank, File file){
	Square sq;
	sq.file = file;
	sq.rank = rank;
}

MoveList initialize_moveList(){
	MoveList mv;
	memset(&mv.moves, 0, sizeof(mv.moves));
	mv.count = 0;
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
	if(index >= moveList->count)
		return NULL;
	return &(moveList->moves[index]);
}

const Piece* piece_at(const GameState* gs, Square square){
	return &(gs->board[square.rank][square.file]);
}