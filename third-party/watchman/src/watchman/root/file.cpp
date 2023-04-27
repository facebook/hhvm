/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/watchman_file.h"
#ifdef __APPLE__
#include <sys/attr.h> // @manual
#endif

void watchman_file::removeFromFileList() {
  if (next) {
    next->prev = prev;
  }
  // file->prev points to the address of either
  // `previous_file->next` OR `root->inner.latest_file`.
  // This next assignment is therefore fixing up either
  // the linkage from the prior file node or from the
  // head of the list.
  if (prev) {
    *prev = next;
  }
}

/* We embed our name string in the tail end of the struct that we're
 * allocating here.  This turns out to be more memory efficient due
 * to the way that the allocator bins sizeof(watchman_file); there's
 * a bit of unusable space after the end of the structure that happens
 * to be about the right size to fit a typical filename.
 * Embedding the name in the end allows us to make the most of this
 * memory and free up the separate heap allocation for file_name.
 */
std::unique_ptr<watchman_file, watchman_dir::Deleter> watchman_file::make(
    const w_string& name,
    watchman_dir* parent) {
  auto file = (watchman_file*)calloc(
      1, sizeof(watchman_file) + sizeof(uint32_t) + name.size() + 1);
  std::unique_ptr<watchman_file, watchman_dir::Deleter> filePtr(
      file, watchman_dir::Deleter());

  auto lenPtr = (uint32_t*)(file + 1);
  *lenPtr = name.size();

  auto data = (char*)(lenPtr + 1);
  memcpy(data, name.data(), name.size());
  data[name.size()] = 0;

  file->parent = parent;
  file->exists = true;

  return filePtr;
}

watchman_file::~watchman_file() {
  removeFromFileList();
}

void free_file_node(struct watchman_file* file) {
  file->~watchman_file();
  free(file);
}

/* vim:ts=2:sw=2:et:
 */
