#!/bin/bash

if echo "$1" | grep -q -e 'install_dir='; then
  # Skip --install_dir
  shift 1
fi

DIR=$(pwd -P)

# MacOS portability
SHA1SUM=(openssl dgst -sha1 -r)

xargs_args=(bash -c "${SHA1SUM[*]} \"\$@\" | perl -ne 'print if s/^(.{40}).*/\$1/s'" --)
if git rev-parse --show-toplevel >& /dev/null; then
  root=$(git rev-parse --show-toplevel)
  compiler=(git describe --all --long "--abbrev=40" --always)
  find_files=(git ls-files -- hphp)
elif hg root >& /dev/null; then
  root=$(hg root)
  if [ -f "$root/fbcode/.projectid" ]; then
    root="$root/fbcode"
  fi
  compiler=(hg --config 'trusted.users=*' log -r. -T'{branch}-0-g{sub(r"^$",node,mirrornode("fbcode","git"))}\n')
  find_files=(hg files -I hphp/)
  if [[ -d "$root/.eden/root" ]]; then
      xargs_args=(getfattr --only-values -hn user.sha1)
  fi
else
  root=$(dirname "$(readlink -e "${BASH_SOURCE[0]}/../..")")
  compiler=(date +%s_%N)
  find_files=(find hphp -type f '!' -iregex '.*\(~\|#.*\|\.swp\|/tags\|/.bash_history\|/out\)')
fi

################################################################################

unset CDPATH
cd "$root" || exit 1

if [ -z "${COMPILER_ID}" ]; then
  COMPILER_ID=$("${compiler[@]}")
fi

if [ -z "${USE_LOWPTR}" ]; then
  USE_LOWPTR="0"
fi

################################################################################

# Compute a hash that can be used as a unique repo schema identifier.  The
# identifier incorporates the current git revision and local modifications to
# managed files, but it intentionally ignores unmanaged files (even though they
# could conceivably contain source code that meaningfully changes the repo
# schema), because for some work flows the added instability of schema IDs is a
# cure worse than the disease.
if [ -z "${HHVM_REPO_SCHEMA}" ] ; then
    # Use Perl as BSD grep (MacOS) does not support negated groups
    HHVM_REPO_SCHEMA=$("${find_files[@]}" |\
        { perl -ne 'chomp; print "$_\n" if !m#^hphp/(bin|facebook(?!/extensions)|hack(?!/src)|neo|public_tld|test|tools|util|vixl|zend)|(/\.|/test/)# && !-l && -f' | \
        LC_ALL=C sort | xargs -d'\n' "${xargs_args[@]}" ; echo "${USE_LOWPTR}" ; } | "${SHA1SUM[@]}" | cut -b-40)
fi

################################################################################

# Hash the executable and a few other shared objects which will be embedded in
# the binary to form the build-id. Unlike the  others, this cannot be overridden
# and uniquely identifies a particular binary. Skip if the script wasn't passed
# any files
if [[ $# -eq 0 ]] ; then
    BUILD_ID="UNKNOWN"
else
    BUILD_ID=$("${SHA1SUM[@]}" "$@" | cut -d ' ' -f 1 | "${SHA1SUM[@]}" | cut -b-40)
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

if [ -z "${COMPILER_ID}" ]; then
  exit 1
fi
if [ -z "${HHVM_REPO_SCHEMA}" ]; then
  exit 1
fi
if [ -z "${BUILD_ID}" ]; then
  exit 1
fi
