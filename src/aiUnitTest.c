// compile: gcc -Wall -g -o bin/aiTest src/aiUnitTest.c src/chess_types.c src/legalMoveGen.c src/strategy.c src/eval.c src/textui.c -Isrc -lm

#include <stdio.h>
#include "chess_types.h"
#include "strategy.h"
#include "textui.h"

//function declarations
void test_checkmate_medium(void);
void test_checkmate_hard(void);
void test_ai_promotion_to_queen(void);
void test_anteater_chain_eat(void);

//AI should find the only mating move at medium depth (depth=4)
//Ra3->A8: rook slides to rank 8, B7 rook already seals rank 7 — blue king cornered at J8
void test_checkmate_medium(void)
{
    GameState gs   = initalize_empty_GameState();
    Move     *move;

    //blue king in top-right corner, two yellow rooks box it in
    gs.board[0][J] = make_piece(KING, BLUE);
    gs.board[1][B] = make_piece(ROOK, YELLOW);   //seals rank 7 (display) / rank 1 (internal)
    gs.board[5][A] = make_piece(ROOK, YELLOW);   //slides to A8 for checkmate
    gs.board[7][H] = make_piece(KING, YELLOW);
    gs.turn = YELLOW;

    printf("--- Test 1: checkmate delivery (medium, depth=4) ---\n");
    printf("// CHECKMATE CHECK: yellow rook at A3 should slide to A8\n");
    printf("// B7 rook seals escape along rank 7 — blue king at J8 has no safe square\n");
    displayBoard(&gs, 0, 0, YELLOW);

    move = SelectBestMove(&gs, YELLOW, 4);
    if (move == NULL)
    {
        printf("FAIL: AI returned NULL\n\n");
        return;
    }

    if (move->from.rank == 5 && move->from.file == A &&
        move->to.rank   == 0 && move->to.file   == A)
    {
        printf("PASS: AI delivered checkmate (Ra3 -> A8#)\n\n");
    }
    else
    {
        printf("FAIL: AI missed checkmate — played %c%d -> %c%d instead of A3 -> A8\n\n",
               'A' + move->from.file, 8 - move->from.rank,
               'A' + move->to.file,   8 - move->to.rank);
    }
}

//AI should find the only mating move at hard depth (depth=8)
//Rj3->J8: rook slides to rank 8, C7 rook already seals rank 7 — blue king cornered at A8
void test_checkmate_hard(void)
{
    GameState gs   = initalize_empty_GameState();
    Move     *move;

    //blue king in top-left corner, two yellow rooks box it in from the other side
    gs.board[0][A] = make_piece(KING, BLUE);
    gs.board[1][C] = make_piece(ROOK, YELLOW);   //seals rank 7 (display) / rank 1 (internal)
    gs.board[5][J] = make_piece(ROOK, YELLOW);   //slides to J8 for checkmate
    gs.board[7][H] = make_piece(KING, YELLOW);
    gs.turn = YELLOW;

    printf("--- Test 2: checkmate delivery (hard, depth=8) ---\n");
    printf("// CHECKMATE CHECK: yellow rook at J3 should slide to J8\n");
    printf("// C7 rook seals rank 7 — blue king at A8 has no safe square\n");
    displayBoard(&gs, 0, 0, YELLOW);

    move = SelectBestMove(&gs, YELLOW, 8);
    if (move == NULL)
    {
        printf("FAIL: AI returned NULL\n\n");
        return;
    }

    if (move->from.rank == 5 && move->from.file == J &&
        move->to.rank   == 0 && move->to.file   == J)
    {
        printf("PASS: AI delivered checkmate (Rj3 -> J8#)\n\n");
    }
    else
    {
        printf("FAIL: AI missed checkmate — played %c%d -> %c%d instead of J3 -> J8\n\n",
               'A' + move->from.file, 8 - move->from.rank,
               'A' + move->to.file,   8 - move->to.rank);
    }
}

//ant reaching rank 0 or rank 7 is auto-promoted to queen inside make_move_in_place
//no dispPromotion dialog is shown for AI moves — chess_types.c hardcodes QUEEN
void test_ai_promotion_to_queen(void)
{
    GameState gs        = initalize_empty_GameState();
    UndoData  undo;
    Move     *choice;
    Move      applied;

    //yellow ant one push from promotion, no other high-value moves available
    gs.board[1][E] = make_piece(ANT,  YELLOW);   //rank 1 -> rank 0 on next push
    gs.board[7][H] = make_piece(KING, YELLOW);
    gs.board[0][A] = make_piece(KING, BLUE);
    gs.turn = YELLOW;

    printf("--- Test 3: AI auto-promotes ant to queen ---\n");
    printf("// PROMOTION CHECK: AI should push ant from E7 to E8\n");
    printf("// make_move_in_place sets piecetype=QUEEN on rank 0 automatically (no dialog for AI)\n");
    displayBoard(&gs, 0, 0, YELLOW);

    choice = SelectBestMove(&gs, YELLOW, 4);
    if (choice == NULL)
    {
        printf("FAIL: AI returned NULL\n\n");
        return;
    }

    applied = *choice;
    make_move_in_place(&gs, applied, &undo);

    printf("Board after AI move:\n");
    displayBoard(&gs, 0, 0, YELLOW);

    if (applied.to.rank != 0)
    {
        printf("FAIL: AI did not play the promotion — played %c%d -> %c%d\n\n",
               'A' + applied.from.file, 8 - applied.from.rank,
               'A' + applied.to.file,   8 - applied.to.rank);
        return;
    }

    if (gs.board[0][applied.to.file].piecetype == QUEEN)
    {
        printf("PASS: ant promoted to queen at %c8\n\n", 'A' + applied.to.file);
    }
    else
    {
        printf("FAIL: ant reached rank 0 but piece type is %d (expected QUEEN=%d)\n\n",
               gs.board[0][applied.to.file].piecetype, QUEEN);
    }
}

