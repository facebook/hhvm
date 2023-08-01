/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/executors/ThreadPoolExecutor.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

class ExecutorObserver : public folly::ThreadPoolExecutor::Observer {
 public:
  void threadStarted(
      folly::ThreadPoolExecutor::ThreadHandle* threadHandle) override {
    CHECK(!initializationComplete_);
    evbs_.wlock()->push_back(
        folly::IOThreadPoolExecutorBase::getEventBase(threadHandle));
  }
  void threadStopped(folly::ThreadPoolExecutor::ThreadHandle*) override {}

  std::vector<folly::EventBase*> extractEvbs() {
    CHECK(!std::exchange(initializationComplete_, true));
    return evbs_.exchange({});
  }

 private:
  bool initializationComplete_{false};
  folly::Synchronized<std::vector<folly::EventBase*>> evbs_;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
