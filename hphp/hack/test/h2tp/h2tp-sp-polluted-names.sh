#!/bin/sh
# Demonstrate that h2tp can handle file- and directory-name components
# that contain spaces.

ME_="$(basename $0)"
warn () { printf '%s\n' "$*" >&2; }
die () { warn "$ME_: $*"; exit 1; }

h2tp=$1

dir=

# These trap statements, along with a trap on 0 below, ensure that
# the temporary directory, $dir, is removed upon exit as well as
# upon receipt of any of the listed signals.
for sig_ in 1 2 3 13 15; do
  eval "trap 'exit $(expr $sig_ + 128)' $sig_"
done

# Remove $dir upon regular exit as well as upon
# receipt of any catchable signal.  Preserve exit status.
trap 'st_=$?; rm -rf "$dir"; exit $st_' 0

set -e
dir=$(mktemp -t -d h2tp-spaced-names-XXXXXX)

in_dir="$dir/in bad"
out_dir="$dir/out bad"
f="$in_dir/bad file.php"
mkdir "$in_dir"
printf '%s\n' '<?hh' 'echo("hello");' > "$f"
printf '%s\n' '<?php' 'require_once ($GLOBALS["HACKLIB_ROOT"]);' \
  'echo ("hello");' > $dir/exp-out

$h2tp "$in_dir" "$out_dir" > $dir/stdout
diff -u "$out_dir/bad file.php" $dir/exp-out
