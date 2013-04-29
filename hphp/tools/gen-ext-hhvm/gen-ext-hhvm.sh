#!/bin/sh

# $1 Machine architecture (currently: "x64" or "arm")
# $2 Object file to read symbols from
# $3 ext_hhvm.cpp source to generate
# $4 ext_hhvm.h header to generate
readelf -s -W $2 | grep 'FUNC.*GLOBAL' | \
    sed -e 's/^.*DEFAULT[0-9 ]*//' | \
    $HPHP_HOME/hphp/tools/gen-ext-hhvm/gen-ext-hhvm \
    $1 $4 $3 $HPHP_HOME/hphp/idl/*.idl.json

