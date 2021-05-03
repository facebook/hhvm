#!/bin/sh

AUTOLOAD_DB_DIR="${AUTOLOAD_DB_DIR:-.var}"

if [ "${AUTOLOAD_DB_DIR}" = "!mkdtemp" ]; then
  AUTOLOAD_DB_DIR="$(mktemp -dt hhvm-hsl-minitest-XXXXXX)"
fi

if [ -e "hphp/hsl/.hhconfig" ]; then
  cd hphp/hsl || exit 1
fi

if [ -z "$HHVM_BIN" ]; then
  HHVM_BIN="hhvm"

  FBCODE_HHVM="$(dirname "$0")/../../buck-out/dbgo/gen/hphp/hhvm/hhvm/hhvm"
  if [ -x "$FBCODE_HHVM" ]; then
    HHVM_BIN="$FBCODE_HHVM"
  fi
fi

mkdir -p "${AUTOLOAD_DB_DIR}"
exec "${HHVM_BIN}" \
  -vEval.HSLSystemlibEnabled=false \
  -vAutoload.Enabled=true \
  "-vAutoload.DB.Path=${AUTOLOAD_DB_DIR}/autoload.db" \
  "-vAutoload.DBPath=${AUTOLOAD_DB_DIR}/autoload.db" \
  minitest/main.hack "$@"
