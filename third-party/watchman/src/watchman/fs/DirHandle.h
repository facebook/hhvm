/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include "watchman/fs/FileInformation.h"

namespace watchman {

struct DirEntry {
  bool has_stat;
  const char* d_name;
  FileInformation stat;
};

class DirHandle {
 public:
  virtual ~DirHandle() = default;
  virtual const DirEntry* readDir() = 0;
#ifndef _WIN32
  virtual int getFd() const = 0;
#endif
};

/**
 * Returns a dir handle to path.
 * Does not follow symlinks if strict == true.
 * Throws std::system_error if the dir could not be opened.
 */
std::unique_ptr<DirHandle> openDir(const char* path, bool strict = true);

} // namespace watchman
