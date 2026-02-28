/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/IgnoreSet.h"

// The path and everything below it is ignored.
#define FULL_IGNORE 0x1
// The grand-children of the path are ignored, but not the path
// or its direct children.
#define VCS_IGNORE 0x2

namespace watchman {

void IgnoreSet::add(const w_string& path, bool is_vcs_ignore) {
  (is_vcs_ignore ? ignore_vcs : ignore_dirs).insert(path);

  tree.insert(path, is_vcs_ignore ? VCS_IGNORE : FULL_IGNORE);

  if (!is_vcs_ignore) {
    dirs_vec.push_back(path);
  }
}

bool IgnoreSet::isIgnored(const char* path, uint32_t pathlen) const {
  const char* skip_prefix;
  uint32_t len;
  auto leaf = tree.longestMatch((const unsigned char*)path, (int)pathlen);

  if (!leaf) {
    // No entry -> not ignored.
    return false;
  }

  if (pathlen < leaf->key.size()) {
    // We wanted "buil" but matched "build"
    return false;
  }

  if (pathlen == leaf->key.size()) {
    // Exact match.  This is an ignore if we are in FULL_IGNORE,
    // but not in VCS_IGNORE mode.
    return leaf->value == FULL_IGNORE ? true : false;
  }

  // Our input string was longer than the leaf key string.
  // We need to ensure that we observe a directory separator at the
  // character after the common prefix, otherwise we may be falsely
  // matching a sibling entry.
  skip_prefix = path + leaf->key.size();
  len = pathlen - leaf->key.size();

  if (!is_slash(*skip_prefix)) {
    // we wanted "foo/bar" but we matched something like "food"
    // this is not an ignore situation.
    return false;
  }

  if (leaf->value == FULL_IGNORE) {
    // Definitely ignoring this portion of the tree
    return true;
  }

  // we need to apply vcs_ignore style logic to determine if we are ignoring
  // this path.  This devolves to: "is there a '/' character after the end of
  // the leaf key prefix?"

  if (pathlen <= leaf->key.size()) {
    // There can't be a slash after this portion of the tree, therefore
    // this is not ignored.
    return false;
  }

  // Skip over the '/'
  skip_prefix++;
  len--;

#ifndef _WIN32
  // If we find a '/' from this point, we are ignoring this path.
  return memchr(skip_prefix, '/', len) != nullptr;
#else
  // On windows, both '/' and '\' are possible.
  while (len > 0) {
    if (is_slash(*skip_prefix)) {
      return true;
    }
    skip_prefix++;
    len--;
  }
  return false;
#endif
}

bool IgnoreSet::isIgnoreVCS(const w_string& path) const {
  return ignore_vcs.find(path) != ignore_vcs.end();
}

bool IgnoreSet::isIgnoreDir(const w_string& path) const {
  return ignore_dirs.find(path) != ignore_dirs.end();
}

} // namespace watchman
