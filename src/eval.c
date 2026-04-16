#include "eval.h"
#include "chess_types.h"
//#include "stdlib.h" for testing & debugging
//#include "stdio.h" for testing & debugging

/*HELPER FUNCTIONS DECLARATIONS*/
int isClear(GameState *gs, int fa, int ra, int fb, int rb);
int isClearDiagonal(GameState *gs, int fa, int ra, int fb, int rb);
Color getOpponent(Color current);
int getPSTBonus(PieceType type, int f, int lookupRank);
int KingDangerScore(GameState *gs, Color side);
int shieldPenalty(GameState *gs, Color side);
int evalPassedAnts(GameState *gs, Color side);
int evalDoubledAnts(GameState *gs, Color side);
int evalIsolatedAnts(GameState *gs, Color side);


int canAttackSquare(GameState *gs, int fa, int ra, int fb, int rb){
    Piece piece = gs->board[ra][fa]; // points to current piece in the board
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
       return isClearDiagonal(gs, fa, ra, fb, rb);

    case ROOK:
       if (df != 0 && dr != 0) return 0;
       // walk along rank or file from source toward target. if any square along the way is not EMPTY: return 0 return 1
        return isClear(gs, fa, ra, fb, rb);

    case QUEEN:
        if(abs(df) == abs(dr)) return isClearDiagonal(gs, fa, ra, fb, rb);
        else if(df == 0 || dr == 0) return isClear(gs, fa, ra, fb, rb);
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
    Color side = gs -> turn;
    
    for(int r = 0; r < 8; r++){
        for(int f = 0; f < 10; f++){
            Piece piece = gs->board[r][f];
            if(piece.piecetype == EMPTY) continue;
            if(piece.piecetype == ANT) continue; // ant handled in evalPawnStructure
            //if(piece.piecetype == KING) continue; // king handled in evalKingSafety
    
            int weight = MOB_WEIGHTS[piece.piecetype];
            int mobilityCount = 0;

            for(int tr = 0; tr < 8; tr++){
                for(int tf = 0; tf < 10; tf++){
                    if((tf == f) && (tr == r)) continue; // same square
                    Piece target = gs->board[tr][tf];
                    if((target.piecetype != EMPTY) && (target.color == piece.color)) continue; //blocked by own piece
                    if (canAttackSquare(gs, f, r, tf, tr)) mobilityCount++;
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

    score -= KingDangerScore(gs, side); // our king in danger = bad
    score += KingDangerScore (gs, opp); // their king is in danger = good
    score -= shieldPenalty(gs, side);
    score += shieldPenalty(gs, opp);

    return score;
}


int evalMaterial(GameState *gs){
    int score = 0;
    Color side = gs-> turn;

    for(int r = 0; r < 8; r++){
        for(int f = 0; f < 10; f++){
            Piece piece = gs->board[r][f];

            if(piece.piecetype == EMPTY) continue;
            
            int value = PIECE_VALUE[piece.piecetype];

            if(piece.color == side) score += value; // our piece - good

            else score-= value; // their piece - bad
        }
    }
 return score;
}

int evalPST(GameState *gs){
    int score = 0;
    Color side = gs->turn;

    for(int r = 0; r < 8; r++){
        for(int f = 0; f < 10; f++){
            Piece piece = gs->board[r][f];

            if (piece.piecetype == EMPTY) continue;
            /*
            YELLOW reads rank directly
            BLUE mirrors rank so same table works for both
            mirroredRank = 9 - r
            */
            int lookupRank;
            if(piece.color == YELLOW) lookupRank = r;
            else lookupRank = 7 - r;

            int bonus = getPSTBonus(piece.piecetype, f, lookupRank);

            if (piece.color == side) score += bonus;

            else score -= bonus; 
        }
    }
 return score;
}

int evalAnteater(GameState *gs){

    int score = 0;
    Color side = gs->turn;

    // 4 orthogonal directions
    int directions[4][2] = {
        {1, 0},   // right
        {-1, 0},  // left
        {0, 1},   // up
        {0, -1}   // down
    };

    for(int r = 0; r < 8; r++){
        for(int f = 0; f < 10; f++){

            Piece piece = gs->board[r][f];
            if(piece.piecetype != ANTEATER) continue;

            Color myColor = piece.color;
            Color enemy = getOpponent(myColor);

            int bonus = 0;

            // check each direction
            for(int d = 0; d < 4; d++){
                int df = directions[d][0];
                int dr = directions[d][1];

                int nf = f + df;
                int nr = r + dr;

                // first square MUST be enemy ant
                if(nf < 0 || nf > 9 || nr < 0 || nr > 7) continue;

                Piece first = gs->board[nr][nf];
                if(first.piecetype != ANT || first.color != enemy) continue;

                // start chain
                int length = 0;

                while(nf >= 0 && nf <= 9 && nr >= 0 && nr <= 7){
                    Piece curr = gs->board[nr][nf];

                    if(curr.piecetype == ANT && curr.color == enemy){
                        length++;
                        nf += df;
                        nr += dr;
                    } else {
                        break;
                    }
                }

                // reward chain (scale it)
                bonus += length * length * ANTEATER_ADJ_BONUS;
                // quadratic scaling rewards longer chains more
            }

            if(myColor == side) score += bonus;
            else score -= bonus;
        }
    }

    return score;
}


int evalPawnStructure(GameState *gs){
    int score = 0;
    Color side = gs->turn;
    Color opp = getOpponent(side);

    score += evalPassedAnts(gs, side) - evalPassedAnts(gs, opp);
    score -= evalDoubledAnts(gs, side) - evalDoubledAnts(gs, opp);
    score -= evalIsolatedAnts(gs, side) - evalIsolatedAnts(gs, opp);

    return score;
}

int evaluate(GameState *gs){
    int score = 0;

    int material = evalMaterial(gs);
    //printf("Material: %d\n", material); fflush(stdout);

    int pst = evalPST(gs);
    //printf("PST: %d\n", pst); fflush(stdout);

    int mobility = evalMobility(gs);
    //printf("Mobility: %d\n", mobility); fflush(stdout);

    int king = evalKingSafety(gs);
    //printf("KingSafety: %d\n", king); fflush(stdout);

    int pawn = evalPawnStructure(gs);
    //printf("PawnStruct: %d\n", pawn); fflush(stdout);

    int anteater = evalAnteater(gs);
    //printf("Anteater: %d\n", anteater); fflush(stdout);

    score = material + pst + mobility + king + pawn + anteater;
    //printf("TOTAL: %d\n", score); fflush(stdout);

    return score;
}

/*HELPER FUNCTIONS*/
int isClearDiagonal(GameState *gs, int fa, int ra, int fb, int rb){
    int step_f = (fa < fb) ? 1 : -1;
    int step_r = (ra < rb) ? 1 : -1;

    int currF = fa + step_f;
    int currR = ra + step_r;

    while((currF!= fb) && (currR != rb)){
        if(gs->board[currR][currF].piecetype != EMPTY) return 0;

        currF += step_f;
        currR += step_r;
    }
    return 1;
}

int isClear(GameState *gs, int fa, int ra, int fb, int rb){
    int step_f = (fb > fa) ? 1 : (fb < fa) ? -1 : 0;
    int step_r = (rb > ra) ? 1 : (rb < ra) ? -1 : 0;

    int currF = fa + step_f;
    int currR = ra + step_r;

    while(currF != fb || currR != rb){

        if(gs->board[currR][currF].piecetype != EMPTY) return 0;

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
        case ANT: return PST_ANT[lookupRank][f];
        case KNIGHT: return PST_KNIGHT[lookupRank][f];
        case BISHOP: return PST_BISHOP[lookupRank][f];
        case ROOK: return PST_ROOK[lookupRank][f];
        case QUEEN: return PST_QUEEN[lookupRank][f];
        case KING: return PST_KING[lookupRank][f];
        case ANTEATER: return PST_ANTEATER[lookupRank][f];
        default: return 0;
    }
}

int KingDangerScore(GameState* gs, Color side){
        //find king square (kf, kr) for side
        //if king not found: return 0
        int kf = -1;
        int kr = -1;

        for(int r = 0; r < 8; r++){
            for(int f = 0; f < 10; f++){
                if(gs->board[r][f].piecetype == KING && gs->board[r][f].color == side){
                    kr = r;
                    kf = f;
                }

            }
        }
        if(kf == -1) return 0;

        int danger = 0;
        Color enemy = getOpponent(side);

        for(int zr = kr-1; zr <= kr + 1; zr++){
            for(int zf = kf - 1; zf <= kf + 1; zf++){

            if (zf < 0 || zf > 9) continue;
            if (zr < 0 || zr > 7) continue;
            if(zf == kf && zr == kr) continue;

                for(int ar = 0; ar < 8; ar++){
                    for(int af = 0; af < 10; af++){
                        Piece attacker = gs->board[ar][af];

                        if(attacker.piecetype == EMPTY) continue;
                        if(attacker.color != enemy) continue;

                        /*FIX -- INITIALIZE ATTACKER_WEIGHT*/
                        if(canAttackSquare(gs, af, ar, zf, zr) == 1) danger += ATTACKER_WEIGHT[attacker.piecetype]; 
                }
            }
        }
    }
    if (danger > 99) danger = 99;
    /*FIX -- DANGER_TABLE*/
    return DANGER_TABLE[danger];
}        



int shieldPenalty(GameState *gs, Color side){
    int kf = -1;
    int kr = -1;

    /* find the king */
    for(int r = 0; r < 8; r++){
        for(int f = 0; f < 10; f++){
            if(gs->board[r][f].piecetype == KING &&
               gs->board[r][f].color     == side){
                kf = f;
                kr = r;
            }
        }
    }
    if(kf == -1) return 0;

    int penalty = 0;

    /* shield rank is directly in front of king */
    int shieldRank  = (side == YELLOW) ? kr + 1 : kr - 1;
    int shieldRank2 = (side == YELLOW) ? kr + 2 : kr - 2;

    /* check three files: left of king, king file, right of king */
    for(int df = -1; df <= 1; df++){
        int sf = kf + df;
        if(sf < 0 || sf > 9) continue;

        /* check if shield rank is on the board */
        if(shieldRank >= 0 && shieldRank <= 7 &&
           gs->board[shieldRank][sf].piecetype == ANT &&
           gs->board[shieldRank][sf].color     == side){
            continue;  /* ant in place — no penalty */
        }
        /* check if ant is one step further back */
        else if(shieldRank2 >= 1 && shieldRank2 <= 8 &&
                gs->board[shieldRank2][sf].piecetype == ANT &&
                gs->board[shieldRank2][sf].color     == side){
            penalty += SHIELD_PUSHED;
        }
        else{
            penalty += SHIELD_MISSING;
        }
    }
    return penalty;
}

//no enemies ahead on same or adjacent files
int evalPassedAnts(GameState *gs, Color side){
    int score = 0;
    Color enemy = getOpponent(side);
    for(int r = 0; r < 8; r++){
        for(int f = 0; f < 10; f++){
            Piece piece = gs->board[r][f];
            if(piece.piecetype != ANT) continue;
            if(piece.color != side) continue;

            int passed = 1;
            
            int start = (side == YELLOW) ? r + 1 : r - 1;
            int end = (side == YELLOW) ? 7 : 0;
            int step = (side == YELLOW) ? 1 : -1;

            for(int rr = start; (side == YELLOW ? rr<= end : rr >= end); rr+=step){
                for(int df = -1; df<= 1; df++){
                    int nf = f + df;

                    if(nf < 0 || nf > 9) continue;

                    Piece check = gs->board[rr][nf];

                    if(check.piecetype == ANT && check.color == enemy){
                        passed = 0;
                        break;
                    }
                }
                if(!passed) break;
            }

            if(passed){
                int bonus = (side == YELLOW) ? r*10 : (9-r) * 10;
                score += bonus;
            }
        }
    }
    return score;
}

int evalDoubledAnts(GameState *gs, Color side){
    int score = 0;
    for(int r = 0; r < 8; r++){
        int count = 0;
        for(int f = 0; f < 10; f++){
            Piece piece = gs->board[r][f];

            if(piece.piecetype == ANT && piece.color == side) count ++;
        }
        if(count > 1) score += (count - 1) * DOUBLED_ANT_PENALTY;
    }
    return score;
}

int evalIsolatedAnts(GameState* gs, Color side){
    int score = 0;
    for(int r = 0; r < 8; r++){
        for(int f = 0; f < 10; f++){
            Piece piece = gs->board[r][f];
            
            if(piece.piecetype != ANT || piece.color != side) continue;

            int isolated = 1;

            for(int df = -1; df <= 1; df += 2){
                int nf = df + f;

                if(nf < 0 || nf > 9) continue;

                for(int rr = 0; rr < 8; rr++){
                    Piece neighbor = gs->board[rr][nf];

                    if(neighbor.piecetype == ANT && neighbor.color == side){
                        isolated = 0;
                        break;
                    }
                }
                if(!isolated) break;
            }
            if(isolated) score += ISOLATED_ANT_PENALTY;
        }
    }
    return score;
}