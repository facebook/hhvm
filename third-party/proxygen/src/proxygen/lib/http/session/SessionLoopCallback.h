/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/EventBase.h>

namespace proxygen {

/**
 * A LoopCallback base class for session-level callbacks that ensures a
 * captured RequestContext does not propagate.
 *
 * When EventBase::runInLoop() is called with the default parameters, it
 * captures the caller's ambient RequestContext. When the callback later
 * executes, that captured context is restored, causing any per-request
 * callbacks in onSet/onUnset to fire. For session-level callbacks that
 * process work for multiple requests (e.g., writing all pending streams,
 * processing reads for all streams), this results in calling per-request
 * callbacks incorrectly as the cross-request work cannot be related to
 * a single request.
 *
 * SessionLoopCallback provides scheduleInLoop() which passes nullptr for
 * the RequestContext, preventing this contamination. Private inheritance
 * prevents subclasses from calling EventBase::runInLoop() directly, since
 * the implicit conversion to LoopCallback* is inaccessible outside this
 * class.
 */
class SessionLoopCallback : private folly::EventBase::LoopCallback {
 public:
  using folly::EventBase::LoopCallback::cancelLoopCallback;
  using folly::EventBase::LoopCallback::isLoopCallbackScheduled;

  /**
   * Schedule this callback to run in the EventBase loop without capturing
   * the current RequestContext.
   */
  void scheduleInLoop(folly::EventBase* evb, bool thisIteration = false) {
    evb->runInLoop(this, thisIteration, /*rctx=*/nullptr);
  }

 protected:
  /**
   * Subclasses implement their callback logic here.
   */
  virtual void runSessionLoopCallback() noexcept = 0;

 private:
  void runLoopCallback() noexcept final {
    runSessionLoopCallback();
  }
};

} // namespace proxygen
