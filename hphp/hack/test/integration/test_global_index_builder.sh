#!/bin/bash

CMD="$1"
REPO="$2"

NAME=autocomplete
TEMP_REPO="$(mktemp -d)"
TEMP_FILE="${TEMP_REPO}/${NAME}.txt"
touch "${TEMP_FILE}"

if ! "${CMD}" --text "${TEMP_FILE}" "${REPO}"; then
  echo "FAILURE: ${CMD} failed to run"
  exit 1
fi

TEMP_WC="${TEMP_REPO}/${NAME}.wc.txt"
wc "${TEMP_FILE}" | cut -d " " --fields=3,5,6 > "${TEMP_WC}"
if ! diff "${TEMP_WC}" "${REPO}/${NAME}.wc.txt"; then
  echo "FAILURE: different line, word, or byte counts:"
  cat "${TEMP_WC}"
  exit 1
fi

TEMP_SHA1SUM="${TEMP_REPO}/${NAME}.sha1sum.txt"
sha1sum "${TEMP_FILE}" | cut -d " " -f1 > "${TEMP_SHA1SUM}"
if ! diff "${TEMP_SHA1SUM}" "${REPO}/${NAME}.sha1sum.txt"; then
  echo "FAILURE: different SHA-1 checksum:"
  cat "${TEMP_SHA1SUM}"
  exit 1
fi

rm -rf "${TEMP_REPO}"
