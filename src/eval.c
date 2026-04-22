#include "eval.h"
#include "chess_types.h"
#include <stdlib.h>
//#include "stdio.h" for testing & debugging

/*HELPER FUNCTIONS DECLARATIONS*/
int isClear(GameState *gs, int fa, int ra, int fb, int rb);
int isClearDiagonal(GameState *gs, int fa, int ra, int fb, int rb);
Color getOpponent(Color current);
int KingDangerScore(GameState *gs, Color side);
int shieldPenalty(GameState *gs, Color side);
int evalPassedAnts(GameState *gs, Color side);
int evalDoubledAnts(GameState *gs, Color side);
int evalIsolatedAnts(GameState *gs, Color side);
int evalTempo(GameState *gs);


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
            if(piece.piecetype == KING) continue; // king handled in evalKingSafety
    
            int weight = MOB_WEIGHTS[piece.piecetype];
            int mobilityCount = 0;
            int df, dr;

            for(int tr = 0; tr < 8; tr++){
                for(int tf = 0; tf < 10; tf++){
                    if(tf == f && tr == r) continue; // same square

                    df = tf - f;
                    dr = tr -r;

                    int possible = 1;
                    switch(piece.piecetype){
                        case KNIGHT:
                            possible = ((abs(df)==2 && abs(dr)==1) ||
                                        (abs(df)==1 && abs(dr)==2));
                            break;
                        case BISHOP:
                            possible = (abs(df) == abs(dr));
                            break;
                        case ROOK:
                            possible = (df == 0 || dr == 0);   
                            break;
                        case QUEEN:
                            possible = (abs(df)==abs(dr) || df==0 || dr==0);
                            break;
                        case ANTEATER:
                            possible = (abs(df) <= 1 && abs(dr) <= 1);  
                            break;
                        default:
                            possible = 1;
                            break;
                    }
                    if(!possible) continue;

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
    Color side = gs->turn;
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

    int sideCount[10] = {0};
    int oppCount[10] = {0};

    /*Track passed ants in same pass*/
    for(int f = 0; f < 10; f++){
        for(int r = 0; r < 8; r++){
            Piece p = gs->board[r][f];
            if(p.piecetype != ANT) continue;
            if(p.color == side) sideCount[f]++;
            else oppCount[f]++;
        }
    }

    /*doubld ants - one loop over files*/
    for(int f = 0; f < 10; f++){
        if(sideCount[f] > 1)
            score -= (sideCount[f] - 1) * DOUBLED_ANT_PENALTY;
        if(oppCount[f] > 1)
            score += (oppCount[f] - 1) * DOUBLED_ANT_PENALTY;
    }

    /*isolated ants - one loop over files*/
    for(int f = 0; f < 10; f++){
    if(sideCount[f] > 0){
        int hasNeighbour = (f > 0 && sideCount[f-1] > 0) ||
                           (f < 9 && sideCount[f+1] > 0);
        if(!hasNeighbour)
            score -= sideCount[f] * ISOLATED_ANT_PENALTY;
    }
    if(oppCount[f] > 0){              /* separate block — correct */
        int hasNeighbour = (f > 0 && oppCount[f-1] > 0) ||
                           (f < 9 && oppCount[f+1] > 0);
        if(!hasNeighbour)
            score += oppCount[f] * ISOLATED_ANT_PENALTY;
    }
}                                     /* closes for loop */

score += evalPassedAnts(gs, side) - evalPassedAnts(gs, opp);
return score;
}

int evalKingTropism(GameState *gs){
    int score = 0;
    Color side = gs->turn;

    int sideKf = -1;
    int sideKr = -1;
    int oppKf = -1;
    int oppKr = -1;

    for(int r = 0; r < 8; r++){
        for(int f = 0; f < 10; f++){
            Piece p = gs->board[r][f];
            if(p.piecetype != KING) continue;
            if (p.color == side) {sideKf = f; sideKr = r;}
            else {oppKf = f, oppKr = r;}            
        }
    }

    if (oppKf == -1 || sideKf == -1) return 0;

    for(int r = 0; r < 8; r++){
        for(int f = 0; f < 10; f++){
            Piece p = gs->board[r][f];
            if(p.piecetype == EMPTY) continue;
            if(p.piecetype == ANT) continue;
            if(p.piecetype == KING) continue;

            int targetKf = (p.color == side) ? oppKf  : sideKf;
            int targetKr = (p.color == side) ? oppKr  : sideKr;

            int dist = (abs(f - targetKf) > abs(r - targetKr))
            ? abs(f - targetKf)
            : abs(r - targetKr);
            int tropismBonus = 10 - dist;
            int weight; 
            switch(p.piecetype){
                case QUEEN : weight = 3; break;
                case ROOK: weight = 2; break;
                case BISHOP: weight = 1; break;
                case KNIGHT: weight = 1; break;
                case ANTEATER : weight = 1; break;
                default: weight = 0; break;
            }

            int bonus = tropismBonus * weight;
            if(p.color == side) score += bonus;
            else score -= bonus;
        }
    }
    return score;
}

int evalTempo(GameState *gs){
    int score = 0; 
    Color side = gs->turn;

    for(int r = 0; r < 8; r++){
        for(int f = 0; f < 10; f++){
            Piece p = gs->board[r][f];
            if(p.piecetype == KING) continue;
            if(p.piecetype == EMPTY) continue;
            if(p.piecetype == ANT) continue;

            int inEnemyHalf = 0;

            if(p.color == YELLOW && r >= 4) inEnemyHalf = 1;
            if(p.color == BLUE && r <= 3) inEnemyHalf = 1;

            if(inEnemyHalf){
                if(p.color == side) score += 5;
                else score -= 5;
            }
        }
    }
    return score;
}
int evalKingEscape(GameState *gs){
    int score = 0;
    Color side = gs->turn;

    int sideEscape = 0;
    int oppEscape = 0;


    for(int r = 0; r < 8; r++){
        for(int f = 0; f < 10; f++){
            Piece p = gs->board[r][f];
            if(p.piecetype != KING) continue;

            int escapes = 0;

            for(int dr = -1; dr <= 1; dr++){
                for(int df = -1; df <= 1; df++){
                    if(dr == 0 && df == 0) continue;

                    int nr = r + dr;
                    int nf = f + df;

                    if(nr < 0 || nr > 7) continue;
                    if(nf <0 || nf > 9) continue;

                    Piece target = gs -> board[nr][nf];

                    if(target.piecetype != EMPTY && target.color == p.color) continue;

                    escapes++;
                }
            }
            if(p.color == side) sideEscape = escapes;
            else oppEscape = escapes;
        }
    }
    score += (8 - oppEscape) * 10;
    score -= (8 - sideEscape) * 10;

    return score;
}

int evalBackRank(GameState *gs){
    int score = 0;
    Color side = gs->turn;

    int yellowHomeRank = 0;
    int blueHomeRank = 7;

    for(int r = 0; r < 8; r++){
        for(int f = 0; f < 10; f++){
            Piece p = gs->board[r][f];
            if(p.piecetype != KING) continue;

            int homeRank = (p.color == YELLOW) ? yellowHomeRank : blueHomeRank;
            int forwardRank = (p.color == YELLOW) ? r + 1 : r - 1;

            if(r != homeRank) continue;

            int blocked = 0;
            for(int df = -1; df <= 1; df++){
                int nf = f + df;
                if(nf < 0 || nf > 9) continue;
                if(forwardRank < 0 || forwardRank > 7) continue;

                Piece fwd = gs->board[forwardRank][nf];
                if(fwd.piecetype != EMPTY && fwd.color == p.color) blocked++;
            }
            int penalty = blocked * 8;
            if(p.color == side) score -= penalty;
            else score += penalty;
    }
}
return score;
}

int getGamePhase(GameState *gs){
    int phase = 0;
    for(int r = 0; r < 8; r++){
        for(int f = 0; f < 10; f++){
            Piece p = gs->board[r][f];
            switch(p.piecetype){
                case KNIGHT: phase += PHASE_KNIGHT; break;
                case ROOK: phase += PHASE_ROOK; break;
                case QUEEN: phase += PHASE_QUEEN; break;
                case BISHOP: phase += PHASE_BISHOP; break;
                default: break;
            }
        }
    }
    if (phase > PHASE_MAX) phase = PHASE_MAX;
    return phase;
}
/*
BLENDS OPENING AND ENG GAME PST VALUES BASED ON PHASE
PHASE = PHASE_MAX -> FULL OPENING WEIGHT
PHASE = 0 -> FULL ENDGAME WEIGHT
PHASE = INBETWEEN -> LINEAR BLEND
FORMULA: SCORE = (OPENSCORE * PHASE + ENDSCORE * (PHASE_MAX - PHASE)) / PHASE_MAX
*/
int taperedPST(GameState *gs, int phase){
    int score = 0;
    Color side = gs->turn;

    for(int r = 0; r < 8; r++){
        for(int f = 0; f < 10; f++){
            Piece p = gs->board[r][f];
            if(p.piecetype == EMPTY) continue;

            int lookupRank = (p.color == YELLOW) ? r : (7-r);

            int openScore = 0;
            int endScore = 0;

            switch(p.piecetype){
                case ANT: 
                openScore = PST_ANT_OPENING[lookupRank][f]; 
                endScore = PST_ANT_END[lookupRank][f];
                break;
                case KNIGHT:
                openScore = PST_KNIGHT_OPENING[lookupRank][f];
                endScore = PST_KNIGHT_END[lookupRank][f];
                break;
                case BISHOP:
                openScore = PST_BISHOP_OPENING[lookupRank][f];
                endScore = PST_BISHOP_END[lookupRank][f];
                break;
                case QUEEN: 
                openScore = PST_QUEEN_OPENING[lookupRank][f];
                endScore = PST_QUEEN_END[lookupRank][f];
                break;
                case KING:
                openScore = PST_KING_OPENING[lookupRank][f];
                endScore = PST_KING_END[lookupRank][f];
                break;
                case ROOK:
                openScore = PST_ROOK_OPENING[lookupRank][f];
                endScore = PST_ROOK_END[lookupRank][f];
                break;
                case ANTEATER:
                openScore = PST_ANTEATER_OPENING[lookupRank][f];
                endScore = PST_ANTEATER_END[lookupRank][f];
                break;
                default:
                break;
            }
            int blended = ((openScore * phase) + (endScore * (PHASE_MAX - phase))) / PHASE_MAX;

            if(p.color == side) score += blended;
            else score -= blended;
        }
    }
    return score;
}

int evaluate(GameState *gs){
    int score = 0;
    int phase = getGamePhase(gs);

    int material = evalMaterial(gs);
    //printf("Material: %d\n", material); fflush(stdout);

    int pst = taperedPST(gs, phase);

    int mobility = evalMobility(gs);
    //printf("Mobility: %d\n", mobility); fflush(stdout);

    int king = evalKingSafety(gs);
    //printf("KingSafety: %d\n", king); fflush(stdout);

    int pawn = evalPawnStructure(gs);
    //printf("PawnStruct: %d\n", pawn); fflush(stdout);

    int anteater = evalAnteater(gs);
    //printf("Anteater: %d\n", anteater); fflush(stdout);

    int kingTropism = evalKingTropism(gs);
    //printf("King Tropism: %d\n", kingTropism); flush(stdout);

    int kingEscape = evalKingEscape(gs);
    //printf("King Escape: %d\n", kingEscape); flush(stdout);

    int backRank = evalBackRank(gs);
    //printf("King Back Rank %d\n", backRank); flush(stdout);

    int tempo = evalTempo(gs);
    //print("Tempo: %d\n", tempo); flush(stdout);


    score = material + pst + mobility + king + pawn + anteater + kingTropism + kingEscape + backRank + tempo;
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


int KingDangerScore(GameState* gs, Color side){
        Square king_square;
        int kf;
        int kr;

        if (!gs->cache_valid) {
            refresh_piece_cache(gs);
        }

        king_square = (side == YELLOW) ? gs->yellow_king_square : gs->blue_king_square;
        kr = king_square.rank;
        kf = king_square.file;
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

                        /*Everything below is just for time optimization*/
                        int df, dr;

                        df = zf - af;
                        dr = zr - ar;

                        int possible = 1;
                        switch(attacker.piecetype){
                            case KNIGHT:
                            possible = (abs(df)==2 || abs(dr)==1) && (abs(df)==1 || abs(dr)==2);
                            break;

                            case BISHOP:
                            possible = (abs(df) == abs(dr));
                            break;

                            case ROOK:
                            possible = (df == 0 || dr == 0);
                            break;

                            case QUEEN:
                            possible = (abs(df) == abs(dr) || df == 0 || dr == 0);
                            break;

                            case ANT:
                            possible = (attacker.color == YELLOW) ? (dr == 1 && abs(df) == 1) : (dr == -1 && abs(df) == 1);
                            break;

                            case KING:
                            possible = (abs(df) <= 1 && abs(dr) <= 1);
                            break;

                            case ANTEATER:
                            //not a threat to king
                            possible = 0;
                            break;

                            default:
                            possible = 0;
                        }
                        if(!possible) continue;
                        if(canAttackSquare(gs, af, ar, zf, zr) == 1) danger += ATTACKER_WEIGHT[attacker.piecetype]; 
                }
            }
        }
    }
    if (danger > 99) danger = 99;
    return DANGER_TABLE[danger];
}        



int shieldPenalty(GameState *gs, Color side){
    Square king_square;
    int kf;
    int kr;

    if (!gs->cache_valid) {
        refresh_piece_cache(gs);
    }

    king_square = (side == YELLOW) ? gs->yellow_king_square : gs->blue_king_square;
    kf = king_square.file;
    kr = king_square.rank;
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
        else if(shieldRank2 >= 0 && shieldRank2 <= 7 &&
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
    for(int f = 0; f < 10; f++){
        int count = 0;
        for(int r = 0; r < 8; r++){
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