/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/services/RequestWorkerThread.h>

#include <folly/io/async/EventBaseManager.h>
#include <proxygen/lib/services/ServiceWorker.h>

namespace proxygen {

static const uint32_t requestIdBits = 56;
static const uint64_t requestIdMask = ((1ULL << requestIdBits) - 1);

RequestWorkerThread::RequestWorkerThread(FinishCallback& callback,
                                         uint8_t threadId,
                                         const std::string& evbName)
    : WorkerThread(folly::EventBaseManager::get(), evbName),
      nextRequestId_(static_cast<uint64_t>(threadId) << requestIdBits),
      callback_(callback) {
}

RequestWorkerThread::~RequestWorkerThread() {
  // It is important to reset the underlying event base in advance of this
  // class' destruction as it may be that there are functions awaiting
  // execution that possess a reference to this class.
  resetEventBase();
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
  WorkerThread::setup();
  callback_.workerStarted(this);
}

void RequestWorkerThread::cleanup() {
  WorkerThread::cleanup();
  callback_.workerFinished(this);
}

} // namespace proxygen
