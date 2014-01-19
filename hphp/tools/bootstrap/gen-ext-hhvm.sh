#!/bin/sh

# $1 Operating System (currently: "linux" or "darwin")
# $2 Machine architecture (currently: "x64" or "arm")
# $3 Object file to read symbols from
# $4 ext_hhvm.cpp source to generate
# $5 ext_hhvm.h header to generate
if [ "$1" = "darwin" ]; then
  gobjdump --section=".text" --section="LC_SEGMENT..text.hot.built." -t $3 | \
      awk '{ if ($2=="g") print $8 }' | \
      $HPHP_HOME/hphp/tools/bootstrap/gen-ext-hhvm \
      $2 $5 $4 $HPHP_HOME/hphp/system/idl/*.idl.json
else
  readelf -s -W $3 | grep 'FUNC.*GLOBAL' | \
      sed -e 's/^.*DEFAULT[0-9 ]*//' | \
      $HPHP_HOME/hphp/tools/bootstrap/gen-ext-hhvm \
      $2 $5 $4 $HPHP_HOME/hphp/system/idl/*.idl.json
fi
