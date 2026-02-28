/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/watchman_dir.h"
#include "watchman/watchman_file.h"

void watchman_dir::Deleter::operator()(watchman_file* file) const {
  free_file_node(file);
}

watchman_dir::watchman_dir(w_string name, watchman_dir* parent)
    : name(std::move(name)), parent(parent) {}

w_string watchman_dir::getFullPath() const {
  return getFullPathToChild(w_string_piece());
}

watchman_file* watchman_dir::getChildFile(w_string_piece name_2) const {
  auto it = files.find(name_2);
  if (it == files.end()) {
    return nullptr;
  }
  return it->second.get();
}

watchman_dir* watchman_dir::getChildDir(w_string_piece name_2) const {
  auto it = dirs.find(name_2);
  if (it == dirs.end()) {
    return nullptr;
  }
  return it->second.get();
}

w_string watchman_dir::getFullPathToChild(w_string_piece extra) const {
  uint32_t length = 0;
  if (extra.size()) {
    length = extra.size() + 1 /* separator */;
  }
  for (const watchman_dir* d = this; d; d = d->parent) {
    length += d->name.size() + 1 /* separator OR final NUL terminator */;
  }

  auto* s = watchman::StringHeader::alloc(length - 1, W_STRING_BYTE);

  char* buf = s->buf();
  char* end = buf + s->len;

  *end = 0;
  if (extra.size()) {
    end -= extra.size();
    memcpy(end, extra.data(), extra.size());
  }
  for (const watchman_dir* d = this; d; d = d->parent) {
    if (d != this || (extra.size())) {
      --end;
      *end = '/';
    }
    end -= d->name.size();
    memcpy(end, d->name.data(), d->name.size());
  }

  return w_string{s};
}

/* vim:ts=2:sw=2:et:
 */
