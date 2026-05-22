#include "chess_types.h"
#include "string.h"
#include "stdlib.h"
// simple helper functions
static inline bool in_bounds_sq(Square s)
{
	return s.rank >= 0 && s.rank < 8 && s.file >= A && s.file < BOARD_FILES;
}

static inline bool is_empty_piece(Piece p)
{
	return p.piecetype == EMPTY;
}

static inline Color opponent_color(Color color)
{
	return (color == YELLOW) ? BLUE : YELLOW;
}

static PieceList* piece_list_for_color(GameState *gs, Color color)
{
	return (color == YELLOW) ? &gs->yellow_pieces : &gs->blue_pieces;
}

static Square* king_square_for_color(GameState *gs, Color color)
{
	return (color == YELLOW) ? &gs->yellow_king_square : &gs->blue_king_square;
}

static bool anteater_has_chain_capture(const GameState *gs, Square square, Color color)
{
	static const int dr[4] = {-1, 1, 0, 0};
	static const int df[4] = {0, 0, -1, 1};

	for (int i = 0; i < 4; i++) {
		Square target = make_square(square.rank + dr[i], square.file + df[i]);
		Piece piece;

		if (!in_bounds_sq(target)) {
			continue;
		}

		piece = gs->board[target.rank][target.file];
		if (piece.piecetype == ANT && piece.color != color) {
			return true;
		}
	}

	return false;
}

// resets the cache of all pieces
static void reset_piece_cache(GameState *gs)
{
	gs->yellow_pieces.count = 0;
	gs->blue_pieces.count = 0;
	gs->yellow_king_square = make_square(-1, A);
	gs->blue_king_square = make_square(-1, A);
	// safety
	for (int r = 0; r < 8; r++) {
		for (int f = 0; f < 10; f++) {
			gs->piece_list_index[r][f] = -1;
		}
	}
}
// add piece to cache
static void add_piece_to_cache(GameState *gs, Square square, Piece piece)
{
	PieceList *list;
	Square *king_square;
	int index;

	if (piece.piecetype == EMPTY) {
		gs->piece_list_index[square.rank][square.file] = -1;
		return;
	}

	list = piece_list_for_color(gs, piece.color);
	// add at the back
	index = list->count;
	list->squares[index] = square;
	list->count++;
	// update the index to keep it accurate
	gs->piece_list_index[square.rank][square.file] = index;

	// have to edit king square in the game state if we add a king
	if (piece.piecetype == KING) {
		king_square = king_square_for_color(gs, piece.color);
		*king_square = square;
	}
}
// remove piece from cache(for captures)
static void remove_piece_from_cache(GameState *gs, Square square, Piece piece)
{
	PieceList *list;
	int index;
	int last_index;
	Square last_square;

	if (piece.piecetype == EMPTY || !in_bounds_sq(square)) {
		return;
	}

	// get the index of the piece that was captured
	index = gs->piece_list_index[square.rank][square.file];
	if (index < 0) {
		return;
	}

	// place the back piece in the place of the deleted piece and decrement hte size
	list = piece_list_for_color(gs, piece.color);
	last_index = list->count - 1;
	last_square = list->squares[last_index];
	list->squares[index] = last_square;
	list->count--;
	// no more index because piece is gone
	gs->piece_list_index[square.rank][square.file] = -1;

	if (index != last_index) {
		gs->piece_list_index[last_square.rank][last_square.file] = index;
	}
	// if king is somehow deleted fix king square
	if (piece.piecetype == KING) {
		*king_square_for_color(gs, piece.color) = make_square(-1, A);
	}
}

static void move_piece_in_cache(GameState *gs, Square from, Square to, Piece piece)
{
	int index;

	if (piece.piecetype == EMPTY) {
		return;
	}
	// find the index of the piece
	index = gs->piece_list_index[from.rank][from.file];
	if (index < 0) {
		return;
	}
	// fix the pieces location in the cache and the index
	piece_list_for_color(gs, piece.color)->squares[index] = to;
	gs->piece_list_index[to.rank][to.file] = index;
	gs->piece_list_index[from.rank][from.file] = -1;

	// fix king if it moves
	if (piece.piecetype == KING) {
		*king_square_for_color(gs, piece.color) = to;
	}
}

