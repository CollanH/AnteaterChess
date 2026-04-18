#include "chess_types.h"
#include "test.h"

#include "legalMoveGen.h"

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


#include <stdio.h>
#include <stdbool.h>
#include "chess_types.h"
#include "test.h"

// ===== HELPERS =====

bool move_exists(MoveList* list, Square from, Square to) {
    for (int i = 0; i < list->count; i++) {
        Move* m = moveList_at(list, i);
        if (m->from.rank == from.rank &&
            m->from.file == from.file &&
            m->to.rank == to.rank &&
            m->to.file == to.file) {
            return true;
        }
    }
    return false;
}

void expect(bool cond, const char* msg) {
    if (cond) printf("PASS: %s\n", msg);
    else      printf("FAIL: %s\n", msg);
}

// ===== PRINT "TO" SQUARES =====

void print_move_targets(MoveList* list) {
    char board[8][10];

    // initialize empty board
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 10; j++) {
            board[i][j] = '.';
        }
    }

    // mark all "to" squares
    for (int i = 0; i < list->count; i++) {
        Move* m = moveList_at(list, i);
        board[m->to.rank][m->to.file] = 'X';
    }

    // print board
    printf("\nMove targets:\n");
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 10; j++) {
            printf("%c", board[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// ===== TESTS =====

void test_rook_moves() {
    char str[81] =
        ".........."
        ".........."
        ".........."
        "....R....."
        ".........."
        ".........."
        ".........."
        "..........";

    GameState gs = string_to_gs(str);
    gs.turn = YELLOW;

    Square rook = make_square(3, E);
    MoveList moves = findPossibleMoves(&gs, rook);

    print_move_targets(&moves);

}

void test_rook_blocked() {
    char str[81] =
        ".........."
        ".........."
        ".........."
        "...PR....."
        ".........."
        ".........."
        ".........."
        "..........";

    GameState gs = string_to_gs(str);
    gs.turn = YELLOW;

    Square rook = make_square(3, E);
    MoveList moves = findPossibleMoves(&gs, rook);

    print_move_targets(&moves);


}

void test_rook_capture() {
    char str[81] =
        ".........."
        ".........."
        ".........."
        "...rR....."
        ".........."
        ".........."
        ".........."
        "..........";

    GameState gs = string_to_gs(str);
    gs.turn = YELLOW;

    Square rook = make_square(3, E);
    MoveList moves = findPossibleMoves(&gs, rook);

    print_move_targets(&moves);


}

void test_knight_moves() {
    char str[81] =
        ".........."
        ".........."
        "....P....."
        "...N......"
        "....P....."
        ".........."
        ".........."
        "..........";

    GameState gs = string_to_gs(str);
    gs.turn = YELLOW;

    Square knight = make_square(3, D);
    MoveList moves = findPossibleMoves(&gs, knight);

    print_move_targets(&moves);

    expect(moves.count == 8, "knight ignores blockers");
}

void test_king_into_check() {
    char str[81] =
        ".........."
        ".........."
        "....r....."
        "....K....."
        ".........."
        ".........."
        ".........."
        "..........";

    GameState gs = string_to_gs(str);
    gs.turn = YELLOW;

    Square king = make_square(3, E);
    MoveList moves = findPossibleMoves(&gs, king);

    print_move_targets(&moves);

}

void test_pinned_piece() {
    char str[81] =
        ".........."
        "....r....."
        ".........."
        "....B....."
        "....K....."
        ".........."
        ".........."
        "..........";

    GameState gs = string_to_gs(str);
    gs.turn = YELLOW;

    Square bishop = make_square(3, E);
    MoveList moves = findPossibleMoves(&gs, bishop);

    print_move_targets(&moves);

    expect(moves.count == 0, "pinned piece cannot move");
}



// ===== MAIN =====

int main() {
    test_rook_moves();
    test_rook_blocked();
    test_rook_capture();
    test_knight_moves();
    test_king_into_check();
    test_pinned_piece();

    return 0;
}