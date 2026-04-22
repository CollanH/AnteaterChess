#include "gui.h"
#include "legalMoveGen.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// window and board layout
#define WINDOW_WIDTH 1180
#define WINDOW_HEIGHT 760
#define SQUARE_SIZE 80
#define BOARD_X 30
#define BOARD_Y 60

// right-side info panel layout
#define PANEL_X   860
#define PANEL_Y    60
#define PANEL_W   280
#define PANEL_H   640

//move log panel 
#define LOG_X 1160
#define LOG_Y 60
#define LOG_W 280
#define LOG_H 640
#define LOG_MAX_ENTRIES 30 //how mnay moves to store and display 

// different screens used by the gui
typedef enum {
    SCREEN_MATCHUP,
    SCREEN_COLOR,
    SCREEN_CLOCK,
    SCREEN_DIFFICULTY,
    SCREEN_GAME,
    SCREEN_PROMOTION,
    SCREEN_ENDGAME
} Screen;

// core sdl objects
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static TTF_Font *font = NULL;
static TTF_Font *smallFont = NULL;

//stopping anteater chaing
static int stopChainBtnX =0; 
static int stopChainBtnY = 0; 
static int stopChainBtnW = 120; 
static int stopChainBtnH = 44; 
int stopChainPressed = 0; 


//textures
static SDL_Texture *pieceTextures[2][8]; 
static SDL_Texture *squareTextures[2]; 

//pawn promotion 
static PieceType promotionPiece = QUEEN; //which piece currently highlighted
static int promotionPending = 0; //set to 1 when pawn reaches back rank 

//stop anteater chain tbd

//forward declarations
static void renderGameScreen(int showClocks); 
static void renderEndScreen(const char *message); 
static void renderPromotionScreen(void); 

// current gui/game state
static Screen currentScreen = SCREEN_MATCHUP;
static GameState *currentGameState = NULL;
static Color humanColor = YELLOW;

// player clocks in seconds
static int yellowSecs = 300;
static int blueSecs = 300;

// mouse-based move input state
static int clickCount = 0;
static Square firstClick;
static Move pendingMove;
static int moveReady = 0;

// highlighted legal moves for the selected piece
static MoveList highlightMoves;
static int hasHighlight = 0;

// undo button state and clickable bounds
static int undoPressed = 0;
static int undoBtnX = 0;
static int undoBtnY = 0;
static int undoBtnW = 160;
static int undoBtnH = 50;

// status/error message shown in the right panel
static char statusMsg[128] = "";

//move log 
static char moveLog[LOG_MAX_ENTRIES][48]; 
static int logCount = 0; 

//picture file names
static const char *pieceFileNames[2][8] = {
    
    {"", "../board_images/yking.bmp", 
    "../board_images/yqueen.bmp", 
    "../board_images/yanteater.bmp", 
    "../board_images/ybishop.bmp", 
    "../board_images/yknight.bmp", 
    "../board_images/yrook.bmp", 
    "../board_images/yant.bmp" }, 

    {"", "../board_images/bking.bmp", 
    "../board_images/bqueen.bmp", 
    "../board_images/banteater.bmp", 
    "../board_images/bbishop.bmp", 
    "../board_images/bknight.bmp", 
    "../board_images/brook.bmp", 
    "../board_images/bant.bmp"}
};


//loading piece bmp and square bmp 
static void loadPieceTextures(void) {
    int color, pieceType; 
    SDL_Surface *surf; 

    for (color = 0; color<2; color++) 
        for (pieceType =0; pieceType < 8; pieceType++)
            pieceTextures[color][pieceType] = NULL; 
    squareTextures[0] = NULL;
    squareTextures[1] = NULL; 

    //loading piece bmp using sdl_loadbmp to convert to texture 
    for (color=0; color<2; color++) {
        for (pieceType=1; pieceType<8; pieceType++) {
            surf = SDL_LoadBMP(pieceFileNames[color][pieceType]); 
            if (surf) {
                SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format,255,0,255)); 
                pieceTextures[color][pieceType] = SDL_CreateTextureFromSurface(renderer,surf); 
                SDL_FreeSurface(surf); 

                if(!pieceTextures[color][pieceType]) {
                    fprintf(stderr, "Warning: texture failed for %s, %s\n", pieceFileNames[color][pieceType], SDL_GetError()); 
                }
            } else {
                fprintf(stderr, "Warning: could not load %s: %s\n", pieceFileNames[color][pieceType], SDL_GetError()); 
            }

        
        }
    }

    //board square bmps
    surf = SDL_LoadBMP("../board_images/ysquare.bmp"); 
    if(surf) {
        squareTextures[0] = SDL_CreateTextureFromSurface(renderer,surf); 
        SDL_FreeSurface(surf); 
    } else {
        fprintf(stderr, "Warning: could not load ysquare.bmp: %s\n", SDL_GetError()); 
    }

    surf = SDL_LoadBMP("../board_images/bsquare.bmp"); 
    if(surf) {
        squareTextures[1] = SDL_CreateTextureFromSurface(renderer, surf); 
        SDL_FreeSurface(surf); 
    
    } else {
        fprintf(stderr, "Warning: could not laod bsquare.bmp: %s\n", SDL_GetError()); 
    }
}

