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

if [ "$1" = "license" -o "$1" = "all" ]; then
  cd $HPHP_HOME
  [ $VERBOSE -eq 1 ] && echo "Updating license headers"
  # TODO: At the moment, license.php fails on PCRE_ERROR_MATCHLIMIT
  # Fix that script then change this to detect errors properly
  $HHVM $HPHP_TOOLS/license.php 2>&1 | grep -v PCRE_ERROR_MATCHLIMIT
  #check_err $? "Failed updating license headers"
fi
