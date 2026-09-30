# Chess Engine (C++)

C++ group project implementing a chess engine that reconstructs a game from its move history, generates candidate moves, evaluates positions, and writes its selected next move to a CSV file.

## Overview

The aim of this project was to translate chess rules and decision-making into a working algorithm, from board representation and special-move handling to recursive game-tree exploration. 

## How it works

1. **Board representation:** store the board using square/piece objects alongside 64-bit bitboards for each side and piece type.
2. **Move generation:** calculate moves for each piece, with logic for castling, en passant, pawn promotion, and checks.
3. **Position evaluation:** score candidate positions using material, positional criteria, king safety, castling, and other terms.
4. **Move selection:** explore candidate continuations recursively using a minimax-style evaluation, with Zobrist hashes used to track position history and repetition.
5. **Input/output:** read coordinate moves from `history.csv`, reconstruct the current position, and write the selected move to `move.csv`.

## Repository structure

```text
cpp-chess-engine/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── src/
│   ├── main.cpp       # Load game history and request the engine's next move
│   ├── board.h        # Board state, move logic, evaluation and search
│   ├── pieces.h       # Chess piece definitions and movement directions
│   ├── io.cpp         # CSV input/output implementation
│   └── io.h
└── examples/
    └── history.csv    # Empty sample history: the starting position
```


### Input / output format

The input file contains one coordinate move per line, such as:

```text
e2e4
e7e5
g1f3
```

The program reads the first four characters of each line as a source and destination square (`file`, `rank`, `file`, `rank`). An empty history represents the initial position. It determines the side to move from the number of recorded moves, starting with White. The output `move.csv` contains the engine's selected move in the same four-character format, for example `d2d4`.

This is a course-project file protocol; it is **not** a UCI-compatible engine or a PGN parser. The current entry point uses fixed filenames and does not accept command-line options. Treat the input as trusted, correctly formatted move history.

## What we learnt

- Representing a complex rule set as interoperating C++ structures and algorithms
- Using bitwise representations, position hashing, and recursive search for a turn-based game
- Why handling edge cases such as special moves and checks matters when integrating game logic
- Collaborating on a shared codebase and bringing together game logic, input/output, and build configuration
