#!/bin/bash

type -p "pigz" && GZIP=pigz || GZIP=gzip
TMPDIR=/tmp/hf-prof

set -x
rm -rf $TMPDIR
mkdir $TMPDIR

HHVM_PID=${HHVM_PID:-$(pgrep -o '^hhvm$')}

if [[ -z $HHVM_PID ]] ; then
    echo "Error getting hhvm PID"
    exit 1
fi

perf record -BN --no-buffering -ag -e instructions:u -o /tmp/perf.data -- sleep ${SLEEP_TIME:-200}

perf script -i /tmp/perf.data --fields comm,pid,ip | sed -ne "/^[^   ]\+[   ]*$HHVM_PID[   ]*\$/,+2p" | $GZIP -c > $TMPDIR/perf.pds.gz

nm -S ${HHVM_BIN_PATH:-/proc/$HHVM_PID/exe} > $TMPDIR/hhvm.nm

pushd $TMPDIR/..

TARFILE=${TMPDIR}.tgz

tar cvzf $TARFILE `basename $TMPDIR`

popd

echo "Generated file $TARFILE"
