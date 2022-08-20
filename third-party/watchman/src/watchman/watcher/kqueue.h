/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/Constants.h"
#include "watchman/fs/FileDescriptor.h"
#include "watchman/fs/Pipe.h"
#include "watchman/watcher/Watcher.h"

#ifdef HAVE_KQUEUE

namespace watchman {

class Configuration;

struct KQueueWatcher : public Watcher {
  FileDescriptor kq_fd;
  Pipe terminatePipe_;

  struct maps {
    std::unordered_map<w_string, FileDescriptor> name_to_fd;
    /* map of active watch descriptor to name of the corresponding item */
    std::unordered_map<int, w_string> fd_to_name;

    explicit maps(json_int_t sizeHint) {
      name_to_fd.reserve(sizeHint);
      fd_to_name.reserve(sizeHint);
    }
  };
  folly::Synchronized<maps> maps_;
  bool recursive_;

  struct kevent keventbuf[WATCHMAN_BATCH_LIMIT];

  explicit KQueueWatcher(
      const w_string& root_path,
      const Configuration& config,
      bool recursive = true);

  std::unique_ptr<DirHandle> startWatchDir(
      const std::shared_ptr<Root>& root,
      const char* path) override;

  bool startWatchFile(struct watchman_file* file) override;

  Watcher::ConsumeNotifyRet consumeNotify(
      const std::shared_ptr<Root>& root,
      PendingChanges& coll) override;

  bool waitNotify(int timeoutms) override;
  void stopThreads() override;
};

} // namespace watchman

#endif
