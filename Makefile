CC     = gcc
CFLAGS = -Wall -g
SRC    = src
BIN    = bin

GUI_SRCS = $(SRC)/chess.c $(SRC)/strategy.c $(SRC)/eval.c \
           $(SRC)/legalMoveGen.c $(SRC)/chess_types.c $(SRC)/gui.c

TEXT_SRCS = $(SRC)/textChess.c $(SRC)/strategy.c $(SRC)/eval.c \
            $(SRC)/legalMoveGen.c $(SRC)/chess_types.c $(SRC)/textui.c

TEST_SRCS = $(SRC)/test.c $(SRC)/chess_types.c $(SRC)/legalMoveGen.c

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
    SDL2_PREFIX  = $(shell brew --prefix sdl2)
    SDL2T_PREFIX = $(shell brew --prefix sdl2_ttf)
    SDL2I_PREFIX = $(shell brew --prefix sdl2_image)
    CFLAGS      += -I$(SDL2_PREFIX)/include -I$(SDL2_PREFIX)/include/SDL2 -I$(SDL2T_PREFIX)/include -I$(SDL2I_PREFIX)/include
    LIBS         = -L$(SDL2_PREFIX)/lib -L$(SDL2T_PREFIX)/lib -L$(SDL2I_PREFIX)/lib -lSDL2 -lSDL2_ttf -lSDL2_image -lm
else
    CFLAGS      += $(shell pkg-config --cflags sdl2 SDL2_ttf SDL2_image)
    LIBS         = $(shell pkg-config --libs sdl2 SDL2_ttf SDL2_image) -lm
endif

all: $(BIN)/chess

$(BIN)/chess: $(GUI_SRCS)
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $@ $(GUI_SRCS) -I$(SRC) $(LIBS)

textchess: $(TEXT_SRCS)
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/textChess $(TEXT_SRCS) -I$(SRC) -lm

test: $(TEST_SRCS)
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/test $(TEST_SRCS) -I$(SRC) -lm
	$(BIN)/test

STRATEGY_TEST_SRCS = $(SRC)/strategyTest.c $(SRC)/chess_types.c \
                     $(SRC)/legalMoveGen.c $(SRC)/strategy.c $(SRC)/eval.c

teststrategy: $(STRATEGY_TEST_SRCS)
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/strategyTest $(STRATEGY_TEST_SRCS) -I$(SRC) -lm
	$(BIN)/strategyTest

run: $(BIN)/chess
	cd $(BIN) && ./chess

clean:
	rm -f $(BIN)/chess $(BIN)/textChess $(BIN)/test $(BIN)/strategyTest

tar: $(BIN)/chess
	tar -czf Chess_Alpha_src.tar.gz \
	    --exclude='$(BIN)' \
	    --exclude='*.o' \
	    --exclude='*.tar.gz' \
	    --exclude='chess_log.txt' \
	    README INSTALL COPYRIGHT Makefile src/ doc/
	tar -czf Chess_Alpha.tar.gz \
	    README INSTALL COPYRIGHT \
	    $(BIN)/chess \
	    doc/Chess_UserManual.pdf
