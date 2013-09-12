#!/bin/sh

SED=`which sed`
if [ ! -x "$SED" ]; then
  echo "sed not found" 1>&2
  exit 1
fi

if [ -z "${INSTALL_DIR}" ]; then
  INFILE=${HPHP_HOME}/hphp/parser/hphp.y
  OUTFILE=${HPHP_HOME}/hphp/parser/hphp.tab.cpp
  OUTHEADER=${HPHP_HOME}/hphp/parser/hphp.tab.hpp

  BISON=`which bison`
  if [ ! -x "$BISON" ]; then
    echo "bison not found" 1>&2
    exit 1
  fi
else
  INFILE=hphp.y
  OUTFILE=${INSTALL_DIR}/hphp.tab.cpp
  OUTHEADER=${INSTALL_DIR}/hphp.tab.hpp

  EXTERNAL_TOOLS_ROOT=`readlink -f ${FBCODE_DIR}/third-party/centos5.2-native/`
  export BISON_PKGDATADIR=\
  ${EXTERNAL_TOOLS_ROOT}/bison/bison-2.4.1/da39a3e/share/bison

  BISON=${EXTERNAL_TOOLS_ROOT}/bison/bison-2.4.1/da39a3e/bin/bison
fi

$BISON -pCompiler --verbose --locations -d -o${OUTFILE} ${INFILE}
if [ $? -ne 0 ] ; then
  exit 1
fi

# Do a bunch of replacements on the generated files. We need to find the min and
# max token numbers and define them in the header, so we convert the yytokentype
# enum to a series of macro invocations, and then find the first and last macro
# invocations.
$SED -i -r -e 's/(T_\w+) = ([0-9]+)\s*,?/YYTOKEN(\2, \1)/g' \
     -e "s/   enum yytokentype/#ifndef YYTOKEN_MAP\n#define YYTOKEN_MAP enum yytokentype\n#define YYTOKEN(num, name) name = num,\n#endif\n   YYTOKEN_MAP/" \
    ${OUTHEADER}

cat ${OUTHEADER} | grep "     YYTOKEN(" | head -n 1 | \
    $SED -r -e 's/     YYTOKEN.([0-9]+).*/#ifndef YYTOKEN_MIN\n#define YYTOKEN_MIN \1\n#endif/' >> ${OUTHEADER}
cat ${OUTHEADER} | grep "     YYTOKEN(" | tail -n 1 | \
    $SED -r -e 's/     YYTOKEN.([0-9]+).*/#ifndef YYTOKEN_MAX\n#define YYTOKEN_MAX \1\n#endif/' >> ${OUTHEADER}

# Renaming some stuff in the cpp file
$SED -i \
     -e "s/first_line/line0/g"   \
     -e "s/last_line/line1/g"     \
     -e "s/first_column/char0/g"  \
     -e "s/last_column/char1/g"   \
     -e "s/union/struct/g"        \
     -e "s/YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));/YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));\n      memset(yyptr, 0, YYSTACK_BYTES (yystacksize));/" \
     -e "s/YYSTACK_RELOCATE (yyvs_alloc, yyvs)/YYSTACK_RELOCATE_RESET (yyvs_alloc, yyvs)/" \
     -e "s/YYSTACK_FREE (yyss)/YYSTACK_FREE (yyss);\n  YYSTACK_CLEANUP/" \
     ${OUTFILE}

