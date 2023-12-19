#!/bin/bash

# This test makes sure that --ide-find-refs-by-symbol3 works when it uses multiworker

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

root="$(mktemp /tmp/hhtest_repo_XXXX -d)"
tmp="$(mktemp /tmp/hhtest_hh_tmpdir_XXXX -d)"
cd "$root" || err 21 'cannot cd to root'

export HH_TMPDIR="$tmp"
export HH_LOCALCONF_PATH="$tmp"
export HH_TEST_MODE=1

# lay down some files
mkdir ".hg"  # make it a mercurial directory so we get deterministic errors from hg
printf "<?hh\nclass A {}\n" > "A.php"
for name in B C D E F G H I J K; do  # make 10 files reference A -- enough that multiworker will trigger, per threshold in findRefsService.ml
  printf "<?hh\nclass %s extends A {}\n" "$name" > "$name.php"
done
touch ".hhconfig"
cat <<EOF > "hh.conf"
use_mini_state = false
load_state_natively_v4 = false
lazy_decl = true
lazy_parse = true
lazy_init2 = true
max_workers = 1
remote_type_check_enabled = false
EOF

# start hh_server.
"$hh_server" . --daemon 1> "$tmp/stdout.log" 2> "$tmp/stderr.log"
status="$?"
[[ "$status" == 0 ]] || err 50 "Expected hh_server to start with error code 0 not $status"

# run hh_client. It should succeed
"$hh_client" 1> "$tmp/stdout.log" 2> "$tmp/stderr.log"
status="$?"
[[ "$status" == 0 ]] || err 51 "Expected hh_client to exit with error code 0 not $status"

# --find refs to class "A"
"$hh_client" --ide-find-refs-by-symbol3 "A|Class,\A" - "" 1> "$tmp/stdout.log" 2> "$tmp/stderr.log"
status="$?"
[[ "$status" == 0 ]] || err 60 "Expected hh_client --find-class-refs to exit with error code 0 not $status"
< "$tmp/stdout.log" grep -i 'D.php' > /dev/null || err 61 "--find-class-refs results incomplete - $tmp/stdout.log"

"$hh_client" stop

2>&1 echo "success!"
cd "/" || err 90 'cannot go to /'
rm -rf "$root"
rm -rf "$tmp"
exit 0
