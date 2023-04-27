/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/connpool/ThreadIdleSessionController.h>

namespace proxygen {

ThreadIdleSessionController::ThreadIdleSessionController(
    uint32_t totalIdleSessions)
    : totalIdleSessions_(totalIdleSessions) {
}

void ThreadIdleSessionController::onAttachIdle(SessionHolder* holder) {
  idleSessionsLRU_.push_back(*holder);
  purgeExcessIdleSessions();
}

void ThreadIdleSessionController::onDetachIdle(SessionHolder* holder) {
  idleSessionsLRU_.erase(idleSessionsLRU_.iterator_to(*holder));
}

void ThreadIdleSessionController::purgeExcessIdleSessions() {
  while (idleSessionsLRU_.size() > totalIdleSessions_) {
    SessionHolder& holder = idleSessionsLRU_.front();
    holder.drain();
  }
}

uint32_t ThreadIdleSessionController::getTotalIdleSessions() const {
  return idleSessionsLRU_.size();
}
} // namespace proxygen
