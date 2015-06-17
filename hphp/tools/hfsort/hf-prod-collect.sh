#!/bin/bash

type -p "pigz" && GZIP=pigz || GZIP=gzip
TMPDIR=/tmp/hf-prof

set -x
rm -rf $TMPDIR
mkdir $TMPDIR

HHVM_PID=$(pgrep -o '^hhvm$')

if [[ -z $HHVM_PID ]] ; then
    echo "Error getting hhvm PID"
    exit 1
fi

${PERF_PATH}perf record -ag -e instructions -o /tmp/perf.data -- sleep ${SLEEP_TIME:-200}

HHVM_ROOT=$(readlink -e /proc/$HHVM_PID/root)
LOCAL_PID=$(cat $HHVM_ROOT/hphp/sockets/www.pid)
if [[ $LOCAL_PID != $HHVM_PID ]] ; then
    cp $HHVM_ROOT/tmp/perf-$LOCAL_PID.map $HHVM_ROOT/tmp/perf-$HHVM_PID.map
fi

${PERF_PATH}perf script -i /tmp/perf.data -f comm,ip -chhvm | grep -A2 hhvm | sed 's/--/ /' | $GZIP -c > $TMPDIR/perf.pds.gz

nm -S /proc/$HHVM_PID/exe > $TMPDIR/hhvm.nm

pushd $TMPDIR/..

TARFILE=${TMPDIR}.tgz

tar cvzf $TARFILE `basename $TMPDIR`

popd

echo "Generated file $TARFILE"
