#ifndef CHESS_TYPES_H
#define CHESS_TYPES_H

#include <stdbool.h>
#include <stdint.h>

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

// piece list has an array of squares which contain the locations of all pieces for one side of the board
typedef struct {
    Square squares[40];
    int count;
} PieceList;

typedef struct GameState {
    Piece board[8][10];
    PieceList yellow_pieces;
    PieceList blue_pieces;
    Square yellow_king_square;
    Square blue_king_square;
    Square anteater_chain_square;
	// if the yellow left rook is on A7 for example, if we look up piece_list_index[7][A] it
	// will tell us where in yellow's PieceList the yellow left rook location is stored
	// the location of the yellow left rook in the list can change as the list will
	// compact when a piece is captured which is why this index is useful
    int piece_list_index[8][10];
    bool cache_valid;
    struct GameState* prev_state;
    bool yellow_qscastle;
	bool yellow_kscastle;
	bool blue_qscastle;
	bool blue_kscastle;

    bool anteater_ate;
    Square en_passant_square;
    Color turn;
} GameState;



typedef struct UndoData {
    Piece moved_piece;          // piece as it was before move
    Piece captured_piece;       // piece removed by this move, if any

    Square old_en_passant_square;
    Square old_anteater_chain_square;
    Square captured_square;     // for en passant or normal captures
    Square rook_from;           // only used if castling
    Square rook_to;             // only used if castling

    uint16_t flags;             // packed booleans
} UndoData;

enum {
    UNDO_YELLOW_QSCASTLE = 1 << 0,
    UNDO_YELLOW_KSCASTLE = 1 << 1,
    UNDO_BLUE_QSCASTLE   = 1 << 2,
    UNDO_BLUE_KSCASTLE   = 1 << 3,
    UNDO_ANTEATER_ATE    = 1 << 4,
    UNDO_TURN_BLUE       = 1 << 5,
    UNDO_WAS_PROMOTION   = 1 << 6,
    UNDO_WAS_CASTLE      = 1 << 7,
    UNDO_WAS_NOOP        = 1 << 8
};


// 0 = Anteater Chess (default), 1 = Standard Chess (8x8, no anteater piece)
extern int standard_chess_mode;

// number of files in the current mode: 8 for Standard Chess, 10 for Anteater Chess
#define BOARD_FILES (standard_chess_mode ? 8 : 10)

// FOR GUI TO CALL
GameState* undo(GameState* gs);
GameState* make_move(const GameState* gs, Move move);
GameState* promote_pawn(const GameState* gs, Square pawn, PieceType pt);
GameState* make_move_ai(const GameState* gs, Move move);

void undo_move_in_place(GameState *gs, Move move, const UndoData *undo);
void make_move_in_place(GameState *gs, Move move, UndoData *undo);

Square make_square(int rank, File file);
// initializes an empty move list
MoveList initialize_moveList();
// returns a pointer to a move in moveList at index
Move* moveList_at(MoveList* moveList, int index);
// deletes a move at an index
void delete_move(MoveList* moveList, int index);
// appends a move at the end of a moveList
void append_move(MoveList* moveList, Move move);
Piece* piece_at(GameState* gs, Square square);
Piece make_piece(PieceType pt, Color color);
void replace_piece(GameState* gs, Piece piece, Square square);
GameState initalize_empty_GameState();
bool square_equals(Square a, Square b);
void refresh_piece_cache(GameState *gs);
bool is_promotion_move(const GameState *gs, Move move);
#endif /* CHESS_TYPES_H */
