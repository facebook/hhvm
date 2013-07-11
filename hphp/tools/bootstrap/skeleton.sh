#!/bin/sh

# $1 Source IDL file
# $2 (optional) location to store skeleton

if [ -z "$HPHP_HOME" ]; then
  echo "HPHP_HOME dir not set" >&2
  exit 1
fi

HHVM=$HPHP_HOME/hphp/hhvm/hhvm
SOURCE=$1

if [ -z "$2" ]; then
  DESTDIR=$HPHP_HOME/hphp/runtime/ext/
else
  DESTDIR=$2
fi

if [ ! -x "$HHVM" ]; then
  echo "No hhvm binary found in $HHVM, build hhvm first" >&2
  exit 1
fi

if [ ! -f "$SOURCE" ]; then
  echo "Unable to find $SOURCE IDL file" >&2
  exit 1
fi

if [ ! -d "$DESTDIR" ]; then
  echo "$DESTDIR is not a directory" >&2
  exit 1
fi

EXTNAME=`basename $SOURCE .idl.json`
if [ -f "${DESTDIR}/ext_${EXTNAME}.h" -o -f "${DESTDIR}/ext_${EXTNAME}.cpp" ]; then
  echo "Target file(s) already exist" >&2
  exit 1
fi

echo "Creating skeleton for $EXTNAME in $DESTDIR/ext_${EXTNAME}.{cpp,h}"
$HHVM $HPHP_HOME/hphp/system/idl/idl.php cpp ${SOURCE} ${DESTDIR}/ext_${EXTNAME}.h ${DESTDIR}/ext_${EXTNAME}.cpp
