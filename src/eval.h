
#ifndef EVAL_H
#define EVAL_H
#include "chess_types.h"
#include <stdio.h>

/*CENTIPAWN PIECE WEIGHTS*/
static const int PIECE_VALUE[8] ={
    0, // EMPTY
    0, // KING
    900, // QUEEN
    175, // ANTEATER
    330, // BISHOP
    320, // KNIGHT
    500, // ROOK
    100,  // ANT
};

/*PIECE SQUARE TABLE*/
static const int PST_ANT[8][10]={
    {  0,  0,   5,   5,   0,   0,   5,  50,   0,  0 },
    {  0,  0,  10,  -5,   0,   0, -10,  50,   0,  0 },
    {  0,  0,  10,  10,  20,  20,  20,  50,   0,  0 },
    {  0,  0,   5,  10,  25,  25,  30,  50,   0,  0 },
    {  0,  0,   5,  10,  25,  25,  30,  50,   0,  0 },
    {  0,  0,  10,  10,  20,  20,  20,  50,   0,  0 },
    {  0,  0,  10,  -5,   0,   0, -10,  50,   0,  0 },
    {  0,  0,   5,   5,   0,   0,   5,  50,   0,  0 }

};

static const int PST_KNIGHT[8][10] ={
    {  0,-50, -40, -30, -30, -30, -30, -40, -50,  0 },
    {  0,-40, -20,   0,   5,   5,   0, -20, -40,  0 },
    {  0,-30,   0,  10,  15,  15,  10,   0, -30,  0 },
    {  0,-30,   5,  15,  20,  20,  15,   5, -30,  0 },
    {  0,-30,   5,  15,  20,  20,  15,   5, -30,  0 },
    {  0,-30,   0,  10,  15,  15,  10,   0, -30,  0 },
    {  0,-40, -20,   0,   5,   5,   0, -20, -40,  0 },
    {  0,-50, -40, -30, -30, -30, -30, -40, -50,  0 }
};

static const int PST_BISHOP[8][10] ={
    {  0,-20, -10, -10, -10, -10, -10, -10, -20,  0 },
    {  0,-10,   5,   0,   0,   0,   0,   5, -10,  0 },
    {  0,-10,  10,  10,  10,  10,  10,  10, -10,  0 },
    {  0,-10,   0,  10,  10,  10,  10,   0, -10,  0 },
    {  0,-10,   0,  10,  10,  10,  10,   0, -10,  0 },
    {  0,-10,  10,  10,  10,  10,  10,  10, -10,  0 },
    {  0,-10,   5,   0,   0,   0,   0,   5, -10,  0 },
    {  0,-20, -10, -10, -10, -10, -10, -10, -20,  0 }
};

static const int PST_ROOK[8][10] ={
    {  0,  0,  -5,  -5,  -5,  -5,  -5,   5,   0,  0 },
    {  0,  0,   0,   0,   0,   0,   0,  10,   0,  0 },
    {  0,  0,   0,   0,   0,   0,   0,  10,   0,  0 },
    {  0,  5,   0,   0,   0,   0,   0,  10,   0,  0 },
    {  0,  5,   0,   0,   0,   0,   0,  10,   0,  0 },
    {  0,  0,   0,   0,   0,   0,   0,  10,   0,  0 },
    {  0,  0,   0,   0,   0,   0,   0,  10,   0,  0 },
    {  0,  0,  -5,  -5,  -5,  -5,  -5,   5,   0,  0 }
};

static const int PST_QUEEN[8][10]={
    {  0,-20, -10, -10,  -5,  -5, -10, -10, -20,  0 },
    {  0,-10,   0,   5,   0,   0,   0,   0, -10,  0 },
    {  0,-10,   5,   5,   5,   5,   5,   0, -10,  0 },
    {  0,  0,   0,   5,   5,   5,   5,   0,  -5,  0 },
    {  0, -5,   0,   5,   5,   5,   5,   0,  -5,  0 },
    {  0,-10,   0,   5,   5,   5,   5,   0, -10,  0 },
    {  0,-10,   0,   0,   0,   0,   0,   0, -10,  0 },
    {  0,-20, -10, -10,  -5,  -5, -10, -10, -20,  0 }

};

static const int PST_KING[8][10]={
    {  0, 20,  30,  10,   0,   0,  10,  30,  20,  0 },
    {  0, 20,  20,   0,   0,   0,   0,  20,  20,  0 },
    {  0,-10, -20, -20, -20, -20, -20, -20, -10,  0 },
    {  0,-20, -30, -30, -40, -40, -30, -30, -20,  0 },
    {  0,-20, -30, -30, -40, -40, -30, -30, -20,  0 },
    {  0,-10, -20, -20, -20, -20, -20, -20, -10,  0 },
    {  0, 20,  20,   0,   0,   0,   0,  20,  20,  0 },
    {  0, 20,  30,  10,   0,   0,  10,  30,  20,  0 }
};

