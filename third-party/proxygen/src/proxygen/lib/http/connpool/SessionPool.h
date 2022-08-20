/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/EventBase.h>

#include <proxygen/lib/http/connpool/SessionHolder.h>

namespace proxygen {

class HTTPTransaction;
class ThreadIdleSessionController;
class ServerIdleSessionController;

/**
 * This class stores HTTPSessionBase objects. It simplifies a lot of
 * the process of checking when a session goes bad, invalidating
 * references to it if it is destroyed, and tracking which sessions can
 * support more transactions and which cannot (i.e. it can store both
 * HTTP/1.1 and HTTP2 sessions at the same time).
 *
 * It can only be used from one thread, and sessions are not shared
 * between threads.
 *
 * Destroying a SessionPool will cause all upstream sessions to be
 * immediately forced closed. To drain this session pool gracefully, call
 * drain(). This will immediately close with FIN any idle sessions. As
 * active sessions become idle, they will eventually be closed too.
 *
 * After the drain timeout is reached, delete the SessionPool to force
 * closed any remaining sessions. Remember that this object must be
 * deleted in the event base thread it was used in.
 */
class SessionPool : private SessionHolder::Callback {
 public:
  /**
   * Construct an empty SessionPool.
   *
   * @param stats An optional interface for stats.
   * @param maxPooledSessions The number of sessions allowed in this pool
   * @param idleTimeout The duration in milliseconds after which a session
   *                    that has no outgoing transactions may be purged
   *                    from the pool.
   * @param maxAge The maximum lifetime of a session in milliseconds.  After
   *               this period elapses the session will be (somewhat lazily)
   *               drained and purged from the pool.
   * @param threadIdleSessionController - An optional component that allows
   * pruning idle sessions across different sessions pools.
   * @param serverIdleSessionController - An optional component that allows
   * moving idle sessions between threads.
   */

  explicit SessionPool(
      SessionHolder::Stats* stats = nullptr,
      uint32_t maxPooledSessions = 1,
      std::chrono::milliseconds idleTimeout = std::chrono::milliseconds(1000),
      std::chrono::milliseconds maxAge = std::chrono::milliseconds(0),
      ThreadIdleSessionController* threadIdleSessionController = nullptr,
      ServerIdleSessionController* serverIdleSessionController = nullptr);
  /**
   * Destroying a SessionPool causes the sessions within it to drain. When
   * a session becomes idle (no outgoing transactions) it will close
   * automatically.
   */
  ~SessionPool() override;

  /**
   * Set/get the maximum number of idle sessions that can be
   * pooled. Lowering the maximum number of sessions purges excess idle
   * sessions.
   */
  void setMaxIdleSessions(uint32_t num);
  uint32_t getMaxIdleSessions() const;

  /**
   * Set/get the number of milliseconds that a session must remain
   * unused before it may be purged.
   */
  void setTimeout(std::chrono::milliseconds);
  std::chrono::milliseconds getTimeout() const;

  /**
   * Returns the number of idle sessions. That is, sessions with no open
   * outgoing transactions.
   */
  uint32_t getNumIdleSessions() const;

  /**
   * Returns the number of sessions that already have at least one
   * outgoing transaction open yet still support opening at least one more
   * outgoing transaction.
   */
  uint32_t getNumActiveNonFullSessions() const;

  /**
   * Returns the number of sessions that already have the maximum number
   * of outgoing transactions open on them.
   */
  uint32_t getNumFullSessions() const;

  /**
   * Returns the number of active sessions (txns > 0).
   */
  uint32_t getNumActiveSessions() const;

  /**
   * Returns the total number of pooled sessions, regardless of activity.
   */
  uint32_t getNumSessions() const;

  /**
   * Returns true if this SessionPool has no sessions in it. This implies
   * getNumSessions() == 0
   */
  bool empty() const;