void refresh_piece_cache(GameState *gs)
{
	if (gs == NULL) {
		return;
	}

	reset_piece_cache(gs);
	// fix the cache with the board info

	for (int r = 0; r < 8; r++) {
		for (int f = 0; f < 10; f++) {
			Piece piece = gs->board[r][f];
			if (piece.piecetype != EMPTY) {
				add_piece_to_cache(gs, make_square(r, (File)f), piece);
			}
		}
	}

	gs->cache_valid = true;
}

bool is_promotion_move(const GameState *gs, Move move)
{
	Piece moved_piece;

	if (gs == NULL || !in_bounds_sq(move.from) || !in_bounds_sq(move.to)) {
		return false;
	}

	moved_piece = gs->board[move.from.rank][move.from.file];
	return moved_piece.piecetype == ANT &&
		(move.to.rank == 0 || move.to.rank == 7);
}

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
	*moveList_at(moveList, moveList->count) = move;
	moveList->count++;
}

void delete_move(MoveList* moveList, int index){
	*moveList_at(moveList, index) = *moveList_at(moveList, moveList->count - 1);
	moveList->count--;
}

Move* moveList_at(MoveList* moveList, int index){
	if(index > moveList->count)
		return NULL;
	return &(moveList->moves[index]);
}

Piece* piece_at(GameState* gs, Square square){
	return &(gs->board[square.rank][square.file]);
}

void replace_piece(GameState* gs, Piece piece, Square square){
	gs->board[square.rank][square.file] = piece;
	// ok to break the cache here cause its only used in the copy move
	gs->cache_valid = false;

}

Piece make_piece(PieceType pt, Color color) {
	Piece p;
	p.piecetype = pt;
	p.color = color;

	return p;
}

bool square_equals(Square a, Square b) {
	return a.file == b.file && a.rank == b.rank;
}

GameState* undo(GameState* gs) {
	if (gs == NULL) return NULL;

	GameState* prev = gs->prev_state;
	free(gs);
	return prev;

}

GameState* promote_pawn(const GameState* gs, Square pawn, PieceType pt) {
	GameState* new_gs = malloc(sizeof(GameState));
	if (new_gs == NULL) {
		return NULL;
	}

	*new_gs = *gs;

	switch(pt) {
		case QUEEN:
			piece_at(new_gs, pawn)->piecetype = QUEEN;
			break;
		case KNIGHT:
			piece_at(new_gs, pawn)->piecetype = KNIGHT;
			break;
		case BISHOP:
			piece_at(new_gs, pawn)->piecetype = BISHOP;
			break;
		case ANTEATER:
			piece_at(new_gs, pawn)->piecetype = ANTEATER;
			break;
		case ROOK:
			piece_at(new_gs, pawn)->piecetype = ROOK;
			break;
		default:
			break;
	}

	new_gs->prev_state = gs;
	refresh_piece_cache(new_gs);
	return new_gs;
}

GameState* make_move(const GameState* gs, Move move) {
	GameState* new_gs = malloc(sizeof(GameState));
	UndoData undo;

	if (new_gs == NULL) {
		return NULL;
	}

	*new_gs = *gs;
	new_gs->prev_state = (GameState*)gs;
	make_move_in_place(new_gs, move, &undo);

	return new_gs;
}

GameState* make_move_ai(const GameState* gs, Move move) {
	return make_move(gs, move);
}

