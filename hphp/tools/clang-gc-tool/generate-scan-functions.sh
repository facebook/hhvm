#!/bin/bash

#
# This script is used to generate GC scan methods for a particular directory.
# It works by scraping a dump of an fbcode/hphp build log to get compiler
# flags to pass to clang for each .cpp file under fbcode/hphp.
# The generated files go into a parallel directory hierarchy rooted at
# $DIR/scan-methods (where $DIR is usually hphp).
# Any warning messages are stored in $DIR/scan-methods/all-warnings.log.
#

CURRDIR="$( cd "$( dirname "$0" )" && pwd )"

ROOT=$CURRDIR/../../..

LOG_FILE="clang-build.log.gz"
declare -i NUM_JOBS=64  # hacky way to run jobs in parallel.
DO_SETUP=0
RM_OLD_SCANS=1
VERBOSE=0
PLUGIN_VERBOSE=""
FILE_REGEX='.*'
PLUGIN_DLL=$(find "$ROOT"/_build/*/hphp/tools/clang-gc-tool -name "libclang-gc-tool.so" | head -1)
USE_XARGS=""
USE_PARALLEL=""

# hack to process/skip options passed in by fbconfig
while [[ "$1" == "--"* ]]; do
    if [[ "$1" == "--install_dir="* ]]; then
        OUTDIR=$(echo "$1" | cut -d '=' -f 2)
        MODE=$(echo "$CONFIG_INSTALL_DIR" | perl -pe "s@.*_build/([^/]+)/.*@\1@")
    fi
    shift;
done

while getopts hl:j:srkvf:p:d:m:x OPT; do
    case "$OPT" in
        h)
            echo "-h         help"
            echo "-l <file>  log file ($LOG_FILE)"
            echo "-j <n>     number of parallel jobs ($NUM_JOBS)"
            echo "-r         remove old scan-methods directory (yes)"
            echo "-k         keep old scan-methods directory (no)"
            echo "-s         generate initial log file (no)"
            echo "-f <regex> filter for build log ($FILE_REGEX)"
            echo "-p <path>  path to plugin ($PLUGIN_DLL)"
            echo "-d <dir>   source directory to generate scan functions for"
            echo "-m <mode>  override build mode ($MODE)"
            echo "-x         use xargs"
            echo "-v         verbose (no)"
            exit 0
            ;;
        l)
            LOG_FILE=$OPTARG
            ;;
        j)
            NUM_JOBS=$OPTARG
            ;;
        s)
            DO_SETUP=1
            ;;
        r)
            RM_OLD_SCANS=1
            ;;
        k)
            RM_OLD_SCANS=0
            ;;
        f)
            FILE_REGEX=$OPTARG
            ;;
        d)
            DIR=$OPTARG
            ;;
        m)
            MODE=$OPTARG
            ;;
        x)
            USE_XARGS=1
            ;;
        v)
            VERBOSE=1
            PLUGIN_VERBOSE="-plugin-arg-add-scan-methods -v"
            ;;
    esac
done

shift $((OPTIND - 1))

if [[ $DO_SETUP == "1" ]]; then
    LOG_FILE='clang-build.log'
    echo "Doing first time setup (fbmake clean && fbconfig --clang --with-project-version=clang:dev -r hphp && fbmake --verbose dbg -k > $CURRDIR/$LOG_FILE)."
    (cd "$ROOT" && rm -f "$CURRDIR/$LOG_FILE" && fbmake clean && fbconfig --clang --with-project-version=clang:dev --extra-cxxflags=-Wno-inconsistent-missing-override -r hphp && (fbmake --verbose dbg -j"$NUM_JOBS" -k > "$CURRDIR/$LOG_FILE"))
    (cd "$ROOT" && gzip -f --best "$CURRDIR/$LOG_FILE")
    LOG_FILE='clang-build.log.gz'
    if [[ "$?" != "0" ]]; then
        echo "Error generating build log.  See $CURRDIR/$LOG_FILE for errors."
        exit 1
    fi
    PLUGIN_DLL=$(find "$ROOT"/_build/*/hphp/tools/clang-gc-tool -name "libclang-gc-tool.so" | head -1)
else
    if [ ! -f "$LOG_FILE" ]; then
        echo "Must provide a valid build log."
        exit 1
    fi
fi

if [ -z "$DIR" ]; then
    if [ -z "$1" ]; then
        echo "No directory specified."
        exit 1
    fi
    DIR=$1
