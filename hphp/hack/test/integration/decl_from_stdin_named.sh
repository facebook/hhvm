#!/bin/bash

# Regression test for "--identify-function <src"

set -u # terminate upon read of uninitalized variable
set -e # terminate upon non-zero-exit-codes (in case of pipe, only checks at end of pipe)
set -o pipefail # in a pipe, the whole pipe runs, but its exit code is that of the first failure
export PS4='+[$(date +%k:%M:%S)]($(basename ${BASH_SOURCE}):${LINENO}): ${FUNCNAME[0]:+${FUNCNAME[0]}(): }'
trap 'echo "exit code $? at line $LINENO" >&2' ERR

# Parse command-line arguments
hh_client="$1"
srcs_dir="$2"
echo "hh_client=$hh_client"
echo "srcs_dir=$srcs_dir"
test -x "$hh_client" # verify that it's an executable
test -f "$srcs_dir/decl_from_stdin1.php" # verify that files are present
test -f "$srcs_dir/decl_from_stdin2.php"

# New directory for test case. Make it a mercurial dir, and an empty hh project
TEMPDIR="$(mktemp -d)"
cd "$TEMPDIR"
mkdir ".hg"
touch ".hhconfig"
cp "$srcs_dir/decl_from_stdin1.php" .
cp "$srcs_dir/decl_from_stdin2.php" .
cat <<EOF > "hh.conf"
max_workers = 1
EOF
export HH_LOCALCONF_PATH="$TEMPDIR"  # so it uss our hh.conf
export HH_TEST_MODE=1  # avoid writing a bunch of telemetry

# From now on we'll use our own error handling instead of "set -e"
set +e

# Start up hh
timeout 20s "$hh_client"
code=$?
logname=$("$hh_client" --logname)
[[ $code == 7 ]] && { echo "Too many retries; skipping test."; exit 0; }
[[ $code == 124 ]] && { echo "Timeout starting hh; skipping test."; exit 0; }
[[ $code != 0 ]] && { echo "hh exit code $code"; exit "$code"; }

# Repro. This repro also catches a weird case where the second call failed
# only if the first one had previously been run.
results1=$(< "$srcs_dir/decl_from_stdin1.php" "$hh_client" --identify-function 13:9 --stdin-name decl_from_stdin1.php)
code1=$?
echo "RESULTS1: decl_from_stdin1.php"
echo "$results1"
results2=$(< "$srcs_dir/decl_from_stdin2.php" "$hh_client" --identify-function 8:9 --stdin-name decl_from_stdin2.php)
code2=$?
echo "RESULTS2: decl_from_stdin2.php"
echo "$results2"

# Cleanup
"$hh_client" stop > /dev/null 2>&1
rmdir ".hg"
rm ".hhconfig"
rm "hh.conf"
rm "decl_from_stdin1.php"
rm "decl_from_stdin2.php"
cd /
rmdir "$TEMPDIR"

# Exit conditions
[[ $code1 == 0 && $results1 != "" && $code2 == 0 && $results2 != "" ]] && exit 0
cat "$logname"
exit 1
