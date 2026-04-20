#include "chess_types.h"
#include "test.h"
#include "stdlib.h"

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
		gs.anteater_ate = false;
		gs.blue_kscastle = true;
		gs.blue_qscastle = true;
		gs.yellow_kscastle = true;
		gs.yellow_qscastle = true;
		gs.en_passant_square = make_square(-1,-1);
		gs.prev_state = NULL;
		gs.turn = YELLOW;


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
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "chess_types.h"
#include "legalMoveGen.h"

// from your project somewhere

// ----------------------------------------
// helpers
// ----------------------------------------

static char piece_to_char(Piece p) {
	if (p.piecetype == EMPTY) return '.';

	char c = '?';
	switch (p.piecetype) {
		case KING:   c = 'K'; break;
		case QUEEN:  c = 'Q'; break;
		case ROOK:   c = 'R'; break;
		case BISHOP: c = 'B'; break;
		case KNIGHT: c = 'N'; break;
		case ANT:    c = 'P'; break;   // change to 'A' if you want anteaters shown as A
	    case ANTEATER:   c = 'A'; break;   // change to 'A' if you want anteaters shown as A
		default:     c = '?'; break;
	}

	if (p.color == BLUE) {
		c = (char)tolower(c);
	}
	return c;
}

static int file_char_to_index(char c) {
	c = (char)toupper(c);
	if (c < 'A' || c > 'J') return -1;
	return c - 'A';
}

static bool parse_square(const char* str, Square* out) {
	// accepts things like A0, j7, E3
	if (!str || strlen(str) < 2) return false;

	int file = file_char_to_index(str[0]);
	if (file == -1) return false;

	if (!isdigit((unsigned char)str[1])) return false;
	int rank = str[1] - '0';

	if (rank < 0 || rank > 7) return false;

	*out = make_square(rank, file);
	return true;
}

static bool square_in_move_list(Square sq, MoveList list) {
	for (int i = 0; i < list.count; i++) {
		if (square_equals(list.moves[i].to, sq)) {
			return true;
		}
	}
	return false;
}

static void print_board(GameState* gs) {
	printf("\n     A  B  C  D  E  F  G  H  I  J\n");
	printf("   ---------------------------------\n");

	for (int r = 0; r < 8; r++) {
		printf(" %d |", r);
		for (int f = 0; f < 10; f++) {
			Piece p = *piece_at(gs, make_square(r, f));
			printf(" %c ", piece_to_char(p));   // ALWAYS 3 chars
		}
		printf("|\n");
	}
	printf("   ---------------------------------\n");
}

static void print_board_with_moves(GameState* gs, MoveList moves, Square hovered) {
	printf("\n     A  B  C  D  E  F  G  H  I  J\n");
	printf("   --------------------------------\n");

	for (int r = 0; r < 8; r++) {
		printf(" %d |", r);
		for (int f = 0; f < 10; f++) {
			Square sq = make_square(r, f);
			Piece p = *piece_at(gs, sq);

			if (square_equals(sq, hovered)) {
				printf("[%c]", piece_to_char(p));   // exactly 3 chars
			}
			else if (square_in_move_list(sq, moves)) {
				printf(" * ");                     // exactly 3 chars
			}
			else {
				printf(" %c ", piece_to_char(p));  // exactly 3 chars
			}
		}
		printf("|\n");
	}

	printf("   --------------------------------\n");
}

static bool find_move_to_square(MoveList moves, Square to, Move* out_move) {
	for (int i = 0; i < moves.count; i++) {
		if (square_equals(moves.moves[i].to, to)) {
			*out_move = moves.moves[i];
			return true;
		}
	}
	return false;
}

// ----------------------------------------
// main interactive test loop
// ----------------------------------------

void interactive_move_test(GameState* start) {
	GameState* gs = start;
	char input[64];

	while (1) {
		print_board(gs);

		printf("\nHover square (example E6), or type q to quit: ");
		if (!fgets(input, sizeof(input), stdin)) {
			break;
		}

		input[strcspn(input, "\n")] = '\0';

		if (strcmp(input, "q") == 0 || strcmp(input, "Q") == 0) {
			break;
		}

		Square hovered;
		if (!parse_square(input, &hovered)) {
			printf("Invalid square.\n");
			continue;
		}

		Piece hovered_piece = *piece_at(gs, hovered);
		if (hovered_piece.piecetype == EMPTY) {
			printf("No piece on that square.\n");
			continue;
		}

		MoveList moves = findPossibleMoves(gs, hovered);

		if (moves.count == 0) {
			printf("That piece has no legal moves.\n");
			continue;
		}

		print_board_with_moves(gs, moves, hovered);

		printf("\nLegal moves from %s:\n", input);
		for (int i = 0; i < moves.count; i++) {
			printf("  -> %c%d\n", 'A' + moves.moves[i].to.file, moves.moves[i].to.rank);
		}

		printf("\nMove to: ");
		if (!fgets(input, sizeof(input), stdin)) {
			break;
		}

		input[strcspn(input, "\n")] = '\0';

		Square destination;
		if (!parse_square(input, &destination)) {
			printf("Invalid destination square.\n");
			continue;
		}

		Move chosen_move;
		if (!find_move_to_square(moves, destination, &chosen_move)) {
			printf("That is not a legal move for the selected piece.\n");
			continue;
		}

		gs = make_move(gs, chosen_move);
	}
}

int main(void) {
	GameState gs = initalize_empty_GameState();

	//set up initial board
	gs.board[0][A] = make_piece(ROOK,     BLUE);
	gs.board[0][B] = make_piece(KNIGHT,   BLUE);
	gs.board[0][C] = make_piece(BISHOP,   BLUE);
	gs.board[0][D] = make_piece(ANTEATER, BLUE);
	gs.board[0][E] = make_piece(QUEEN,    BLUE);
	gs.board[0][F] = make_piece(KING,     BLUE);
	gs.board[0][G] = make_piece(ANTEATER, BLUE);
	gs.board[0][H] = make_piece(BISHOP,   BLUE);
	gs.board[0][I] = make_piece(KNIGHT,   BLUE);
	gs.board[0][J] = make_piece(ROOK,     BLUE);
	for (int f = 0; f < 10; f++) gs.board[1][f] = make_piece(ANT, BLUE);
	for (int f = 0; f < 10; f++) gs.board[6][f] = make_piece(ANT, YELLOW);
	gs.board[7][A] = make_piece(ROOK,     YELLOW);
	gs.board[7][B] = make_piece(KNIGHT,   YELLOW);
	gs.board[7][C] = make_piece(BISHOP,   YELLOW);
	gs.board[7][D] = make_piece(ANTEATER, YELLOW);
	gs.board[7][E] = make_piece(QUEEN,    YELLOW);
	gs.board[7][F] = make_piece(KING,     YELLOW);
	gs.board[7][G] = make_piece(ANTEATER, YELLOW);
	gs.board[7][H] = make_piece(BISHOP,   YELLOW);
	gs.board[7][I] = make_piece(KNIGHT,   YELLOW);
	gs.board[7][J] = make_piece(ROOK,     YELLOW);

	printf("Initial board:\n");
	print_board(&gs);

	//run automated tests
	printf("\nRunning tests...\n");
	test_rook_moves();
	test_rook_blocked();
	test_rook_capture();
	test_knight_moves();
	test_king_into_check();
	test_pinned_piece();

	printf("\nAll tests complete.\n");
	return 0;
}


