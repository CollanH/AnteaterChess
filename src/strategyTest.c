#include <stdio.h>
#include "chess_types.h"
#include "strategy.h"

//function declarations
void expect_move(const char *testName, Move *result, int fromRank, File fromFile, int toRank, File toFile);

void expect_move(const char *testName, Move *result, int fromRank, File fromFile, int toRank, File toFile)
{
    if (result == NULL)
    {
        printf("FAIL: %s — AI returned NULL\n", testName);
        return;
    }
    if (result->from.rank == fromRank && result->from.file == fromFile &&
        result->to.rank   == toRank   && result->to.file   == toFile)
    {
        printf("PASS: %s\n", testName);
    }
    else
    {
        printf("FAIL: %s — got %c%d -> %c%d, expected %c%d -> %c%d\n",
               testName,
               'A' + result->from.file, 8 - result->from.rank,
               'A' + result->to.file,   8 - result->to.rank,
               'A' + fromFile,          8 - fromRank,
               'A' + toFile,            8 - toRank);
    }
}

//AI should take a free queen sitting on the same rank as a rook
void test_take_free_queen(void)
{
    GameState gs = initalize_empty_GameState();

    //yellow rook on rank 4, blue queen sitting undefended on same rank
    gs.board[3][A] = make_piece(ROOK,  YELLOW);
    gs.board[3][E] = make_piece(QUEEN, BLUE);

    //kings required for legal move gen
    gs.board[7][F] = make_piece(KING, YELLOW);
    gs.board[0][F] = make_piece(KING, BLUE);
    gs.turn = YELLOW;

    Move *m = SelectBestMove(&gs, YELLOW, 2);
    expect_move("take free queen", m, 3, A, 3, E);
}

//AI should find mate in one: Rb5 -> Rb8 delivers back rank checkmate
void test_mate_in_one(void)
{
    GameState gs = initalize_empty_GameState();

    //blue king cornered at J8, yellow rooks control escape squares
    gs.board[0][J] = make_piece(KING, BLUE);
    gs.board[1][A] = make_piece(ROOK, YELLOW);  //covers entire row 1, cuts off J7 and I7
    gs.board[5][B] = make_piece(ROOK, YELLOW);  //needs to move to B8 for checkmate

    gs.board[7][A] = make_piece(KING, YELLOW);
    gs.turn = YELLOW;

    Move *m = SelectBestMove(&gs, YELLOW, 2);
    expect_move("mate in one", m, 5, B, 0, B);
}

//AI should not move a piece to a square where it gets immediately captured
void test_no_blunder(void)
{
    GameState gs = initalize_empty_GameState();

    //yellow queen exposed — AI (blue) should take it
    gs.board[3][E] = make_piece(QUEEN, YELLOW);
    gs.board[4][D] = make_piece(QUEEN, BLUE);   //blue queen can take yellow queen

    gs.board[7][F] = make_piece(KING, YELLOW);
    gs.board[0][F] = make_piece(KING, BLUE);
    gs.turn = BLUE;

    Move *m = SelectBestMove(&gs, BLUE, 2);
    expect_move("take hanging queen", m, 4, D, 3, E);
}

//apply_move should correctly flip whose turn it is
void test_apply_move_turn_flip(void)
{
    GameState gs = initalize_empty_GameState();
    gs.board[6][E] = make_piece(ANT,  YELLOW);
    gs.board[7][F] = make_piece(KING, YELLOW);
    gs.board[0][F] = make_piece(KING, BLUE);
    gs.turn = YELLOW;

    Move m;
    m.from = make_square(6, E);
    m.to   = make_square(5, E);

    GameState after = apply_move(&gs, m);

    if (after.turn == BLUE)
        printf("PASS: apply_move flips turn\n");
    else
        printf("FAIL: apply_move did not flip turn\n");
}

//apply_move should clear the source square
void test_apply_move_clears_from(void)
{
    GameState gs = initalize_empty_GameState();
    gs.board[6][E] = make_piece(ANT,  YELLOW);
    gs.board[7][F] = make_piece(KING, YELLOW);
    gs.board[0][F] = make_piece(KING, BLUE);
    gs.turn = YELLOW;

    Move m;
    m.from = make_square(6, E);
    m.to   = make_square(5, E);

    GameState after = apply_move(&gs, m);

    if (after.board[6][E].piecetype == EMPTY)
        printf("PASS: apply_move clears source square\n");
    else
        printf("FAIL: apply_move left piece on source square\n");
}

int main(void)
{
    printf("--- Strategy Tests ---\n\n");

    printf("--- apply_move ---\n");
    test_apply_move_turn_flip();
    test_apply_move_clears_from();

    printf("\n--- AI move selection ---\n");
    test_take_free_queen();
    test_no_blunder();
    test_mate_in_one();

    printf("\nDone.\n");
    return 0;
}
