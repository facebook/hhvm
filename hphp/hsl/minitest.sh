#!/bin/sh

AUTOLOAD_DB_DIR="${AUTOLOAD_DB_DIR:-.var}"

if [ "${AUTOLOAD_DB_DIR}" = "!mkdtemp" ]; then
  AUTOLOAD_DB_DIR="$(mktemp -dt hhvm-hsl-minitest-XXXXXX)"
fi

if [ -z "$HHVM_BIN" ]; then
  HHVM_BIN="hhvm"
fi

if [ -e "hphp/hsl/.hhconfig" ]; then
  cd hphp/hsl || exit 1
fi

mkdir -p "${AUTOLOAD_DB_DIR}"
exec "${HHVM_BIN}" \
  -vEval.HSLSystemlibEnabled=false \
  -vAutoload.Enabled=true \
  "-vAutoload.DB.Path=${AUTOLOAD_DB_DIR}/autoload.db" \
  "-vEval.CoeffectEnforcementLevels.zoned=0" \
  minitest/main.hack "$@"
