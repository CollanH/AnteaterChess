
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

#define PHASE_KNIGHT 1
#define PHASE_BISHOP 1
#define PHASE_ROOK 2
#define PHASE_QUEEN 4
#define PHASE_MAX 24

/*PIECE SQUARE TABLE -OPENING GAME PHASE*/
static const int PST_ANT_OPENING[8][10] = {
    {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  
    {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  
    { -5,   0,   5,  10,  15,  15,  10,   5,   0,  -5},  
    { -5,   0,  10,  20,  25,  25,  20,  10,   0,  -5},  
    { -5,   0,  10,  20,  25,  25,  20,  10,   0,  -5},  
    { -5,   0,   5,  10,  15,  15,  10,   5,   0,  -5},  
    {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0},  
    {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0}   
};

static const int PST_KNIGHT_OPENING[8][10] ={
    {-50, -40, -30, -30, -30, -30, -30, -30, -40, -50},
    {-40, -20,   0,   5,   5,   0, -20, -20, -40, -40},
    {-30,   5,  15,  15,  15,  15,   5,   5, -30, -30},
    {-30,   5,  20,  25,  25,  20,   5,   5, -30, -30},
    {-30,   5,  20,  25,  25,  20,   5,   5, -30, -30},
    {-30,   5,  15,  15,  15,  15,   5,   5, -30, -30},
    {-40, -20,   0,   5,   5,   0, -20, -20, -40, -40},
    {-50, -40, -30, -30, -30, -30, -30, -30, -40, -50}
};

static const int PST_BISHOP_OPENING[8][10] ={
    {-20, -10, -10, -10, -10, -10, -10, -10, -10, -20},
    {-10,   5,   0,   0,   0,   0,   0,   0,   5, -10},
    {-10,  10,  10,  10,  10,  10,  10,  10,  10, -10},
    {-10,   5,  15,  15,  15,  15,  15,  15,   5, -10},
    {-10,   5,  15,  15,  15,  15,  15,  15,   5, -10},
    {-10,  10,  10,  10,  10,  10,  10,  10,  10, -10},
    {-10,   5,   0,   0,   0,   0,   0,   0,   5, -10},
    {-20, -10, -10, -10, -10, -10, -10, -10, -10, -20}
};

static const int PST_ROOK_OPENING[8][10] ={
    {0, 0, -5, -5, -5, -5, -5, 5, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 10, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 10, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 10, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 10, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 10, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 10, 0, 0},
    {0, 0, -5, -5, -5, -5, -5, 5, 0, 0}
};

static const int PST_QUEEN_OPENING[8][10] ={
    {-20, -15, -10, -5, -5, -5, -5, -10, -15, -20},
    {-15,   0,   0,  0,  0,  0,  0,   0,   0, -15},
    {-10,   0,   5,  5,  5,  5,  5,   5,   0, -10},
    { -5,   0,   5,  5,  5,  5,  5,   5,   0,  -5},
    { -5,   0,   5,  5,  5,  5,  5,   5,   0,  -5},
    {-10,   0,   5,  5,  5,  5,  5,   5,   0, -10},
    {-15,   0,   0,  0,  0,  0,  0,   0,   0, -15},
    {-20, -15, -10, -5, -5, -5, -5, -10, -15, -20}
};

static const int PST_KING_OPENING[8][10] = {
    { 30,  40,  20,   0, -20, -20,   0,  20,  40,  30},  
    { 20,  20,   0, -10, -20, -20, -10,   0,  20,  20},
    {-10, -20, -20, -20, -20, -20, -20, -20, -20, -10},
    {-20, -30, -30, -40, -40, -40, -40, -30, -30, -20},
    {-20, -30, -30, -40, -40, -40, -40, -30, -30, -20},
    {-10, -20, -20, -20, -20, -20, -20, -20, -20, -10},
    { 20,  20,   0, -10, -20, -20, -10,   0,  20,  20},
    { 30,  40,  20,   0, -20, -20,   0,  20,  40,  30}
};

static const int PST_ANTEATER_OPENING[8][10] ={
    {-10, -5, -5, -5, -5, -5, -5, -5, -5, -10},
    { -5,  5, 10, 10, 10, 10, 10, 10,  5,  -5},
    { -5, 10, 15, 15, 15, 15, 15, 15, 10,  -5},
    { -5, 10, 15, 20, 20, 20, 20, 15, 10,  -5},
    { -5, 10, 15, 20, 20, 20, 20, 15, 10,  -5},
    { -5, 10, 15, 15, 15, 15, 15, 15, 10,  -5},
    { -5,  5, 10, 10, 10, 10, 10, 10,  5,  -5},
    {-10, -5, -5, -5, -5, -5, -5, -5, -5, -10}
};

/*PST Table for end game phase*/
static const int PST_ANT_END[8][10] = {
    {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, 
    {  5,   5,   5,   5,   5,   5,   5,   5,   5,   5},
    { 10,  10,  15,  15,  15,  15,  15,  15,  10,  10},
    { 15,  15,  20,  25,  25,  25,  25,  20,  15,  15},
    { 20,  20,  25,  30,  35,  35,  30,  25,  20,  20},
    { 30,  30,  35,  40,  45,  45,  40,  35,  30,  30},
    { 50,  50,  55,  60,  65,  65,  60,  55,  50,  50},
    {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0}  
};
static const int PST_ROOK_END[8][10] = {
    { 10,  10,  10,  10,  10,  10,  10,  10,  10,  10},
    { 15,  15,  15,  15,  15,  15,  15,  15,  15,  15},  
    {  5,   5,   5,   5,   5,   5,   5,   5,   5,   5},
    {  5,   5,   5,   5,   5,   5,   5,   5,   5,   5},
    {  5,   5,   5,   5,   5,   5,   5,   5,   5,   5},
    {  5,   5,   5,   5,   5,   5,   5,   5,   5,   5},
    {  5,   5,   5,   5,   5,   5,   5,   5,   5,   5},
    {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0}  
};
static const int PST_KNIGHT_END[8][10] = {
    {-40, -30, -20, -20, -20, -20, -20, -20, -30, -40},
    {-30, -10,   5,   5,   5,   5,   5,   5, -10, -30},
    {-20,   5,  15,  20,  20,  20,  20,  15,   5, -20},
    {-20,   5,  20,  25,  30,  30,  25,  20,   5, -20},
    {-20,   5,  20,  25,  30,  30,  25,  20,   5, -20},
    {-20,   5,  15,  20,  20,  20,  20,  15,   5, -20},
    {-30, -10,   5,   5,   5,   5,   5,   5, -10, -30},
    {-40, -30, -20, -20, -20, -20, -20, -20, -30, -40}
};
static const int PST_QUEEN_END[8][10] = {
    {-15, -10,  -5,  -5,  -5,  -5,  -5,  -5, -10, -15},
    {-10,   5,   5,   5,   5,   5,   5,   5,   5, -10},
    { -5,   5,  10,  15,  15,  15,  15,  10,   5,  -5},
    { -5,   5,  15,  20,  20,  20,  20,  15,   5,  -5},
    { -5,   5,  15,  20,  20,  20,  20,  15,   5,  -5},
    { -5,   5,  10,  15,  15,  15,  15,  10,   5,  -5},
    {-10,   5,   5,   5,   5,   5,   5,   5,   5, -10},
    {-15, -10,  -5,  -5,  -5,  -5,  -5,  -5, -10, -15}
};
static const int PST_KING_END[8][10] = {
    {-30, -20, -10, -10, -10, -10, -10, -10, -20, -30},
    {-20,  -5,   5,  10,  10,  10,  10,   5,  -5, -20},
    {-10,   5,  15,  20,  20,  20,  20,  15,   5, -10},
    {-10,   5,  20,  25,  30,  30,  25,  20,   5, -10},
    {-10,   5,  20,  25,  30,  30,  25,  20,   5, -10},
    {-10,   5,  15,  20,  20,  20,  20,  15,   5, -10},
    {-20,  -5,   5,  10,  10,  10,  10,   5,  -5, -20},
    {-30, -20, -10, -10, -10, -10, -10, -10, -20, -30}
};
static const int PST_ANTEATER_END[8][10] = {
    {-15, -10,  -5,  -5,  -5,  -5,  -5,  -5, -10, -15},
    {-10,   5,  10,  10,  10,  10,  10,  10,   5, -10},
    { -5,  10,  15,  15,  15,  15,  15,  15,  10,  -5},
    { -5,  10,  15,  25,  25,  25,  25,  15,  10,  -5},
    { -5,  10,  15,  25,  25,  25,  25,  15,  10,  -5},
    { -5,  10,  15,  15,  15,  15,  15,  15,  10,  -5},
    {-10,   5,  10,  10,  10,  10,  10,  10,   5, -10},
    {-15, -10,  -5,  -5,  -5,  -5,  -5,  -5, -10, -15}
};
static const int PST_BISHOP_END[8][10] = {
    {-15, -10,  -5,  -5,  -5,  -5,  -5,  -5, -10, -15},
    {-10,   5,   5,   5,   5,   5,   5,   5,   5, -10},
    { -5,   5,  10,  15,  15,  15,  15,  10,   5,  -5},
    { -5,   5,  15,  20,  20,  20,  20,  15,   5,  -5},
    { -5,   5,  15,  20,  20,  20,  20,  15,   5,  -5},
    { -5,   5,  10,  15,  15,  15,  15,  10,   5,  -5},
    {-10,   5,   5,   5,   5,   5,   5,   5,   5, -10},
    {-15, -10,  -5,  -5,  -5,  -5,  -5,  -5, -10, -15}
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

/*FUNCTION DECLARATION*/
// Master evalution function - called by negamax
int evaluate(GameState *gs);
//counts the raw piece values on the board
int evalMaterial(GameState *gs);
//adds positional bonuses from Piece-Square Tables
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

/*BETA: Game phase*/
int getGamePhase(GameState *gs);
/* blends opening and endgame PST values based on game phase */
int taperedPST(GameState *gs, int phase);

#endif