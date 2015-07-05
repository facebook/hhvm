#!/bin/sh

unset CDPATH
DIR="$( cd "$( dirname "$0" )" && pwd )"

if [ -z "${INSTALL_DIR}" ]; then
  INFILE=${DIR}/hphp.ll
  OUTFILE=${DIR}/lex.yy.cpp

  FLEX=`which flex`
  if [ ! -x "$FLEX" ]; then
    echo "flex not found" 1>&2
    exit 1
  fi
else
  INFILE=hphp.ll
  OUTFILE=${INSTALL_DIR}/lex.yy.cpp

  FLEX=$(readlink -f $(ls -t ${FBCODE_DIR}/third-party2/flex/2.5.35/centos5.2-native/*/bin/flex | head -1))
fi

$FLEX -i -f -Phphp -R -8 --bison-locations -o${OUTFILE} ${INFILE}
if [ $? -ne 0 ] ; then
  exit 1
fi

SED=`which sed`
if [ ! -x "$SED" ]; then
  echo "sed not found" 1>&2
  exit 1
fi

$SED -i \
  -e "1i// @""generated" \
  -e "s@/.*lex.yy.cpp@lex.yy.cpp@g" \
  -e "s@/.*hphp.ll@hphp.ll@g" \
  $OUTFILE

# We still want the files in our tree since they are checked in
if [ -n "${INSTALL_DIR}" ]; then
  cp $OUTFILE ${DIR}/lex.yy.cpp
fi
