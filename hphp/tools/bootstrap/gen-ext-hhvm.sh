#!/bin/sh

DIR="$( cd "$( dirname "$0" )" && pwd )"

if [ ! -d `dirname $3` ] ; then
   mkdir -p `dirname $3`
fi
 
# $1 Machine architecture (currently: "x64" or "arm")
# $2 Idl file to read definitions from
# $3 ext_hhvm.cpp source to generate
# $4 ext_hhvm.h header to generate
$GEN_EXT_HHVM $1 $4 $3 $2
