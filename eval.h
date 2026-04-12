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
    0, // 1 = KING
    900, // 2 = QUEEN
    175, // 3 = ANTEATER
    330, // 4 = BISHOP
    320, // 5 = KNIGHT
    500, // 6 = ROOK
    100,  // 7 = PAWN
};


/*
EVALUATION WEIGHTS
tunable constants used across the eval functions
adjust once engine play legal chess
*/

/*Mobility - centipawns per reachable square, per piece type*/
#define MOB_KNIGHT 4
#define MOB_BISHOP 5
#define MOB_ROOK 3
#define MOB_Queen 2
#define MOB_ANTEATER 1

/*Pawn structure penalties*/
#define DOUBLED_ANT_PENALTY 20
#define ISOLATED_ANT_PENALTY 20

/*King Safety - shield penalities*/
#define SHIELD_MISSING 15
#define SHIELD_PUSHED 7

/*Anteater bonus per adjacent enemy ant*/
#define ANTEATER_ADJ_BONUS 30


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