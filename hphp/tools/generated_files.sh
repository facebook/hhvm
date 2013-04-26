#!/bin/sh

check_err()
{
  if [ "$1" -ne "0" ]; then
    echo "ERROR # $1 : $2" 1>&2
    exit $1
  fi
}

[ -z "$HPHP_HOME" ] && check_err 1 "HPHP_HOME environment variable not set"

VERBOSE=1

HHVM=$HPHP_HOME/hphp/hhvm/hhvm
if [ -x "$HHVM" ]; then
  export HHVM_SYSTEMLIB=$HPHP_HOME/bin/systemlib.php
else
  HHVM=`which hhvm`
  export HHVM_SYSTEMLIB=`dirname $HHVM`/systemlib.php
fi

[ ! -x "$HHVM" ] && check_err 1 "$HHVM is not executable"

HPHP_TOOLS=$HPHP_HOME/hphp/tools/

if [ "$1" = "help" ]; then
  echo "$0 systemlib  - Build bin/systemlib.php"
  echo "$0 constants  - Build hphp/system/constants.h"
  echo "$0 class_map  - Build hphp/system/class_map.cpp"
  echo "$0 lexer      - Regenerate the lexer"
  echo "$0 parser     - Regenerate the parser"
  echo "$0 license    - Add license headers to all files"
  echo ""
  echo "$0 all  - All of the above in listed order"
  exit 0
fi

if [ "$1" = "systemlib" -o "$1" = "all" ]; then
  cd $HPHP_HOME
  [ $VERBOSE -eq 1 ] && echo "Generating bin/systemlib.php"
  $HHVM hphp/system/lib/gen_systemlib.php bin/
  check_err $? "Failed generating bin/systemlib.php"
fi

if [ "$1" = "constants" -o "$1" = "all" ]; then
  cd $HPHP_HOME
  [ $VERBOSE -eq 1 ] && echo "Generating hphp/system/constants.h"
  $HHVM hphp/idl/class_map.php hphp/system/constants.h hphp/system/globals/constdef.json
fi

if [ "$1" = "class_map" -o "$1" = "all" ]; then
  cd $HPHP_HOME
  [ $VERBOSE -eq 1 ] && echo "Generating hphp/system/class_map.h"
  $HHVM hphp/idl/class_map.php hphp/system/class_map.cpp hphp/system/globals/constdef.json \
	`find hphp/idl -name '*.idl.json'`
fi

if [ "$1" = "lexer" -o "$1" = "all" ]; then
  cd $HPHP_HOME/hphp/util/parser
  [ $VERBOSE -eq 1 ] && echo "Generating lexer"
  FLEX=`which flex`
  if [ -x "$FLEX" ]; then
    $FLEX -i -f -Phphp -R -8 --bison-locations -o lex.yy.cpp hphp.ll
    check_err $? "Failed generating lexer"
  else
    echo "No flex with which to generate lexer"
  fi
fi

if [ "$1" = "parser" -o "$1" = "all" ]; then
  cd $HPHP_HOME/hphp/util/parser
  BISON=`which bison`
  SED=`which sed`
  if [ -x "$BISON" -a -x "$SED" ]; then
    [ $VERBOSE -eq 1 ] && echo "Generating parser"
    $BISON -pCompiler --verbose --locations -d -onew_hphp.tab.cpp hphp.y
    rm -f new_hphp.output
    cat new_hphp.tab.hpp | \
      $SED -E "s/(T_\w+) = ([0-9]+)/YYTOKEN(\\2, \\1)/" | \
      $SED -E "{
                N
                s/\{([ \r\n\t]+YYTOKEN\(([0-9]+),)/{\n#ifndef YYTOKEN_MIN\n#define YYTOKEN_MIN \\2\n#endif\\1/
               }" | \
      $SED -E "{
                N
                s/(YYTOKEN\(([0-9]+), T_\w+\)[ \r\n\t]+\};)/\\1\n#ifndef YYTOKEN_MAX\n#define YYTOKEN_MAX \\2\n#endif\n/
               }" | \
      $SED -E "s/   enum yytokentype/#ifndef YYTOKEN_MAP\n#define YYTOKEN_MAP enum yytokentype\n#define YYTOKEN(num, name) name = num\n#endif\n   YYTOKEN_MAP/" \
      > new_hphp.tab.hpp.tmp
    rm new_hphp.tab.hpp
    (diff -q hphp.tab.hpp new_hphp.tab.hpp.tmp >/dev/null ||
      mv -f new_hphp.tab.hpp.tmp hphp.tab.hpp) &&
      rm -f new_hphp.tab.hpp.tmp
    cat new_hphp.tab.cpp | \
      $SED -e "s/first_line/line0/" \
           -e "s/last_line/line1/" \
           -e "s/first_column/char0/" \
           -e "s/last_column/char1/" \
           -e "s/union/struct/" \
           -e "s/YYSTACK_ALLOC \(YYSTACK_BYTES \(yystacksize\)\);\n/YYSTACK_ALLOC \(YYSTACK_BYTES \(yystacksize\)\);\n        memset(yyptr, 0, YYSTACK_BYTES (yystacksize));\n/" \
           -e "s/YYSTACK_RELOCATE \(yyvs_alloc, yyvs\)/YYSTACK_RELOCATE_RESET (yyvs_alloc, yyvs)/" \
           -e "s/YYSTACK_FREE \(yyss\)/YYSTACK_FREE (yyss);\n  YYSTACK_CLEANUP/" \
      > new_hphp.tab.cpp.tmp
    rm new_hphp.tab.cpp
    (diff -q new_hphp.tab.cpp.tmp ../../compiler/parser/hphp.tab.cpp >/dev/null ||
      mv -f new_hphp.tab.cpp.tmp ../../compiler/parser/hphp.tab.cpp) &&
      rm -f new_hphp.tab.cpp.tmp
  else
    [ $VERBOSE -eq 1 ] && echo "No bison/sed with which to generate parser"
  fi
fi

if [ "$1" = "license" -o "$1" = "all" ]; then
  cd $HPHP_HOME
  [ $VERBOSE -eq 1 ] && echo "Updating license headers"
  # TODO: At the moment, license.php fails on PCRE_ERROR_MATCHLIMIT
  # Fix that script then change this to detect errors properly
  $HHVM $HPHP_TOOLS/license.php 2>&1 | grep -v PCRE_ERROR_MATCHLIMIT
  #check_err $? "Failed updating license headers"
fi