//releasing textures from memory 
static void freePieceTextures(void) {
    int color, pieceType; 
    for (color=0 ; color<2; color++){
        for (pieceType = 0; pieceType< 8; pieceType++) {
            if (pieceTextures[color][pieceType]) {
                SDL_DestroyTexture(pieceTextures[color][pieceType]); 
                pieceTextures[color][pieceType] = NULL; 
            }
        }
    }

    if (squareTextures[0]) {
        SDL_DestroyTexture(squareTextures[0]); 
        squareTextures[0] = NULL; 

    }
    if(squareTextures[1]) {
        SDL_DestroyTexture(squareTextures[1]); 
        squareTextures[1] = NULL; 
    }
}

//drawing board square at pixel position, if bmp load fails->falls back to og rectangle
static void drawSquare(int x, int y, int squareIndex) {
    SDL_Rect d; //where the rectangle will be drawn 
    d.x = x; 
    d.y = y; 
    d.w = SQUARE_SIZE; 
    d.h = SQUARE_SIZE; 

    if(squareTextures[squareIndex]) {
        SDL_RenderCopy(renderer, squareTextures[squareIndex], NULL, &d); 
    } else {
        if (squareIndex == 0) {
            SDL_SetRenderDrawColor(renderer, 245,230,140,255); 
        } else {
            SDL_SetRenderDrawColor(renderer, 70,120,190,255); 
        }

        SDL_RenderFillRect(renderer, &d);
    }
}



// draw a line of text using sdl_ttf
static void drawText(const char *text, int x, int y, SDL_Color color, TTF_Font *f)
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect dst;

    if (!text || !f) {
        return;
    }

    surface = TTF_RenderText_Blended(f, text, color);
    if (!surface) {
        return;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    dst.x = x;
    dst.y = y;
    dst.w = surface->w;
    dst.h = surface->h;

    SDL_RenderCopy(renderer, texture, NULL, &dst);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}
//drawing piece from bmp 
static void drawPiece(Piece p, int x, int y) {
    int colorIndex; 
    SDL_Texture *text; 
    SDL_Rect d; 

    if(p.piecetype == EMPTY) 
        return; 
    colorIndex = (p.color == YELLOW) ? 0:1; 
    text = pieceTextures[colorIndex][p.piecetype]; 

    if(text) {
        //drwaing to fit inside square
        d.x = x+4; 
        d.y = y+4; 
        d.w = SQUARE_SIZE -8; 
        d.h = SQUARE_SIZE -8; 
        SDL_RenderCopy(renderer,text,NULL, &d); 
    } else {
        //if does not draw inside square, then colored letter
        char txt[2]; 
        SDL_Color c; 

        switch (p.piecetype) {
            case KING: txt[0] = 'K'; break; 
            case QUEEN: txt[0] = 'Q'; break; 
            case BISHOP: txt[0] = 'B'; break; 
            case KNIGHT: txt[0] = 'N'; break; 
            case ROOK: txt[0] = 'R'; break; 
            case ANT: txt[0] = 'A'; break; 
            case ANTEATER: txt[0] = 'T'; break; 
            default: txt[0] = '?'; break; 
        }

        txt[1] = '\0';

        c = (p.color ==YELLOW) ? (SDL_Color) {245,210,60,255} : (SDL_Color) {70,140,255,255};

        drawText(txt, x+30, y+25, c, font); 
    }
}



// draw a filled rectangle
static void fillRect(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b)
{
    SDL_Rect rect;

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderFillRect(renderer, &rect);
}

