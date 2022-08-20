/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/connpool/SessionPool.h>

#include <folly/futures/Future.h>

namespace proxygen {

/**
 * ServerIdleSessionController keeps track of all idle sessions belonging
 * to a **single server** across all threads.
 *
 * Server class uses it to move idle transactions between threads, if necessary.
 * All public methods are thread-safe.
 */
class ServerIdleSessionController {
 public:
  explicit ServerIdleSessionController() {
  }

  /**
   * Transfer idle session from another thread, if available.
   * Returns nullptr if nothing is available.
   */
  folly::Future<HTTPSessionBase*> getIdleSession();

  /**
   * Add/remove session info (called by SessionPool when state changes).
   */
  void addIdleSession(const HTTPSessionBase* session, SessionPool* sessionPool);
  void removeIdleSession(const HTTPSessionBase* session);

  /**
   * Stop all session transfers.
   */
  void markForDeath();

  /**
   * Resize idle pool.
   */
  void setMaxIdleCount(unsigned int maxIdleCount) {
    std::lock_guard<std::mutex> lock(lock_);
    maxIdleCount_ = maxIdleCount;
  }

 protected:
  struct IdleSessionInfo {
    const HTTPSessionBase* session;
    SessionPool* sessionPool;
  };

  using IdleSessionList = std::list<IdleSessionInfo>;
  using IdleSessionListIter = IdleSessionList::iterator;

  /**
   * Find available session pool (thread) to tranfer an idle session from.
   * Remove it from the map.
   */
  SessionPool* FOLLY_NULLABLE popBestIdlePool();

  bool isMarkedForDeath() {
    std::lock_guard<std::mutex> lock(lock_);
    return markedForDeath_;
  }

  std::mutex lock_;
  /*
   * List of idle sessions. Normally, addIdleSession() adds to the end and
   * popBestIdlePool() removes from the beginning, thus keeping the list sorted
   * by idle age.
   * Additionally, we also support arbitrary removals if some session stops
   * being idle or dies or gets re-used out of order if many threads attempt
   * session transfer at the same time.
   */
  IdleSessionList sessionsByIdleAge_;
  // Store iterators in sessionsByIdleAge_ to be able to find sessions.
  std::unordered_map<const HTTPSessionBase*, IdleSessionListIter> sessionMap_;
  bool markedForDeath_{false};

  // Default idle pool size to 2.
  unsigned int maxIdleCount_{2};
};

} // namespace proxygen
