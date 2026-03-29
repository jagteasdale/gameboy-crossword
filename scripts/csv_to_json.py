#!/usr/bin/env python3
"""Convert crossword clues CSV to JSON format."""

import csv
import json
import sys

def convert_csv_to_json(csv_path):
    across = []
    down = []

    with open(csv_path, 'r', encoding='utf-8') as f:
        reader = csv.reader(f)
        for row in reader:
            if len(row) < 4:
                continue
            direction, number, answer, clue = row[0], row[1], row[2], row[3]
            entry = {
                "number": int(number),
                "clue": clue,
                "answer": answer.upper()
            }
            if direction.lower() == "across":
                across.append(entry)
            elif direction.lower() == "down":
                down.append(entry)

    across.sort(key=lambda x: x["number"])
    down.sort(key=lambda x: x["number"])
    result = {"across": across, "down": down}
    return json.dumps(result, indent=2)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <input.csv>", file=sys.stderr)
        sys.exit(1)
    print(convert_csv_to_json(sys.argv[1]))
