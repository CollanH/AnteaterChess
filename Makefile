CC     = gcc
CFLAGS = -Wall -g
SRC    = src
BIN    = bin

GUI_SRCS = $(SRC)/chess.c $(SRC)/strategy.c $(SRC)/eval.c \
           $(SRC)/legalMoveGen.c $(SRC)/chess_types.c $(SRC)/gui.c

TEXT_SRCS = $(SRC)/textChess.c $(SRC)/strategy.c $(SRC)/eval.c \
            $(SRC)/legalMoveGen.c $(SRC)/chess_types.c $(SRC)/textui.c

TEST_SRCS = $(SRC)/test.c $(SRC)/chess_types.c $(SRC)/legalMoveGen.c

LIBS     = -lSDL2 -lSDL2_ttf -lm

all: $(BIN)/chess

$(BIN)/chess: $(GUI_SRCS)
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $@ $(GUI_SRCS) -I$(SRC) $(LIBS)

textchess: $(TEXT_SRCS)
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/textChess $(TEXT_SRCS) -I$(SRC)

test: $(TEST_SRCS)
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/test $(TEST_SRCS) -I$(SRC)
	$(BIN)/test

clean:
	rm -f $(BIN)/chess $(BIN)/textChess $(BIN)/test

tar: $(BIN)/chess
	cd .. && tar -czf chess/Chess_Alpha_src.tar.gz \
	    --exclude='chess/$(BIN)' \
	    --exclude='chess/*.o' \
	    --exclude='chess/*.tar.gz' \
	    --exclude='chess/chess_log.txt' \
	    chess/README chess/INSTALL chess/COPYRIGHT chess/Makefile chess/src/ chess/doc/
	cd .. && tar -czf chess/Chess_Alpha.tar.gz \
	    chess/README chess/INSTALL chess/COPYRIGHT \
	    chess/$(BIN)/chess \
	    chess/doc/Chess_UserManual.pdf
