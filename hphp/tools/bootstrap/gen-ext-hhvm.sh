#!/bin/sh

DIR="$( cd "$( dirname "$0" )" && pwd )"

# $1 Operating System (currently: "linux" or "darwin")
# $2 Machine architecture (currently: "x64" or "arm")
# $3 Object file to read symbols from
# $4 ext_hhvm.cpp source to generate
# $5 ext_hhvm.h header to generate
if [ "$1" = "darwin" ]; then
  gobjdump --section=".text" --section="LC_SEGMENT..text.hot.built." -t $3 | \
      awk '{ if ($2=="g") print $8 }' | \
      $GEN_EXT_HHVM \
      $2 $5 $4 $DIR/../../system/idl/*.idl.json
else
  readelf -s -W $3 | grep 'FUNC.*GLOBAL' | \
      sed -e 's/^.*DEFAULT[0-9 ]*//' | \
      $GEN_EXT_HHVM \
      $2 $5 $4 $DIR/../../system/idl/*.idl.json
fi