// return 1 if a point is inside a rectangle
static int pointInRect(int x, int y, int rx, int ry, int rw, int rh)
{
    return (x >= rx && x <= rx + rw && y >= ry && y <= ry + rh);
}

// return the single-letter label for a piece
static char pieceLetter(Piece p)
{
    switch (p.piecetype) {
        case KING: return 'K';
        case QUEEN: return 'Q';
        case BISHOP: return 'B';
        case KNIGHT: return 'N';
        case ROOK: return 'R';
        case ANT: return 'A';
        case ANTEATER: return 'T';
        default: return '.';
    }
}

// draw one menu button
static void drawButton(int x, int y, int w, int h, const char *label, int selected)
{
    if (selected) {
        fillRect(x - 4, y - 4, w + 8, h + 8, 50, 150, 255);
        fillRect(x, y, w, h, 210, 230, 255);
    } else {
        fillRect(x, y, w, h, 210, 210, 210);
    }

    drawText(label, x + 20, y + 12, (SDL_Color){0, 0, 0, 255}, font);
}

// draw a generic menu screen
static void renderMenuScreen(const char *title, const char **options, int numOptions, int selected)
{
    int i;

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    fillRect(180, 120, 640, 500, 245, 245, 245);

    drawText("ANTEATER CHESS", 360, 160, (SDL_Color){0, 0, 0, 255}, font);
    drawText(title, 320, 220, (SDL_Color){0, 0, 0, 255}, font);

    for (i = 0; i < numOptions; i++) {
        drawButton(250, 300 + i * 80, 500, 55, options[i], selected == i);
    }

    SDL_RenderPresent(renderer);
}

//move Log 
void addMoveLog(Color color, Move move) {
    char entry[48]; 
    const char *colorStr = (color == YELLOW) ? "Yellow" : "Blue"; 
    char fromFile = (char)('A'+move.from.file); 
    char toFile   = (char)('A' + move.to.file);
    int  fromRank = move.from.rank + 1;
    int  toRank   = move.to.rank   + 1;
 
    snprintf(entry, sizeof(entry), "%s: %c%d -> %c%d",
             colorStr, fromFile, fromRank, toFile, toRank);
 
    if (logCount < LOG_MAX_ENTRIES) {
        strncpy(moveLog[logCount], entry, sizeof(moveLog[0]) - 1);
        moveLog[logCount][sizeof(moveLog[0]) - 1] = '\0';
        logCount++;
    } else {
        //making room for more entries 
        int i;
        for (i = 0; i < LOG_MAX_ENTRIES - 1; i++)
            strncpy(moveLog[i], moveLog[i + 1], sizeof(moveLog[0]));
        strncpy(moveLog[LOG_MAX_ENTRIES - 1], entry, sizeof(moveLog[0]) - 1);
        moveLog[LOG_MAX_ENTRIES - 1][sizeof(moveLog[0]) - 1] = '\0';
    }
}

//drawing move log 
static void renderMoveLogPanel(void) {
    int i ; 
    int yCursor = LOG_Y + 10; 

    //background
    fillRect(LOG_X, LOG_Y, LOG_W, LOG_H, 35,35,35); 
    
    //title
    drawText("MOVE LOG", LOG_X + 10, yCursor, (SDL_Color){255,255,255,255}, smallFont); 
    yCursor += 28; 

    SDL_SetRenderDrawColor(renderer,100,100,100,255); 
    SDL_RenderDrawLine(renderer, LOG_X, yCursor, LOG_X + LOG_W, yCursor); 
    yCursor += 6; 

    //each log entry 
    for (i=9; i<logCount; i++) {
        SDL_Color c; 

        if (strncmp(moveLog[i], "Yellow", 6) == 0) {
            c = (SDL_Color){245,210,60,255}; 
        } else {
            c = (SDL_Color){70,140,255,255}; 
        }

        drawText(moveLog[i], LOG_X+6, yCursor, c, smallFont); 
        yCursor += 20; 
        if (yCursor > LOG_Y + LOG_H-20) 
        break; 
    }

}

