#!/bin/bash
set -u # terminate upon read of uninitalized variable
set -e # terminate upon non-zero-exit-codes (in case of pipe, only checks at end of pipe)
set -o pipefail # in a pipe, the whole pipe runs, but its exit code is that of the first failure
trap 'echo "exit code $? at line $LINENO" >&2' ERR

set -x # echo every statement in the script

FBCODE_ROOT="$(dirname "${BASH_SOURCE[0]}")/../../.."

cd "${FBCODE_ROOT}"

buck2 run //hphp/hack/src:generate_full_fidelity