fi

if [[ -z "$MODE" ]]; then
    MODE=$(echo "$PLUGIN_DLL" | perl -pe "s@.*_build/([^/]+)/.*@\1@")
fi

if [ -z "$OUTDIR" ]; then
    OUTDIR=$(find "$ROOT"/_build/$MODE/$DIR/scan-methods/*/generated-install -name "*" | head -1)
fi

BUILD_ID=$(echo "$OUTDIR" | perl -pe "s@.*scan-methods/([^/]+)/.*@\1@")

ALL_SCAN_H=$(dirname "$OUTDIR")/all-scan.h
ALL_SCAN_H2="$OUTDIR/all-scan.h"
ALL_SCAN_DECL_H=$(dirname "$OUTDIR")/all-scan-decl.h
ALL_SCAN_DECL_H2="$OUTDIR/all-scan-decl.h"

#if [[ $VERBOSE == "1" ]]; then
    echo "Scan function output directory: $OUTDIR"
#fi
rm -f "$OUTDIR/warnings.log" "$OUTDIR/all-warnings.log" "$ALL_SCAN_H" "$ALL_SCAN_H2" "$ALL_SCAN_DECL_H" "$ALL_SCAN_H2"
if [[ $RM_OLD_SCANS == "1" ]]; then
    echo "Removing $OUTDIR"
    rm -rf "$OUTDIR"
    mkdir -p "$OUTDIR"
fi
touch "$ALL_SCAN_H" "$ALL_SCAN_H2" "$ALL_SCAN_DECL_H" "$ALL_SCAN_H2"

LOG_DISTCC_REXP='(?:[^/]+/)*log_distcc_stats'
DISTCC_REXP='(?:[^/]+/)*distcc'
CCACHE_REXP='/(?:[^/]+/)*ccache'
#CLANG_REXP='((?:[^/]+/)*clang(?:\+\+)?)'
CLANG_REXP='(?:[^/]+/)*(clang(?:\+\+)?)'
BIN_REXP='(?:[^/]+/)+?bin\s'
COMPILE_REXP="s|^.*cpp_compile$DISTCC_REXP$LOG_DISTCC_REXP.$CCACHE_REXP$CLANG_REXP$BIN_REXP|$CURRDIR/\1|"

OFILE_REXP='s|-o\s([^/]+/)*[^\.]+\.o||'

BADARGS_REXP="s/(-nostdinc|-c|-fstack-protector|-fno-omit-frame-pointer|-fno-strict-aliasing|-Qunused-arguments|-Wunused-variable|-Woverloaded-virtual|-fPIC|-MMD|-MF\s\'[^\']+?\')(\s)/\2/g"
SWIZZLE_DS1_REXP="s|\'-D([\w]+)=<([^\']+)>\'|-D\1=<\2>|g"
SWIZZLE_DS2_REXP="s|\'-D([\w]+)=([^\']+)\'|-D\1=\2|g"
SWIZZLE_IS_REXP="s|\'-I([^\']+)\'|-I\1|g"

RESULTS=$(mktemp)

LOG_FILE_BASENAME=$(basename "$LOG_FILE")
LOG_FILE_EXT="${LOG_FILE_BASENAME##*.}"

if [[ $LOG_FILE_EXT == "gz" ]]; then
    CAT=zcat
else
    CAT=cat
fi

# Massage log command line to extra compile commands for each file.
"$CAT" "$LOG_FILE" | grep cpp_compile | grep -v "heap-trace.cpp" | grep -P "$FILE_REGEX" | perl -pe "$COMPILE_REXP" | perl -pe "$OFILE_REXP" | perl -pe "$BADARGS_REXP" | perl -pe "$SWIZZLE_DS1_REXP" | perl -pe "$SWIZZLE_DS2_REXP" | perl -pe "$SWIZZLE_IS_REXP" > "$RESULTS"

MORE_ARGS="-x c++ -fexceptions -fcxx-exceptions -Wno-unused-variable -Wno-inconsistent-missing-override -Wno-deprecated-declarations -Wno-c++14-extensions -w"

if [ -n "$USE_XARGS" ]; then
    echo "Using xargs."
    xargs -P"$NUM_JOBS" -L1 ./generate-scan-functions-single.sh "$ROOT" "$DIR" "$OUTDIR" "$PLUGIN_DLL" "$VERBOSE" < "$RESULTS" # >> "$OUTDIR/warnings.log"

elif [ -n "$USE_PARALLEL" ]; then
    echo "Using parallel."
    HOSTS=$(mktemp)
    distcc --show-hosts | perl -pe "s|([^/]*)(?:/([0-9]+))?|\2/\1|" > "$HOSTS"
    # --slf $HOSTS
    parallel --gnu --colsep ' ' -P"$NUM_JOBS" -L1 ./generate-scan-functions-single.sh "$ROOT" "$DIR" "$OUTDIR" "$PLUGIN_DLL" "$VERBOSE" < "$RESULTS" # >> "$OUTDIR/warnings.log"

else
    declare -i I=0
    while read COMPILE; do
        read -a COMPILE_ARGS <<< "$COMPILE"
        if [ -n "1" ]; then
            ./generate-scan-functions-single.sh "$ROOT" "$DIR" "$OUTDIR" "$PLUGIN_DLL" "$VERBOSE" "${COMPILE_ARGS[@]}" &
        else
            NUM_ARGS=${#COMPILE_ARGS[@]}
            FILE=${COMPILE_ARGS[${NUM_ARGS} - 1]}
            if [[ $FILE != "$DIR/"* ]]; then
                # ignore files outside specified directory.
                continue
            fi
            FULLNAME=$(basename "$FILE")
            EXTENSION="${FULLNAME##*.}"
            if [[ "$EXTENSION" != "cpp" && "$EXTENSION" != "cc" ]]; then
                # ignore everything but C++ files.
                continue
            fi
            CLANG=${COMPILE_ARGS[0]}
            COMPILE_ARGS=("${COMPILE_ARGS[@]:1}")
            NUM_ARGS=${#COMPILE_ARGS[@]}
            COMPILE_ARGS[${NUM_ARGS} - 1]=$FILE
            CLANG_CMD="$CLANG -cc1 -load $PLUGIN_DLL -plugin add-scan-methods -plugin-arg-add-scan-methods $OUTDIR $PLUGIN_VERBOSE $MORE_ARGS ${COMPILE_ARGS[@]}"
            if [[ $VERBOSE == "1" ]]; then
                echo "$CLANG_CMD"
            else
                echo -n "."
            fi
            (cd "$ROOT" && $CLANG_CMD >> "$OUTDIR/warnings.log") &
        fi
        I+=1
        if (( I % NUM_JOBS == 0 )); then
            wait
        fi
    done < "$RESULTS"
    wait
fi

# Generate master scan file.
if [[ $VERBOSE == "1" ]]; then
    echo
    echo "Master scan files: $ALL_SCAN_H $ALL_SCAN_H2"
fi

# actual scan methods
PROLOG=$(printf '// This file is automatically generated.  Do not edit.\n#ifndef incl_HPHP_ALL_SCAN_H\n#define incl_HPHP_ALL_SCAN_H\n#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend.h"\n#define incl_HPHP_ZEND_PHP_CONFIG_H_\n')
EPILOG="#endif"
FMT='#include "_build/'$MODE'/'$DIR'/scan-methods/'$BUILD_ID'/generated-install/%P"\n'
(cd "$OUTDIR" && echo "$PROLOG" > "$ALL_SCAN_H" && (find . -name "*-scan.h" ! -name "all-scan*.h" -printf "$FMT" | sort) >> "$ALL_SCAN_H" && echo "$EPILOG" >> "$ALL_SCAN_H")
cp -f "$ALL_SCAN_H" "$ALL_SCAN_H2"

# declarations for scan methods
PROLOG=$(printf '// This file is automatically generated.  Do not edit.\n#ifndef incl_HPHP_ALL_SCAN_DECL_H\n#define incl_HPHP_ALL_SCAN_DECL_H\n')
EPILOG="#endif"
FMT='#include "_build/'$MODE'/'$DIR'/scan-methods/'$BUILD_ID'/generated-install/%P"\n'
(cd "$OUTDIR" && echo "$PROLOG" > "$ALL_SCAN_DECL_H" && (find . -name "*-decl.h" ! -name "all-scan*.h" -printf "$FMT" | sort) >> "$ALL_SCAN_DECL_H" && echo "$EPILOG" >> "$ALL_SCAN_DECL_H")
cp -f "$ALL_SCAN_DECL_H" "$ALL_SCAN_DECL_H2"

# collect up and uniquify warnings
"$CURRDIR"/find-warnings.sh "$OUTDIR" > "$OUTDIR/all-warnings.log"