// draw the main in-game screen
static void renderGameScreen(int showClocks)
{
    int row;
    int col;


    // clear the background
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_RenderClear(renderer);

    // draw the board squares and pieces
    for (row = 0; row < 8; row++) {
        for (col = 0; col < 10; col++) {
            int boardRow;
            int boardCol;
            int x;
            int y;
            int squareIndex; 
            Piece p;

            // flip the board when the human is blue
            if (humanColor == YELLOW) {
                boardRow = row;
                boardCol = col;
            } else {
                boardRow = 7 - row;
                boardCol = 9 - col;
            }

            x = BOARD_X + col * SQUARE_SIZE;
            y = BOARD_Y + row * SQUARE_SIZE;

            //draw square BMP
            squareIndex = (row+col) %2; 
            drawSquare(x,y,squareIndex); 

            p = currentGameState->board[boardRow][boardCol]; 

            //drawing bmp on top of square
            if(p.piecetype != EMPTY) 
                drawPiece(p,x,y); 
        }
    }

    // draw highlighted legal destination squares
    if (hasHighlight) {
        int i;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); 

        for (i = 0; i < highlightMoves.count; i++) {
            int hcol;
            int hrow;
            SDL_Rect rect;

            if (humanColor == YELLOW) {
                hcol = highlightMoves.moves[i].to.file;
                hrow = highlightMoves.moves[i].to.rank;
            } else {
                hcol = 9 - highlightMoves.moves[i].to.file;
                hrow = 7 - highlightMoves.moves[i].to.rank;
            }

            rect.x = BOARD_X + hcol * SQUARE_SIZE + 8;
            rect.y = BOARD_Y + hrow * SQUARE_SIZE + 8;
            rect.w = SQUARE_SIZE - 16;
            rect.h = SQUARE_SIZE - 16;

            SDL_SetRenderDrawColor(renderer, 0, 220, 0, 160);
            SDL_RenderFillRect(renderer, &rect);
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE); 
    }

    // draw file labels at the bottom of the board
    for (col = 0; col <= 9; col++) {
        char lbl[2];

        if (humanColor == YELLOW) {
            lbl[0] = 'A' + col;
        } else {
            lbl[0] = 'J' - col;
        }
        lbl[1] = '\0';

        drawText(lbl,
                 BOARD_X + col * SQUARE_SIZE + 34,
                 BOARD_Y + 8 * SQUARE_SIZE + 10,
                 (SDL_Color){255, 255, 255, 255},
                 smallFont);
    }

    // draw rank labels on the left of the board
    for (row = 0; row <= 7; row++) {
        char lbl[8];

        snprintf(lbl, sizeof(lbl), "%d",
                 (humanColor == YELLOW) ? (8-row) : (row + 1)); 

        drawText(lbl,
                 BOARD_X - 20,
                 BOARD_Y + row * SQUARE_SIZE + 28,
                 (SDL_Color){255, 255, 255, 255},
                 smallFont);
    }

    // draw the right-side info panel
    fillRect(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, 52, 52, 52);

    {
        int pad = 20;
        int contentX = PANEL_X + pad;
        int contentW = PANEL_W - 2 * pad;
        int yCursor = PANEL_Y + pad;
        int msgBoxH = 80;
        char timeStr[32];

        // draw panel title
        drawText("GAME INFO",
                 contentX,
                 yCursor,
                 (SDL_Color){255, 255, 255, 255},
                 font);

        yCursor += 45;

        // draw the two player clocks
        if (showClocks) {
            snprintf(timeStr, sizeof(timeStr), "Yellow: %d:%02d",
                     yellowSecs / 60, yellowSecs % 60);
            drawText(timeStr,
                     contentX,
                     yCursor,
                     (SDL_Color){245, 210, 60, 255},
                     font);

            yCursor += 36;

            snprintf(timeStr, sizeof(timeStr), "Blue: %d:%02d",
                     blueSecs / 60, blueSecs % 60);
            drawText(timeStr,
                     contentX,
                     yCursor,
                     (SDL_Color){70, 140, 255, 255},
                     font);

            yCursor += 50;
        }

        // draw the status message box
        fillRect(contentX, yCursor, contentW, msgBoxH, 70, 70, 70);
        drawText(statusMsg,
                 contentX + 8,
                 yCursor + 10,
                 (SDL_Color){255, 255, 255, 255},
                 smallFont);

        yCursor += msgBoxH + 20;

        // set and draw the undo button
        undoBtnW = contentW;
        undoBtnH = 44;
        undoBtnX = contentX;
        undoBtnY = yCursor;

        fillRect(undoBtnX, undoBtnY, undoBtnW, undoBtnH, 200, 50, 50);
        drawText("UNDO",
                 undoBtnX + (undoBtnW / 2) - 20, undoBtnY + 12, (SDL_Color){255,255,255,255}, smallFont); 
        yCursor += undoBtnH + 12; 

        //stopping the chain button for the anteater 
        if (currentGameState && currentGameState->anteater_ate) {
            stopChainBtnW = contentW; 
            stopChainBtnX = contentX; 
            stopChainBtnH = 44; 
            stopChainBtnY =yCursor; 

            //button 
            fillRect(stopChainBtnX, stopChainBtnY, stopChainBtnW, stopChainBtnH, 220,120,0); 
            drawText("STOP CHAIN", stopChainBtnX + (stopChainBtnW/2) -40, stopChainBtnY + 12, (SDL_Color){255,255,255,255}, smallFont); 
            yCursor += stopChainBtnH+12; 
        }

        //showing player turn 
        const char *turnStr = (currentGameState->turn == YELLOW) ? "Yellow's turn" : "Blue's turn"; 
        SDL_Color turnColor = (currentGameState->turn == YELLOW) ? (SDL_Color) {245,210,60,255} : (SDL_Color){70,140,255,255}; 
        drawText(turnStr, contentX, yCursor, turnColor, smallFont);

        //moving panel to far right 
        renderMoveLogPanel(); 
        SDL_RenderPresent(renderer); 
    }

    SDL_RenderPresent(renderer);
}

