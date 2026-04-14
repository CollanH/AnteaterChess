#ifndef CHESS_TYPES_H
#define CHESS_TYPES_H

#include <stdbool.h>

typedef enum {
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J
} File;

typedef enum {
    EMPTY,
    KING,
    QUEEN,
    ANTEATER,
    BISHOP,
    KNIGHT,
    ROOK,
    ANT
} PieceType;

typedef enum {
	YELLOW,
	BLUE,

} Color;

typedef struct {
	int rank;
	File file;
} Square;

typedef struct {
    PieceType piecetype;
    Color color;
} Piece;

typedef struct {
    Square from;
    Square to;
} Move;

typedef struct {
    Move moves[250];
    int count;
} MoveList;

typedef struct GameState {
    Piece board[8][10];
    struct GameState* prev_state;
    bool yellow_castled;
    bool blue_castled;
    bool anteater_ate;
    Square en_passant_square;
    Color turn;
} GameState;




Square make_square(int rank, File file);
// initializes an empty move list
MoveList initialize_moveList();
// returns a pointer to a move in moveList at index
Move* moveList_at(MoveList* moveList, int index);
// deletes a move at an index
void delete_move(MoveList* moveList, int index);
// appends a move at the end of a moveList
void append_move(MoveList* moveList, Move move);
const Piece* piece_at(const GameState* gs, Square square);
GameState make_move(const GameState* gs, Move move);
#endif /* CHESS_TYPES_H */