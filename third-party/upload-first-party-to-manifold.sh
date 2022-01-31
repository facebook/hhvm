#!/bin/sh
# MUST BE USED BY AN FB EMPLOYEE ON A MANAGED MACHINE

set -e
VER="$1"

# Assume we're not running this in 2100 or later
if [ "${VER#20*}" = "$VER" ]; then
  echo "Usage: $0 YYYY.MM.DD.xx"
  exit
fi

DOWNLOAD_DIR="$(mktemp -d)"

grep --only-matching -E "https://github.com/.+$VER.+\.gz" -- */CMakeLists.txt | \
while read -r LINE; do
  URL="${LINE#*:}"
  FILE="${URL##*/}"
  # Do we have `vFOO.tar.gz`, or `project-vFOO.tar.gz`? We always need
  # the latter
  if [ "${FILE#v20}" != "$FILE" ]; then
    # https://github.com/ORG/PROJECT/
    PROJECT="$(echo "$URL" | cut -f5 -d/)"
    FILE="$PROJECT-$FILE"
  fi
  wget -O "$DOWNLOAD_DIR/$FILE" "$URL"
  manifold put \
    "$DOWNLOAD_DIR/$FILE" \
    "hhvm_opensource_dependency_cache/flat/$FILE"
done

rm -rf "$DOWNLOAD_DIR"
