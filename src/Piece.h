//
// Created by clair on 4/9/2026.
//

#ifndef CHESS_PIECE_H
#define CHESS_PIECE_H
typedef enum pieceName {
	KING,
	QUEEN,
	ROOK,
	BISHOP,
	KNIGHT
	ANTEATER,
	ANT;
} pieceName;

typedef enum file {
	A, B, C, D, E, F, G, H, I, J;
} pieceName;

typedef enum color {
	black, white;
};

typedef struct Piece {
	pieceName;
	rank;
	file;

}Piece;

#endif //CHESS_PIECE_H
