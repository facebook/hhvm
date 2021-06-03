#!/bin/bash

TOP_TRANS=100
TOP_FUNCS=20

ulimit -c 0

EVENT_TYPES="cycles
             branch-misses
             L1-icache-misses
             L1-dcache-misses
             cache-misses
             LLC-store-misses
             iTLB-misses
             dTLB-misses"

usage() {
  echo "Usage: $(basename $0) <DIRECTORY_WITH_TC_DUMP> <EVENT_TYPE> [-noperf]"

  echo "  EVENT_TYPE:"
  for EVENT_TYPE in $EVENT_TYPES ; do
    echo "    * $EVENT_TYPE"
  done

  exit 1
}

if [[ $# -lt 2 ]] ; then
  usage
fi

if [ -z "$TC_PRINT" ] ; then
    TC_PRINT=$(which tc-print 2> /dev/null)
fi
if [[ ! -x $TC_PRINT ]] ; then
    echo "ERROR: Didn't find tc-print binary."
    echo "  Make sure tc-print in your PATH or set TC_PRINT to point to it."
    exit 1
fi

SCRIPT_DIR=$(dirname $0)
DIR=$1
EVENT_TYPE=$2

VALID_EVENT_TYPE=0
for CAND_TYPE in $EVENT_TYPES ; do
  if [[ "$EVENT_TYPE" == "$CAND_TYPE"  ]] ; then
    VALID_EVENT_TYPE=1
    break
  fi
done

if [[ $VALID_EVENT_TYPE -eq 0 ]] ; then
  usage
fi

USE_PERF_DATA=1

for OPTION in $@ ; do
  case $OPTION in
    "-noperf"     ) USE_PERF_DATA=0
  esac
done

if [ ! -d "$DIR" ] ; then
    echo "Error: directory $DIR not found"
    exit 1
fi

set -e

if [ $USE_PERF_DATA -eq 1 ] ; then
    PERF_SCRIPT_DATA=$DIR/perf.pds.gz

    PERF_RAW=$DIR/perf.pdr

    if [ ! -f "$PERF_RAW" ] ; then
        if [ ! -f "$PERF_SCRIPT_DATA" ] ; then
            echo "Error: file $PERF_SCRIPT_DATA not found"
            exit 1
        fi
        zcat $PERF_SCRIPT_DATA | $SCRIPT_DIR/../perf-script-raw.py > $PERF_RAW
        if [ $? -ne 0 ] ; then
          echo "Error: output of 'perf script' has an invalid format"
          rm "$PERF_RAW"
          exit 1
        fi
    fi

    sed -i 's/^worker/hhvm/g' "$PERF_RAW"

    PERF_OPTION=" -p $PERF_RAW "
    SUFFIX="-$EVENT_TYPE"
else
    PERF_OPTION=
    SUFFIX=
fi

OUTFILE_TOP_TRANS=$DIR/top$TOP_TRANS-trans$SUFFIX.txt
OUTFILE_TOP_FUNCS=$DIR/top$TOP_FUNCS-funcs$SUFFIX.txt
OUTFILE_BC_STATS=$DIR/bc-stats$SUFFIX.txt
OUTFILE_TOP_FUNCS_SIZE=$DIR/top$TOP_FUNCS-funcs-size.txt

export REPO_FILE=$DIR/hhvm.hhbc

echo "Processing data from $DIR"
echo "Generating bytecode stats, top $TOP_TRANS translations, and top $TOP_FUNCS functions..."
$TC_PRINT -d $DIR -r $REPO_FILE $PERF_OPTION -e $EVENT_TYPE -b > $OUTFILE_BC_STATS &

$TC_PRINT -d $DIR -r $REPO_FILE $PERF_OPTION -t $TOP_TRANS -e $EVENT_TYPE > $OUTFILE_TOP_TRANS &

$TC_PRINT -d $DIR -r $REPO_FILE $PERF_OPTION -T $TOP_FUNCS -e $EVENT_TYPE > $OUTFILE_TOP_FUNCS &

$TC_PRINT -d $DIR -r $REPO_FILE -S $TOP_FUNCS > $OUTFILE_TOP_FUNCS_SIZE &

wait

echo "Generated files
  $OUTFILE_BC_STATS
  $OUTFILE_TOP_TRANS
  $OUTFILE_TOP_FUNCS
  $OUTFILE_TOP_FUNCS_SIZE"


echo "Dumping top $TOP_FUNCS functions' translations and cfgs..."
COUNT=0

function print_func {
    FUNC=$1
    (OUTFILE_SRC=$DIR/func-$FUNC.txt
    $TC_PRINT -d $DIR -r $REPO_FILE $PERF_OPTION -f $FUNC -e $EVENT_TYPE > $OUTFILE_SRC
    echo "   $OUTFILE_SRC") &

    (OUTFILE_CFG=$DIR/func-$FUNC.dot
    $TC_PRINT -d $DIR -r $REPO_FILE $PERF_OPTION -g $FUNC -e $EVENT_TYPE > $OUTFILE_CFG
    echo "   $OUTFILE_CFG") &
}

FUNC_LIST=$DIR/func_list.txt

cat $OUTFILE_TOP_FUNCS | sed '/+/d;/|/d;/^$/d;/FuncId/d' | \
    tail -n $TOP_FUNCS | tr -s ' ' | cut -d ' ' -f 2 > $FUNC_LIST

tail -n $TOP_FUNCS $OUTFILE_TOP_FUNCS_SIZE | cut -f1 | tr -d ' ' \
    >> $FUNC_LIST

for FUNC in $(sort -nu $FUNC_LIST) ; do

    print_func $FUNC

    COUNT=$(($COUNT + 1))
    if [[ $COUNT -eq 5 ]] ; then
        wait
        COUNT=0
    fi
done

wait

echo "Done"
