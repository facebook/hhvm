#!/bin/bash

if echo "$1" | grep -q -e 'install_dir='; then
  # Skip --install_dir
  shift 1
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
  compiler="hg --config trusted.users='*' log -l1 -r'reverse(::.) & file(\"$root/**\")' -T'{branch}-0-g{sub(r\"^\$\",node,mirrornode(\"fbcode\",\"git\"))}\\n' 2> /dev/null"
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
cd "$root" || exit 1

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
      grep -v '^hphp/\(bin\|hphp\|neo\|public_tld\|test\|tools\|util\|vixl\|zend\)' | \
      tr '\n' '\0' | xargs -0 cat | sha1sum | cut -b-40)
fi

################################################################################

# Hash the executable and a few other shared objects which will be embedded in
# the binary to form the build-id. Unlike the others, this cannot be overridden
# and uniquely identifies a particular binary. Skip if the script wasn't passed
# any files
if [[ $# -eq 0 ]] ; then
    BUILD_ID="UNKNOWN"
else
    args=$*
    BUILD_ID=$(sh -c "sha1sum ${args} | cut -d ' ' -f 1 | sha1sum | cut -b-40")
fi

################################################################################

if [ -z "${INSTALL_DIR}" ]; then
  INSTALL_DIR=${DIR}
fi

COMPILER_FILE="${INSTALL_DIR}/generated-compiler-id.txt"
REPO_SCHEMA_FILE="${INSTALL_DIR}/generated-repo-schema-id.txt"
BUILD_ID_FILE="${INSTALL_DIR}/generated-build-id.txt"

echo -n "${COMPILER_ID}" > "${COMPILER_FILE}"
echo -n "${HHVM_REPO_SCHEMA}" > "${REPO_SCHEMA_FILE}"
echo -n "${BUILD_ID}" > "${BUILD_ID_FILE}"
