#include "chess_types.h"
#include "string.h"
#include "stdlib.h"

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

const Piece* piece_at(const GameState* gs, Square square){
	return &(gs->board[square.rank][square.file]);
}

void replace_piece(GameState* gs, Piece piece, Square square){
	gs->board[square.rank][square.file] = piece;

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


GameState* make_move(const GameState* gs, Move move) {
	GameState* new_gs = malloc(sizeof(GameState));
	*new_gs = *gs;

	new_gs->prev_state = (GameState*)gs;
	Piece piece = *piece_at(gs, move.from);
	if(piece.piecetype!=ANTEATER) {
		new_gs->anteater_ate = false;
	}
	else if(piece_at(gs,move.to)->piecetype == ANT) {
		new_gs->anteater_ate = true;
	}

	if (square_equals(move.to, make_square(7, A))) {
		new_gs->yellow_qscastle = false;
	}
	if (square_equals(move.to, make_square(7, J))) {
		new_gs->yellow_kscastle = false;
	}
	if (square_equals(move.to, make_square(0, A))) {
		new_gs->blue_qscastle = false;
	}
	if (square_equals(move.to, make_square(0, J))) {
		new_gs->blue_kscastle = false;
	}

	if (piece.piecetype == ROOK) {
		if (square_equals(move.from, make_square(7, A))) {
			new_gs->yellow_qscastle = false;
		}
		else if (square_equals(move.from, make_square(7, J))) {
			new_gs->yellow_kscastle = false;
		}
		else if (square_equals(move.from, make_square(0, A))) {
			new_gs->blue_qscastle = false;
		}
		else if (square_equals(move.from, make_square(0, J))) {
			new_gs->blue_kscastle = false;
		}
	}
	else if (piece.piecetype == ANT) {
		if (move.from.rank - move.to.rank == 2) {
			new_gs->en_passant_square = make_square(move.to.rank + 1, move.to.file);
		}
		else if (move.from.rank - move.to.rank == -2) {
			new_gs->en_passant_square = make_square(move.to.rank - 1, move.to.file);
		}
	}
	else if (piece.piecetype == KING) {
		if (piece.color == YELLOW) {
			new_gs->yellow_qscastle = false;
			new_gs->yellow_kscastle = false;
		}
		else {
			new_gs->blue_qscastle = false;
			new_gs->blue_kscastle = false;
		}
	}

	if (piece.piecetype == ANT && square_equals(move.to, gs->en_passant_square)) {
		if (piece.color == YELLOW) {
			replace_piece(new_gs, make_piece(EMPTY, YELLOW),
				make_square(gs->en_passant_square.rank + 1, gs->en_passant_square.file));
		}
		else {
			replace_piece(new_gs, make_piece(EMPTY, YELLOW),
				make_square(gs->en_passant_square.rank - 1, gs->en_passant_square.file));
		}
	}

	if (piece.piecetype == KING && move.from.file - move.to.file == 2) {
		if (piece.color == YELLOW) {
			replace_piece(new_gs, make_piece(ROOK, YELLOW), make_square(7, E));
			replace_piece(new_gs, make_piece(EMPTY, YELLOW), make_square(7, A));
		}
		else {
			replace_piece(new_gs, make_piece(ROOK, BLUE), make_square(0, E));
			replace_piece(new_gs, make_piece(EMPTY, YELLOW), make_square(0, A));
		}
	}
	else if (piece.piecetype == KING && move.from.file - move.to.file == -2) {
		if (piece.color == YELLOW) {
			replace_piece(new_gs, make_piece(ROOK, YELLOW), make_square(7, G));
			replace_piece(new_gs, make_piece(EMPTY, YELLOW), make_square(7, J));
		}
		else {
			replace_piece(new_gs, make_piece(ROOK, BLUE), make_square(0, G));
			replace_piece(new_gs, make_piece(EMPTY, YELLOW), make_square(0, J));
		}
	}

	replace_piece(new_gs, piece, move.to);
	replace_piece(new_gs, make_piece(EMPTY, YELLOW), move.from);

	new_gs->turn = !new_gs->turn;

	return new_gs;
}
static inline bool in_bounds_sq(Square s)
{
	return s.rank >= 0 && s.rank < 8 && s.file >= A && s.file <= J;
}

static inline bool is_empty_piece(Piece p)
{
	return p.piecetype == EMPTY;
}

void undo_move_in_place(GameState *gs, Move move, const UndoData *undo)
{
    Square from = move.from;
    Square to   = move.to;
    int rankDistance = to.rank - from.rank;
    int fileDistance = to.file - from.file;
    int direction;
    int i;

    // restore simple state first
    gs->yellow_qscastle = (undo->flags & UNDO_YELLOW_QSCASTLE) != 0;
    gs->yellow_kscastle = (undo->flags & UNDO_YELLOW_KSCASTLE) != 0;
    gs->blue_qscastle   = (undo->flags & UNDO_BLUE_QSCASTLE) != 0;
    gs->blue_kscastle   = (undo->flags & UNDO_BLUE_KSCASTLE) != 0;
    gs->anteater_ate    = (undo->flags & UNDO_ANTEATER_ATE) != 0;
    gs->turn            = (undo->flags & UNDO_TURN_BLUE) ? BLUE : YELLOW;
    gs->en_passant_square = undo->old_en_passant_square;

    // undo castling rook move first so squares are free
    if (undo->flags & UNDO_WAS_CASTLE) {
        gs->board[undo->rook_from.rank][undo->rook_from.file] =
            gs->board[undo->rook_to.rank][undo->rook_to.file];
        gs->board[undo->rook_to.rank][undo->rook_to.file] =
            make_piece(EMPTY, YELLOW);
    }

    // move main piece back
    gs->board[from.rank][from.file] = undo->moved_piece;
    gs->board[to.rank][to.file]     = make_piece(EMPTY, YELLOW);

    // restore captured piece
    if (in_bounds_sq(undo->captured_square) && !is_empty_piece(undo->captured_piece)) {
        gs->board[undo->captured_square.rank][undo->captured_square.file] = undo->captured_piece;
    }

    // undo anteater eaten path
    if (undo->moved_piece.piecetype == ANTEATER) {
        if (rankDistance == 0 && (fileDistance > 1 || fileDistance < -1)) {
            direction = (fileDistance > 0) ? 1 : -1;
            for (i = from.file + direction; i != to.file; i += direction) {
                gs->board[from.rank][i] = make_piece(ANT, BLUE);
            }
        }

        if (fileDistance == 0 && (rankDistance > 1 || rankDistance < -1)) {
            direction = (rankDistance > 0) ? 1 : -1;
            for (i = from.rank + direction; i != to.rank; i += direction) {
                gs->board[i][from.file] = make_piece(ANT, BLUE);
            }
        }
    }


}

void make_move_in_place(GameState *gs, Move move, UndoData *undo)
{
    Square from = move.from;
    Square to   = move.to;
    Piece moved = gs->board[from.rank][from.file];
    Piece dest  = gs->board[to.rank][to.file];

    int rankDistance;
    int fileDistance;
    int direction;
    int i;

    undo->moved_piece           = moved;
    undo->captured_piece        = make_piece(EMPTY, YELLOW);
    undo->old_en_passant_square = gs->en_passant_square;
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

    // default: move piece from -> to
    gs->board[to.rank][to.file]     = moved;
    gs->board[from.rank][from.file] = make_piece(EMPTY, YELLOW);

    // normal capture on destination
    if (!is_empty_piece(dest)) {
        undo->captured_piece  = dest;
        undo->captured_square = to;
    }

    // anteater special rule: eats ants along rank/file if moved >1 in straight line
    if (moved.piecetype == ANTEATER) {
        if (rankDistance == 0 && (fileDistance > 1 || fileDistance < -1)) {
            direction = (fileDistance > 0) ? 1 : -1;
            for (i = from.file + direction; i != to.file; i += direction) {
                // save only if piece exists; if multiple pieces can be eaten here,
                // this undo format assumes they are always ants and can be restored
                // by scanning again in undo.
                gs->board[from.rank][i] = make_piece(EMPTY, YELLOW);
            }
        }

        if (fileDistance == 0 && (rankDistance > 1 || rankDistance < -1)) {
            direction = (rankDistance > 0) ? 1 : -1;
            for (i = from.rank + direction; i != to.rank; i += direction) {
                gs->board[i][from.file] = make_piece(EMPTY, YELLOW);
            }
        }

        gs->anteater_ate = true;
    } else {
        gs->anteater_ate = false;
    }

    // en passant: ant moves diagonally to empty square
    if (moved.piecetype == ANT) {
        if (fileDistance != 0 && is_empty_piece(dest)) {
            Square epCaptured = make_square(from.rank, to.file);

            undo->captured_piece  = gs->board[epCaptured.rank][epCaptured.file];
            undo->captured_square = epCaptured;

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
                // kingside
                undo->rook_from = make_square(from.rank, J);
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
        }
    }

    // rook moved: revoke that rook's castling right
    if (moved.piecetype == ROOK) {
        if (square_equals(from, make_square(7, A))) gs->yellow_qscastle = false;
        if (square_equals(from, make_square(7, J))) gs->yellow_kscastle = false;
        if (square_equals(from, make_square(0, A))) gs->blue_qscastle   = false;
        if (square_equals(from, make_square(0, J))) gs->blue_kscastle   = false;
    }

    // rook captured on home square: revoke that side's castling right too
    if (!is_empty_piece(undo->captured_piece) && undo->captured_piece.piecetype == ROOK) {
        if (square_equals(undo->captured_square, make_square(7, A))) gs->yellow_qscastle = false;
        if (square_equals(undo->captured_square, make_square(7, J))) gs->yellow_kscastle = false;
        if (square_equals(undo->captured_square, make_square(0, A))) gs->blue_qscastle   = false;
        if (square_equals(undo->captured_square, make_square(0, J))) gs->blue_kscastle   = false;
    }

    // flip turn
    gs->turn = (gs->turn == YELLOW) ? BLUE : YELLOW;
}



GameState initalize_empty_GameState() {
	GameState gs;
	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 10; j++) {
			gs.board[i][j] = make_piece(EMPTY, YELLOW);


		}
	}

	gs.turn = YELLOW;
	gs.anteater_ate = false;
	gs.yellow_kscastle = true;
	gs.yellow_qscastle = true;
	gs.blue_kscastle = true;
	gs.blue_qscastle = true;
	gs.en_passant_square = make_square(-2,-1);
	gs.prev_state = NULL;

	return gs;
}