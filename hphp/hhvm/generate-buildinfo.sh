#!/bin/bash

# Hacky way of disabling fbmake's pre_command.
if [ ! -z "${FBMAKE_PRE_COMMAND_OUTDIR}" ]; then
  exit 0
fi

if echo $1 | grep -q -e 'install_dir=' -e 'fbcode_dir=' ; then
  # Skip --install_dir and --fbcode_dir.
  shift 2
fi

DIR=$(pwd -P)

if git rev-parse --show-toplevel >& /dev/null; then
  root=$(git rev-parse --show-toplevel)
  compiler="git describe --all --long --abbrev=40 --always"
  find_files="git ls-files -- hphp"
elif hg root >& /dev/null; then
  root=$(hg root)
  if [ -f "$root/fbcode/.projectid" ]; then
    root="$root/fbcode"
  fi
  compiler="hg --config trusted.users='*' log -l1 -r'reverse(::.) & file(\"$root/**\")' -T'{branch}-0-g{sub(r\"^\$\",node,mirrornode(\"fbcode\",\"git\"))}\n' 2> /dev/null"
  find_files="hg files -I hphp/"
else
  root=$DIR/../../
  # Building outside of a git repo, use system time instead.  This will make the
  # sha appear to change constantly, but without any insight into file state,
  # it's the safest fallback
  compiler='date +%s_%N'
  find_files='find hphp \( -type f -o -type l \) \! -iregex ".*\(~\|#.*\|\.swp\|/tags\|/.bash_history\|/out\)" | LC_ALL=C sort'
fi

################################################################################

unset CDPATH
cd $root || exit 1

if [ -z "${COMPILER_ID}" ]; then
  COMPILER_ID=$(sh -c "$compiler")
fi

################################################################################

# Compute a hash that can be used as a unique repo schema identifier.  The
# identifier incorporates the current git revision and local modifications to
# managed files, but it intentionally ignores unmanaged files (even though they
# could conceivably contain source code that meaningfully changes the repo
# schema), because for some work flows the added instability of schema IDs is a
# cure worse than the disease.
if [ -z "${HHVM_REPO_SCHEMA}" ] ; then
  HHVM_REPO_SCHEMA=$(sh -c "$find_files" | \
      grep -v '^hphp/\(benchmarks\|bin\|hack\|hphp\|neo\|public_tld\|test\|tools\|util\|vixl\|zend\)' | \
      tr '\n' '\0' | xargs -0 cat | sha1sum | cut -b-40)
fi

################################################################################

if [ -z "${INSTALL_DIR}" ]; then
  INSTALL_DIR=${DIR}
fi

COMPILER_FILE="${INSTALL_DIR}/generated-compiler-id.txt"
REPO_SCHEMA_FILE="${INSTALL_DIR}/generated-repo-schema-id.txt"

INPUT=$1
OUTPUT="${INSTALL_DIR}/hhvm"

echo -n "${COMPILER_ID}" > "${COMPILER_FILE}"
echo -n "${HHVM_REPO_SCHEMA}" > "${REPO_SCHEMA_FILE}"
