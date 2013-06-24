#!/bin/bash
#
# The Bash shell script executes a command with a time-out.
# Upon time-out expiration SIGTERM (15) is sent to the process. If the signal
# is blocked, then the subsequent SIGKILL (9) terminates it.
#
# Based on the Bash documentation example.

# Hello Chet,
# please find attached a "little easier"  :-)  to comprehend
# time-out example.  If you find it suitable, feel free to include
# anywhere: the very same logic as in the original examples/scripts, a
# little more transparent implementation to my taste.
#
# Dmitry V Golovashkin <Dmitry.Golovashkin@sas.com>

scriptName="${0##*/}"

declare -i DEFAULT_TIMEOUT=9
declare -i DEFAULT_INTERVAL=1
declare -i DEFAULT_DELAY=1

# Timeout.
declare -i timeout=DEFAULT_TIMEOUT
# Interval between checks if the process is still alive.
declare -i interval=DEFAULT_INTERVAL
# Delay between posting the SIGTERM signal and destroying the process by SIGKILL.
declare -i delay=DEFAULT_DELAY

function printUsage() {
    cat <<EOF

Synopsis
    $scriptName [-t timeout] [-i interval] [-d delay] command
    Execute a command with a time-out.
    Upon time-out expiration SIGTERM (15) is sent to the process. If SIGTERM
    signal is blocked, then the subsequent SIGKILL (9) terminates it.

    -t timeout
        Number of seconds to wait for command completion.
        Default value: $DEFAULT_TIMEOUT seconds.

    -i interval
        Interval between checks if the process is still alive.
        Positive integer, default value: $DEFAULT_INTERVAL seconds.

    -d delay
        Delay between posting the SIGTERM signal and destroying the
        process by SIGKILL. Default value: $DEFAULT_DELAY seconds.

As of today, Bash does not support floating point arithmetic (sleep does),
therefore all delay/time values must be integers.
EOF
}

# Options.
while getopts ":t:i:d:" option; do
    case "$option" in
        t) timeout=$OPTARG ;;
        i) interval=$OPTARG ;;
        d) delay=$OPTARG ;;
        *) printUsage; exit 1 ;;
    esac
done
shift $((OPTIND - 1))

# $# should be at least 1 (the command to execute), however it may be strictly
# greater than 1 if the command itself has options.
if (($# == 0 || interval <= 0)); then
    printUsage
    exit 1
fi

# kill -0 pid   Exit code indicates if a signal may be sent to $pid process.
(
    ((t = timeout))

    while ((t > 0)); do
        sleep $interval
        kill -0 $$ || exit 0
        ((t -= interval))
    done

    # Be nice, post SIGTERM first.
    # The 'exit 0' below will be executed if any preceeding command fails.
    kill -s SIGTERM $$ && kill -0 $$ || exit 0
    sleep $delay
    kill -s SIGKILL $$
) 2> /dev/null &

exec "$@"
