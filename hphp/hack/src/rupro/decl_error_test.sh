#!/usr/bin/env bash

# Test the error message reported by `decl_file` due to a missing file. Required
# arguments (see `TARGETS`):
#
#    --decl-file=<path>           'decl_file' executable
#    --name-table-builder=<path>  'hh_naming_table_builder' executable

set -uo pipefail

naming_table_builder=${1/#--naming-table-builder=/}
decl_file=${2/#--decl-file=/}

# Get scratch dir
scratch=$(mktemp -d /tmp/rupro-decl-error-test-XXXXX)
function on_exit {
  rm -rf "$scratch"
}
trap on_exit EXIT

# A minimal repo
repo="$scratch/repo"
mkdir -p "$repo" && touch "$repo/.hhconfig"

# Classes A, B, C and D
printf "<?hh\nclass A {}\n"           > "$repo/A.php"
printf "<?hh\nclass B extends A {}\n" > "$repo/B.php"
printf "<?hh\nclass C extends B {}\n" > "$repo/C.php"
printf "<?hh\nclass D extends C {}\n" > "$repo/D.php"

# A naming table
naming_table="$scratch/repo.sql"
"$naming_table_builder" --www "$repo" --output "$naming_table" --use-direct-decl-parser > /dev/null 2>&1

# Try to decl 'D.php'
"$decl_file" --root "$repo" --naming-table "$naming_table" --folded "$repo/D.php" 2>/dev/null 1>"$scratch/D.decl.actual"
good_decl_exit_code="$?"
actual=$(< "$scratch/D.decl.actual")
if [[ "$good_decl_exit_code" == 0 && "$actual" =~ ^Some.* ]]; then
    good_decl_pass=1
else
    good_decl_pass=0
fi

# Remove 'A.php'
rm "$scratch/repo/A.php"

# Try to decl 'D.php'
"$decl_file" --root "$repo" --naming-table "$naming_table" --folded "$repo/D.php" 1>/dev/null 2>"$scratch/D.decl.actual"
bad_decl_exit_code="$?"
actual=$(grep -v "^Build*" < "$scratch/D.decl.actual") # Filter buck2 "Build ID:" line
# This is the error we expect
expect="Error: Failed to declare \\D because of error in ancestor \\A (via \\C, \\B, \\A): Failed to parse decls in root|A.php: No such file or directory (os error 2)"
if [[ "$bad_decl_exit_code" == 1 && "$expect" == "$actual" ]]; then
    bad_decl_pass=1
else
    bad_decl_pass=0
fi

# Check:
#   - before removing A.php
#     -'decl_file' exited with status code 0
#     - we successfully declared D
#   - after removing A.php
#     - 'decl_file' exited with status code 1
#     - the actual output matches the expected
if [[ "$good_decl_pass" == 1 && "$bad_decl_pass" == 1 ]]; then
    echo "Passed"
    exit 0
else
    echo "Failed"
    exit 1
fi
