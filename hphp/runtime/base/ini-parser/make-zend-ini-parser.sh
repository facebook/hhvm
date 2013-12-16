#!/bin/sh

if [ -z "${INSTALL_DIR}" ]; then
  INFILE=${HPHP_HOME}/hphp/runtime/base/ini-parser/zend-ini.y
  OUTFILE=${HPHP_HOME}/hphp/runtime/base/ini-parser/zend-ini.tab.cpp
  OUTHEADER=${HPHP_HOME}/hphp/runtime/base/ini-parser/zend-ini.tab.hpp

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
