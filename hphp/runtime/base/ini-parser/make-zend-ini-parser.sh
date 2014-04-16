#!/bin/sh

SED=`which sed`
if [ ! -x "$SED" ]; then
  echo "sed not found" 1>&2
  exit 1
fi

DIR="$( cd "$( dirname "$0" )" && pwd )"

if [ -z "${INSTALL_DIR}" ]; then
  INFILE=${DIR}/zend-ini.y
  OUTFILE=${DIR}/zend-ini.tab.cpp
  OUTHEADER=${DIR}/zend-ini.tab.hpp

  BISON=`which bison`
  if [ ! -x "$BISON" ]; then
    echo "bison not found" 1>&2
    exit 1
  fi
else
  echo ${INSTALL_DIR}

  INFILE=zend-ini.y
  OUTFILE=${INSTALL_DIR}/zend-ini.tab.cpp
  OUTHEADER=${INSTALL_DIR}/zend-ini.tab.hpp

  EXTERNAL_TOOLS_ROOT=`readlink -f ${FBCODE_DIR}/third-party/centos5.2-native/`
  BISON_DIR=${EXTERNAL_TOOLS_ROOT}/bison/bison-2.4.1/da39a3e/
  export BISON_PKGDATADIR=${BISON_DIR}/share/bison

  BISON=${BISON_DIR}/bin/bison
fi

$BISON -pini_ --verbose --locations --defines=${OUTHEADER} -o${OUTFILE} ${INFILE}
if [ $? -ne 0 ] ; then
  exit 1
fi
     
# Renaming some stuff in the cpp file
$SED -i \
  -e "s/\".*zend-ini.tab.cpp\"/\"zend-ini.tab.cpp\"/" \
   ${OUTFILE}

# We still want the files in our tree since they are checked in
if [ -n "${INSTALL_DIR}" ]; then
  $SED -i -e "1i// @""generated" $OUTFILE
  $SED -i -e "1i// @""generated" $OUTHEADER
  cp $OUTFILE ${DIR}/zend-ini.tab.cpp
  cp $OUTHEADER ${DIR}/zend-ini.tab.hpp
fi
