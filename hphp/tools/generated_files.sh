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
[ ! -x "$HHVM" ] && HHVM=`which hhvm`
[ ! -x "$HHVM" ] && check_err 1 "$HHVM is not executable"

HPHP_TOOLS=$HPHP_HOME/hphp/tools/

if [ "$1" = "help" ]; then
  echo "$0 hhvm       - Build hphp/system/runtime/ext/*.ext_hhvm.cpp"
  echo "$0 infotabs   - Build hphp/system/runtime/ext_hhvm/ext_hhvm_infotabs.cpp"
  echo "$0 systemlib  - Build bin/systemlib.php"
  echo "$0 constants  - Build hphp/system/constants.h"
  echo "$0 class_map  - Build hphp/system/class_map.cpp"
  echo "$0 license    - Add license headers to all files"
  echo ""
  echo "$0 all  - All of the above in listed order"
  exit 0
fi

# $1 - Binary to pull symbols from
# $2 - .ext_hhvm.cpp file to build
make_hhvm()
{
  [ ! -f "$1" ] && check_err 1 "No object file to generate $2 from, did you build first?"
  [ $VERBOSE -eq 1 ] && echo "Generating $2"
  $HHVM gen_ext_hhvm.php $2 . /dev/null /dev/null "" $1
  check_err $? "Failed generating $2"
}

if [ "$1" = "hhvm" -o "$1" = "all" ]; then
  cd $HPHP_HOME/hphp/runtime/ext
  SOURCES=`find . -name '*.cpp' | grep -v ext_hhvm | grep -v sep | cut -c 3- | cut -d . -f 1`
  RUNTIME_BUILD=$HPHP_HOME/hphp/CMakeFiles/hphp_runtime_static.dir
  cd $HPHP_HOME/hphp/runtime/ext_hhvm
  if [ -z "$SOURCES" -a $VERBOSE -eq 1 ]; then
    echo "No extensions found while generating *.ext_hhvm.cpp"
  fi
  for i in $SOURCES; do
    make_hhvm $RUNTIME_BUILD/runtime/ext/$i.cpp.o ../ext/$i.ext_hhvm.cpp
  done
  make_hhvm $RUNTIME_BUILD/runtime/base/builtin_functions.cpp.o ../base/builtin_functions.ext_hhvm.cpp
  EXTHHVM_BUILD=$HPHP_HOME/hphp/runtime/ext_hhvm/CMakeFiles/ext_hhvm_static.dir
  make_hhvm $EXTHHVM_BUILD/ext_hhvm_noinline.cpp.o ext_hhvm_noinline.ext_hhvm.cpp
fi

if [ "$1" = "infotabs" -o "$1" = "all" ]; then
  cd $HPHP_HOME/hphp/runtime/ext_hhvm

  SOURCES=`find ../ext -name '*.ext_hhvm.cpp'`
  [ $VERBOSE -eq 1 ] && echo "Generating hphp/runtime/ext_hhvm/ext_hhvm_infotabs.h"
  $HHVM gen_infotabs_header.php ext_hhvm_infotabs.h $SOURCES ../base/builtin_functions.ext_hhvm.cpp ext_hhvm_noinline.ext_hhvm.cpp

  [ $VERBOSE -eq 1 ] && echo "Generating hphp/runtime/ext_hhvm/ext_hhvm_infotabs.cpp"
  $HHVM gen_infotabs.php ext_hhvm_infotabs.cpp . "" "" ""
  check_err $? "Failed generating hphp/runtime/ext_hhvm/ext_hhvm_infotabs.cpp"
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
  $HHVM hphp/idl/class_map.php hphp/system/constants.h hphp/system/globals/constdef.php
fi

if [ "$1" = "class_map" -o "$1" = "all" ]; then
  cd $HPHP_HOME
  [ $VERBOSE -eq 1 ] && echo "Generating hphp/system/class_map.h"
  $HHVM hphp/idl/class_map.php hphp/system/class_map.cpp hphp/system/globals/constdef.php \
	`find hphp/idl -name '*.idl.php'`
fi

if [ "$1" = "license" -o "$1" = "all" ]; then
  cd $HPHP_HOME
  [ $VERBOSE -eq 1 ] && echo "Updating license headers"
  # TODO: At the moment, license.php fails on PCRE_ERROR_MATCHLIMIT
  # Fix that script then change this to detect errors properly
  $HHVM $HPHP_TOOLS/license.php 2>&1 | grep -v PCRE_ERROR_MATCHLIMIT
  #check_err $? "Failed updating license headers"
fi
