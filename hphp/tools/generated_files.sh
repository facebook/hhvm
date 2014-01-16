#!/bin/sh

check_err()
{
  if [ "$1" -ne "0" ]; then
    echo "ERROR # $1 : $2" 1>&2
    exit $1
  fi
}

if [ -z "$HPHP_HOME" ]; then
  HPHP_HOME="`dirname $0`/../../"
fi
cd $HPHP_HOME

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
  echo "$0 lexer      - Regenerate the lexer"
  echo "$0 parser     - Regenerate the parser"
  echo "$0 ini-lexer  - Regenerate the INI lexer"
  echo "$0 ini-parser - Regenerate the INI parser"
  echo "$0 license    - Add license headers to all files"
  echo ""
  echo "$0 all  - All of the above in listed order"
  exit 0
fi

if [ "$1" = "lexer" -o "$1" = "all" ]; then
  [ $VERBOSE -eq 1 ] && echo "Generating lexer"
  FLEX=`which flex`
  if [ -x "$FLEX" ]; then
    cd hphp/parser
    $FLEX -i -f -Phphp -R -8 --bison-locations -o lex.yy.cpp hphp.ll
    cd ../..
    check_err $? "Failed generating lexer"
  else
    echo "No flex with which to generate lexer"
  fi
fi

if [ "$1" = "parser" -o "$1" = "all" ]; then
  [ $VERBOSE -eq 1 ] && echo "Generating parser"

  HPHP_HOME=. hphp/parser/make-parser.sh || exit 1
  sed -e 's@^#line \([0-9]*\) ".*/\([^/]*\)"$@#line \1 "\2"@' \
    hphp/parser/hphp.tab.cpp > \
    hphp/compiler/parser/hphp.tab.cpp
fi

if [ "$1" = "ini-lexer" -o "$1" = "all" ]; then
  [ $VERBOSE -eq 1 ] && echo "Generating INI lexer"
  FLEX=`which flex`
  if [ -x "$FLEX" ]; then
    cd hphp/runtime/base/ini-parser
    $FLEX -o zend-ini.yy.cpp zend-ini.ll
    cd ../../../..
    check_err $? "Failed generating lexer"
  else
    echo "No flex with which to generate lexer"
  fi
fi

if [ "$1" = "ini-parser" -o "$1" = "all" ]; then
  [ $VERBOSE -eq 1 ] && echo "Generating INI parser"
  HPHP_HOME=. hphp/runtime/base/ini-parser/make-zend-ini-parser.sh || exit 1
  sed -i -e 's@^#line \([0-9]*\) ".*/\([^/]*\)"$@#line \1 "\2"@' \
    hphp/runtime/base/ini-parser/zend-ini.tab.cpp \
    hphp/runtime/base/ini-parser/zend-ini.tab.hpp
fi

if [ "$1" = "license" -o "$1" = "all" ]; then
  [ $VERBOSE -eq 1 ] && echo "Updating license headers"
  # TODO: At the moment, license.php fails on PCRE_ERROR_MATCHLIMIT
  # Fix that script then change this to detect errors properly
  $HHVM $HPHP_TOOLS/license.php 2>&1 | grep -v PCRE_ERROR_MATCHLIMIT
  #check_err $? "Failed updating license headers"
fi
