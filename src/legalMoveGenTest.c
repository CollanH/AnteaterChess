#include "legalMoveGen.h"
#include "chess_types.h"
#include "stdio.h"
#include "test.h"
typedef struct {
	char board[81];   // +1 for null terminator
	bool expected;
	const char *name;
} CheckTest;




/*
*int main() {
	// testing check function
	GameState gs = initalize_empty_GameState();
	replace_piece(&gs, make_piece(KING, YELLOW), make_square(5, C));
	replace_piece(&gs, make_piece(QUEEN, BLUE), make_square(5, F));
	//1
	printf("%d",inCheck(&gs, YELLOW));
	replace_piece(&gs, make_piece(ROOK, BLUE), make_square(5, F));
	//1
	printf("%d",inCheck(&gs, YELLOW));
	replace_piece(&gs, make_piece(BISHOP, BLUE), make_square(5, F));
	//0
	printf("%d",inCheck(&gs, YELLOW));
	replace_piece(&gs, make_piece(EMPTY, YELLOW), make_square(5, F));
	replace_piece(&gs, make_piece(BISHOP, YELLOW), make_square(3, E));
	//0
	printf("%d",inCheck(&gs, YELLOW));
	replace_piece(&gs, make_piece(BISHOP, BLUE), make_square(3, E));
	//1
	printf("%d",inCheck(&gs, YELLOW));
	replace_piece(&gs, make_piece(EMPTY, BLUE), make_square(3, E));
	//0
	printf("%d",inCheck(&gs, YELLOW));
	replace_piece(&gs, make_piece(ANTEATER, BLUE), make_square(4, C));
	//0
	printf("%d",inCheck(&gs, YELLOW));
	replace_piece(&gs, make_piece(EMPTY, BLUE), make_square(4, C));
	replace_piece(&gs, make_piece(QUEEN, BLUE), make_square(5, F));
	replace_piece(&gs, make_piece(ANT, BLUE), make_square(5, E));
	// 0
	printf("%d",inCheck(&gs, YELLOW));


	return 0;

}
 */

int main() {
	typedef struct {
		char board[81];
		bool expected;
		const char *name;
	} CheckTest;

	CheckTest check_tests[] = {
			{
					// 1. rook vertical check: blue rook on (0,4), yellow king on (5,4)
					"....r....."
					".........."
					".........."
					".........."
					".........."
					"....K....."
					".........."
					"..........",
					true,
					"rook vertical check"
			},

			{
					// 2. rook blocked: blocker is really on same file between rook and king
					// rook on (0,4), blue pawn on (2,4), yellow king on (5,4)
					"....r....."
					".........."
					"....p....."
					".........."
					".........."
					"....K....."
					".........."
					"..........",
					false,
					"rook blocked"
			},

			{
					// 3. bishop diagonal check: bishop on (0,0), king on (5,5)
					"b........."
					".........."
					".........."
					".........."
					".........."
					".....K...."
					".........."
					"..........",
					true,
					"bishop diagonal check"
			},

			{
					// 4. knight check: knight on (3,4), king on (5,5)
					".........."
					".........."
					".........."
					"....n....."
					".........."
					".....K...."
					".........."
					"..........",
					true,
					"knight check"
			},

			{
					// 5. pawn check: blue pawn attacks downward
					// blue pawn on (4,4) attacks (5,5)
					".........."
					".........."
					".........."
					".........."
					"....p....."
					".....K...."
					".........."
					"..........",
					true,
					"blue pawn check"
			},

			{
					// 6. adjacent king check: blue king next to yellow king
					".........."
					".........."
					".........."
					".........."
					".........."
					"....kK...."
					".........."
					"..........",
					true,
					"adjacent kings"
			},

			{
					// 7. multiple attackers: rook already gives check; bishop also attacks
					// rook on (0,4), bishop on (2,2), king on (5,4)
					"....r....."
					".........."
					"..b......."
					".........."
					".........."
					"....K....."
					".........."
					"..........",
					true,
					"multiple attackers"
			},

			{
					// 8. no check: king alone
					".........."
					".........."
					".........."
					".........."
					".........."
					"....K....."
					".........."
					"..........",
					false,
					"no check"
			},

			{
					// 9. edge of board rook check: rook and king on same rank
					".........."
					".........."
					".........."
					".........."
					".........."
					"r...K....."
					".........."
					"..........",
					true,
					"edge rook check"
			},

			{
					// 10. same-color piece should not count
					// yellow rook near yellow king
					".........."
					".........."
					".........."
					".........."
					".........."
					"R...K....."
					".........."
					"..........",
					false,
					"same color piece"
			}
	};

	int num_tests = sizeof(check_tests) / sizeof(CheckTest);

	for(int i = 0; i < num_tests; i++) {
		GameState gs = string_to_gs(check_tests[i].board);
		bool result = inCheck(&gs, YELLOW);

		if(result != check_tests[i].expected) {
			printf("FAILED: %s\n", check_tests[i].name);
		} else {
			printf("PASSED: %s\n", check_tests[i].name);
		}
	}
	return 0;

}