//pawn promotion screen : popup w/ choices q,r,b,n
static void renderPromotionScreen(void) {
    PieceType choices[4] = {QUEEN, ROOK, BISHOP, KNIGHT}; 
    const char *labels[4] = {"Queen", "Rook", "Bishop", "Knight"}; 
    int i; 

    SDL_SetRenderDrawColor(renderer, 30,30,30,255); 
    SDL_RenderClear(renderer); 

    //backgroudn box
    fillRect(190,200,800,360,245,245,245); 
    drawText("Promote Pawn", 390,220, (SDL_Color){0,0,0, 255}, font); 
    drawText("Click the piece you want to promote to:", 350, 260, (SDL_Color) {100,100,100,255}, smallFont); 

    for (i = 0; i < 4; i++) {
        int buttonX = 210+i*190; 
        int buttonY = 290; 
        int buttonW = 170; 
        int buttonH = 200; 
        int isHovered = (promotionPiece == choices[i]); 
        SDL_Texture *text; 
        SDL_Rect d; 
        Piece preview; 

        //highlighting on hover
        if (isHovered) {
            fillRect(buttonX-4, buttonY-4, buttonW+8, buttonH+8, 50,150,255); 
        }

        //button background
        fillRect(buttonX, buttonY, buttonW, buttonH, 
            isHovered ? 210: 230, 
            isHovered ? 230: 230, 
            255);
            
        //drawing piece using color 
        preview.piecetype = choices[i]; 
        preview.color = currentGameState -> turn; 

        text = pieceTextures[(preview.color == YELLOW) ? 0: 1][choices[i]]; 

        if (text) {
            d.x = buttonX + 15; 
            d.y = buttonY + 10; 
            d.w = buttonW -30; 
            d.h = buttonH -55; 
            SDL_RenderCopy(renderer, text, NULL, &d); 
        } else {
            drawText(labels[i], buttonX + 40, buttonY + 60, (SDL_Color){0,0,0,255}, font); 
        }

        //piece label at bottom
        drawText(labels[i], buttonX + (buttonW /2) -20, buttonY + buttonH-35, (SDL_Color){0,0,0,255}, smallFont); 


    }
    SDL_RenderPresent(renderer); 
}

//displaying promotiion screen that waits for player to choose: returns PieceType player chose
PieceType dispPromotion(void) {
    PieceType choices[4] = {QUEEN, ROOK, BISHOP, KNIGHT}; 
    SDL_Event event; 
    int i; 

    currentScreen = SCREEN_PROMOTION; 
    promotionPiece = QUEEN; //default selection if nothing hvoered yet

    while(1) {
        renderPromotionScreen(); 
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                return QUEEN; 

            //updating highlight 
            if (event.type == SDL_MOUSEMOTION) {
                for(i = 0; i < 4; i++) {
                    int buttonX = 210+i*190; 
                    if(pointInRect(event.motion.x, event.motion.y, buttonX, 290,170,200)) {
                        promotionPiece = choices[i]; 
                    }
                }
            }

            //reutrn chosen piece
            if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                for (i=0; i<4; i++) {
                    int buttonX = 210 + i*190; 
                    if(pointInRect(event.button.x, event.button.y, buttonX, 290, 170,200)) {
                        currentScreen = SCREEN_GAME; 
                        return choices[i]; 
                    }
                }
            }
        }

        SDL_Delay(16); //slows loop down 
    }

}

