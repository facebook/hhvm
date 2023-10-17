#!/bin/bash

# This test makes sure that --find-class-refs still works after file move

set -u

err() {
    local status="$1"
    local msg="$2"
    1>&2 printf '%s\n' "$msg"
    exit "$status"
}


hh_client=$1
hh_server=$2
test -x "$hh_client" || err 10 'hh_client is not executable'
test -x "$hh_server" || err 12 'hh_client is not executable'

# defensively go to root directory
cd "/" || err 15 'cannot go to /'

# make new directory for test case
tempdir="$(mktemp /tmp/hhtest_repo_XXXX -d)"
test -d "$tempdir" || err 20 'cannot create tempdir'
cd "$tempdir" || err 21 'cannot change to tempdir'

# Make it a mercurial directory so we get deterministic errors from hg.
# Our attempts to get revision number will be met with "Error getting hg revision:
# ... abort: unknown revision 'master'"
mkdir ".hg"
test -d ".hg" || err 22 'cannot create tempdir/.hg'

# We'll use this instead of /tmp/hh_server
hh_tmpdir="$(mktemp /tmp/hhtest_hh_tmpdir_XXXX -d)"
test -d "$hh_tmpdir" || err 23 'cannot create hh_tmpdir'
export HH_TMPDIR="$hh_tmpdir"

# write files
cat <<EOF > "c.php"
<?hh
class C {
  public static function cf(): void {D::df();}
}
EOF
test -f "c.php" || err 30 'cannot write c.php'

cat <<EOF > "d.php"
<?hh
class D {
  public static function df(): void {C::cf();}
}
EOF
test -f "d.php" || err 31 'cannot write d.php'


# touch config directory
touch ".hhconfig"
test -f ".hhconfig" || err 40 'cannot create empty .hhconfig'

# create hh.conf
cat <<EOF > "hh.conf"
use_mini_state = false
load_state_natively_v4 = false
lazy_decl = true
lazy_parse = true
lazy_init2 = true
max_workers = 1
remote_type_check_enabled = false
EOF
test -f "hh.conf" || err 41 'cannot create hh.conf'

# set environment variable to point to hh.conf
export HH_LOCALCONF_PATH="$tempdir"
export HH_TEST_MODE=1

# start hh_server.
"$hh_server" . --daemon 1>out.log 2>err.log
status="$?"
[[ "$status" == 0 ]] || err 50 "Expected hh_server to start with error code 0 not $status"

# run hh_client. It should succeed
"$hh_client" 1>out.log 2>err.log
status="$?"
[[ "$status" == 0 ]] || err 51 "Expected hh_client to exit with error code 0 not $status"

# --find-class-refs C. It should find 1 result
"$hh_client" --find-class-refs C 1>out.log 2>err.log
status="$?"
[[ "$status" == 0 ]] || err 60 "Expected hh_client --find-class-refs to exit with error code 0 not $status"
<"out.log" grep -i '1 total results' > /dev/null || err 61 "--find-class-refs results incomplete - $tempdir/out.log"

# move a file!
mv d.php e.php || err 70 "failed to move file"

# --find-class-refs C. Current buggy behavior is that it crashes
"$hh_client" --find-class-refs C 1>out.log 2>err.log
status="$?"
[[ "$status" == 0 ]] || err 80 "Expected second hh_client --find-class-refs to exit with error code 0 not $status"
<"out.log" grep -i '1 total results' > /dev/null || err 81 "second --find-class-refs results incomplete - $tempdir/out.log"

"$hh_client" stop # incidentally, "hh_client <dir> stop" doesn't seem to stop it; hence why we did "cd" earlier

2>&1 echo "success!"
cd "/" || err 90 'cannot go to /'
rm -rf "$tempdir"
rm -rf "$hh_tmpdir"
exit 0
