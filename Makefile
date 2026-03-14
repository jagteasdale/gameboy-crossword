# Game Boy Crossword - Makefile
# Requires GBDK-2020: https://github.com/gbdk-2020/gbdk-2020

# GBDK path - adjust this to your installation
GBDK_HOME ?= /opt/gbdk

CC = $(GBDK_HOME)/bin/lcc
PNG2ASSET = $(GBDK_HOME)/bin/png2asset

# Compiler flags
CFLAGS = -Wa-l -Wl-m -Wl-j -Iinclude -Isrc
# MBC5+RAM+BATTERY (0x1B) with 8KB SRAM (1 bank)
LDFLAGS = -Wm-yt0x1B -Wm-ya1

# Project settings
PROJECT_NAME = crossword
ROM_NAME = $(PROJECT_NAME).gb

# Source files
SRC_DIR = src
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)

# Asset files
ASSET_DIR = assets
TILE_SOURCES = $(wildcard $(ASSET_DIR)/tiles/*.c)
SPRITE_SOURCES = $(wildcard $(ASSET_DIR)/sprites/*.c)
FONT_SOURCES = $(wildcard $(ASSET_DIR)/fonts/*.c)
ASSET_OBJECTS = $(TILE_SOURCES:.c=.o) $(SPRITE_SOURCES:.c=.o) $(FONT_SOURCES:.c=.o)

# All objects
ALL_OBJECTS = $(OBJECTS) $(ASSET_OBJECTS)

# Default target
all: $(ROM_NAME)

$(ROM_NAME): $(ALL_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Convert PNG assets to C source
assets: convert_tiles convert_sprites convert_fonts

convert_tiles:
	@echo "Converting tile assets..."
	@for f in $(ASSET_DIR)/tiles/*.png; do \
		[ -f "$$f" ] && $(PNG2ASSET) $$f -c $${f%.png}.c || true; \
	done

convert_sprites:
	@echo "Converting sprite assets..."
	@for f in $(ASSET_DIR)/sprites/*.png; do \
		[ -f "$$f" ] && $(PNG2ASSET) $$f -c $${f%.png}.c -sw 8 -sh 8 || true; \
	done

convert_fonts:
	@echo "Converting font assets..."
	@for f in $(ASSET_DIR)/fonts/*.png; do \
		[ -f "$$f" ] && $(PNG2ASSET) $$f -c $${f%.png}.c || true; \
	done

# Puzzle converter
PUZZLE_CONVERTER = tools/puzzle_converter.py
PUZZLE_DIR = data
PUZZLE_SOURCES = $(wildcard $(PUZZLE_DIR)/*.json)

# Convert JSON puzzles to C source
convert_puzzles:
	@echo "Converting puzzle data..."
	@for f in $(PUZZLE_SOURCES); do \
		echo "  Converting $$f"; \
		python3 $(PUZZLE_CONVERTER) $$f; \
	done

clean:
	rm -f $(ROM_NAME) $(SRC_DIR)/*.o $(SRC_DIR)/*.lst $(SRC_DIR)/*.sym
	rm -f $(ASSET_DIR)/*/*.o $(ASSET_DIR)/*/*.lst
	rm -f *.map *.sym *.noi
	rm -f $(PUZZLE_DIR)/*.c

# Run in emulator (adjust path to your emulator)
run: $(ROM_NAME)
	bgb $(ROM_NAME) &

# Alternative emulators
run-sameboy: $(ROM_NAME)
	sameboy $(ROM_NAME) &

run-mgba: $(ROM_NAME)
	mgba-qt $(ROM_NAME) &

.PHONY: all clean run run-sameboy run-mgba assets convert_tiles convert_sprites convert_fonts convert_puzzles
