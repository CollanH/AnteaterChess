#include "gui.h"
#include "legalMoveGen.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
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
#define PANEL_X 860
#define PANEL_Y 60
#define PANEL_W 280
#define PANEL_H 640

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

// promotion menu selection
static int promotionPiece = QUEEN;

// status/error message shown in the right panel
static char statusMsg[128] = "";

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

            // draw alternating square colors
            if ((row + col) % 2 == 0) {
                fillRect(x, y, SQUARE_SIZE, SQUARE_SIZE, 245, 230, 140);
            } else {
                fillRect(x, y, SQUARE_SIZE, SQUARE_SIZE, 70, 120, 190);
            }

            p = currentGameState->board[boardRow][boardCol];

            // draw the piece as a colored letter
            if (p.piecetype != EMPTY) {
                char txt[2];
                SDL_Color c;

                txt[0] = pieceLetter(p);
                txt[1] = '\0';

                if (p.color == YELLOW) {
                    c = (SDL_Color){245, 210, 60, 255};
                } else {
                    c = (SDL_Color){70, 140, 255, 255};
                }

                drawText(txt, x + 30, y + 25, c, font);
            }
        }
    }

    // draw highlighted legal destination squares
    if (hasHighlight) {
        int i;

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
    }

    // draw file labels at the bottom of the board
    for (col = 0; col < 10; col++) {
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
    for (row = 0; row < 8; row++) {
        char lbl[8];

        snprintf(lbl, sizeof(lbl), "%d",
                 (humanColor == YELLOW) ? (row + 1) : (8 - row));

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
        int msgBoxH = 120;
        char timeStr[32];

        // draw panel title
        drawText("GAME INFO",
                 contentX,
                 yCursor,
                 (SDL_Color){255, 255, 255, 255},
                 font);

        yCursor += 50;

        // draw the two player clocks
        if (showClocks) {
            snprintf(timeStr, sizeof(timeStr), "Yellow: %d:%02d",
                     yellowSecs / 60, yellowSecs % 60);
            drawText(timeStr,
                     contentX,
                     yCursor,
                     (SDL_Color){245, 210, 60, 255},
                     font);

            yCursor += 40;

            snprintf(timeStr, sizeof(timeStr), "Blue: %d:%02d",
                     blueSecs / 60, blueSecs % 60);
            drawText(timeStr,
                     contentX,
                     yCursor,
                     (SDL_Color){70, 140, 255, 255},
                     font);

            yCursor += 60;
        }

        // draw the status message box
        fillRect(contentX, yCursor, contentW, msgBoxH, 70, 70, 70);
        drawText(statusMsg,
                 contentX + 10,
                 yCursor + 15,
                 (SDL_Color){255, 255, 255, 255},
                 smallFont);

        yCursor += msgBoxH + 40;

        // set and draw the undo button
        undoBtnW = 160;
        undoBtnH = 50;
        undoBtnX = PANEL_X + (PANEL_W - undoBtnW) / 2;
        undoBtnY = yCursor;

        fillRect(undoBtnX, undoBtnY, undoBtnW, undoBtnH, 200, 50, 50);
        drawText("UNDO",
                 undoBtnX + 45,
                 undoBtnY + 12,
                 (SDL_Color){255, 255, 255, 255},
                 smallFont);
    }

    SDL_RenderPresent(renderer);
}

// draw the promotion menu screen
static void renderPromotionScreen(void)
{
    const char *options[] = {"Queen", "Rook", "Bishop", "Knight"};
    renderMenuScreen("PROMOTE PAWN", options, 4, promotionPiece);
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
        NULL
    };
    int i;
    for (i = 0; fontPaths[i] != NULL; i++) {
        font      = TTF_OpenFont(fontPaths[i], 28);
        smallFont = TTF_OpenFont(fontPaths[i], 18);
        if (font && smallFont) break;
    }

    if (!font || !smallFont) {
        return 0;
    }

    return 1;
}

// clean up sdl resources
void guiQuit(void)
{
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
