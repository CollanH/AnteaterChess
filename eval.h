#ifndef EVAL_H
#define EVAL_H
/*
eval.h - Chess board Evaluator
#include "eval.h" in any file that needs to call evaluate() 
or any of the subfunctions
*/

/*TODO: Define Board and Piece Structs with group in a 
board.h file and place #include "board.h" here*/

/*
CENTIPAWN PIECE WEIGHTS
used by evalMaterial()
using centipawn scale
100 cp = 1 pawn
TODO: make sure piece code order matches board.h
*/

static const int PIECE_VALUE[] ={
    0, // 0 = EMPTY
    100,  // 1 = PAWN
    175, // 2 = ANTEATER
    320, // 3 = KNIGHT
    330, // 4 = BISHOP
    500, // 5 = ROOK
    900, // 6 = QUEEN
    0 // 7 = KING
};


/*
EVALUATION WEIGHTS
tunable constants used across the eval functions
adjust once engine play legal chess
*/

/*Function Declarations*/

// Master evalution function - called by negamax
int evaluate(/*defined Board struct*/);
//counts the raw piece values on the board
int evalMaterial(/*defined Board struct*/);
//adds positional bonuses from Piece-Square Tables
int evalPST(/*defined Board struct*/);

//counts reachable squares for each piece (excluding pawns and kings)
int evalMobility(/*defined Board struct*/);

//evaluates passed, doubled, and isolated pawns
int evalPawnStructure(/*defined Board Struct*/);

//penalises each king based on potential attack in 3x3 zone & missing or advanced pawns in pawn shield
int evalKingSafety(/*defined Board Struct*/);

//evaluates anteater piece - bonus for multi-capture potential & penalty if piece is idle
int evalAnteater(/*defined Board Struct*/);
/*
Returns 1 if the piece on square 'from' can attack
square 'to' given the current board occupancy.
Returns 0 otherwise.
*/
int canAttackSquare(/*defined Board Struct, int from, int to*/);



#endif