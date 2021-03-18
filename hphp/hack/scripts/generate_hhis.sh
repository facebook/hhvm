#!/bin/sh
set -e

if [ -z "$1" -o -z "$2" -o -z "$3" -o -x "$4" ]; then
  echo "Usage: $0 HH_PARSE_EXECUTABLE SOURCE_DIR OUTPUT_DIR STAMP_FILE"
  exit 1
fi

HH_PARSE="$1"
SOURCE_DIR="${2%/}"
OUTPUT_DIR="${3%/}"
STAMP_FILE="$4"

if [ -e "$OUTPUT_DIR" ]; then
  rm -rf "$OUTPUT_DIR"
fi

FILE_LIST=$(mktemp)

find "$SOURCE_DIR" -type f -name '*.hack' -o -name '*.php' | while read FILE; do
  OUTPUT_FILE="$OUTPUT_DIR/${FILE#${SOURCE_DIR}}"
  OUTPUT_FILE="${OUTPUT_FILE/.php}"
  OUTPUT_FILE="${OUTPUT_FILE/.hack}"
  OUTPUT_FILE="${OUTPUT_FILE}.hhi"
  mkdir -p "$(dirname "$OUTPUT_FILE")"
  "$HH_PARSE" --generate-hhi "$FILE" > "$OUTPUT_FILE"
  echo "$OUTPUT_FILE" >> "$FILE_LIST"
done

mv "$FILE_LIST" "$STAMP_FILE"
