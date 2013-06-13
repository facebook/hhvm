#!/bin/sh

cd $FBCODE_DIR

# fbconfig passes a couple --foo arguments.
shift;
shift;

SYSTEMLIB=$INSTALL_DIR/systemlib.php

echo "<?hh" > ${SYSTEMLIB}
echo -n "// @" >> ${SYSTEMLIB}
echo "generated" >> ${SYSTEMLIB}

for i in $@; do
  if [ ! -f "$i" ]; then
    echo "File $i is in the index, but does not exist" >&2
    exit 1
  fi

  BN=`basename $i`
  BNPHP=`basename $BN .php`
  BNHHAS=`basename $BN .hhas`
  if [ "$BNPHP.php" = "$BN" ]; then
    # First, .php files are included with their open tags stripped
    HEADER=`head -1 $i | cut -c 1-5`
    if [ ! "${HEADER}" = "<?php" ]; then
      echo "Unexpected header in file $i" >&2
      exit 1
    fi
    echo "" >> ${SYSTEMLIB}
    cat $i | grep -v '<?php' >> ${SYSTEMLIB}
  else
    if [ ! "$BNHHAS.hhas" = "$BN" ]; then
      echo "File $i is neither PHP not HHAS source" >&2
      exit 1
    fi
  fi
done

echo "" >> ${SYSTEMLIB}
echo "<?hhas" >> ${SYSTEMLIB}

# Then .hhas files are included en-masse
for i in $@; do
  BN=`basename $i`
  BNHHAS=`basename $BN .hhas`
  if [ "$BNHHAS.hhas" = "$BN" ]; then
    echo "" >> ${SYSTEMLIB}
    cat $i >> ${SYSTEMLIB}
  fi
done