  /**
   * Call this function to add a newly-created session to this
   * server to this object's session pool. This SessionPool object will
   * now manage the session. After passing a session to this function, you
   * must not directly open any more transactions on it. If the session is
   * not in a good state, it will be drained and eventually closed.
   *
   * If the session has an info callback set, it would continue to be invoked
   * until the session is destroyed. Ensure the callback lifetime is at least as
   * long as the session lifetime or reset the info callback to nullptr before
   * putting the session in the pool.
   *
   * @param session The session to put in the pool
   */
  void putSession(HTTPSessionBase* session);

  /**
   * Gets a transaction from the first usable session in the session
   * pools. There are 3 session session pools: one pool contains
   * sessions with no outgoing transactions in 'idleSessionList_'.
   * 'fullSessionList_' contains sessions  that are already completely in
   * use and don't support any more transactions. The last pool is
   * 'unfilledSessionList_' and contains sessions that are in
   * use, but that can support more outgoing transactions.
   *
   * This function checks 'unfilledSessionList_' first. If no sessions are
   * found, it checks idleSessionList_. If still no session is found,
   * nullptr is returned.
   */
  HTTPTransaction* getTransaction(HTTPTransaction::Handler*);

  /**
   * Remove oldest idle session from idleSessionList_.
   */
  HTTPSessionBase* FOLLY_NULLABLE removeOldestIdleSession();

  /**
   * Drain all sessions even if the idle timeout has not expired.
   */
  void drainAllSessions();

  /**
   * Immediately close the socket in both directions for all sessions in the
   * pool, discarding any queued writes that haven't yet been transferred to
   * the kernel, and send a RST to the client.
   *
   * All transactions receive onWriteError with errorCode
   */
  void closeWithReset();

  folly::EventBase* getEventBase() {
    return evb_;
  }

 private:
  /**
   * Purge the excess idle sessions according to the configured limits.
   */
  void purgeExcessIdleSessions();

  /**
   * Calls drain() on all the sessions in the list and empties the list.
   */
  void drainSessionList(SessionList& list);

  /**
   * Calls dropConnection() and all sessions to empty the list.
   */
  void closeSessionListWithReset(SessionList& list);

  /**
   * Attempt to open a transaction on one of the sessions in the given
   * list. Return the transaction if successful, else nullptr.
   */
  HTTPTransaction* attemptOpenTransaction(
      HTTPTransaction::Handler* upstreamHandler, SessionList& list);

  // SessionHolder::Callback methods
  void detachIdle(SessionHolder*) override;
  void detachPartiallyFilled(SessionHolder*) override;
  void detachFilled(SessionHolder*) override;
  void attachIdle(SessionHolder*) override;
  void attachPartiallyFilled(SessionHolder*) override;
  void attachFilled(SessionHolder*) override;
  void addDrainingSession(HTTPSessionBase*) override;

  SessionHolder::Stats* stats_{nullptr};
  // Max number of connections stored in the pool.
  uint32_t maxConns_;
  std::chrono::milliseconds timeout_;
  std::chrono::milliseconds maxAge_;

  // List of all idle sessions in this SessionPool. Sessions
  // are sorted in descending order of lastUseTime in the list.
  SessionList idleSessionList_;
  // List of active sessions which may have space for
  // another transaction. Sessions are sorted in descending order of
  // lastUseTime in the list. Note that this list will never contain
  // sessions that are using a serial L7 protocol like HTTP/1.0 (and
  // 1.1 since we don't support pipelining).
  SessionList unfilledSessionList_;
  // List of active sessions are full and cannot open any more
  // transactions.
  SessionList fullSessionList_;
  // Manages idle sessions for the same thread across servers.
  ThreadIdleSessionController* threadIdleSessionController_{nullptr};
  // Manages idle sessions for the same server across threads.
  ServerIdleSessionController* serverIdleSessionController_{nullptr};

  folly::EventBase* const evb_{nullptr};
};

std::ostream& operator<<(std::ostream& os, const SessionPool& pool);

} // namespace proxygen
