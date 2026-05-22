# Anteater Chess

A custom chess variant played on an 8×10 board, featuring a unique piece called the **Anteater** that can chain-capture enemy pawns. Built in C with an SDL2 GUI and a full negamax AI engine. Also supports standard 8×8 chess.

---

## The Anteater Piece

The Anteater moves one square in any orthogonal direction (like a restricted rook). It can only capture enemy pawns — but when it does, it can keep going and capture multiple pawns in a single turn as long as each captured square is adjacent to the last. This chain-capture mechanic is the defining feature of the variant.

---

## Game Modes

- **Anteater Chess** — 8×10 board, includes the Anteater piece
- **Standard Chess** — 8×8 board, standard rules

Both modes support:
- Human vs Human
- Human vs AI (adjustable difficulty)
- AI vs AI
- Optional chess clock
- Color selection (play as Yellow or Blue)

---

## Building

**Dependencies:**
```
GCC 9.0+, GNU Make, SDL2, SDL2_ttf, SDL2_image
```

**Install SDL2 (macOS):**
```bash
brew install sdl2 sdl2_ttf sdl2_image
```

**Install SDL2 (Ubuntu/Debian):**
```bash
sudo apt-get install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
```

**Build and run:**
```bash
make
make run
```

> `make run` is required (not `./bin/chess` directly) — the binary looks for piece images relative to its working directory.

**Clean:**
```bash
make clean
```

**Running over SSH (Linux server):**

X11 forwarding is required to display the GUI remotely:
```bash
ssh -X username@server
```
On macOS clients, install [XQuartz](https://www.xquartz.org) first.

---

## AI Engine

The engine uses negamax with the following features:

- Alpha-beta pruning
- Iterative deepening (depth 2 → 8) with a 9-second time limit
- Aspiration windows
- Transposition table (1M entries, Zobrist hashing)
- Quiescence search
- Null move pruning (R=3)
- Late move reductions (LMR)
- Killer move heuristic (2 slots per ply)
- History heuristic
- MVV-LVA move ordering
- Tapered evaluation (opening/endgame PST blend)
- Mobility, king safety, pawn structure, and piece development evaluation

See [`REFERENCES.txt`](REFERENCES.txt) for algorithm sources and implementation notes.

---

## Docs

- [`doc/Chess_UserManual.pdf`](doc/Chess_UserManual.pdf) — how to play
- [`doc/Chess_SoftwareSpec.pdf`](doc/Chess_SoftwareSpec.pdf) — software specification

---

## Project Structure

```
src/                  source code
board_images.bmp/     piece and board sprite assets
doc/                  user manual and software spec
Makefile
```
