#!/bin/bash

if [[ $# -lt 1 ]] ; then
    echo "Usage: $(basename $0) TIME_IN_SECONDS"
    echo "        Note: you need to have HHVM_ADMIN_KEY in your environment"
    exit 1
fi

if [ -z "$HHVM_ADMIN_KEY" ] ; then
    echo "ERROR: You need to set HHVM_ADMIN_KEY in you environment."
    exit 1
fi

SLEEP_TIME=$1
TMPDIR=/tmp/tcd
SAMPLE_PERIOD=9973
CYCLE_PERIOD=999983

rm -rf $TMPDIR
mkdir $TMPDIR
rm -f ./perf.pds

for OPTIONS in "-e cycles:pp                   -c $CYCLE_PERIOD"  \
               "-e branch-misses:pp            -c $SAMPLE_PERIOD" \
               "-e L1-icache-load-misses       -c $SAMPLE_PERIOD" \
               "-e L1-dcache-load-misses       -c $SAMPLE_PERIOD" \
               "-e cache-misses                -c $SAMPLE_PERIOD" \
               "-e LLC-store-misses            -c $SAMPLE_PERIOD" \
               "-e iTLB-load-misses            -c $SAMPLE_PERIOD" \
               "-e dTLB-load-misses            -c $SAMPLE_PERIOD" \
               "-e dTLB-store-misses           -c $SAMPLE_PERIOD" ; do
    echo "Collecting for $SLEEP_TIME seconds: $OPTIONS"

    perf record -BN -a $OPTIONS -- sleep $SLEEP_TIME
    perf script --fields comm,event,ip,sym -chhvmworker,hhvmworker.ND0,hhvmworker.ND1,hhvm,worker > tmp.pds 2> /dev/null
    cat tmp.pds >> perf.pds
done

gzip -c perf.pds > $TMPDIR/perf.pds.gz

HHVM_PID=$(pgrep -xn hhvm)

if [ -z "$HHVM_PID" ] ; then
    echo "Error getting hhvm PID"
    exit 1
fi

HHVM_BIN=$(readlink /proc/$HHVM_PID/exe)
CHROOT=$(readlink /proc/$HHVM_PID/root)
LOCAL_REPO_FILE=$CHROOT$(strings /proc/$HHVM_PID/cmdline | grep "^Repo.Path" | cut -d\= -f2)

echo "Reading symbols from $HHVM_BIN..."
nm $HHVM_BIN > $TMPDIR/hhvm.nm

echo "Dumping TC..."
curl -G 'http://localhost:8093/vm-dump-tc' --data-urlencode "auth=$HHVM_ADMIN_KEY"
echo # The output from curl doesn't have a newline at the end
DUMP_DIR=$CHROOT/tmp/

echo "Collecting TC dump and repo files..."
cp $DUMP_DIR/tc_* $TMPDIR
if [ -f "$LOCAL_REPO_FILE" ] ; then
    cp $LOCAL_REPO_FILE $TMPDIR/hhvm.hhbc
fi

echo "TC dump generated at $TMPDIR"
