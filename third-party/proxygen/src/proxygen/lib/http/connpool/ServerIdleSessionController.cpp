/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/connpool/ServerIdleSessionController.h>

namespace proxygen {

folly::Future<HTTPSessionBase*> ServerIdleSessionController::getIdleSession() {
  folly::Promise<HTTPSessionBase*> promise;
  folly::Future<HTTPSessionBase*> future = promise.getFuture();
  SessionPool* maxPool = nullptr;

  {
    std::lock_guard<std::mutex> lock(lock_);
    maxPool = popBestIdlePool();
    if (markedForDeath_ || !maxPool || !maxPool->getEventBase()) {
      return folly::makeFuture<HTTPSessionBase*>(nullptr);
    }
  }

  if (maxPool->getEventBase()->isInEventBaseThread()) {
    LOG(ERROR) << "Idle session already belongs to current thread!";
    return folly::makeFuture<HTTPSessionBase*>(nullptr);
  }

  maxPool->getEventBase()->runInEventBaseThread(
      [this, maxPool, promise = std::move(promise)]() mutable {
        // Caller (in this case Server::getTransaction()) needs to guarantee
        // that 'this' still exists.
        HTTPSessionBase* session =
            isMarkedForDeath() ? nullptr : maxPool->removeOldestIdleSession();
        if (session) {
          session->detachThreadLocals(true);
        }
        promise.setValue(session);
      });
  return future;
}

void ServerIdleSessionController::addIdleSession(const HTTPSessionBase* session,
                                                 SessionPool* sessionPool) {
  std::lock_guard<std::mutex> lock(lock_);
  if (sessionMap_.find(session) != sessionMap_.end()) {
    // removeIdleSession should've been called before re-adding
    LOG(ERROR) << "Session " << session << " already exists!";
    return;
  }
  if (sessionsByIdleAge_.size() < maxIdleCount_) {
    auto newIt = sessionsByIdleAge_.insert(sessionsByIdleAge_.end(),
                                           {session, sessionPool});
    sessionMap_[session] = newIt;
  }
}

void ServerIdleSessionController::removeIdleSession(
    const HTTPSessionBase* session) {
  std::lock_guard<std::mutex> lock(lock_);
  auto it = sessionMap_.find(session);
  if (it != sessionMap_.end()) {
    sessionsByIdleAge_.erase(it->second);
    sessionMap_.erase(it);
  }
}

void ServerIdleSessionController::markForDeath() {
  std::lock_guard<std::mutex> lock(lock_);
  markedForDeath_ = true;
  sessionMap_.clear();
  sessionsByIdleAge_.clear();
}

// must be called under lock_
SessionPool* FOLLY_NULLABLE ServerIdleSessionController::popBestIdlePool() {
  if (!sessionsByIdleAge_.empty()) {
    auto ret = *sessionsByIdleAge_.begin();
    sessionsByIdleAge_.erase(sessionsByIdleAge_.begin());
    sessionMap_.erase(ret.session);
    return ret.sessionPool;
  }
  return nullptr;
}

} // namespace proxygen
