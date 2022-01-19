#!/bin/sh

unset CDPATH
DIR="$( cd "$( dirname "$0" )" && pwd )"

INFILE=zend-ini.y

if [ -z "${INSTALL_DIR}" ]; then
  # Running manually, not under buck; update files in source tree.
  INSTALL_DIR="${DIR}"
fi

BISON=$(which bison)
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
GUARD_NAME=$(echo "${INFILE}" | awk '{print toupper($0)}' | sed 's|[./-]|_|g')
sed -i \
    -e "s/\".*zend-ini.tab.cpp\"/\"zend-ini.tab.cpp\"/" \
    -e "s|YY_.*_INCLUDED|YY_YY_${GUARD_NAME}_INCLUDED|g" \
    "${OUTFILE}" \
    "${OUTHEADER}"

# Mark the checked-in files as generated.
if [ "${INSTALL_DIR}" = "${DIR}" ]; then
  sed -i -e "1i// @""generated" "${OUTFILE}"
  sed -i -e "1i// @""generated" "${OUTHEADER}"
fi