void undo_move_in_place(GameState *gs, Move move, const UndoData *undo)
{
    Square from = move.from;
    Square to   = move.to;

    if (undo->flags & UNDO_WAS_NOOP) {
        return;
    }

    if (!gs->cache_valid) {
        refresh_piece_cache(gs);
    }

    // restore simple state first
    gs->yellow_qscastle = (undo->flags & UNDO_YELLOW_QSCASTLE) != 0;
    gs->yellow_kscastle = (undo->flags & UNDO_YELLOW_KSCASTLE) != 0;
    gs->blue_qscastle   = (undo->flags & UNDO_BLUE_QSCASTLE) != 0;
    gs->blue_kscastle   = (undo->flags & UNDO_BLUE_KSCASTLE) != 0;
    gs->anteater_ate    = (undo->flags & UNDO_ANTEATER_ATE) != 0;
    gs->turn            = (undo->flags & UNDO_TURN_BLUE) ? BLUE : YELLOW;
    gs->en_passant_square = undo->old_en_passant_square;
    gs->anteater_chain_square = undo->old_anteater_chain_square;

    // undo castling rook move first so squares are free
    if (undo->flags & UNDO_WAS_CASTLE) {
        move_piece_in_cache(gs,
            undo->rook_to,
            undo->rook_from,
            gs->board[undo->rook_to.rank][undo->rook_to.file]);
        gs->board[undo->rook_from.rank][undo->rook_from.file] =
            gs->board[undo->rook_to.rank][undo->rook_to.file];
        gs->board[undo->rook_to.rank][undo->rook_to.file] =
            make_piece(EMPTY, YELLOW);
    }

    // move main piece back
    move_piece_in_cache(gs, to, from, undo->moved_piece);
    gs->board[from.rank][from.file] = undo->moved_piece;
    gs->board[to.rank][to.file]     = make_piece(EMPTY, YELLOW);

    // restore captured piece
    if (in_bounds_sq(undo->captured_square) && !is_empty_piece(undo->captured_piece)) {
        gs->board[undo->captured_square.rank][undo->captured_square.file] = undo->captured_piece;
        add_piece_to_cache(gs, undo->captured_square, undo->captured_piece);
    }

    gs->cache_valid = true;
}

