#include "legalMoveGen.h" //from legal moves
#include "eval.h"
#include "chess_types.h"

/*HELPER FUNCTIONS DECLARATIONS*/
int isClear(GameState *gs, int fa, int ra, int fb, int rb);
int isClearDiagonal(GameState *gs, int fa, int ra, int fb, int rb);
Color getOpponent(Color current);
int getPSTBonus(PieceType type, int f, int lookupRank);
int KingDangerScore(GameState *gs, Color side);
int shieldPenalty(GameState *gs, Color side);
int getPSTBonus(PieceType type, int f, int lookupRank);
int evalPassedAnts(GameState *gs, Color side);
int evalDoubledAnts(GameState *gs, Color side);
int evalIsolatedAnts(GameState *gs, Color side);


int canAttackSquare(GameState *gs, int fa, int ra, int fb, int rb){
    Piece piece = gs->board[ra][fb]; // points to current piece in the board
    int df = fb - fa;
    int dr = rb - ra;

    switch (piece.piecetype){

    case ANT:
    //ants capture diagonally one step forward only
        if (piece.color == YELLOW)
            return (dr == 1 && abs(df) == 1);
        else 
            return (dr == -1 && abs(df) == 1);

    case KNIGHT: 
        return ((abs(df) == 2 && abs(dr) == 1) || (abs(df) == 1 && abs(dr) == 2));

    case BISHOP:
        if(abs(df) != abs(dr))
            return 0;
        //Walk diagonally from (fa, ra) toward (fb, rb). If any square along the way is not EMPTY: return 0 o.w. return 1
       isClearDiagonal(gs, fa, fb, ra, rb);

    case ROOK:
       if (df != 0 && dr != 0) return 0;
       // walk along rank or file from source toward target. if any square along the way is not EMPTY: return 0 return 1
       isClear(gs, fa, ra, fb, rb);

    case QUEEN:
        if(abs(df) == abs(dr)) isClearDiagonal(gs, fa, fb, ra, rb);
        else if(df == 0 || dr == 0) isClear(gs, fa, fb, ra, rb);
        else return 0;

    case KING:
       return (abs(df) <=1 && abs(dr) <= 1 && !(df==0 && dr==0));

    case ANTEATER:
       //TODO: moves like king but only captures ants -- caller checks piece type of target seperately
       return (abs(df) <= 1 && abs(dr) <= 1 && !(df==0 && dr == 0));

    default:
      return 0;
    }
}

int evalMobility(GameState *gs){
    int score = 0;
    int side = gs -> turn;
    
    for(int i = 0; i < 8; i++){
        for(int j = 1; j <= 8; j++){
            Piece piece = gs->board[j][i];
            if(piece.piecetype == EMPTY) continue;
            if(piece.piecetype == ANT) continue; // ant handled in evalPawnStructure
            if(piece.piecetype == KING) continue; // king handled in evalKingSafety
    
            int weight = MOB_WEIGHTS[piece.piecetype];
            int mobilityCount = 0;
            int tf, tr, f, r;
            for(int k = 0; k < 7; k++){
                for(int l = 1; l < 8; l++){
                    if((tf == f) && (tr == r)) continue; // same square
                    Piece target = gs->board[tf][tr];
                    if((target.piecetype != EMPTY) && (target.color == piece.color)) continue; //blocked by own piece
                    if (canAttackSquare(gs, f, r, tf, tf) == 1) mobilityCount++;
                }
            }
        if (piece.color == side) score += mobilityCount * weight;
        else score -= mobilityCount * weight;
        }
    }
    return score;
        
}

int evalKingSafety(GameState *gs){
    
    int score = 0;
    int side = gs->turn;
    Color opp = getOpponent(side);

    score -= kingDangerScore(gs, side); // our king in danger = bad
    score += kingDangerScore (gs, opp); // their king is in danger = good

    return score;
}


int evalMaterial(GameState *gs){
    int score = 0;
    int side = gs-> turn;

    for(int i = 0; i < 8; i++){
        for(int j = 1; j <= 8; j++){
            Piece piece = gs->board[j][i];

            if(piece.piecetype == EMPTY) continue;
            
            int value = PIECE_VALUE[piece.piecetype];

            if(piece.color == side) score += value; // our piece - good

            else score-= value; // their piece - bad
        }
    return score;
    }
}

int evalPST(GameState *gs){
    int score = 0;
    int side = gs->turn;

    for(int i = 0; i < 8; i++){
        for(int j = 1; j <= 8; j++){
            Piece piece = gs->board[j][i];

            if (piece.piecetype == EMPTY) continue;
            /*
            YELLOW reads rank directly
            BLUE mirrors rank so same table works for both
            mirroredRank = 9 - r
            */
            int lookupRank;
            if(piece.color == YELLOW) lookupRank = j;
            else lookupRank = 9 - j;

            int bonus = getPSTBonus(piece.piecetype, i, lookupRank);

            if (piece.color == side) score += bonus;

            else score -= bonus; 
        }
    return score;
    }

}