// draw the game-over screen
static void renderEndScreen(const char *message)
{
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    fillRect(200, 200, 600, 360, 245, 245, 245);

    drawText("GAME OVER", 420, 280, (SDL_Color){0, 0, 0, 255}, font);
    drawText(message, 300, 360, (SDL_Color){0, 0, 0, 255}, font);
    drawText("Click anywhere to return to menu", 320, 440, (SDL_Color){100, 100, 100, 255}, smallFont);

    SDL_RenderPresent(renderer);

}

// initialize sdl and fonts
int guiInit(void)
{

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 0;
    }

    if (TTF_Init() < 0) {
        return 0;
    }


    window = SDL_CreateWindow("Anteater Chess",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH,
                              WINDOW_HEIGHT,
                              0);
    if (!window) {
        return 0;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        return 0;
    }

    //try common font paths across distros
    const char *fontPaths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/Users/coran/Library/Fonts/DejaVuSans.ttf",
        "/opt/homebrew/Caskroom/font-dejavu/2.37/dejavu-fonts-ttf-2.37/ttf/DejaVuSans.ttf",
        NULL
    };

    int i;
    for (i = 0; fontPaths[i] !=NULL; i++) {
        font = TTF_OpenFont(fontPaths[i], 28);
        smallFont = TTF_OpenFont(fontPaths[i], 19);

        if(font && smallFont)
            break;
    }

    if (!font || !smallFont) {
        return 0;
    }

    loadPieceTextures(); //putting all piece and bmp into memory

    return 1;
}

// clean up sdl resources
void guiQuit(void)
{
    freePieceTextures(); 
    if (smallFont) {
        TTF_CloseFont(smallFont);
    }

    if (font) {
        TTF_CloseFont(font);
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }

    if (window) {
        SDL_DestroyWindow(window);
    }

    TTF_Quit();
    SDL_Quit();
}

// store the active game state pointer
void setGameState(GameState *gs)
{
    currentGameState = gs;
}

// store the human player's color
void setHumanColor(Color c)
{
    humanColor = c;
}

// show the matchup selection menu
int matchupMenu(void)
{
    const char *options[] = {"Human vs Human", "Human vs AI", "AI vs AI"};
    int selected = 0;
    SDL_Event e;

    currentScreen = SCREEN_MATCHUP;

    while (1) {
        renderMenuScreen("Choose Matchup", options, 3, selected);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                return 0;
            }

            if (e.type == SDL_MOUSEMOTION) {
                if (pointInRect(e.motion.x, e.motion.y, 250, 300, 500, 55)) {
                    selected = 0;
                } else if (pointInRect(e.motion.x, e.motion.y, 250, 380, 500, 55)) {
                    selected = 1;
                } else if (pointInRect(e.motion.x, e.motion.y, 250, 460, 500, 55)) {
                    selected = 2;
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (pointInRect(e.button.x, e.button.y, 250, 300, 500, 55)) {
                    return 0;
                }
                if (pointInRect(e.button.x, e.button.y, 250, 380, 500, 55)) {
                    return 1;
                }
                if (pointInRect(e.button.x, e.button.y, 250, 460, 500, 55)) {
                    return 2;
                }
            }
        }
    }
}

// show the color selection menu
Color colorMenu(void)
{
    const char *options[] = {"YELLOW (yellow goes first)", "BLUE"};
    int selected = 0;
    SDL_Event e;

    currentScreen = SCREEN_COLOR;

    while (1) {
        renderMenuScreen("Choose Color", options, 2, selected);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                return YELLOW;
            }

            if (e.type == SDL_MOUSEMOTION) {
                if (pointInRect(e.motion.x, e.motion.y, 250, 300, 500, 55)) {
                    selected = 0;
                } else if (pointInRect(e.motion.x, e.motion.y, 250, 380, 500, 55)) {
                    selected = 1;
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (pointInRect(e.button.x, e.button.y, 250, 300, 500, 55)) {
                    return YELLOW;
                }
                if (pointInRect(e.button.x, e.button.y, 250, 380, 500, 55)) {
                    return BLUE;
                }
            }
        }
    }
}

