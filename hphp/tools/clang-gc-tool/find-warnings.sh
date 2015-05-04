#!/bin/sh

LOGDIR=$1
if [ -z "$LOGDIR" ]; then
    LOGDIR=scan-methods
fi

find $LOGDIR -name "warnings.log" -print0 | xargs -0 cat | grep "warning:" | sort | uniq
