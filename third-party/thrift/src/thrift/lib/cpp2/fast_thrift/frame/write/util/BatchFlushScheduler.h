/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/io/async/EventBase.h>
#include <folly/io/async/HHWheelTimer.h>

#include <boost/intrusive/list.hpp>

#include <chrono>
#include <functional>
#include <utility>

namespace apache::thrift::fast_thrift::frame::write::util {

class BatchFlushScheduler : private folly::EventBase::LoopCallback,
                            private folly::HHWheelTimer::Callback {
 public:
  using FlushList = boost::intrusive::list<
      folly::EventBase::LoopCallback,
      boost::intrusive::constant_time_size<false>>;

  enum class DeferredFlushMode {
    EndOfCurrentLoop,
    EndOfNextLoop,
  };

  explicit BatchFlushScheduler(DeferredFlushMode mode) noexcept : mode_(mode) {}

  ~BatchFlushScheduler() override { cancelAll(); }

  BatchFlushScheduler(const BatchFlushScheduler&) = delete;
  BatchFlushScheduler& operator=(const BatchFlushScheduler&) = delete;
  BatchFlushScheduler(BatchFlushScheduler&&) = delete;
  BatchFlushScheduler& operator=(BatchFlushScheduler&&) = delete;

  void setEventBase(folly::EventBase* eventBase) noexcept {
    eventBase_ = eventBase;
  }

  void setFlushFunction(std::function<void()> flushFn) noexcept {
    flushFn_ = std::move(flushFn);
  }

  void clearFlushFunction() noexcept { flushFn_ = nullptr; }

  void setFlushList(FlushList* flushList) noexcept {
    if (flushList_ == flushList) {
      return;
    }
    cancelDeferredFlush();
    flushList_ = flushList;
  }

  void scheduleDeferredFlush() noexcept {
    // Flush-list scheduling still requires an attached EventBase lifecycle:
    // the flush function is owned by handlerAdded/handlerRemoved.
    if (deferredFlushScheduled_ || eventBase_ == nullptr) {
      return;
    }
    if (flushList_ != nullptr) {
      flushList_->push_back(*this);
    } else {
      eventBase_->runInLoop(this, true);
    }
    deferredFlushScheduled_ = true;
  }

  void scheduleTimeout(std::chrono::milliseconds timeout) noexcept {
    if (timeout == std::chrono::milliseconds::zero()) {
      scheduleDeferredFlush();
      return;
    }
    if (eventBase_ == nullptr || timeoutScheduled_) {
      return;
    }
    eventBase_->timer().scheduleTimeout(this, timeout);
    timeoutScheduled_ = true;
  }

  void scheduleEarlyFlush() noexcept {
    if (earlyFlushRequested_) {
      return;
    }
    earlyFlushRequested_ = true;
    cancelTimeout();
    scheduleDeferredFlush();
  }

  void cancelDeferredFlush() noexcept {
    if (deferredFlushScheduled_) {
      cancelLoopCallback();
      deferredFlushScheduled_ = false;
      rescheduled_ = false;
    }
  }

  void cancelTimeout() noexcept {
    if (timeoutScheduled_) {
      folly::HHWheelTimer::Callback::cancelTimeout();
      timeoutScheduled_ = false;
    }
  }

  void cancelAll() noexcept {
    cancelDeferredFlush();
    cancelTimeout();
    earlyFlushRequested_ = false;
  }

  void flushNow() noexcept {
    earlyFlushRequested_ = false;
    flush();
  }

  bool hasScheduledDeferredFlush() const noexcept {
    return deferredFlushScheduled_;
  }

  bool hasScheduledTimeout() const noexcept { return timeoutScheduled_; }

 private:
  void runLoopCallback() noexcept final {
    if (flushList_ != nullptr) {
      completeDeferredFlush();
      flush();
      return;
    }

    if (mode_ == DeferredFlushMode::EndOfNextLoop &&
        !std::exchange(rescheduled_, true)) {
      eventBase_->runInLoop(this, true);
      return;
    }

    completeDeferredFlush();
    flush();
  }

  void timeoutExpired() noexcept final {
    timeoutScheduled_ = false;
    flush();
  }

  void completeDeferredFlush() noexcept {
    deferredFlushScheduled_ = false;
    rescheduled_ = false;
    earlyFlushRequested_ = false;
  }

  void flush() noexcept {
    if (flushFn_) {
      flushFn_();
    }
  }

  DeferredFlushMode mode_;
  folly::EventBase* eventBase_{nullptr};
  FlushList* flushList_{nullptr};
  std::function<void()> flushFn_;
  bool deferredFlushScheduled_{false};
  bool timeoutScheduled_{false};
  bool rescheduled_{false};
  bool earlyFlushRequested_{false};
};

} // namespace apache::thrift::fast_thrift::frame::write::util
