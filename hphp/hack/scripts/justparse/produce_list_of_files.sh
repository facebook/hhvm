#!/bin/bash
set -u

mkdir ./sort.TEMPDIR ||:

LC_ALL=C; export LC_ALL

find . -name "*.php" | grep -vF "/." | sort -R | head -n 10000 > ./files.txt.TEMP
1>&2 printf "%s\n" "sort is complaining that it can not write to stderr because head interrupted it"
