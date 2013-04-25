#!/bin/sh

# $1 Object file to read symbols from
# $2 ext_hhvm.cpp source to generate
# $3 ext_hhvm.h header to generate
readelf -s -W $1 | grep 'FUNC.*GLOBAL' | \
    sed -e 's/^.*DEFAULT[0-9 ]*//' | \
    $HPHP_HOME/hphp/tools/gen-ext-hhvm/gen-ext-hhvm \
    $3 $2 $HPHP_HOME/hphp/idl/*.idl.json

