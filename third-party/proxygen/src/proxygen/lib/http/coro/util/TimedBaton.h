/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/CancellationToken.h>
#include <folly/coro/Baton.h>
#include <folly/coro/Task.h>
#include <folly/io/async/DestructorCheck.h>
#include <folly/io/async/EventBase.h>
#include <folly/logging/xlog.h>

namespace proxygen::coro {

/**
 * This class wraps folly::coro::Baton and provides two additional features:
 *  1) cancellation
 *  2) an optional timeout
 *
 * TimedBaton can only be used by a single thread (it is not thread safe).  If
 * using a non-zero timeout value, it must be used in an active EventBase
 * thread.
 *
 * Once the baton is signaled, it remains in the signaled state until reset().
 */
class TimedBaton
    : public folly::HHWheelTimer::Callback
    , public folly::DestructorCheck {
 public:
  explicit TimedBaton(
      folly::EventBase* eventBase,
      std::chrono::milliseconds timeout = std::chrono::milliseconds{5000})
      : eventBase_(eventBase), timeout_(timeout) {
  }

  enum class Status : uint8_t { notReady, signalled, timedout, cancelled };

  void signal(Status status = Status::signalled) {
    XCHECK(!eventBase_ || eventBase_->isInEventBaseThread());
    if (status == status_) {
      return;
    }
    cancelTimeout();
    if (status_ != Status::notReady && status_ != Status::signalled) {
      XLOG(DBG4) << "Ignoring TimedBaton signal while in error state="
                 << uint32_t(status_);
    } else {
      status_ = status;
    }
    baton_.post();
  }

  folly::EventBase* getEventBase() const {
    return eventBase_;
  }

  [[nodiscard]] Status getStatus() const {
    return status_;
  }

  void reset() {
    XCHECK(!eventBase_ || eventBase_->isInEventBaseThread());
    status_ = Status::notReady;
    baton_.reset();
  }

  [[nodiscard]] std::chrono::milliseconds getTimeout() const {
    return timeout_;
  }

  void setTimeout(std::chrono::milliseconds timeout) {
    XCHECK(!eventBase_ || eventBase_->isInEventBaseThread());
    timeout_ = timeout;
    if (waiting_ > 0) {
      if (timeout_.count() > 0) {
        XCHECK(eventBase_) << "Must provide an event base when timeout is set";
        eventBase_->timer().scheduleTimeout(this, timeout_);
      } else {
        cancelTimeout();
      }
    }
  }

  folly::coro::Task<Status> wait() noexcept {
    XCHECK(!eventBase_ || eventBase_->isInEventBaseThread());
    const auto& cancelToken =
        co_await folly::coro::co_current_cancellation_token;
    if (cancelToken.isCancellationRequested()) {
      status_ = Status::cancelled;
      co_return status_;
    }
    folly::CancellationCallback cancellationCallback{
        cancelToken, [this, gone = destroyed_]() mutable {
          if (!eventBase_ || eventBase_->isInEventBaseThread()) {
            this->signal(Status::cancelled);
            return;
          }

          eventBase_->runInEventBaseThread([this, gone = std::move(gone)]() {
            if (!*gone) {
              this->signal(Status::cancelled);
            }
          });
        }};
    if (timeout_.count() > 0 && !baton_.ready()) {
      XCHECK(eventBase_) << "Must provide an event base when timeout is set";
      eventBase_->timer().scheduleTimeout(this, timeout_);
    }
    waiting_++;
    Safety safety(*this);
    co_await baton_;
    if (safety.destroyed()) {
      co_return Status::cancelled;
    }
    waiting_--;
    co_return status_;
  }

  ~TimedBaton() override {
    // must be destructed in evb thread
    XCHECK(!eventBase_ || eventBase_->isInEventBaseThread());
    *destroyed_ = true;
    if (status_ == Status::notReady) {
      signal(Status::cancelled);
    }
  }

 protected:
  void timeoutExpired() noexcept override {
    XLOG(DBG6) << "in TimedBaton timeout expired";
    status_ = Status::timedout;
    baton_.post();
  }

  void callbackCanceled() noexcept override {
    signal(Status::cancelled);
  }

 private:
  folly::EventBase* eventBase_;
  std::chrono::milliseconds timeout_;
  folly::coro::Baton baton_;
  Status status_{Status::notReady};
  uint8_t waiting_{0};
  std::shared_ptr<bool> destroyed_{std::make_shared<bool>(false)};
};

} // namespace proxygen::coro
