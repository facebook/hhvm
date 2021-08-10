#!/bin/bash

set -eu

find "$SRCDIR" -name '*.hhi' | while read -r FILE; do
  OUTPUT_FILE="${FILE#${SRCDIR}}"
  OUTPUT_FILE="${OUT}/${OUTPUT_FILE}"
  mkdir -p "$(dirname "${OUTPUT_FILE}")"
  cp "${FILE}" "${OUTPUT_FILE}"
done
touch "${OUT}/hhi_lib.stamp"
