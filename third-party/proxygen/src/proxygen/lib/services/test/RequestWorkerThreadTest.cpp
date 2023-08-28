/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/portability/GTest.h>

#include "proxygen/lib/services/RequestWorkerThread.h"

namespace proxygen {

class FinishCallbackTest : public RequestWorkerThread::FinishCallback {
 public:
  FinishCallbackTest(uint64_t* requestId,
                     folly::Executor::KeepAlive<folly::EventBase> keepalive)
      : requestId_(requestId), keepalive_(std::move(keepalive)) {
  }

  void workerStarted(RequestWorkerThread*) override {
  }
  void workerFinished(RequestWorkerThread* worker) override {
    // The worker has finished executing its main work, but it still may
    // complete work scheduled on the event base, so let's test that.
    worker->getEventBase()->runInEventBaseThread([&] {
      // Because this code is called on pre-destruction and because there are
      // keepalives in the test below, this code will still run.
      *requestId_ = RequestWorkerThread::nextRequestId();
      keepalive_.reset();
    });
  }

 private:
  uint64_t* requestId_;
  folly::Executor::KeepAlive<folly::EventBase> keepalive_;
};

TEST(RequestWorkerThreadTest, nextRequestIdDuringShutdown) {
  auto executor = std::make_unique<folly::IOThreadPoolExecutor>(
      1,
      std::make_shared<folly::NamedThreadFactory>("test"),
      folly::EventBaseManager::get(),
      folly::IOThreadPoolExecutor::Options().setWaitForAll(false));
  folly::EventBase* firstEventBase = executor->getAllEventBases()[0].get();
  auto requestId = static_cast<uint64_t>(-1);
  FinishCallbackTest finishCallback(&requestId,
                                    executor->getAllEventBases()[0]);
  std::thread stopThread;
  // This will prevent the event bases from getting destructor
  auto worker = RequestWorkerThread(finishCallback, 0, firstEventBase);
  firstEventBase->runOnDestructionStart([&]() { worker.cleanup(); });
  worker.setup();
  worker.forceStop();
  executor->stop();
  ASSERT_NE(requestId, -1);
}
} // namespace proxygen
