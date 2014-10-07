#!/bin/sh

cd $FBCODE_DIR

# fbconfig passes a couple --foo arguments.
shift;
shift;

SYSTEMLIB=$INSTALL_DIR/systemlib.php

lib=""
if [ "$1" = "--lib" ] ; then
  lib=1
  shift;
  SYSTEMLIB_ROOT=$INSTALL_DIR/$1
  SYSTEMLIB=$SYSTEMLIB_ROOT.php
  shift;
fi

prefix=""
if [ "$1" = "--rel" ] ; then
  prefix=$INSTALL_DIR/
  shift
fi

# If we put the line we're generating into this file,
# then the linter will think the generator itself
# is generated.  Encode it into a variable for safe keeping.
AT="@"

echo "<?hh" > ${SYSTEMLIB}
echo "// {$AT}generated" >> ${SYSTEMLIB}

for ii in $@; do
  i=$prefix$ii
  if [ ! -f "$i" ]; then
    echo "File $i is in the index, but does not exist" >&2
    exit 1
  fi

  BN=`basename $i`
  BNPHP=`basename $BN .php`
  BNNSPHP=`basename $BN .ns.php`
  BNHHAS=`basename $BN .hhas`
  if [ "$BNPHP.php" = "$BN" ]; then
    # First, .php files are included with their open tags stripped
    if head -1 $i | grep -qv '^<?\(php\|hh\)'; then
      echo "Unexpected header in file $i" >&2
      exit 1
    fi
    echo "" >> ${SYSTEMLIB}
    if [ ! "$BNNSPHP.ns.php" = "$BN" ]; then
      echo "namespace {" >> ${SYSTEMLIB}
    fi
    tail -n +2 $i >> ${SYSTEMLIB}
    if [ ! "$BNNSPHP.ns.php" = "$BN" ]; then
      echo "}" >> ${SYSTEMLIB}
    fi
  else
    if [ ! "$BNHHAS.hhas" = "$BN" ]; then
      echo "File $i is neither PHP nor HHAS source" >&2
      exit 1
    fi
  fi
done

echo "" >> ${SYSTEMLIB}
if [ -n "$lib" ] ; then
  SYSTEMLIB=$SYSTEMLIB_ROOT.hhas
  echo > ${SYSTEMLIB}
else
  echo "<?hhas" >> ${SYSTEMLIB}
fi

# Then .hhas files are included en-masse
for i in $@; do
  BN=`basename $i`
  BNHHAS=`basename $BN .hhas`
  if [ "$BNHHAS.hhas" = "$BN" ]; then
    echo "" >> ${SYSTEMLIB}
    cat $prefix$i >> ${SYSTEMLIB}
  fi
done
