#!/bin/sh

cd $FBCODE_DIR

# fbconfig passes a couple --foo arguments.
shift;
shift;

SYSTEMLIB=$INSTALL_DIR/systemlib.php

# If we put the line we're generating into this file, then the linter will think
# the generator itself is generated.  Encode it into a variable for safe
# keeping.
AT="@"

echo "<?hh // partial" > ${SYSTEMLIB}
echo "// {$AT}generated" >> ${SYSTEMLIB}

for i in $@; do
  if [ ! -f "$i" ]; then
    echo "File '$i' is in system/php.txt, but does not exist" >&2
    exit 1
  fi

  BN=$(basename $i)
  BNPHP=$(basename $BN .php)
  BNNSPHP=$(basename $BN .ns.php)
  BNHHAS=$(basename $BN .hhas)
  if [ "$BNPHP.php" = "$BN" ]; then
    # First, .php files are included with their open tags stripped.
    if head -1 $i | grep -qv '^<?\(php\|hh\)'; then
      echo "Unexpected header in file '$i'" >&2
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
      echo "File '$i' is neither PHP nor HHAS source" >&2
      exit 1
    fi
  fi
done

echo "" >> ${SYSTEMLIB}
echo "<?hhas" >> ${SYSTEMLIB}

# Then .hhas files are included en-masse.
for i in $@; do
  BN=$(basename $i)
  BNHHAS=$(basename $BN .hhas)
  if [ "$BNHHAS.hhas" = "$BN" ]; then
    echo "" >> ${SYSTEMLIB}
    cat $i >> ${SYSTEMLIB}
  fi
done
