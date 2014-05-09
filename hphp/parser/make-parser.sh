#!/bin/sh

SED=`which sed`
if [ ! -x "$SED" ]; then
  echo "sed not found" 1>&2
  exit 1
fi

DIR="$( cd "$( dirname "$0" )" && pwd )"

if [ -z "${INSTALL_DIR}" ]; then
  INFILE=${DIR}/hphp.y
  OUTFILE=${DIR}/hphp.tab.cpp
  OUTHEADER=${DIR}/hphp.tab.hpp

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
  BISON_DIR=${EXTERNAL_TOOLS_ROOT}/bison/bison-2.4.1/da39a3e/
  export BISON_PKGDATADIR=${BISON_DIR}/share/bison

  BISON=${BISON_DIR}/bin/bison
fi

$BISON -pCompiler --verbose --locations -d -o${OUTFILE} ${INFILE}
if [ $? -ne 0 ] ; then
  exit 1
fi

# Lots of our logic relies on knowing the shape of the token table. Sadly it is 
# an enum without introspection, so instead make it macros so we can control its
# shape on re-requires of the .hpp file
$SED -i -r \
     -e 's/(T_\w+)\s+=\s+([0-9]+)\s*,?/YYTOKEN(\2, \1)/g' \
     -e "s/\s+enum\s+yytokentype/#ifndef YYTOKEN_MAP\n#define YYTOKEN_MAP enum yytokentype\n#define YYTOKEN(num, name) name = num,\n#endif\n   YYTOKEN_MAP/" \
     -e "s/#ifndef YY_COMPILER_H.*//g" \
     -e "s/# define YY_COMPILER_H.*//g" \
    ${OUTHEADER}

# remove the include guard's #endif (-e doesn't work for this)
$SED -i '$ d' ${OUTHEADER}

# We don't want to rely on the grammar to have a fixed start and end token, so
# lets parse the file and make two macros for the min and max
cat ${OUTHEADER} | grep "^\s\+YYTOKEN(" | head -n 1 | \
    $SED -r -e 's/\s+YYTOKEN.([0-9]+).*/#ifndef YYTOKEN_MIN\n#define YYTOKEN_MIN \1\n#endif/' >> ${OUTHEADER}
cat ${OUTHEADER} | grep "^\s\+YYTOKEN(" | tail -n 1 | \
    $SED -r -e 's/\s+YYTOKEN.([0-9]+).*/#ifndef YYTOKEN_MAX\n#define YYTOKEN_MAX \1\n#endif/' >> ${OUTHEADER}

# Why this is in the .hpp in bison 3 is anybody's guess...
$SED -i \
     -e 's@int Compilerparse.*@@' \
     ${OUTHEADER}

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
     -e "s/\".*hphp.tab.cpp\"/\"hphp.tab.cpp\"/" \
     ${OUTFILE}

# We still want the files in our tree since they are checked in
if [ -n "${INSTALL_DIR}" ]; then
  $SED -i -e "1i// @""generated" $OUTFILE
  $SED -i -e "1i// @""generated" $OUTHEADER
  cp $OUTFILE ${DIR}/hphp.tab.cpp
  cp $OUTHEADER ${DIR}/hphp.tab.hpp
fi
