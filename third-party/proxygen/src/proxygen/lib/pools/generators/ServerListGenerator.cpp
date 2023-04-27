/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/pools/generators/ServerListGenerator.h>

#include <folly/Conv.h>
#include <folly/io/async/EventBase.h>

using folly::EventBase;
using std::vector;
using std::chrono::milliseconds;

namespace proxygen {

void ServerListGenerator::attachEventBase(EventBase* base) {
  CHECK(!eventBase_);
  CHECK(base->isInEventBaseThread());

  eventBase_ = base;
}

void ServerListGenerator::detachEventBase() {
  CHECK(!eventBase_ || eventBase_->isInEventBaseThread());

  eventBase_ = nullptr;
}

void ServerListGenerator::listServersBlocking(vector<ServerConfig>* results,
                                              milliseconds timeout) {
  // Run a EventBase to drive the asynchronous listServers() call until it
  // finishes.
  EventBase eventBase;
  ServerListCallback callback;
  attachEventBase(&eventBase);
  listServers(&callback, timeout);
  eventBase.loop();
  detachEventBase();

  if (callback.status != ServerListCallback::SUCCESS) {
    if (!callback.errorPtr) {
      LOG(FATAL)
          << "ServerListGenerator finished without invoking callback, timeout:"
          << timeout.count();
    }
    std::rethrow_exception(callback.errorPtr);
  }

  results->swap(callback.servers);
}

} // namespace proxygen
