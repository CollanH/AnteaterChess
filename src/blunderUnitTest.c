#include <stdio.h>
#include "chess_types.h"
#include "strategy.h"
#include "textui.h"

//function declarations
void blunder_take_free_queen(void);
void blunder_recapture_trap(void);
void blunder_queen_for_pawn(void);
void blunder_mate_over_material(void);

//AI should take the undefended queen sitting on the same rank as the rook
//blunder: missing free material entirely
void blunder_take_free_queen(void)
{
    GameState gs = initalize_empty_GameState();

    //yellow rook on A4, blue queen undefended at E4 on the same rank
    gs.board[4][A] = make_piece(ROOK,  YELLOW);
    gs.board[4][E] = make_piece(QUEEN, BLUE);
    gs.board[7][J] = make_piece(KING,  YELLOW);
    gs.board[0][J] = make_piece(KING,  BLUE);
    gs.turn = YELLOW;

    printf("--- Test 1: take free queen ---\n");
    printf("// BLUNDER CHECK: AI (yellow) should capture the undefended blue queen at E4\n");
    printf("// Blunder would be: missing the free queen and playing something else\n");
    displayBoard(&gs, 0, 0, YELLOW);

    Move *m = SelectBestMove(&gs, YELLOW, 4);

    if (m == NULL)
    {
        printf("FAIL: AI returned NULL\n\n");
        return;
    }

    if (m->to.rank == 4 && m->to.file == E)
    {
        printf("PASS: AI captured the free queen at E4\n\n");
    }
    else
    {
        printf("FAIL: AI missed free queen — played %c%d -> %c%d instead of A4 -> E4\n\n",
               'A' + m->from.file, 8 - m->from.rank,
               'A' + m->to.file,   8 - m->to.rank);
    }
}

//AI should NOT capture the blue pawn at C5 — blue rook on C8 recaptures the queen
//without quiescence search the AI stops looking at depth limit and misses the recapture
//the free knight at H4 is the correct target (+3 vs -8 after recapture)
void blunder_recapture_trap(void)
{
    GameState gs = initalize_empty_GameState();

    //yellow queen at B4 can take pawn at C5 diagonally, but rook on C file recaptures
    gs.board[4][B] = make_piece(QUEEN,  YELLOW);
    gs.board[3][C] = make_piece(ANT,    BLUE);    //pawn looks capturable
    gs.board[0][C] = make_piece(ROOK,   BLUE);    //rook defends pawn via C file
    gs.board[4][H] = make_piece(KNIGHT, BLUE);    //undefended knight — correct target
    gs.board[7][J] = make_piece(KING,   YELLOW);
    gs.board[0][J] = make_piece(KING,   BLUE);
    gs.turn = YELLOW;

    printf("--- Test 2: avoid recapture trap ---\n");
    printf("// BLUNDER CHECK: queen at B4 should NOT take pawn at C5\n");
    printf("// C8 rook recaptures queen after QxC5 — net -8 for yellow\n");
    printf("// Correct play: take the free knight at H4 (+3)\n");
    displayBoard(&gs, 0, 0, YELLOW);

    Move *m = SelectBestMove(&gs, YELLOW, 4);

    if (m == NULL)
    {
        printf("FAIL: AI returned NULL\n\n");
        return;
    }

    if (m->from.rank == 4 && m->from.file == B &&
        m->to.rank   == 3 && m->to.file   == C)
    {
        printf("FAIL: AI walked into recapture trap — QxC5 loses queen to rook (blunder!)\n\n");
    }
    else
    {
        printf("PASS: AI avoided recapture trap — played %c%d -> %c%d\n\n",
               'A' + m->from.file, 8 - m->from.rank,
               'A' + m->to.file,   8 - m->to.rank);
    }
}

