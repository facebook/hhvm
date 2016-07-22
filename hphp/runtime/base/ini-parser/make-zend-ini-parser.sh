#!/bin/sh

unset CDPATH
DIR="$( cd "$( dirname "$0" )" && pwd )"

# If we're using buck, then we'll be in a sandboxed source directory instead of
# the repo.  We want the path to the repo so we can check in the generated
# parser artifacts.
if [ -n "${FBCODE_DIR}" ]; then
  DIR="${FBCODE_DIR}/hphp/runtime/base/ini-parser"
fi

INFILE=zend-ini.y

if [ -z "${INSTALL_DIR}" ]; then
  INSTALL_DIR="${DIR}"
  BISON=$(which bison)
else
  BISON=$(readlink -f $(ls -t "${FBCODE_DIR}"/third-party2/bison/2.4.1/centos5.2-native/*/bin/bison | head -1))
  BISON_DIR=$(dirname $(dirname "${BISON}"))
  export BISON_PKGDATADIR="${BISON_DIR}/share/bison"
fi

if [ ! -x "${BISON}" ]; then
  echo "bison not found" 1>&2
  exit 1
fi

OUTFILE="${INSTALL_DIR}/zend-ini.tab.cpp"
OUTHEADER="${INSTALL_DIR}/zend-ini.tab.hpp"

"${BISON}" -pini_ --verbose --locations --defines="${OUTHEADER}" -o "${OUTFILE}" "${INFILE}"
if [ $? -ne 0 ]; then
  exit 1
fi

# Renaming some stuff in the cpp file.
sed -i \
    -e "s/\".*zend-ini.tab.cpp\"/\"zend-ini.tab.cpp\"/" \
    "${OUTFILE}"

# We still want the files in our tree since they are checked in.
if [ "${INSTALL_DIR}" != "${DIR}" ]; then
  sed -i -e "1i// @""generated" "${OUTFILE}"
  sed -i -e "1i// @""generated" "${OUTHEADER}"
  cp "${OUTFILE}" "${DIR}/zend-ini.tab.cpp"
  cp "${OUTHEADER}" "${DIR}/zend-ini.tab.hpp"
fi
