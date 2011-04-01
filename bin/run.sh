#!/bin/sh
#$1: output directory
#$2: program name
#$3: extra flags, for exmaple, RELEASE=1
   #echo make -j $3 PROJECT_NAME=$2 TIME_LINK=1 -C $1
cp $HPHP_HOME/bin/CMakeLists.base.txt $1/CMakeLists.txt
cd $1
cmake -D PROGRAM_NAME:string=$2 . || exit $?

if [ -n "$HPHP_VERBOSE" ]; then
  make $MAKEOPTS > /dev/tty || exit $?
else
  make $MAKEOPTS || exit $?
fi
