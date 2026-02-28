#!/bin/sh
# Update the pinned versions of first-party dependencies
#
# It is likely that patches will need updating - see README.md
set -e
OLD="$1"
NEW="$2"

if [ -z "$1" ] || [ -z "$2" ]; then
  echo "Usage: $0 OLD NEW"
  echo " OLD and NEW should be of the form YYYY.MM.DD.xx (usually 00)"
  exit 0
fi

sed -i.orig -e "s/$OLD/$NEW/g" -- */CMakeLists.txt

egrep --only-matching "https://github.com/.+$NEW.+\.gz" -- */CMakeLists.txt | \
while read -r MATCH; do
  CMAKE_FILE="${MATCH%%:*}"
  URL="${MATCH#*:}"

  FILE="$(mktemp)"
  wget -O "$FILE" "$URL"
  HASH="$(openssl dgst -sha256 "$FILE" | awk '{print $NF}')"
  rm "$FILE"
  sed -i.orig -e 's/SHA[0-9]*=[0-9a-z]*/SHA256='"$HASH/" "$CMAKE_FILE"
done