// show the clock selection menu
int clockMenu(void)
{
    const char *options[] = {"5 Minutes", "10 Minutes", "15 Minutes"};
    int selected = 0;
    SDL_Event e;

    currentScreen = SCREEN_CLOCK;

    while (1) {
        renderMenuScreen("Choose Time Control", options, 3, selected);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                return 1;
            }

            if (e.type == SDL_MOUSEMOTION) {
                if (pointInRect(e.motion.x, e.motion.y, 250, 300, 500, 55)) {
                    selected = 0;
                } else if (pointInRect(e.motion.x, e.motion.y, 250, 380, 500, 55)) {
                    selected = 1;
                } else if (pointInRect(e.motion.x, e.motion.y, 250, 460, 500, 55)) {
                    selected = 2;
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (pointInRect(e.button.x, e.button.y, 250, 300, 500, 55)) {
                    return 1;
                }
                if (pointInRect(e.button.x, e.button.y, 250, 380, 500, 55)) {
                    return 2;
                }
                if (pointInRect(e.button.x, e.button.y, 250, 460, 500, 55)) {
                    return 3;
                }
            }
        }
    }
}

// show the ai difficulty menu
int difficultyMenu(void)
{
    const char *options[] = {"Easy", "Medium", "Hard"};
    int selected = 0;
    SDL_Event e;

    currentScreen = SCREEN_DIFFICULTY;

    while (1) {
        renderMenuScreen("Choose AI Difficulty", options, 3, selected);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                return 1;
            }

            if (e.type == SDL_MOUSEMOTION) {
                if (pointInRect(e.motion.x, e.motion.y, 250, 300, 500, 55)) {
                    selected = 0;
                } else if (pointInRect(e.motion.x, e.motion.y, 250, 380, 500, 55)) {
                    selected = 1;
                } else if (pointInRect(e.motion.x, e.motion.y, 250, 460, 500, 55)) {
                    selected = 2;
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (pointInRect(e.button.x, e.button.y, 250, 300, 500, 55)) {
                    return 1;
                }
                if (pointInRect(e.button.x, e.button.y, 250, 380, 500, 55)) {
                    return 2;
                }
                if (pointInRect(e.button.x, e.button.y, 250, 460, 500, 55)) {
                    return 3;
                }
            }
        }
    }
}

// update displayed board state and clocks
void displayBoard(GameState *gs, int ySecs, int bSecs, Color hColor)
{
    yellowSecs = ySecs;
    blueSecs = bSecs;
    setGameState(gs);
    setHumanColor(hColor);
    currentScreen = SCREEN_GAME;
    strcpy(statusMsg, "click source square, then destination");
}

// store legal moves to highlight on the board
void dispLegalMoves(MoveList *moves)
{
    highlightMoves = *moves;
    hasHighlight = 1;
}

