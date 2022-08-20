#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# stop running if any of these steps fail
set -e
WATCHMAN=$(hg root)/fbcode/watchman

if test ! -d /tmp/watchman-gh-pages ; then
  git clone -b gh-pages git@github.com:facebook/watchman.git /tmp/watchman-gh-pages
fi
cd /tmp/watchman-gh-pages

git checkout -- .
git clean -dfx
git fetch
git rebase origin/gh-pages
cd "$WATCHMAN/oss/website"
docker run --volume "$PWD:/srv/jekyll" --volume "/tmp/watchman-gh-pages:/tmp/jekyll-out" --rm jekyll/jekyll:3 jekyll build -d /tmp/jekyll-out

cd /tmp/watchman-gh-pages
git add --all
git commit -m "update website"
git push origin gh-pages
