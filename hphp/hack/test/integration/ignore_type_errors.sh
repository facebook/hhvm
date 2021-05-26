#!/bin/bash

# test for --ignore-type-errors flag
#
# This test makes sure that type-checking a repository that
# definitely contains a type error in it still produces a valid
# saved state when the --ignore-type-errors flag is passed.

set -u
set -o pipefail

err() {
    local status="$1"
    local msg="$2"
    1>&2 printf '%s\n' "$msg"
    exit "$status"
}

# BEGIN ARGUMENT PARSING
hh_server=INVALID
while [[ "$#" -gt 0 ]]; do
    case "$1" in
        --hh[-_]server)
            shift; hh_server="$1" ;;
        --hh[-_]server=*)
            hh_server="${1#*=}" ;;
        *)
            exit 10 ;;
    esac
    shift
done
[[ "$hh_server" != INVALID ]] || err 10 'hh_server not set'
[[ "$hh_server" != '' ]] || err 10 'hh_server is empty'
test -x "$hh_server" || err 110 'hh_server is not executable'
# END ARGUMENT PARSING

# defensively go to root directory
cd "/" || err 20 'cannot go to /'

# make new directory for test case
tempdir="$(mktemp /tmp/hhtest_repo_XXXX -d)"
test -d "$tempdir" || err 20 'cannot create tempdir'

# We'll use this instead of /tmp/hh_server
hh_tmpdir="$(mktemp /tmp/hhtest_hh_tmpdir_XXXX -d)"
test -d "$hh_tmpdir" || err 23 'cannot create hh_tmpdir'
export HH_TMPDIR="$hh_tmpdir"
export HH_TEST_MODE=1  # avoid writing a bunch of telemetry

# write file with gratuitous type error
cat <<EOF > "$tempdir"/type_error.php
<?hh // strict

function foo() : int {
  return "not an int!";
}
EOF
test -f "$tempdir/type_error.php" || err 30 'cannot write type_error.php'

# touch config directory
touch "$tempdir"/.hhconfig
test -f "$tempdir"/.hhconfig || err 40 'cannot create empty .hhconfig'

# hh_server must not exit with a nonzero exit status DESPITE finding
# type errors (because its job is to produce a saved state)
(
    cd "$tempdir" || exit 50
    "$hh_server" . --gen-saved-ignore-type-errors --save-mini savestate --max-procs 2 --config symbolindex_search_provider=NoIndex \
      1>"$tempdir"/out.log 2>"$tempdir"/err.log
)
status="$?"
[[ "$status" = 1 ]] && \
  err 50 'hh_server must not exit abnormally if there are type errors'
[[ "$status" = 2 ]] && err 50 'trying to overwrite an existing state?'
[[ "$status" = 50 ]] && err 50 'cannot navigate to tempdir'

# saved state file should exist
test -f "$tempdir"/savestate.sql || err 60 'savestate.sql was not created'

# error message should mention the type errors
# mention that we generated a saved state
<"$tempdir"/err.log grep -i 'generat\w* saved state' || err 70 'bad log message'
# mention that we're ignoring type errors
<"$tempdir"/err.log grep -i 'ignor\w* type errors' || err 80 'bad log message'
# write some stuff to stdout that looks like a type error
<"$tempdir"/out.log grep -i '^file.*line.*character' || err 90 'no reported errors on stdout'

2>&1 echo "success!"
rm -fr "$hh_tmpdir"
exit 0