void make_move_in_place(GameState *gs, Move move, UndoData *undo)
{
    Square from = move.from;
    Square to   = move.to;
    Piece moved = gs->board[from.rank][from.file];
    Piece dest  = gs->board[to.rank][to.file];
    bool anteater_capture;
    bool keep_turn;
    int rankDistance;
    int fileDistance;

    if (!gs->cache_valid) {
        refresh_piece_cache(gs);
    }

    undo->moved_piece           = moved;
    undo->captured_piece        = make_piece(EMPTY, YELLOW);
    undo->old_en_passant_square = gs->en_passant_square;
    undo->old_anteater_chain_square = gs->anteater_chain_square;
    undo->captured_square       = make_square(-1, A);
    undo->rook_from             = make_square(-1, A);
    undo->rook_to               = make_square(-1, A);
    undo->flags                 = 0;

    if (gs->yellow_qscastle) undo->flags |= UNDO_YELLOW_QSCASTLE;
    if (gs->yellow_kscastle) undo->flags |= UNDO_YELLOW_KSCASTLE;
    if (gs->blue_qscastle)   undo->flags |= UNDO_BLUE_QSCASTLE;
    if (gs->blue_kscastle)   undo->flags |= UNDO_BLUE_KSCASTLE;
    if (gs->anteater_ate)    undo->flags |= UNDO_ANTEATER_ATE;
    if (gs->turn == BLUE)    undo->flags |= UNDO_TURN_BLUE;

    rankDistance = to.rank - from.rank;
    fileDistance = to.file - from.file;
    anteater_capture = moved.piecetype == ANTEATER &&
        dest.piecetype == ANT &&
        dest.color != moved.color;
    keep_turn = false;

    if (moved.piecetype == ANTEATER &&
        !is_empty_piece(dest) &&
        (dest.piecetype != ANT || dest.color == moved.color)) {
        undo->flags |= UNDO_WAS_NOOP;
        return;
    }

    // normal capture on destination
    if (!is_empty_piece(dest) &&
        (moved.piecetype != ANTEATER || dest.piecetype == ANT)) {
        undo->captured_piece  = dest;
        undo->captured_square = to;
        remove_piece_from_cache(gs, to, dest);
    }
	// update cache to reflect move
    move_piece_in_cache(gs, from, to, moved);

    // default: move piece from -> to
    gs->board[to.rank][to.file]     = moved;
    gs->board[from.rank][from.file] = make_piece(EMPTY, YELLOW);

    if (moved.piecetype == ANTEATER) {
        if (anteater_capture && anteater_has_chain_capture(gs, to, moved.color)) {
            gs->anteater_ate = true;
            gs->anteater_chain_square = to;
            keep_turn = true;
        } else {
            gs->anteater_ate = false;
            gs->anteater_chain_square = make_square(-1, A);
        }
    } else {
        gs->anteater_ate = false;
        gs->anteater_chain_square = make_square(-1, A);
    }

    // en passant: ant moves diagonally to empty square
    if (moved.piecetype == ANT) {
        if (fileDistance != 0 && is_empty_piece(dest)) {
            Square epCaptured = make_square(from.rank, to.file);

            undo->captured_piece  = gs->board[epCaptured.rank][epCaptured.file];
            undo->captured_square = epCaptured;

            remove_piece_from_cache(gs, epCaptured, undo->captured_piece);
            gs->board[epCaptured.rank][epCaptured.file] = make_piece(EMPTY, YELLOW);
        }
    }

    // reset en passant square every move
    gs->en_passant_square = make_square(-1, A);

    // ant double push creates en passant square
    if (moved.piecetype == ANT && (rankDistance == 2 || rankDistance == -2)) {
        gs->en_passant_square = make_square((from.rank + to.rank) / 2, from.file);
    }

    // promotion
    if (moved.piecetype == ANT && (to.rank == 0 || to.rank == 7)) {
        gs->board[to.rank][to.file].piecetype = QUEEN;
        undo->flags |= UNDO_WAS_PROMOTION;
    }

    // king moved: revoke both castling rights
    if (moved.piecetype == KING) {
        if (moved.color == YELLOW) {
            gs->yellow_kscastle = false;
            gs->yellow_qscastle = false;
        } else {
            gs->blue_kscastle = false;
            gs->blue_qscastle = false;
        }


        if (from.rank == to.rank && (fileDistance == 2 || fileDistance == -2)) {
            undo->flags |= UNDO_WAS_CASTLE;

            if (fileDistance == 2) {
                // kingside: standard chess rook at H, anteater at J
                undo->rook_from = make_square(from.rank, standard_chess_mode ? H : J);
                undo->rook_to   = make_square(from.rank, from.file + 1);
            } else {
                // queenside
                undo->rook_from = make_square(from.rank, A);
                undo->rook_to   = make_square(from.rank, from.file - 1);
            }

            gs->board[undo->rook_to.rank][undo->rook_to.file] =
                gs->board[undo->rook_from.rank][undo->rook_from.file];
            gs->board[undo->rook_from.rank][undo->rook_from.file] =
                make_piece(EMPTY, YELLOW);
            move_piece_in_cache(gs,
                undo->rook_from,
                undo->rook_to,
                gs->board[undo->rook_to.rank][undo->rook_to.file]);
        }
    }

    // rook moved: revoke that rook's castling right
    {
        File ks_file = standard_chess_mode ? H : J;
        if (moved.piecetype == ROOK) {
            if (square_equals(from, make_square(7, A)))      gs->yellow_qscastle = false;
            if (square_equals(from, make_square(7, ks_file))) gs->yellow_kscastle = false;
            if (square_equals(from, make_square(0, A)))      gs->blue_qscastle   = false;
            if (square_equals(from, make_square(0, ks_file))) gs->blue_kscastle   = false;
        }
        // rook captured on home square: revoke that side's castling right too
        if (!is_empty_piece(undo->captured_piece) && undo->captured_piece.piecetype == ROOK) {
            if (square_equals(undo->captured_square, make_square(7, A)))      gs->yellow_qscastle = false;
            if (square_equals(undo->captured_square, make_square(7, ks_file))) gs->yellow_kscastle = false;
            if (square_equals(undo->captured_square, make_square(0, A)))      gs->blue_qscastle   = false;
            if (square_equals(undo->captured_square, make_square(0, ks_file))) gs->blue_kscastle   = false;
        }
    }

    if (!keep_turn) {
        gs->turn = (gs->turn == YELLOW) ? BLUE : YELLOW;
    }

    gs->cache_valid = true;
}



GameState initalize_empty_GameState() {
	GameState gs;
	memset(&gs, 0, sizeof(gs));
	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 10; j++) {
			gs.board[i][j] = make_piece(EMPTY, YELLOW);


		}
	}

	gs.turn = YELLOW;
	gs.anteater_ate = false;
	gs.anteater_chain_square = make_square(-1, A);
	gs.yellow_kscastle = true;
	gs.yellow_qscastle = true;
	gs.blue_kscastle = true;
	gs.blue_qscastle = true;
	gs.en_passant_square = make_square(-2,-1);
	gs.prev_state = NULL;
	reset_piece_cache(&gs);
	gs.cache_valid = false;

	return gs;
}
