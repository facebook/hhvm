#/bin/sh

HHVM_BIN=$(realpath "$1")
OUT=$2

mkdir -p "${OUT}"
ln -sf "${HHVM_BIN}" "${OUT}/hphp"
ln -sf "${HHVM_BIN}" "${OUT}/php"
ln -sf "${HHVM_BIN}" "${OUT}/hhbbc"
