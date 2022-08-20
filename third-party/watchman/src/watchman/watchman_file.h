/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "watchman/Clock.h"
#include "watchman/fs/FileInformation.h"
#include "watchman/watchman_dir.h"

struct watchman_file {
  /* the parent dir */
  watchman_dir* parent;

  /* linkage to files ordered by changed time.
   * prev points to the address of `next` in the
   * previous file node, or the head of the list. */
  struct watchman_file **prev, *next;

  /* the time we last observed a change to this file */
  watchman::ClockStamp otime;
  /* the time we first observed this file OR the time
   * that this file switched from !exists to exists.
   * This is thus the "created time" */
  watchman::ClockStamp ctime;

  /* whether we believe that this file still exists */
  bool exists;
  /* whether we think this file might not exist */
  bool maybe_deleted;

  /* cache stat results so we can tell if an entry
   * changed */
  watchman::FileInformation stat;

  inline w_string_piece getName() const {
    uint32_t len;
    memcpy(&len, this + 1, 4);
    return w_string_piece(reinterpret_cast<const char*>(this + 1) + 4, len);
  }

  void removeFromFileList();

  watchman_file() = delete;
  watchman_file(const watchman_file&) = delete;
  watchman_file& operator=(const watchman_file&) = delete;
  ~watchman_file();

  static std::unique_ptr<watchman_file, watchman_dir::Deleter> make(
      const w_string& name,
      watchman_dir* parent);
};

void free_file_node(struct watchman_file* file);
