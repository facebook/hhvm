/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/watcher/Watcher.h"

namespace watchman {

Watcher::Watcher(const char* name, unsigned flags) : name(name), flags(flags) {}

Watcher::~Watcher() {}

bool Watcher::startWatchFile(watchman_file*) {
  return true;
}

bool Watcher::start(const std::shared_ptr<Root>&) {
  return true;
}

} // namespace watchman
