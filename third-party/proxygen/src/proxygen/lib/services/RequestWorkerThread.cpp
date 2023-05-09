/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/services/RequestWorkerThread.h>

#include <csignal>
#include <folly/io/async/EventBaseManager.h>
#include <proxygen/lib/services/ServiceWorker.h>

namespace proxygen {

static const uint32_t requestIdBits = 56;
static const uint64_t requestIdMask = ((1ULL << requestIdBits) - 1);

thread_local RequestWorkerThread* RequestWorkerThread::currentRequestWorker_ =
    nullptr;

RequestWorkerThread::RequestWorkerThread(FinishCallback& callback,
                                         uint8_t threadId,
                                         folly::EventBase* evb)
    : nextRequestId_(static_cast<uint64_t>(threadId) << requestIdBits),
      callback_(callback),
      evb_(evb) {
}

RequestWorkerThread::~RequestWorkerThread() {
  currentRequestWorker_ = nullptr;
}

uint8_t RequestWorkerThread::getWorkerId() const {
  return static_cast<uint8_t>(nextRequestId_ >> requestIdBits);
}

uint64_t RequestWorkerThread::nextRequestId() {
  uint64_t requestId = getRequestWorkerThread()->nextRequestId_;
  getRequestWorkerThread()->nextRequestId_ =
      (requestId & ~requestIdMask) | ((requestId + 1) & requestIdMask);
  return requestId;
}

void RequestWorkerThread::flushStats() {
  CHECK(getEventBase()->isInEventBaseThread());
  for (auto& p : serviceWorkers_) {
    p.second->flushStats();
  }
}

void RequestWorkerThread::setup() {
  CHECK(evb_);
  evb_->runImmediatelyOrRunInEventBaseThreadAndWait([&]() {
    sigset_t ss;

    // Ignore some signals
    sigemptyset(&ss);
    sigaddset(&ss, SIGHUP);
    sigaddset(&ss, SIGINT);
    sigaddset(&ss, SIGQUIT);
    sigaddset(&ss, SIGUSR1);
    sigaddset(&ss, SIGUSR2);
    sigaddset(&ss, SIGPIPE);
    sigaddset(&ss, SIGALRM);
    sigaddset(&ss, SIGTERM);
    sigaddset(&ss, SIGCHLD);
    sigaddset(&ss, SIGIO);
    PCHECK(pthread_sigmask(SIG_BLOCK, &ss, nullptr) == 0);

    currentRequestWorker_ = this;
    callback_.workerStarted(this);
  });
}

void RequestWorkerThread::forceStop() {
  forceStopped_.store(true);
  evb_->terminateLoopSoon();
}

void RequestWorkerThread::cleanup() {
  LOG(INFO) << "Worker " << getWorkerId() << " in cleanup";
  if (!forceStopped_.load()) {
    LOG(INFO) << "Looping to finish pending work";
    evb_->loop();
  }
  callback_.workerFinished(this);
}

} // namespace proxygen
