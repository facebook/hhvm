#!/bin/sh

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

  EXTERNAL_TOOLS_ROOT=`readlink -f ${FBCODE_DIR}/third-party/centos5.2-native/`
  FLEX_DIR=${EXTERNAL_TOOLS_ROOT}/flex/flex-2.5.35/
  FLEX=${FLEX_DIR}/bin/flex
fi

$FLEX -i -f -Phphp -R -8 --bison-locations -o${OUTFILE} ${INFILE}
if [ $? -ne 0 ] ; then
  exit 1
fi

# We still want the files in our tree since they are checked in
if [ -n "${INSTALL_DIR}" ]; then

  SED=`which sed`
  if [ ! -x "$SED" ]; then
    echo "sed not found" 1>&2
    exit 1
  fi
  
  $SED -i \
    -e "1i// @""generated" \
    -e "s@/.*lex.yy.cpp@lex.yy.cpp@" \
    $OUTFILE
  cp $OUTFILE ${DIR}/lex.yy.cpp
fi
