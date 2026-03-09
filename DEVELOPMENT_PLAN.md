# Development Plan: Game Boy Crossword

## Overview

This document outlines the implementation plan for a 15x15 cryptic crossword game for the Nintendo Game Boy.

---

## Phase 1: Core Infrastructure (Complete)

### 1.1 Project Setup
- [x] Directory structure
- [x] Makefile for GBDK-2020
- [x] Header files with type definitions
- [x] Basic main loop

### 1.2 Input System
- [x] Button state tracking
- [x] Edge detection (just pressed/released)
- [ ] Auto-repeat for held directions

### 1.3 State Machine
- [x] Game state enum
- [x] State transition logic
- [x] State handler functions

---

## Phase 2: Graphics System (In Progress)

### 2.1 Tile Design
- [x] Create 8x8 pixel font (A-Z, 0-9)
- [x] Design empty cell tile
- [x] Design black/blocked cell tile
- [x] Design cursor/highlight tile
- [x] Punctuation tiles (colon, period, comma, parens, etc.)
- [ ] Create clue number overlay tiles (small subscript numbers)

### 2.2 Screen Layout
```
+--------------------+
|                    |  Rows 0-14: Grid area
|   15x15 GRID       |  (15 tiles tall)
|   (scrollable)     |
|                    |
+--------------------+
| CLUE TEXT AREA     |  Rows 15-17: Clue display
| (3 lines)          |  (3 tiles tall)
+--------------------+
```

### 2.3 Rendering
- [x] Efficient grid rendering (screen buffer)
- [x] Cursor sprite overlay
- [x] Clue text rendering with word wrap
- [ ] Smooth scrolling (optional enhancement)

### 2.4 Letter Input UI
- [x] Letter selection with vertical scroll display
- [x] Show current + adjacent letters
- [ ] Full 5x6 grid overlay (optional enhancement)

---

## Phase 3: Core Gameplay

### 3.1 Grid Navigation
- [x] D-pad cursor movement
- [x] Skip black cells
- [x] Direction toggle (Across/Down)
- [ ] Wrap-around at grid edges (optional)

### 3.2 Letter Entry
- [x] Enter letter input mode
- [x] Letter selection with D-pad
- [x] Confirm/cancel letter
- [x] Auto-advance after entry
- [ ] Delete/backspace functionality

### 3.3 Clue System
- [x] Find clue for current cell
- [x] Display clue in bottom area
- [x] Show clue number and direction
- [ ] Scroll long clues (for very long clues)
- [ ] Full-screen clue view

### 3.4 Validation
- [x] Check puzzle completion
- [ ] Check individual letter
- [ ] Check current word
- [ ] Reveal letter (hint)
- [ ] Reveal word (hint)

---

## Phase 4: Puzzle Data

### 4.1 Data Format
- [x] Grid structure (15x15 cells)
- [x] Clue storage format
- [ ] Compression for clue text
- [ ] Multi-puzzle ROM organization

### 4.2 Sample Puzzles
- [x] One sample puzzle framework
- [ ] Complete sample puzzle with real cryptic clues
- [ ] Additional test puzzles

### 4.3 Puzzle Converter Tool
- [x] Python tool to convert JSON to C (`tools/puzzle_converter.py`)
- [x] Proper crossword cell numbering algorithm
- [x] Makefile target (`make convert_puzzles`)
- [ ] Validate puzzle format (check grid consistency, clue coverage)
- [ ] Integrate generated C files into puzzles.c (currently manual)

---

## Phase 5: Polish & Features

### 5.1 Title Screen
- [ ] Logo/title graphic
- [ ] Animation (optional)
- [ ] Copyright/credits

### 5.2 Menu System
- [ ] Puzzle selection list
- [ ] Show puzzle progress (started/completed)
- [ ] Settings menu

### 5.3 Save System (SRAM)
- [ ] Save current puzzle progress
- [ ] Load saved game
- [ ] Track completed puzzles
- [ ] Battery-backed SRAM support

### 5.4 Audio (Optional)
- [ ] Sound effects for input
- [ ] Completion jingle
- [ ] Background music

### 5.5 Timer (Optional)
- [ ] Track solve time
- [ ] Display timer
- [ ] Save best times

---

## Phase 6: Testing & Release

### 6.1 Emulator Testing
- [ ] Test in BGB
- [ ] Test in SameBoy
- [ ] Test in mGBA
- [ ] Verify timing accuracy

### 6.2 Hardware Testing
- [ ] Test on DMG (original Game Boy)
- [ ] Test on Game Boy Pocket
- [ ] Test on Game Boy Color
- [ ] Test on Game Boy Advance

### 6.3 Polish
- [ ] Fix any rendering glitches
- [ ] Optimize performance
- [ ] Memory usage audit
- [ ] Code cleanup

---

## Technical Notes

### Memory Budget

| Area | Size | Usage |
|------|------|-------|
| Work RAM | 8 KB | Game state, current puzzle grid |
| Video RAM | 8 KB | Tiles and tilemap |
| ROM | 32 KB+ | Code, puzzles, graphics |
| SRAM | 8 KB | Save data (if battery-backed) |

### Puzzle Storage Estimate
- Grid: 225 bytes (15×15)
- Clues: ~2-3 KB per puzzle (compressed)
- Estimate: 3-4 KB per complete puzzle
- A 32 KB ROM could hold ~5-8 puzzles with code

### Performance Considerations
- Redraw only changed tiles
- Use sprites for cursor overlay
- Pre-calculate clue lookups
- Minimize string operations

---

## Recommended Development Order

1. **Get tiles working** - Create proper font and cell graphics
2. **Grid display** - Render a static puzzle correctly
3. **Cursor movement** - Navigate the grid smoothly
4. **Letter entry** - Full input flow working
5. **Clue display** - Show clues for current cell
6. **Completion check** - Detect solved puzzle
7. **Multiple puzzles** - Menu and selection
8. **Save system** - SRAM persistence
9. **Polish** - Audio, animations, title screen

---

## Resources

- [GBDK-2020 Documentation](https://gbdk-2020.github.io/gbdk-2020/docs/api/)
- [Pan Docs](https://gbdev.io/pandocs/) - Game Boy technical reference
- [GB Dev Wiki](https://gbdev.gg8.se/wiki/)
- [Awesome Game Boy Development](https://github.com/gbdev/awesome-gbdev)
