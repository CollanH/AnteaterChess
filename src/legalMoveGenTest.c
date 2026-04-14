#include "legalMoveGen.h"
#include "chess_types.h"
#include "stdio.h"

int main() {
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