static const int PST_ANTEATER[8][10]={
    {  0,  0,  -5,  -5,  -5,  -5,  -5,  -5,   0,  0 },
    {  0, -5,   5,  10,  10,  10,  10,   5,  -5,  0 },
    {  0, -5,  10,  15,  15,  15,  15,  10,  -5,  0 },
    {  0, -5,  10,  15,  20,  20,  15,  10,  -5,  0 },
    {  0, -5,  10,  15,  20,  20,  15,  10,  -5,  0 },
    {  0, -5,  10,  15,  15,  15,  15,  10,  -5,  0 },
    {  0, -5,   5,  10,  10,  10,  10,   5,  -5,  0 },
    {  0,  0,  -5,  -5,  -5,  -5,  -5,  -5,   0,  0 }
};

/*PST TABLE*/
static const int* PST_TABLE[8]={
    NULL, //EMPTY
    (const int*)PST_KING, //KING
    (const int*) PST_QUEEN, 
    (const int*) PST_ANTEATER,
    (const int*) PST_BISHOP,
    (const int*) PST_KNIGHT,
    (const int*) PST_ROOK,
    (const int*) PST_ANT
};

/*MOBILITY WEIGHTS*/
#define MOB_KNIGHT 3
#define MOB_BISHOP 3
#define MOB_ROOK 5
#define MOB_QUEEN 9
#define MOB_ANTEATER 2
#define MOB_ANT 1

/*MOBILITY WEIGHTS ARRAY*/
static const int MOB_WEIGHTS[8]={
    0,
    0,
    MOB_QUEEN,
    MOB_ANTEATER,
    MOB_BISHOP,
    MOB_KNIGHT,
    MOB_ROOK,
    MOB_ANT
};

/*ATTACK PRESSURE*/
static const int ATTACKER_WEIGHT[8]={
    0,
    0,
    5,
    0,
    2,
    2,
    3,
    0
};

/*Pawn structure penalties*/
#define DOUBLED_ANT_PENALTY 20
#define ISOLATED_ANT_PENALTY 20
/*King Safety - shield penalities*/
#define SHIELD_MISSING 5
#define SHIELD_PUSHED 2
/*Anteater bonus per adjacent enemy ant*/
#define ANTEATER_ADJ_BONUS 10

static const int DANGER_TABLE[100] = {
   0,  0,  1,  2,  3,  5,  7, 10, 14, 20,
   28, 37, 49, 64, 82,101,128,160,196,240,
   280,320,360,400,440,480,512,512,512,512,
   512,512,512,512,512,512,512,512,512,512,
   512,512,512,512,512,512,512,512,512,512,
   512,512,512,512,512,512,512,512,512,512,
   512,512,512,512,512,512,512,512,512,512,
   512,512,512,512,512,512,512,512,512,512,
   512,512,512,512,512,512,512,512,512,512,
   512,512,512,512,512,512,512,512,512,512
};

// add implementation of a safety table  for the evalKingSafetey function 

/*FUNCTION DECLARATION*/
// Master evalution function - called by negamax
int evaluate(GameState *gs);
//counts the raw piece values on the board
int evalMaterial(GameState *gs);
//adds positional bonuses from Piece-Square Tables
int evalPST(GameState *gs);
//counts reachable squares for each piece (excluding pawns and kings)
int evalMobility(GameState *gs);
//evaluates passed, doubled, and isolated pawns
int evalPawnStructure(GameState *gs);
//penalises each king based on potential attack in 3x3 zone & missing or advanced pawns in pawn shield
int evalKingSafety(GameState *gs);
//evaluates anteater piece - bonus for multi-capture potential & penalty if piece is idle
int evalAnteater(GameState *gs);
/*Returns 1 if the piece on square 'from' can attack square 'to' given the current board occupancy.Returns 0 otherwise.*/
int canAttackSquare(GameState *gs, int fa, int ra, int fb, int rb);

/*BETA: Reward Checkmate Setup*/
/*Pieces close to enemy king*/
int evalKingTropism(GameState *gs);
/*How trapped is the king*/
int evalKingEscape(GameState *gs);
/*Back Rank Weakness*/
int evalBackRank(GameState *gs);
/*add a tiny bonus for pieces that are on the opponent's half of the board */
int evalTempo(GameState *gs);

#endif