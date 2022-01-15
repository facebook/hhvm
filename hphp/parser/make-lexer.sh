#!/bin/sh

unset CDPATH
DIR="$( cd "$( dirname "$0" )" && pwd )"

INFILE=hphp.ll

if [ -z "${INSTALL_DIR}" ]; then
  # Running manually, not under buck; update files in source tree.
  INSTALL_DIR="${DIR}"
fi

FLEX=$(which flex)
if [ ! -x "${FLEX}" ]; then
  echo "flex not found" 1>&2
  exit 1
fi

OUTFILE="${INSTALL_DIR}/lex.yy.cpp"

"${FLEX}" -i -f -Phphp -R -8 --bison-locations -o "${OUTFILE}" "${INFILE}"
if [ $? -ne 0 ]; then
  exit 1
fi

# Mark the generated lexer as generated and scrub embedded filenames.
sed -i \
    -e "1i// @""generated" \
    -e "s@/.*lex.yy.cpp@lex.yy.cpp@g" \
    -e "s@/.*hphp.ll@hphp.ll@g" \
    "${OUTFILE}"
