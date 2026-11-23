# Connect4: Resume Checkpoint

## Resume prompt

Start with **Phase 1: canonical state model**. Read the repository and this checkpoint, do not repeat completed fixes, and keep the existing UI playable while migrating to a pure rules module.

## Current repository state

- Repository: `Asher-1971/Connect4`
- Branch: `organisation`
- Entry point: `main()` in `game.c`
- Existing modules: `header.h`, `start.c`, `game.c`, `game.h`, `win.c`, `module3.c`, `module3.h`
- Human vs Human: implemented
- Human vs Easy/Normal/Hard AI: implemented
- Save/load: UI hooks only; persistence is not implemented
- Power-ups: disabled by `ENABLE_POWERUPS 0`
- No merge conflict markers remain

## Completed work

- Replaced unsafe `scanf("%d")` integer input with bounded `fgets()`/`strtol()` parsing.
- Aligned `dropPiece` with the header and added bounds/null/full-column validation.
- Added a real `main()` and routed menu entry through `game.c`.
- Added win-length input and 4/5 validation.
- Implemented Easy random, Normal 100 ms, and Hard 1000 ms AI modes.
- Added AI state validation for dimensions, win length, player, move count, cell values, gravity, and player parity.
- Added monotonic AI timing, terminal-state move rejection, gravity cleanup, null guards, and bridge validation.
- Corrected tie detection to verify the physical board and exclude winning boards.
- Fixed EOF newline formatting.
- Verified strict GCC and Clang builds, AddressSanitizer, UndefinedBehaviorSanitizer, 3000 randomized AI oracle states, 10000 frontend win/tie states, a 20x20 stress test, and interactive smoke tests.

## Architectural decisions

Use one authoritative `GameState` with one board representation and `Move`/`MoveType` abstractions.

Do not mix frontend `char` cells with numeric AI cells in future code. Keep move count distinct from current piece count because future bomb/anvil moves can remove pieces.

Target modules:

1. Pure rules/state module
2. Terminal UI module
3. Stable AI API
4. Persistence module
5. Power-up module
6. Transposition table
7. Evaluator/opening book improvements

The UI must only collect input and render state. It must call pure rules functions for validation, move application, win detection, and draw detection.

## Stable AI target

```c
AIMoveResult getBestMove(
    const GameState *state,
    AILevel level,
    uint32_t time_budget_ms);
```

Return a full move:

```c
typedef enum {
    MOVE_NORMAL,
    MOVE_BOMB,
    MOVE_ANVIL
} MoveType;

typedef struct {
    int col;
    MoveType type;
} Move;
```

## Transposition-table rules for later

- Do not implement this phase before canonical state and the full move API.
- Hash the board, dimensions, win length, current player, move count, piece count, power-up flags, and game-rule configuration.
- Cache exact/lower/upper scores and depth.
- Cache `MoveType` as well as the column.
- Never allow power-up-disabled state to reuse power-up-enabled state.

## Migration phases

1. Shared types and canonical `GameState`
2. Pure rules and state transitions
3. Terminal UI extraction
4. Persistence extraction
5. Stable AI API
6. Power-up transitions
7. Transposition table
8. Opening book or evaluator improvements

## Test gates

Every migration phase must pass:

- Strict GCC build
- Strict Clang build
- AddressSanitizer
- UndefinedBehaviorSanitizer
- Connect 4 and Connect 5
- Dimensions 4 through 20
- Horizontal, vertical, and diagonal win checks
- Full-board draws
- Terminal move rejection
- Invalid states and move types
- Bomb/anvil transitions
- Save/load valid and corrupted files
- Randomized state versus independent AI oracle

## Session-sync finding

This chat was created inside a GitHub Codespaces remote session. It cannot be assumed to cloud-sync. Use this checkpoint, exported `chat.json`, Git commits, and Chat: Export Chat as durable sources of truth.

The local chat export contains 24 turns. Full turn-by-turn text is preserved in `docs/CONNECT4_SESSION_CHECKPOINT.json`.
