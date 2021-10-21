#!/bin/sh
#
#  Copyright (c) 2015-present, Facebook, Inc.
#  All rights reserved.
#
#  This source code is licensed under the MIT license found in the
#  LICENSE file in the root directory of this source tree.
#

# Applies a patch series maintained with the `quilt` tool
#
# The format is straightforward: if you have `foo.patch` and `bar.patch`,
# applied in that order, `patches/` contains:
#
# - bar.patch
# - foo.patch
# - series
#
# The 'series' file contains:
#
# ```
# foo.patch
# bar.patch
# ```
#
# Quilt uses the presence of a patches/ subdir to identify the root, similar
# to how Hack uses `.hhconfig` - so, to use quilt with an out-of-source patch
# dir is a little bit of work:
#
# ```
# $ cd ~/code/hhvm/build/third-party/fb-mysql/bundled_fbmysqlclient-prefix/src/bundled_fbmysqlclient/
# $ ln -s ~/code/hhvm/third-party/fb-mysql/patches
# $ export QUILT_PATCHES=$(pwd)/patches
# ```
#
# The essential commands are `quilt add`, `quilt refresh`, `quilt push`, and
# `quilt pop`.

QUILT_PATCHES="$1"
if [ ! -d "$QUILT_PATCHES" ]; then
  echo "Usage: $0 /path/to/patches"
  exit 1
fi

set -e

if [ -n "$HHVM_TP_QUILT" ]; then
  export QUILT_PATCHES
  echo "$0: using quilt executable: $HHVM_TP_QUILT"
  exec "${HHVM_TP_QUILT}" --quiltrc - push -a
fi

echo "$0: applying patches in series, not using quilt."

cat "$QUILT_PATCHES/series" | while read PATCH_FILE; do
  echo "Applying patch '$PATCH_FILE'..."
  if [ -e ".quilt_$PATCH_FILE.stamp" ]; then
    echo "...skipping, already applied."
  elif patch -p1 --force < "$QUILT_PATCHES/$PATCH_FILE"; then
    touch ".quilt_$PATCH_FILE.stamp"
    echo "... applied patch $PATCH_FILE."
  else
    if patch -p1 --reverse --force --dry-run < "$QUILT_PATCHES/$PATCH_FILE"; then
      echo "Failed to apply, appears to have been merged upstream."
    else
      echo "Failed to apply patch '$PATCH_FILE.'"
    fi
    exit 1
  fi
done
echo "Applied all patches!"