// collect a move from mouse clicks
Move getMove(GameState *gs)
{
    SDL_Event e;
    MoveList allMoves;
    MoveList pieceMoves;
    int i;

    moveReady = 0;
    undoPressed = 0;
    clickCount = 0;
    hasHighlight = 0;
    pieceMoves.count = 0;

    while (!moveReady && !undoPressed) {
        renderGameScreen(1);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                exit(0);
            }

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int screenCol = (e.button.x - BOARD_X) / SQUARE_SIZE;
                int screenRow = (e.button.y - BOARD_Y) / SQUARE_SIZE;
                Square sq;
                Piece clickedPiece;

                // check if the undo button was clicked
                if (pointInRect(e.button.x, e.button.y,
                                undoBtnX, undoBtnY,
                                undoBtnW, undoBtnH)) {
                    undoPressed = 1;
                    clickCount = 0;
                    hasHighlight = 0;
                    break;
                }

                // ignore clicks outside the board
                if (screenCol < 0 || screenCol >= 10 || screenRow < 0 || screenRow >= 8) {
                    continue;
                }

                // convert screen coordinates to board coordinates
                if (humanColor == YELLOW) {
                    sq.file = screenCol;
                    sq.rank = screenRow;
                } else {
                    sq.file = 9 - screenCol;
                    sq.rank = 7 - screenRow;
                }

                clickedPiece = gs->board[sq.rank][sq.file];

                // first click selects a piece
                if (clickCount == 0) {
                    if (clickedPiece.piecetype != EMPTY && clickedPiece.color == gs->turn) {
                        firstClick = sq;
                        clickCount = 1;

                        allMoves = legalMoveGen(gs);
                        pieceMoves.count = 0;

                        // collect only the legal moves for the selected piece
                        for (i = 0; i < allMoves.count; i++) {
                            if (allMoves.moves[i].from.file == firstClick.file &&
                                allMoves.moves[i].from.rank == firstClick.rank) {
                                pieceMoves.moves[pieceMoves.count] = allMoves.moves[i];
                                pieceMoves.count++;
                            }
                        }

                        highlightMoves = pieceMoves;
                        hasHighlight = 1;

                        if (pieceMoves.count == 0) {
                            snprintf(statusMsg, sizeof(statusMsg), "that piece has no legal moves");
                        } else {
                            snprintf(statusMsg, sizeof(statusMsg), "piece selected");
                        }
                    } else {
                        snprintf(statusMsg, sizeof(statusMsg), "select one of your own pieces");
                    }
                } else {
                    // if the second click is another friendly piece, switch selection
                    if (clickedPiece.piecetype != EMPTY && clickedPiece.color == gs->turn) {
                        firstClick = sq;

                        allMoves = legalMoveGen(gs);
                        pieceMoves.count = 0;

                        for (i = 0; i < allMoves.count; i++) {
                            if (allMoves.moves[i].from.file == firstClick.file &&
                                allMoves.moves[i].from.rank == firstClick.rank) {
                                pieceMoves.moves[pieceMoves.count] = allMoves.moves[i];
                                pieceMoves.count++;
                            }
                        }

                        highlightMoves = pieceMoves;
                        hasHighlight = 1;

                        if (pieceMoves.count == 0) {
                            snprintf(statusMsg, sizeof(statusMsg), "that piece has no legal moves");
                        } else {
                            snprintf(statusMsg, sizeof(statusMsg), "piece changed");
                        }

                        continue;
                    }

                    // check whether the second click is a legal destination
                    for (i = 0; i < pieceMoves.count; i++) {
                        if (pieceMoves.moves[i].to.file == sq.file &&
                            pieceMoves.moves[i].to.rank == sq.rank) {
                            pendingMove = pieceMoves.moves[i];
                            moveReady = 1;
                            hasHighlight = 0;
                            clickCount = 0;
                            snprintf(statusMsg, sizeof(statusMsg), "move accepted");
                            break;
                        }
                    }

                    if (!moveReady) {
                        snprintf(statusMsg, sizeof(statusMsg), "illegal destination");
                    }
                }
            }
        }

        SDL_Delay(16);
    }

    hasHighlight = 0;
    clickCount = 0;
    return pendingMove;
}

// show a short delay while the ai is "thinking"
void aiMove(Move move)
{
    SDL_Event e;
    int frames = 30;

    (void)move;

    while (frames--) {
        renderGameScreen(1);
        SDL_Delay(50);

        while (SDL_PollEvent(&e)) {
        }
    }
}

// show the winner screen
void dispWin(Color winner)
{
    char msg[128];

    snprintf(msg, sizeof(msg), "%s WINS!", (winner == YELLOW) ? "YELLOW" : "BLUE");
    currentScreen = SCREEN_ENDGAME;
    strcpy(statusMsg, msg);
    renderEndScreen(msg);
    SDL_Delay(2000);
}

// show the stalemate screen
void dispStalemate(void)
{
    currentScreen = SCREEN_ENDGAME;
    strcpy(statusMsg, "STALEMATE!");
    renderEndScreen("STALEMATE!");
    SDL_Delay(2000);
}

// show the timeout result screen
void dispTimeout(Color loser)
{
    char msg[128];

    snprintf(msg, sizeof(msg), "%s WINS BY TIMEOUT!", (loser == YELLOW) ? "BLUE" : "YELLOW");
    currentScreen = SCREEN_ENDGAME;
    strcpy(statusMsg, msg);
    renderEndScreen(msg);
    SDL_Delay(2000);
}

// update the status message shown on the right panel
void printError(const char *msg)
{
    strncpy(statusMsg, msg, sizeof(statusMsg) - 1);
    statusMsg[sizeof(statusMsg) - 1] = '\0';
}

// mark that the undo button was triggered
void dispUndo(void)
{
    undoPressed = 1;
}
