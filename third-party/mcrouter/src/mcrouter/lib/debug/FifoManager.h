/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <condition_variable>
#include <memory>
#include <thread>
#include <unordered_map>

#include <boost/filesystem.hpp>

#include <folly/SharedMutex.h>
#include <folly/Singleton.h>
#include <folly/Synchronized.h>

#include "mcrouter/lib/debug/Fifo.h"

namespace facebook {
namespace memcache {

/**
 * Manager of fifos.
 */
class FifoManager {
 public:
  ~FifoManager();

  /**
   * Fetches (creates if not found) a fifo by its full base path + threadId.
   * The final path of the returned fifo will have the following format:
   * "{fifoBasePath}.{threadId}".
   * At any given point in time, this instance manages at most one fifo per
   * basePath/threadId pair.
   *
   * @param fifoBasePath  Base path of the fifo.
   * @return              The "thread_local" fifo, or nullptr if fifoBasePath
   *                      is empty.
   */
  std::shared_ptr<Fifo> fetchThreadLocal(const std::string& fifoBasePath);

  /**
   * Removes all elements from the fifo manager.
   */
  void clear();

  /**
   * Returns the singleton instance of FifoManager.
   * Note: Keep FifoManager's shared pointer for as little as possible.
   */
  static std::shared_ptr<FifoManager> getInstance();

 private:
  FifoManager();

  folly::Synchronized<
      std::unordered_map<std::string, std::shared_ptr<Fifo>>,
      folly::SharedMutex>
      fifos_;

  // Thread that connects to fifos
  std::thread thread_;
  bool running_{true};
  std::mutex mutex_;
  std::condition_variable cv_;

  /**
   * Fetches a fifo by its full path. If the fifo does not
   * exist yet, creates it and returns it to the caller.
   *
   * @param fifoPath  Full path of the fifo.
   * @return          The fifo.
   */
  std::shared_ptr<Fifo> fetch(const std::string& fifoPath);

  /**
   * Finds a fifo by its full path. If not found, returns null.
   *
   * @param fifoPath  Full path of the fifo.
   * @return          The fifo or null if not found.
   */
  std::shared_ptr<Fifo> find(const std::string& fifoPath);

  /**
   * Creates a fifo and stores it into the map.
   *
   * @param fifoPath  Full path of the fifo.
   * @return          The newly created fifo.
   */
  std::shared_ptr<Fifo> createAndStore(const std::string& fifoPath);

  friend class folly::Singleton<FifoManager>;
};

} // namespace memcache
} // namespace facebook
