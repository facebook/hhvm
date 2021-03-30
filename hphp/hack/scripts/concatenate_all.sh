#!/bin/sh

# First arg is implicit with buck
INSTALL_DIR="${1#--install_dir=}"
ROOT="${2#--root=}"
OUTPUT_FILE="${3#--output_file=}"

if [ "${OUTPUT_FILE#/}" = "${OUTPUT_FILE}" ]; then
  OUTPUT_FILE="${INSTALL_DIR}/${OUTPUT_FILE}"
fi
mkdir -p "$(dirname "${OUTPUT_FILE}")"

set -e

(
  find "${ROOT}" -name "*.php" -o -name "*.hack" | while read -r FILE; do
    echo "///// ${FILE} /////"
    if grep -qE "^namespace .*;" "$FILE"; then
      # [?] is a bit strange, but works with both BSD and GNU sed :)
      sed "/^<[?]hh/d" "$FILE" | \
        sed "s/^\(namespace .*\);/\1 {/"
      echo "}"
    else
      sed "/^<[?]hh/d" "$FILE"
    fi
  done
) > "${OUTPUT_FILE}"
