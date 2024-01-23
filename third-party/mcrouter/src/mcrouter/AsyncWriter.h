/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

#include <folly/Range.h>
#include <folly/SharedMutex.h>
#include <folly/fibers/FiberManager.h>
#include <folly/io/async/EventBase.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

// Forward declaration.
struct awriter_entry_t;

class AsyncWriter {
 public:
  /**
   * @param maxQueueSize: maximum number of run requests in a queue. "run" will
   *                      fail if there is already maxQueueSize requests in
   *                      the queue. 0 means the queue is unlimited.
   */
  explicit AsyncWriter(size_t maxQueueSize = 0);

  /**
   * Starts the awriter thread
   *
   * @param threadName name for the writer thread
   *                   (will be truncated, if length is >15 characters).
   * @return true on success, false on failure (e.g. the thread is already
             running)
   */
  bool start(folly::StringPiece threadName);

  /**
   * Stops the awriter and waits for all the functions to finish.
   */
  void stop() noexcept;

  /**
   * @return true if writer can process requests, false otherwise.
   */
  bool isActive() const {
    return !stopped_;
  }

  /**
   * Add a function to the queue to run asynchronously.
   *
   * @return true on success, false on failure (e.g. when we hit the queue
             size limit)
   */
  bool run(std::function<void()> f);

  /**
   * Increase the maximum queue size. The max queue size will never decrease.
   **/
  void increaseMaxQueueSize(size_t add);

  /**
   * Make the queue have unlimited size.
   */
  void makeQueueSizeUnlimited();

  /**
   * Waits for all the functions to complete
   */
  ~AsyncWriter();

 private:
  size_t maxQueueSize_;
  std::atomic<size_t> queueSize_{0};
  std::atomic<bool> stopped_{false};
  mutable folly::SharedMutex runLock_;

  folly::fibers::FiberManager fiberManager_;
  folly::EventBase eventBase_;
  std::thread thread_;
};

/**
 * @return true on success, false otherwise
 */
bool awriter_queue(AsyncWriter* w, awriter_entry_t* e);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
