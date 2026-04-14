#include "chess_types.h"
#include "test.h"

GameState string_to_gs(char board_string[81]) {
	GameState gs = initalize_empty_GameState();
	int file = 0;
	int rank = 0;
	for(int i = 0; i < 80; i++){
		if(file == 10){
			file = 0;
			rank++;
		}

		Piece piece =
				(board_string[i] == 'K') ? make_piece(KING, YELLOW) :
				(board_string[i] == 'k') ? make_piece(KING, BLUE) :
				(board_string[i] == 'Q') ? make_piece(QUEEN, YELLOW) :
				(board_string[i] == 'q') ? make_piece(QUEEN, BLUE) :
				(board_string[i] == 'A') ? make_piece(ANTEATER, YELLOW) :
				(board_string[i] == 'a') ? make_piece(ANTEATER, BLUE) :
				(board_string[i] == 'B') ? make_piece(BISHOP, YELLOW) :
				(board_string[i] == 'b') ? make_piece(BISHOP, BLUE) :
				(board_string[i] == 'N') ? make_piece(KNIGHT, YELLOW) :
				(board_string[i] == 'n') ? make_piece(KNIGHT, BLUE) :
				(board_string[i] == 'R') ? make_piece(ROOK, YELLOW) :
				(board_string[i] == 'r') ? make_piece(ROOK, BLUE) :
				(board_string[i] == 'P') ? make_piece(ANT, YELLOW) :
				(board_string[i] == 'p') ? make_piece(ANT, BLUE) :
				make_piece(EMPTY, YELLOW);

		gs.board[rank][file] = piece;

		file++;

	}

	return gs;


}