//anteater eats enemy ants in a chain — turn stays with yellow until no adjacent enemy ant remains
//three blue ants lined up: D4->E4, E4->F4, F4->G4 all in one yellow turn
void test_anteater_chain_eat(void)
{
    GameState gs   = initalize_empty_GameState();
    UndoData  undo;
    Move      move;
    int       file;
    int       remaining;

    //anteater at D4, three blue ants to its right along rank 4
    gs.board[4][D] = make_piece(ANTEATER, YELLOW);
    gs.board[4][E] = make_piece(ANT,      BLUE);
    gs.board[4][F] = make_piece(ANT,      BLUE);
    gs.board[4][G] = make_piece(ANT,      BLUE);
    gs.board[7][A] = make_piece(KING,     YELLOW);
    gs.board[0][J] = make_piece(KING,     BLUE);
    gs.turn = YELLOW;

    printf("--- Test 4: anteater chain eats 3 ants in one turn ---\n");
    printf("// CHAIN CHECK: yellow anteater at D4 eats E4, then F4, then G4\n");
    printf("// anteater_ate stays true while adjacent enemy ants remain\n");
    displayBoard(&gs, 0, 0, YELLOW);

    //first capture: D4 -> E4
    move.from = make_square(4, D);
    move.to   = make_square(4, E);
    make_move_in_place(&gs, move, &undo);

    printf("After first capture (D4 -> E4):\n");
    displayBoard(&gs, 0, 0, YELLOW);

    if (!gs.anteater_ate)
    {
        printf("FAIL: anteater_ate should be true — adjacent ant at F4 still present\n\n");
        return;
    }
    if (gs.turn != YELLOW)
    {
        printf("FAIL: turn should remain YELLOW while chain is active\n\n");
        return;
    }
    printf("  chain active (anteater_ate=true, still yellow's turn)\n\n");

    //second capture: E4 -> F4
    move.from = make_square(4, E);
    move.to   = make_square(4, F);
    make_move_in_place(&gs, move, &undo);

    printf("After second capture (E4 -> F4):\n");
    displayBoard(&gs, 0, 0, YELLOW);

    if (!gs.anteater_ate)
    {
        printf("FAIL: anteater_ate should be true — adjacent ant at G4 still present\n\n");
        return;
    }
    if (gs.turn != YELLOW)
    {
        printf("FAIL: turn should remain YELLOW while chain is active\n\n");
        return;
    }
    printf("  chain active (anteater_ate=true, still yellow's turn)\n\n");

    //third capture: F4 -> G4 — no more adjacent enemy ants after this
    move.from = make_square(4, F);
    move.to   = make_square(4, G);
    make_move_in_place(&gs, move, &undo);

    printf("After third capture (F4 -> G4):\n");
    displayBoard(&gs, 0, 0, YELLOW);

    if (gs.anteater_ate)
    {
        printf("FAIL: anteater_ate should be false — no more adjacent enemy ants\n\n");
        return;
    }
    if (gs.turn != BLUE)
    {
        printf("FAIL: turn should advance to BLUE once chain ends\n\n");
        return;
    }

    //verify no blue ants remain on rank 4
    remaining = 0;
    for (file = 0; file < 10; file++)
    {
        if (gs.board[4][file].piecetype == ANT && gs.board[4][file].color == BLUE)
        {
            remaining++;
        }
    }

    if (remaining == 0)
    {
        printf("PASS: anteater ate all 3 ants in one turn, chain ended correctly (turn->BLUE)\n\n");
    }
    else
    {
        printf("FAIL: %d blue ant(s) still on rank 4 after chain\n\n", remaining);
    }
}

int main(void)
{
    printf("=== AI Checkmate, Promotion, and Anteater Chain Tests ===\n\n");

    test_checkmate_medium();
    test_checkmate_hard();
    test_ai_promotion_to_queen();
    test_anteater_chain_eat();

    printf("Done.\n");
    return 0;
}