//AI should NOT trade the queen for the pawn at E4 — blue rook at E7 recaptures
//free bishop at A6 is the correct target (+3 vs -8 after recapture)
void blunder_queen_for_pawn(void)
{
    GameState gs = initalize_empty_GameState();

    //yellow queen at A4, blue pawn at E4 on same rank but defended by rook on E file
    gs.board[4][A] = make_piece(QUEEN,  YELLOW);
    gs.board[4][E] = make_piece(ANT,    BLUE);    //pawn on same rank, looks capturable
    gs.board[1][E] = make_piece(ROOK,   BLUE);    //rook defends pawn via E file
    gs.board[2][A] = make_piece(BISHOP, BLUE);    //undefended bishop — correct target
    gs.board[7][J] = make_piece(KING,   YELLOW);
    gs.board[0][J] = make_piece(KING,   BLUE);
    gs.turn = YELLOW;

    printf("--- Test 3: queen not for pawn ---\n");
    printf("// BLUNDER CHECK: queen at A4 should NOT take pawn at E4\n");
    printf("// E7 rook recaptures queen after QxE4 — net -8 for yellow\n");
    printf("// Correct play: take the free bishop at A6 (+3)\n");
    displayBoard(&gs, 0, 0, YELLOW);

    Move *m = SelectBestMove(&gs, YELLOW, 4);

    if (m == NULL)
    {
        printf("FAIL: AI returned NULL\n\n");
        return;
    }

    if (m->from.rank == 4 && m->from.file == A &&
        m->to.rank   == 4 && m->to.file   == E)
    {
        printf("FAIL: AI sacrificed queen for pawn — rook recaptures (blunder!)\n\n");
    }
    else
    {
        printf("PASS: AI avoided queen-for-pawn trap — played %c%d -> %c%d\n\n",
               'A' + m->from.file, 8 - m->from.rank,
               'A' + m->to.file,   8 - m->to.rank);
    }
}

//AI should deliver checkmate immediately rather than grabbing free material
//Rb3 -> B8 is checkmate: A7 rook covers rank 1, B8 rook covers rank 0, king trapped at J8
//blunder: taking the free blue queen at A5 and letting the game continue
void blunder_mate_over_material(void)
{
    GameState gs = initalize_empty_GameState();

    //blue king cornered at J8, yellow can deliver back-rank mate with Rb8
    gs.board[0][J] = make_piece(KING,  BLUE);
    gs.board[1][A] = make_piece(ROOK,  YELLOW);  //covers rank 1 — cuts off J7 and I7
    gs.board[5][B] = make_piece(ROOK,  YELLOW);  //slides to B8 for checkmate
    gs.board[3][A] = make_piece(QUEEN, BLUE);    //free queen at A5 — tempting distraction
    gs.board[7][H] = make_piece(KING,  YELLOW);
    gs.turn = YELLOW;

    printf("--- Test 4: prefer checkmate over material ---\n");
    printf("// BLUNDER CHECK: AI should play Rb3 -> B8 (checkmate), not grab the free queen at A5\n");
    printf("// Rook on B8 + rook on A7 traps blue king at J8 with no escape squares\n");
    displayBoard(&gs, 0, 0, YELLOW);

    Move *m = SelectBestMove(&gs, YELLOW, 4);

    if (m == NULL)
    {
        printf("FAIL: AI returned NULL\n\n");
        return;
    }

    if (m->from.rank == 5 && m->from.file == B &&
        m->to.rank   == 0 && m->to.file   == B)
    {
        printf("PASS: AI delivered checkmate (Rb3 -> B8#)\n\n");
    }
    else
    {
        printf("FAIL: AI missed checkmate — played %c%d -> %c%d instead of B3 -> B8\n\n",
               'A' + m->from.file, 8 - m->from.rank,
               'A' + m->to.file,   8 - m->to.rank);
    }
}

int main(void)
{
    printf("=== Blunder Unit Tests ===\n\n");

    blunder_take_free_queen();
    blunder_recapture_trap();
    blunder_queen_for_pawn();
    blunder_mate_over_material();

    printf("Done.\n");
    return 0;
}
