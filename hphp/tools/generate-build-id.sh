#!/bin/bash

shift 2
project="$1"
root="$2"
out="${INSTALL_DIR}/build-id"
treeroot="${FBCODE_DIR}/$root"

set -e

cd "$FBCODE_DIR" || exit 2;

function digest() {
  sha1sum "$@" | cut -f1 -d' '
}

# if we're under source control,
# find the last commit that changed this tree
if hg root >&/dev/null; then
  if commithash=$(hg log --pager never --limit 1 "$treeroot" -T'{node}'); then
    buildid="$(digest <(hg diff "$treeroot" && echo "$commithash"))"
  else
    echo "could not find latest commit";
    exit 1;
  fi;
elif git rev-parse --show-toplevel >&/dev/null; then
  if commithash=$(git log --pretty=format:%HH -n 1 "$treeroot"); then
    buildid="$(digest <(git diff -- "$treeroot" && echo "$commithash"))"
  else
    echo "could not find latest commit";
    exit 1;
  fi;
else
  echo "Not under source control--trying to hash source files instead" >&2
  # hash all sources files in $treeroot
  # as a sad alternative if we aren't in source control
  buildid=$(find "$treeroot" \
    \(-name ".c" -or -name ".cpp" -or -name ".h" \) \
    -print0 | xargs -0 cat | digest - )
fi;

echo -n "$project-$buildid" >"$out"
