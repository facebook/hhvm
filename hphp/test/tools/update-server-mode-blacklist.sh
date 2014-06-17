#!/bin/bash

# Some PHP constructs modify whole-process state or are not well-suited to be
# tested in a webserver. We prevent these files from being run in server mode
# by creating .noserver files if they contain one of the bad symbols.

cd $(dirname $0)

words="stdin STDIN stdout STDOUT stderr STDERR \
    apc_ setlocale pcntl_ posix_kill umask \
    ob_list_handlers memory_get_usage PHP_SELF \
    php://fd/ proc_nice clearstatcache \
    php_sapi_name libxml_disable_entity_loader \
    headers_sent ob_get_status \
"
for word in $words; do
    # Doing this in parallel is racy, but the absolute worst that can happen is
    # we'll end up with some garbled text inside a .noserver file.
    (
        echo "Looking for $word"
        for file in $(find ../quick ../slow ../zend/good -name '*.php' -exec grep -rl -F $word {} +); do
            if [[ ! -s $file.noserver ]]; then
                echo $file.noserver
                echo "Used banned symbol $word" > $file.noserver
            fi
        done
    ) &
done

wait
