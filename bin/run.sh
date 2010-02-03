#!/bin/sh
#$1: output directory
#$2: program name
#$3: extra flags, for exmaple, RELEASE=1
if [ ${#SHOW_COMPILE} -gt 0 ] ; then
   echo cp $HPHP_HOME/bin/run.mk $1/Makefile
   echo make -j $3 PROJECT_NAME=$2 TIME_LINK=1 -C $1
fi
cp $HPHP_HOME/bin/run.mk $1/Makefile
make -j $3 PROJECT_NAME=$2 TIME_LINK=1 -C $1
