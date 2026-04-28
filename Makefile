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
	rm -rf Chess_V1.0 Chess_V1.0_src
	mkdir -p Chess_V1.0/bin Chess_V1.0/doc
	cp README INSTALL COPYRIGHT Chess_V1.0/
	cp $(BIN)/chess Chess_V1.0/bin/
	cp -r board_images.bmp Chess_V1.0/
	cp doc/Chess_UserManual.pdf Chess_V1.0/doc/
	tar -czf ../Chess_V1.0.tar.gz Chess_V1.0
	rm -rf Chess_V1.0
	mkdir -p Chess_V1.0_src/doc Chess_V1.0_src/src
	cp README INSTALL COPYRIGHT Makefile Chess_V1.0_src/
	cp -r board_images.bmp Chess_V1.0_src/
	cp doc/Chess_UserManual.pdf doc/Chess_SoftwareSpec.pdf Chess_V1.0_src/doc/
	cp src/*.c src/*.h Chess_V1.0_src/src/
	tar -czf ../Chess_V1.0_src.tar.gz Chess_V1.0_src
	rm -rf Chess_V1.0_src
