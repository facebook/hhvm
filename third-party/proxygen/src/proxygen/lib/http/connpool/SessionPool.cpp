/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/connpool/ServerIdleSessionController.h>
#include <proxygen/lib/http/connpool/SessionPool.h>
#include <proxygen/lib/http/connpool/ThreadIdleSessionController.h>

#include <chrono>
#include <folly/io/async/EventBaseManager.h>

namespace proxygen {

SessionPool::SessionPool(
    SessionHolder::Stats* stats,
    uint32_t maxConns,
    std::chrono::milliseconds timeout,
    std::chrono::milliseconds maxAge,
    ThreadIdleSessionController* threadIdleSessionController,
    ServerIdleSessionController* serverIdleSessionController)
    : stats_(stats),
      maxConns_(maxConns),
      timeout_(timeout),
      maxAge_(maxAge),
      threadIdleSessionController_(threadIdleSessionController),
      serverIdleSessionController_(serverIdleSessionController),
      // Here we rely on workers setting evb in EventBaseManager.
      evb_(folly::EventBaseManager::get()->getEventBase()) {
}

SessionPool::~SessionPool() {
  drainSessionList(idleSessionList_);
  drainSessionList(unfilledSessionList_);
  drainSessionList(fullSessionList_);
  DCHECK(empty());
}

void SessionPool::setMaxIdleSessions(uint32_t num) {
  maxConns_ = num;
  purgeExcessIdleSessions();
}

uint32_t SessionPool::getMaxIdleSessions() const {
  return maxConns_;
}

void SessionPool::setTimeout(std::chrono::milliseconds duration) {
  timeout_ = duration;
}

std::chrono::milliseconds SessionPool::getTimeout() const {
  return timeout_;
}

uint32_t SessionPool::getNumIdleSessions() const {
  return idleSessionList_.size();
}

uint32_t SessionPool::getNumActiveSessions() const {
  return unfilledSessionList_.size() + fullSessionList_.size();
}

uint32_t SessionPool::getNumActiveNonFullSessions() const {
  return unfilledSessionList_.size();
}

uint32_t SessionPool::getNumFullSessions() const {
  return fullSessionList_.size();
}

uint32_t SessionPool::getNumSessions() const {
  return idleSessionList_.size() + unfilledSessionList_.size() +
         fullSessionList_.size();
}

bool SessionPool::empty() const {
  return idleSessionList_.empty() && unfilledSessionList_.empty() &&
         fullSessionList_.empty();
}

void SessionPool::putSession(HTTPSessionBase* session) {
  if (SessionHolder::isPoolable(session)) {
    // Constructing the session holder automatically puts it on the
    // correct list (one of [idle, unfilled, full])
    auto holder = new SessionHolder(session, this, stats_);
    holder->link();
  } else {
    // this is equivalent to what happens in SessionHolder::link which is
    // called from the SessionHolder ctor, but handle separately to avoid
    // needless allocation/deallocation.
    addDrainingSession(session);
    session->drain();
  }
  purgeExcessIdleSessions();
}

HTTPTransaction* SessionPool::getTransaction(
    HTTPTransaction::Handler* upstreamHandler) {
  auto txn = attemptOpenTransaction(upstreamHandler, unfilledSessionList_);
  if (!txn) {
    purgeExcessIdleSessions();
    txn = attemptOpenTransaction(upstreamHandler, idleSessionList_);
  }
  return txn;
}

void SessionPool::purgeExcessIdleSessions() {
  auto thresh = std::chrono::steady_clock::now() - getTimeout();

  CHECK_LE(idleSessionList_.size(), std::numeric_limits<uint32_t>::max());
  int64_t excess =
      static_cast<int64_t>(idleSessionList_.size()) - getMaxIdleSessions();
  while (!idleSessionList_.empty()) {
    SessionHolder* holder = &idleSessionList_.front();
    if (holder->getLastUseTime() > thresh && excess <= 0) {
      break;
    }
    --excess;
    holder->drain();
  }
}

HTTPSessionBase* FOLLY_NULLABLE SessionPool::removeOldestIdleSession() {
  if (!idleSessionList_.empty()) {
    SessionHolder* holder = &idleSessionList_.front();
    CHECK_NOTNULL(holder);
    return holder->release();
  }
  return nullptr;
}

void SessionPool::drainAllSessions() {
  drainSessionList(idleSessionList_);
  drainSessionList(unfilledSessionList_);
  drainSessionList(fullSessionList_);
}

void SessionPool::closeWithReset() {
  closeSessionListWithReset(idleSessionList_);
  closeSessionListWithReset(unfilledSessionList_);
  closeSessionListWithReset(fullSessionList_);
}

void SessionPool::drainSessionList(SessionList& list) {
  while (!list.empty()) {
    SessionHolder& holder = list.back();
    holder.drain();
  }
}

void SessionPool::closeSessionListWithReset(SessionList& list) {
  while (!list.empty()) {
    SessionHolder& holder = list.back();
    holder.closeWithReset();
  }
}

HTTPTransaction* SessionPool::attemptOpenTransaction(
    HTTPTransaction::Handler* upstreamHandler, SessionList& list) {
  SessionHolder* holder = nullptr;
  while (!list.empty()) {
    holder = &list.front();
    if (holder->shouldAgeOut(maxAge_)) {
      holder->drain(); // implicit unlink and delete
      continue;
    }
    auto txn = holder->newTransaction(upstreamHandler);
    holder->unlink();
    holder->link();
    if (txn) {
      return txn;
    }
    // If we weren't able to get a transaction, then link() caused it to
    // move to the full list, so this loop should eventually terminate
  }
  return nullptr;
}

// SessionHolder::Callback methods

void SessionPool::detachIdle(SessionHolder* sess) {
  idleSessionList_.erase(idleSessionList_.iterator_to(*sess));

  if (threadIdleSessionController_) {
    threadIdleSessionController_->onDetachIdle(sess);
  }
  if (serverIdleSessionController_) {
    serverIdleSessionController_->removeIdleSession(&sess->getSession());
  }
}

void SessionPool::detachPartiallyFilled(SessionHolder* sess) {
  unfilledSessionList_.erase(unfilledSessionList_.iterator_to(*sess));
}

void SessionPool::detachFilled(SessionHolder* sess) {
  fullSessionList_.erase(fullSessionList_.iterator_to(*sess));
}

/**
 * Always push to the back and pop from the front (FIFO). This prevents
 * slow servers from being excessively re-used and drawing load to themselves.
 */
void SessionPool::attachIdle(SessionHolder* sess) {
  if (getMaxIdleSessions() == 0 ||
      !sess->getSession().supportsMoreTransactions() ||
      sess->shouldAgeOut(maxAge_)) {
    idleSessionList_.push_back(*sess);
    sess->drain();
  } else {
    idleSessionList_.push_back(*sess);
    if (serverIdleSessionController_) {
      serverIdleSessionController_->addIdleSession(&sess->getSession(), this);
    }
    if (threadIdleSessionController_) {
      threadIdleSessionController_->onAttachIdle(sess);
    }
    purgeExcessIdleSessions();
  }
}

void SessionPool::attachPartiallyFilled(SessionHolder* sess) {
  // round robin partially full sessions to reduce the chance of GOAWAY
  // race conditions from the server.
  unfilledSessionList_.push_back(*sess);
}

void SessionPool::attachFilled(SessionHolder* sess) {
  fullSessionList_.push_back(*sess);
}

void SessionPool::addDrainingSession(HTTPSessionBase* /*session*/) {
}

std::ostream& operator<<(std::ostream& os, const SessionPool& pool) {
  os << "[idle=" << pool.getNumIdleSessions()
     << ", partial=" << pool.getNumActiveNonFullSessions()
     << ", full=" << +pool.getNumFullSessions() << "]";
  return os;
}

} // namespace proxygen
