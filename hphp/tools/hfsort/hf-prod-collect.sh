#!/bin/bash

if [[ $# -lt 1 ]] ; then
	echo "hf-prod-collect.sh sleep_time"
	exit 0
fi 

SLEEP_TIME=$1
TMPDIR=/tmp/hf-prof

set -x

rm -rf $TMPDIR
mkdir $TMPDIR

HHVM_PID=$(pgrep -o '^hhvm$')

if [[ -z $HHVM_PID ]] ; then
    echo "Error getting hhvm PID"
    exit 1
fi

perf record -ag -e instructions -o /tmp/perf.data -- sleep $SLEEP_TIME
perf script -i /tmp/perf.data -f comm,ip -chhvm | gzip -c > $TMPDIR/perf.pds.gz

nm -S /proc/$HHVM_PID/exe > $TMPDIR/hhvm.nm

pushd $TMPDIR/..

TARFILE=${TMPDIR}.tgz

tar cvzf $TARFILE `basename $TMPDIR`

popd

echo "Generated file $TARFILE"
