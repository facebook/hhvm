/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AsyncWriter.h"

#include <folly/Range.h>
#include <folly/fibers/EventBaseLoopController.h>
#include <folly/io/async/EventBase.h>
#include <folly/system/ThreadName.h>

#include "mcrouter/AsyncWriterEntry.h"
#include "mcrouter/McrouterLogFailure.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

AsyncWriter::AsyncWriter(size_t maxQueue)
    : maxQueueSize_(maxQueue),
      fiberManager_(std::make_unique<folly::fibers::EventBaseLoopController>()),
      eventBase_(/* enableTimeMeasurement */ false) {
  auto& c = fiberManager_.loopController();
  dynamic_cast<folly::fibers::EventBaseLoopController&>(c).attachEventBase(
      eventBase_);
}

AsyncWriter::~AsyncWriter() {
  stop();
  assert(!fiberManager_.hasTasks());
}

void AsyncWriter::stop() noexcept {
  {
    std::unique_lock lock(runLock_);
    if (stopped_) {
      return;
    }
    stopped_ = true;
  }

  if (thread_.joinable()) {
    eventBase_.terminateLoopSoon();
    thread_.join();
  } else {
    while (fiberManager_.hasTasks()) {
      eventBase_.loopOnce();
    }
  }
}

bool AsyncWriter::start(folly::StringPiece threadName) {
  std::unique_lock lock(runLock_);
  if (thread_.joinable() || stopped_) {
    return false;
  }

  try {
    thread_ = std::thread([this, threadName]() {
      folly::setThreadName(threadName);

      // will return after terminateLoopSoon is called
      eventBase_.loopForever();

      while (fiberManager_.hasTasks()) {
        eventBase_.loopOnce();
      }
    });
  } catch (const std::system_error& e) {
    LOG_FAILURE(
        "mcrouter",
        memcache::failure::Category::kSystemError,
        "Can not start AsyncWriter thread {}: {}",
        threadName,
        e.what());
    return false;
  }

  return true;
}

bool AsyncWriter::run(std::function<void()> f) {
  std::shared_lock lock(runLock_);
  if (stopped_) {
    return false;
  }

  bool decQueueSize = false;
  if (maxQueueSize_ != 0) {
    auto size = queueSize_.load();
    do {
      if (maxQueueSize_ == size) {
        return false;
      }
    } while (!queueSize_.compare_exchange_weak(size, size + 1));
    decQueueSize = true;
  }

  fiberManager_.addTaskRemote(
      [this, f_ = std::move(f), decQueueSize]() mutable {
        fiberManager_.runInMainContext(std::move(f_));
        if (decQueueSize) {
          --queueSize_;
        }
      });
  return true;
}

void AsyncWriter::increaseMaxQueueSize(size_t add) {
  std::unique_lock lock(runLock_);
  // Don't touch maxQueueSize_ if it's already unlimited (zero).
  if (maxQueueSize_ != 0) {
    maxQueueSize_ += add;
  }
}

void AsyncWriter::makeQueueSizeUnlimited() {
  std::unique_lock lock(runLock_);
  maxQueueSize_ = 0;
}

bool awriter_queue(AsyncWriter* w, awriter_entry_t* e) {
  return w->run([e, w]() {
    if (!w->isActive()) {
      e->callbacks->completed(e, EPIPE);
      return;
    }
    int r = e->callbacks->perform_write(e);
    e->callbacks->completed(e, r);
  });
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
