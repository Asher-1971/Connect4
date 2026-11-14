# Connect 4: Power-Up Edition

An advanced connect four strategy game developed in C programming language that is more complex than the conventional version of connect four in terms of features and customization options.

---

## Table of Contents

- [Overview](#overview)
- [Key Components](#key-components)
- [Technical Architecture](#technical-architecture)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Compilation](#compilation)
  - [Download Pre-compiled Binaries](#download-pre-compiled-binaries)

---

## Overview

The regular Connect 4 game has certain restrictions, such as being confined to 6x7 squares, using two players only from one place, and having no record keeping features. The game **Connect 4: Power-Up Edition** overcomes these problems by providing a menu-based modular C program that allows variable board size and incorporates power-ups for players.

---

## Key Components

### Game Modes
- **Player vs. Player (PvP):** Two-player local mode with customizable names and unique disc icons.
- **Player vs. Computer (PvC):** Single-player mode with two AI difficulty levels:
  - **Easy:** Randomly selects valid columns.
  - **Medium:** Analyzes the board in order to make immediate moves leading to a win or preventing opponent from winning.

### Customization & Rules
- **Dynamic Grid Settings:** Configure grid sizes (such as 8x9, 10x10) and select number of discs for winning sequence (4 or 5 discs).
- **Power-Ups:** Unique discs to use once per game:
  - **Bomb Disc:** Removes surrounding discs when placed.
  - **Anvil Disc:** Removes an entire column when placed.
- **Undo Feature:** Undo last move during gameplay session.

### Saving & Progression
- **Scoring & Best Streaks:** Points scored depend on board size, effectiveness of moves and difficulty of AI. Saves both best and all time win streaks.
- **Saving & Loading Games:** Ability to serialize current game and continue playing later sessions.
- **Leaderboard & History:** Save statistics on persistent storage including total wins, losses, scores, and best streaks.

---

## Technical Architecture

The program employs the standard C libraries such as stdio.h, string.h, time.h, and stdlib.h and exhibits several concepts of system programming:

| Concept | Implementation |
| :--- | :--- |
| **Dynamic Memory Allocation** | Dynamically creates two-dimensional array boards based on user-defined numbers of rows and columns. |
| **Recursion** | Line counting function scans rows, columns, and diagonals dynamically regardless of board size and win length. |
| **Structure and Pointers** | Bundles player information, match status, and moves performed. |
| **File Handling** | Performs data handling for saved matches, match logs, and game ranking. |
| **Input Validations** | Validates menu input and column input to avoid out-of-bound actions. |

---

## Getting Started

### Prerequisites

- **Operating System:** Windows, Linux, or macOS
- **Compiler:** GCC (MinGW on Windows, GCC on Linux, or Clang / Xcode Command Line Tools on macOS)
- **IDE (Optional):** Visual Studio Code or Dev-C++

### Compilation

First, clone the repository and navigate into the project folder:

```bash
git clone [https://github.com/Asher-1971/Connect4](https://github.com/Asher-1971/Connect4)
cd Connect4
```

Then compile and run based on your operating system:

| Operating System | Compile Command | Run Command |
| :--- | :--- | :--- |
| **Windows** | `gcc main.c -o connect4.exe` | `connect4.exe` |
| **Linux** | `gcc main.c -o connect4` | `./connect4` |
| **macOS** | `clang main.c -o connect4` (or `gcc main.c -o connect4`) | `./connect4` |

### Download Pre-compiled Binaries

In case you do not want to compile the source code yourself, pre-compiled binaries are available for Windows, Linux, and macOS in the [Releases](https://github.com/Asher-1971/Connect4/releases) section.

| Operating System | File | Instructions |
| :--- | :--- | :--- |
| **Windows** | `connect4-windows-x64.exe` | Just download and double-click to execute, or execute from Command Prompt. |
| **Linux** | `connect4-linux-x64` | Run `chmod +x connect4-linux-x64 && ./connect4-linux-x64`. |
| **macOS** | `connect4-macos-universal` | Run `chmod +x connect4-macos-universal && ./connect4-macos-universal`. |
