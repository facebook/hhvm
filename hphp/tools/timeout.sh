#!/bin/bash

timeout=$1
shift

# Start the script that does the real work.
"$@" &
pid=$!

# Don't let the whole process take longer than 15 seconds, in case
# Phabricator is acting up.
(
    sleep $timeout
    echo -e "\nTimeout after $timeout seconds, giving up" > /dev/stderr
    kill $$
) &
timeout_pid=$!

wait $pid 2>/dev/null
status=$?
kill $timeout_pid 2>/dev/null

exit $status
