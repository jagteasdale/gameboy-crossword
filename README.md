# Game Boy Crossword

A cryptic crossword game for the original Nintendo Game Boy, designed to run on real hardware.

## Features

- 15x15 crossword grid display with smooth scrolling
- Cryptic crossword clues stored in ROM
- Letter input via D-pad selection
- Multiple puzzles stored in cartridge
- Save progress to SRAM (planned)

## Requirements

### Development
- [GBDK-2020](https://github.com/gbdk-2020/gbdk-2020) - Game Boy Development Kit
- Make

### Testing
- [BGB](https://bgb.bircd.org/) - Recommended emulator (Windows/Wine)
- [SameBoy](https://sameboy.github.io/) - Cross-platform emulator
- [mGBA](https://mgba.io/) - Cross-platform emulator

### Running on Hardware
- Flash cart (EverDrive GB, GB USB Smart Card, etc.)
- Original Game Boy, Game Boy Pocket, Color, or Advance

## Building

1. Install GBDK-2020 and set the path:
   ```bash
   export GBDK_HOME=/Applications/gbdk
   ```

2. Build the ROM:
   ```bash
   make clean && make
   ```

3. Run in emulator:
   ```bash
   make run-sameboy
   # or
   make run-mgba
   ```

## Controls

| Button | Action |
|--------|--------|
| D-Pad | Move cursor / Select letter |
| A | Enter letter input mode / Confirm |
| B | Delete letter / Cancel / Back |
| SELECT | Toggle direction (Across/Down) |
| START | Pause menu |

## Project Structure

```
gameboy-crossword/
├── src/                 # C source files
│   ├── main.c          # Entry point and main loop
│   ├── game.c          # Core game logic and state machine
│   ├── graphics.c      # Rendering and tile management
│   ├── input.c         # Input handling
│   └── puzzles.c       # Puzzle data and loading
├── include/            # Header files
│   ├── types.h         # Type definitions and constants
│   ├── game.h          # Game interface
│   ├── graphics.h      # Graphics interface
│   ├── input.h         # Input interface
│   └── puzzles.h       # Puzzle interface
├── assets/             # Graphics assets
│   ├── tiles/          # Background tiles
│   ├── sprites/        # Sprite graphics
│   └── fonts/          # Font tiles
├── data/               # Puzzle data files
├── tools/              # Build tools and converters
├── Makefile
└── README.md
```

## Adding Puzzles

Puzzles are defined in `src/puzzles.c`. Each puzzle consists of:

1. A 15x15 grid template (solution)
2. Across clues array
3. Down clues array

See the sample puzzle for the format.

## Technical Constraints

### Game Boy Specifications
- **CPU:** Sharp LR35902 @ 4.19 MHz
- **RAM:** 8 KB work RAM + 8 KB video RAM
- **Screen:** 160×144 pixels (20×18 tiles)
- **Colors:** 4 shades of gray
- **Tile size:** 8×8 pixels

### Design Decisions
- 15x15 grid fits in 120×120 pixels
- Remaining space used for clue display
- Letter input via scrolling A-Z selector
- Grid scrolling for cursor tracking

## License

MIT
