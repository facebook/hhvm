/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPByteEventHelpers.h"

namespace proxygen::coro {

size_t PendingByteEvent::fireEvents(std::list<PendingByteEvent>& events,
                                    uint64_t offset) {
  size_t nEvents = 0;
  while (!events.empty() && events.front().sessionOffset <= offset) {
    auto& event = events.front();
    auto cb = std::move(event.callback);
    if (cb) {
      cb->onByteEvent(std::move(event.byteEvent));
    }
    events.pop_front();
    nEvents++;
  }
  return nEvents;
}

void PendingByteEvent::cancelEvents(std::list<PendingByteEvent>& events,
                                    const HTTPError& error) {
  while (!events.empty()) {
    auto& event = events.front();
    auto cb = std::move(event.callback);
    if (cb) {
      cb->onByteEventCanceled(std::move(event.byteEvent), error);
    }
    events.pop_front();
  }
}

} // namespace proxygen::coro
