/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <optional>
#include "watchman/RingBuffer.h"
#include "watchman/fs/Pipe.h"
#include "watchman/watcher/Watcher.h"
#include "watchman/watchman_cmd.h"

#if HAVE_FSEVENTS

namespace watchman {

class Client;
class Configuration;
struct FSEventsStream;
struct FSEventsLogEntry;

struct watchman_fsevent {
  w_string path;
  FSEventStreamEventFlags flags;

  watchman_fsevent(w_string&& path, FSEventStreamEventFlags flags)
      : path(std::move(path)), flags(flags) {}
};

class FSEventsWatcher : public Watcher {
 public:
  explicit FSEventsWatcher(
      bool hasFileWatching,
      const Configuration& config,
      std::optional<w_string> dir = std::nullopt);

  explicit FSEventsWatcher(
      const w_string& root_path,
      const Configuration& config,
      std::optional<w_string> dir = std::nullopt);
  ~FSEventsWatcher();

  bool start(const std::shared_ptr<Root>& root) override;

  folly::SemiFuture<folly::Unit> flushPendingEvents() override;

  std::unique_ptr<DirHandle> startWatchDir(
      const std::shared_ptr<Root>& root,
      const char* path) override;

  Watcher::ConsumeNotifyRet consumeNotify(
      const std::shared_ptr<Root>& root,
      PendingChanges& changes) override;

  bool waitNotify(int timeoutms) override;
  void stopThreads() override;
  void FSEventsThread(const std::shared_ptr<Root>& root);

  json_ref getDebugInfo() override;
  void clearDebugInfo() override;

  static UntypedResponse cmd_debug_fsevents_inject_drop(
      Client* client,
      const json_ref& args);

 private:
  static std::unique_ptr<FSEventsStream> fse_stream_make(
      const std::shared_ptr<Root>& root,
      FSEventsWatcher* watcher,
      FSEventStreamEventId since,
      std::optional<w_string>& failure_reason);
  static void fse_callback(
      ConstFSEventStreamRef,
      void* clientCallBackInfo,
      size_t numEvents,
      void* eventPaths,
      const FSEventStreamEventFlags eventFlags[],
      const FSEventStreamEventId eventIds[]);

  watchman::Pipe fsePipe_;

  std::condition_variable fseCond_;
  struct Items {
    // Unflattened queue of pending events. The fse_callback function will push
    // exactly one vector to the end of this one, flattening the vector would
    // require extra copying and allocations.
    std::vector<std::vector<watchman_fsevent>> items;
    // Sync requests to be inserted into PendingCollection.
    std::vector<folly::Promise<folly::Unit>> syncs;
  };
  folly::Synchronized<Items, std::mutex> items_;

  std::unique_ptr<FSEventsStream> stream_;
  const bool attemptResyncOnDrop_{false};
  const bool hasFileWatching_{false};
  const bool enableStreamFlush_{true};
  std::optional<w_string> subdir{std::nullopt};

  // Incremented in fse_callback
  std::atomic<size_t> totalEventsSeen_{0};
  /**
   * If not null, holds a fixed-size ring of the last `fsevents_ring_log_size`
   * FSEvents events.
   */
  std::unique_ptr<RingBuffer<FSEventsLogEntry>> ringBuffer_;
};

} // namespace watchman

#endif