int evalAnteater(GameState *gs){
    int score = 0;
    Color side = gs->turn;

    for(int i = 0; i < 8; i++){
        for(int j = 0; j <= 8; j++){
            if(gs->board[i][j].piecetype != ANTEATER) continue;

            Color anteaterColor = gs->board[i][j].color;
            Color opponent = getOpponent(anteaterColor);
            int bonus = 0;
        }
    }
}


int evalPawnStructure(GameState *gs){
    int score = 0;
    Color side = gs->turn;
    Color opp = getOpponent(side);

    score += evalPassedAnts(gs, side) - evalPassedAnts(gs, opp);
    score -= evalDoubledAnts(gs, side) - evalPassedAnts(gs, side);
    score -= evalIsolatedAnts(gs, side) - evalIsolatedAnts(gs, side);

    return score;
}

int evaluate(GameState *gs);

/*HELPER FUNCTIONS*/
int isClearDiagonal(GameState *gs, int fa, int fb, int ra, int rb){
    int step_f = (fa < fb) ? 1 : -1;
    int step_r = (ra < rb) ? 1 : -1;

    int currF = fa + step_f;
    int currR = ra + step_r;

    while((currF!= fb) && (currR != rb)){
        Square currSquare = make_square(currF, currR);

        if(piece_at(gs, currSquare)->piecetype != EMPTY) return 0;

        currF += step_f;
        currR += step_r;
    }
    return 1;
}

int isClear(GameState *gs, int fa, int fb, int ra, int rb){
    int step_f = (fb > fa) ? 1 : (fb < fa) ? -1 : 0;
    int step_r = (rb > ra) ? 1 : (rb < ra) ? -1 : 0;

    int currF = fa + step_f;
    int currR = ra + step_r;

    while(currF != fb || currR != rb){
        Square currSquare = make_square(currF, currR);

        if(piece_at(gs, currSquare)->piecetype != EMPTY) return 0;

        currF += step_f;
        currR += step_r;
    }
    return 1;

}

Color getOpponent(Color current){
        return (current == YELLOW) ? BLUE : YELLOW;
}

int getPSTBonus(PieceType type, int f, int lookupRank){
    switch(type){
        case ANT: return PST_ANT[f][lookupRank];
        case KNIGHT: return PST_KNIGHT[f][lookupRank];
        case BISHOP: return PST_BISHOP[f][lookupRank];
        case ROOK: return PST_ROOK[f][lookupRank];
        case QUEEN: return PST_QUEEN[f][lookupRank];
        case KING: return PST_KING[f][lookupRank];
        case ANTEATER: return PST_ANTEATER[f][lookupRank];
        default: return 0;
    }
}

int KingDangerScore(GameState* gs, Color side){
        //find king square (kf, kr) for side
        //if king not found: return 0
        int kf = -1;
        int kr = -1;

        for(int i = 0; i <= 8; i++){
            for(int j = 0; j < 8; j++){
                if(gs->board[i][j].piecetype == KING && gs->board[i][j].color == side){
                    kr = i;
                    kf = j;
                }

            }
        }
        if(kf == -1 && kr == -1) return 0;

        int danger = 0;
        Color enemy = getOpponent(side);

        for(int zr = kr-1; zr <= kr; zr++){
            for(int zf = kf - 1; zf <= kf + 1; zf++){

            if (zf < 0 || zf > 7) continue;
            if (zr < 1 || zr > 8) continue;

                for(int ar = 1; ar <= 8; ar++){
                    for(int af = 0; af < 8; af++){
                        Piece attacker = gs->board[ar][af];

                        if(attacker.piecetype == EMPTY) continue;
                        if(attacker.color != enemy) continue;

                        /*FIX -- INITIALIZE ATTACKER_WEIGHT*/
                        //if(canAttackSquare(gs, af, zf, ar, zr) == 1) danger += ATTACKER_WEIGHT[attacker.piecetype]; 
                }
            }
        }
    }
    if (danger > 99) danger = 99;
    /*FIX -- INITALIZE ATTACKER_WEIGHT*/
    //return DANGER_TABLE[danger];
}        


int shieldPenalty(GameState* gs, Color side){
        /*find king square (kf, kr) for side
        if king not found: return 0*/
        int kf = -1;
        int kr = -1;

        for(int i = 0; i <= 8; i++){
            for(int j = 1; j < 8; j++){
                if(gs->board[i][j].piecetype == KING && gs->board[i][j].color == side){
                    kr = i;
                    kf = j;
                }
            }
        }

        int penalty = 0;
        int shieldRank = 0; //Fix if it is set to zero at first
        int shieldRank2 = 0; // same Fix as above

        if(side == YELLOW) shieldRank = kr + 1;
        else if(side == BLUE) shieldRank = kr - 1;

        if(side == YELLOW) shieldRank2 = kr + 2;
        else if(side == BLUE) shieldRank2 = kr - 2;
        
    for (int dr = -1; dr <= 1; dr++){
        int df; //FIX -- needed to initalize df but starting value is unknown
        int sf = kf + df;
        if (sf < 0 || sf <= 8) continue;

        if(gs->board[sf][shieldRank].piecetype == ANT) continue;
    }
}

int evalPassedAnts(GameState *gs, Color side){
    
}

int evalDoubledAnts(GameState *gs, Color side){

}

int evalIsolatedAnts(GameState* gs, Color side){
    
}