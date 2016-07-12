#!/bin/bash

unset CDPATH
DIR="$( cd "$( dirname "$0" )" && pwd )"

# If we're using buck, then we'll be in a sandboxed source directory instead of
# the repo.  We want the path to the repo so we can check in the generated
# parser artifacts.
if [ -n "${FBCODE_DIR}" ]; then
  DIR="${FBCODE_DIR}/hphp/parser"
fi

INFILE=hphp.y

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

OUTFILE5=${INSTALL_DIR}/hphp.5.tab.cpp
OUTFILE7=${INSTALL_DIR}/hphp.7.tab.cpp
OUTHEADER5=${INSTALL_DIR}/hphp.5.tab.hpp
OUTHEADER7=${INSTALL_DIR}/hphp.7.tab.hpp
OUTHEADER=${INSTALL_DIR}/hphp.tab.hpp

BISON_OPTS="--verbose --locations -d"

TMP=$(mktemp)

# Use a potentially-scary awk script to split the single hphp.y parser into two
# outputs.  It's actually not that bad.
#  - The idea is that anything between a /* !PHP5_ONLY */ line and a /* !END */
#    line are removed from the PHP7 parser, and vice versa for PHP5.
#  - This is done just by looking for those tokens and setting the "flag".
#  - We make sure to still print things for the removed lines, to keep line
#    numbers consistent to ease debugging.

awk \
  '/!END/{flag=0} flag{print "/* REMOVED */"} /!PHP7_ONLY/{print; flag=1} !flag' \
  "${INFILE}" > "${TMP}"
# shellcheck disable=SC2086
"${BISON}" ${BISON_OPTS} -pCompiler5 "-o${OUTFILE5}" "${TMP}"
if [ $? -ne 0 ]; then
  echo "Bison failed to compile PHP5 parser!"
  exit 1
fi

awk \
  '/!END/{flag=0} flag{print "/* REMOVED */"} /!PHP5_ONLY/{print; flag=1} !flag' \
  "${INFILE}" > "${TMP}"
# shellcheck disable=SC2086
"${BISON}" ${BISON_OPTS} -pCompiler7 "-o${OUTFILE7}" "${TMP}"
if [ $? -ne 0 ] ; then
  echo "Bison failed to compile PHP7 parser!"
  exit 1
fi

rm "${TMP}"

# Remove alpha variance in "#line" directives.
if [ "${INSTALL_DIR}" = "${DIR}" ]; then
  sed -i -e "s#${TMP}#${INFILE}#g" -e "s#${DIR}/##g" "${OUTFILE5}"
  sed -i -e "s#${TMP}#${INFILE}#g" -e "s#${DIR}/##g" "${OUTFILE7}"
else
  sed -i "s#${TMP}#${INFILE}#g" "${OUTFILE5}"
  sed -i "s#${TMP}#${INFILE}#g" "${OUTFILE7}"
fi

sed -i \
    -e 's@int Compiler[57]parse.*@@' \
    -e 's@.*int Compiler[57]debug.*@@' \
    -e "s@#ifndef YY_COMPILER[57]_.*@@g" \
    -e "s@# define YY_COMPILER[57]_.*@@g" \
    -e "s@#endif /\* !YY_COMPILER[57]_.*@@g" \
    "${OUTHEADER5}" "${OUTHEADER7}"

cmp "${OUTHEADER5}" "${OUTHEADER7}"
if [ $? -ne 0 ]; then
  echo "PHP5 and PHP7 headers differ, must be the same tokens"
  exit 1
fi
cp "${OUTHEADER5}" "${OUTHEADER}"

# Lots of our logic relies on knowing the shape of the token table.  Sadly it is
# an enum without introspection, so instead make it macros so we can control its
# shape on re-requires of the .hpp file.
sed -i -r \
    -e 's/(T_\w+)\s+=\s+([0-9]+)\s*,?/YYTOKEN(\2, \1)/g' \
    -e "s/\s+enum\s+yytokentype/#ifndef YYTOKEN_MAP\n#define YYTOKEN_MAP enum yytokentype\n#define YYTOKEN(num, name) name = num,\n#endif\n   YYTOKEN_MAP/" \
    "${OUTHEADER}"

# Remove the include guard's #endif (-e doesn't work for this).
sed -i '$ d' "${OUTHEADER}"

# We don't want to rely on the grammar to have a fixed start and end token, so
# let's parse the file and make two macros for the min and max.
TOKEN_MIN=$(grep "^\s\+YYTOKEN(" "${OUTHEADER}" -m 1 | \
  sed -r -e 's/\s+YYTOKEN.([0-9]+).*/#ifndef YYTOKEN_MIN\n#define YYTOKEN_MIN \1\n#endif/')
TOKEN_MAX=$(grep "^\s\+YYTOKEN(" "${OUTHEADER}" | tail -n 1 | \
  sed -r -e 's/\s+YYTOKEN.([0-9]+).*/#ifndef YYTOKEN_MAX\n#define YYTOKEN_MAX \1\n#endif/')

echo -e "${TOKEN_MIN}\n\n${TOKEN_MAX}" >> "${OUTHEADER}"

# Renaming some stuff in the cpp file.
sed -i \
    -e "s/first_line/line0/g"   \
    -e "s/last_line/line1/g"     \
    -e "s/first_column/char0/g"  \
    -e "s/last_column/char1/g"   \
    -e "s/union/struct/g"        \
    -e "s/YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));/YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));\n      memset(yyptr, 0, YYSTACK_BYTES (yystacksize));/" \
    -e "s/YYSTACK_RELOCATE (yyvs_alloc, yyvs)/YYSTACK_RELOCATE_RESET (yyvs_alloc, yyvs)/" \
    -e "s/YYSTACK_FREE (yyss)/YYSTACK_FREE (yyss);\n  YYSTACK_CLEANUP/" \
    -e "s/\".*hphp\\.\\(.\\)\\.tab\\.cpp\"/\"hphp.\\1.tab.cpp\"/" \
    "${OUTFILE5}" "${OUTFILE7}"

# We still want the files in our tree since they are checked in.
if [ "${INSTALL_DIR}" != "${DIR}" ]; then
  sed -i -e "1i// @""generated" "${OUTFILE5}"
  sed -i -e "1i// @""generated" "${OUTFILE7}"
  sed -i -e "1i// @""generated" "${OUTHEADER}"
  cp "${OUTFILE5}" "${DIR}/hphp.5.tab.cpp"
  cp "${OUTFILE7}" "${DIR}/hphp.7.tab.cpp"
  cp "${OUTHEADER}" "${DIR}/hphp.tab.hpp"
fi
