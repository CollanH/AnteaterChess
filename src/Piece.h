//
// Created by clair on 4/9/2026.
//

#ifndef CHESS_PIECE_H
#define CHESS_PIECE_H
typedef enum pieceName {
	EMPTY,
	KING,
	QUEEN,
	ROOK,
	BISHOP,
	KNIGHT,
	ANTEATER,
	ANT
} pieceName;



typedef enum color {
	NONE, BLACK, WHITE
} color;

typedef struct Piece {
	pieceName pieceName;
	color color;

}Piece;

#endif //CHESS_PIECE_H
