#!/bin/sh
#$1: output directory
#$2: program name
#$3: extra flags, for exmaple, RELEASE=1
   #echo make -j $3 PROJECT_NAME=$2 TIME_LINK=1 -C $1
cp $HPHP_HOME/bin/CMakeLists.base.txt $1/CMakeLists.txt
cd $1
cmake -D PROGRAM_NAME:string=$2 . || exit $?
make $HPHP_MAKE || exit $?
