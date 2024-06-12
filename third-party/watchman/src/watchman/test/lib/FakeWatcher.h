/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "watchman/watcher/Watcher.h"

namespace watchman {

class FileSystem;

class FakeWatcher : public Watcher {
 public:
  explicit FakeWatcher(FileSystem& fileSystem, bool failsToStart = false);

  bool start(const std::shared_ptr<Root>& root) override;

  std::unique_ptr<DirHandle> startWatchDir(
      const std::shared_ptr<Root>& root,
      const char* path) override;

  bool waitNotify(int timeoutms) override;
  ConsumeNotifyRet consumeNotify(
      const std::shared_ptr<Root>& root,
      PendingChanges& coll) override;

 private:
  FileSystem& fileSystem_;
  bool failsToStart_;
};

} // namespace watchman
