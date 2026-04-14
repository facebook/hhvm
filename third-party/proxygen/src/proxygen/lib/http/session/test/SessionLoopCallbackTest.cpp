/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/SessionLoopCallback.h>

#include <folly/io/async/EventBase.h>
#include <folly/io/async/Request.h>
#include <folly/portability/GTest.h>

using namespace proxygen;

namespace {

class TestSessionLoopCallback : public SessionLoopCallback {
 public:
  void runSessionLoopCallback() noexcept override {
    contextDuringCallback_ = folly::RequestContext::try_get();
    ran_ = true;
  }

  folly::RequestContext* contextDuringCallback_{nullptr};
  bool ran_{false};
};

} // namespace

// Verify that scheduleInLoop() does NOT propagate the caller's RequestContext
// to the callback, preventing per-request context from contaminating
// session-level work.
TEST(SessionLoopCallbackTest, ScheduleInLoopPreventsRequestContextPropagation) {
  folly::EventBase evb;
  TestSessionLoopCallback cb;

  auto customCtx = std::make_shared<folly::RequestContext>();
  folly::RequestContext::setContext(customCtx);
  ASSERT_EQ(folly::RequestContext::get(), customCtx.get());

  cb.scheduleInLoop(&evb);
  evb.loopOnce();

  ASSERT_TRUE(cb.ran_);
  EXPECT_NE(cb.contextDuringCallback_, customCtx.get());
}

// Same as above but with thisIteration=true.
TEST(SessionLoopCallbackTest,
     ScheduleInLoopThisIterationPreventsRequestContextPropagation) {
  folly::EventBase evb;
  TestSessionLoopCallback cb;

  auto customCtx = std::make_shared<folly::RequestContext>();
  folly::RequestContext::setContext(customCtx);

  cb.scheduleInLoop(&evb, /*thisIteration=*/true);
  evb.loopOnce();

  ASSERT_TRUE(cb.ran_);
  EXPECT_NE(cb.contextDuringCallback_, customCtx.get());
}

// Control test: a plain LoopCallback scheduled via runInLoop() with default
// parameters DOES propagate the caller's RequestContext. This proves the
// behavior that SessionLoopCallback is designed to prevent.
TEST(SessionLoopCallbackTest,
     ControlPlainLoopCallbackDoesPropagateRequestContext) {
  folly::EventBase evb;

  struct PlainCallback : public folly::EventBase::LoopCallback {
    void runLoopCallback() noexcept override {
      contextDuringCallback_ = folly::RequestContext::try_get();
      ran_ = true;
    }
    folly::RequestContext* contextDuringCallback_{nullptr};
    bool ran_{false};
  };

  PlainCallback cb;
  auto customCtx = std::make_shared<folly::RequestContext>();
  folly::RequestContext::setContext(customCtx);

  evb.runInLoop(&cb);
  evb.loopOnce();

  ASSERT_TRUE(cb.ran_);
  EXPECT_EQ(cb.contextDuringCallback_, customCtx.get());
}
