#include "chess_types.h"
#include "string.h"

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
	if(moveList->count < 250)
 		moveList->moves[moveList->count++] = move;
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

void replace_piece(GameState* gs, Piece piece, Square square){
	gs->board[square.rank][square.file] = piece;

}

Piece make_piece(PieceType pt, Color color) {
	Piece p;
	p.piecetype = pt;
	p.color = color;

	return p;
}

GameState make_move(const GameState* gs, Move move){
	GameState new_gs = *gs;
	Piece piece = *piece_at(gs, move.from);
	replace_piece(&new_gs, piece, move.to);
	replace_piece(&new_gs, make_piece(EMPTY, YELLOW), move.from);

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