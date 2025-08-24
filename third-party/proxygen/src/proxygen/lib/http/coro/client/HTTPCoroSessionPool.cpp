/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/client/HTTPCoroSessionPool.h"
#include "proxygen/lib/http/coro/client/HTTPCoroConnector.h"
#include <chrono>
#include <folly/Random.h>
#include <folly/logging/xlog.h>
#include <proxygen/lib/utils/Time.h>

using folly::coro::co_error;
using folly::coro::co_withCancellation;

namespace {
constexpr double kJitterPct = 0.3;

using Ex = proxygen::coro::HTTPCoroSessionPool::Exception;
inline Ex maxWaitersEx() {
  return Ex{Ex::Type::MaxWaiters, "max waiters exceeded"};
}

} // namespace

namespace proxygen::coro {

HTTPCoroSessionPool::Holder::Holder(HTTPCoroSession& inSession,
                                    HTTPCoroSessionPool& pool)
    : session(inSession), pool_(pool) {
  using std::chrono::duration;
  using std::chrono::duration_cast;
  using std::chrono::seconds;

  inSession.addLifecycleObserver(this);

  maxAge_ = duration_cast<seconds>(duration<double>(
      pool_.poolParams_.maxAge.count() *
      (1 + folly::Random::randDouble(-kJitterPct, kJitterPct))));
}

HTTPCoroSessionPool::Holder::~Holder() {
  session.removeLifecycleObserver(this);
  pool_.removeSession(*this, /*checkForWaiters=*/true);
  if (state_ == Idle) {
    pool_.notifyIdleSessionObserver();
  }
}

void HTTPCoroSessionPool::Holder::onTransactionAttached(
    const HTTPCoroSession& sess) {
  XCHECK_EQ(&sess, &session);
  // Shouldn't happen, but maybe there's a race and we should keep going
  XLOG_IF(DFATAL, state_ == Full)
      << "onTransactionAttached to unavailable session";
  if (!session.supportsMoreTransactions()) {
    pool_.moveToFull(*this);
  } else if (state_ == Idle) {
    pool_.moveToAvailable(*this);
  } // else already available (neither full nor idle)
}

void HTTPCoroSessionPool::Holder::onTransactionDetached(
    const HTTPCoroSession& sess) {
  XCHECK_EQ(&sess, &session);
  if (!pool_.poolingEnabled_) {
    session.initiateDrain();
  } else if (state_ == Full && session.supportsMoreTransactions()) {
    pool_.moveToAvailable(*this);
  }
}
void HTTPCoroSessionPool::Holder::onSettingsOutgoingStreamsFull(
    const HTTPCoroSession& sess) {
  XCHECK_EQ(&sess, &session);
  pool_.moveToFull(*this);
}

void HTTPCoroSessionPool::Holder::onSettingsOutgoingStreamsNotFull(
    const HTTPCoroSession& sess) {
  onTransactionDetached(sess);
}

void HTTPCoroSessionPool::Holder::onDrainStarted(const HTTPCoroSession& sess) {
  XLOG(DBG4) << "onDrainStarted pool=" << pool_ << " session=" << session;
  onDestroy(sess);
}

void HTTPCoroSessionPool::Holder::onDestroy(const HTTPCoroSession& sess) {
  XLOG(DBG4) << "Destroying session holder in pool=" << pool_
             << " session=" << session;
  XCHECK_EQ(&sess, &session);
  delete this;
}

void HTTPCoroSessionPool::Holder::onDeactivateConnection(
    const HTTPCoroSession& sess) {
  XLOG(DBG4) << "onDeactivateConnection pool=" << pool_
             << " session=" << session;
  if (sess.supportsMoreTransactions()) {
    pool_.moveToIdle(*this);
  }
}

HTTPCoroSessionPool::HTTPCoroSessionPool(
    folly::EventBase* evb,
    const std::string& server,
    uint16_t port,
    PoolParams poolParams,
    HTTPCoroConnector::ConnectionParams connParams,
    HTTPCoroConnector::SessionParams sessionParams,
    bool allowNameLookup,
    Observer* observer)
    : HTTPCoroSessionPool(evb,
                          folly::SocketAddress{server, port, allowNameLookup},
                          poolParams,
                          std::move(connParams),
                          std::move(sessionParams),
                          observer) {
}

HTTPCoroSessionPool::HTTPCoroSessionPool(
    folly::EventBase* evb,
    folly::SocketAddress socketAddress,
    PoolParams poolParams,
    HTTPCoroConnector::ConnectionParams connParams,
    HTTPCoroConnector::SessionParams sessionParams,
    Observer* observer)
    : eventBase_(evb),
      observer_(observer),
      serverAddress_(std::move(socketAddress)),
      authority_(serverAddress_.describe()),
      poolParams_(poolParams),
      tcpConnParams_(std::move(connParams)),
      sessionParams_(std::move(sessionParams)) {
  setMaxConnections(poolParams.maxConnections);
  setMaxConnectionAttempts(poolParams.maxConnectionAttempts);
  XLOG(DBG4) << "Creating pool for " << *this
             << " maxConnections_=" << poolParams_.maxConnections
             << " observer=" << observer_;
  setSslManager();
}

HTTPCoroSessionPool::HTTPCoroSessionPool(
    folly::EventBase* evb,
    const std::string& server,
    uint16_t port,
    std::shared_ptr<HTTPCoroSessionPool> proxyPool,
    PoolParams poolParams,
    HTTPCoroConnector::ConnectionParams connParams,
    HTTPCoroConnector::SessionParams sessionParams,
    Observer* observer)
    : eventBase_(evb),
      observer_(observer),
      authority_(folly::to<std::string>(server, ":", port)),
      poolParams_(poolParams),
      proxyPool_(std::move(proxyPool)),
      tcpConnParams_(std::move(connParams)),
      sessionParams_(std::move(sessionParams)) {
  setMaxConnections(poolParams.maxConnections);
  setMaxConnectionAttempts(poolParams.maxConnectionAttempts);
  XLOG(DBG4) << "Creating pool for " << *this
             << " maxConnections_=" << poolParams_.maxConnections
             << " observer=" << observer_;
  setSslManager();
}

HTTPCoroSessionPool::HTTPCoroSessionPool(
    folly::EventBase* evb,
    const std::string& server,
    uint16_t port,
    PoolParams poolParams,
    std::shared_ptr<const HTTPCoroConnector::QuicConnectionParams> connParams,
    HTTPCoroConnector::SessionParams sessionParams,
    bool allowNameLookup,
    Observer* observer)
    : eventBase_(evb),
      observer_(observer),
      serverAddress_(server, port, allowNameLookup),
      authority_(serverAddress_.describe()),
      poolParams_(poolParams),
      quicConnParams_(std::move(connParams)),
      sessionParams_(std::move(sessionParams)) {
  setMaxConnections(poolParams.maxConnections);
  setMaxConnectionAttempts(poolParams.maxConnectionAttempts);
  XLOG(DBG4) << "Creating pool for " << *this
             << " maxConnections_=" << poolParams_.maxConnections
             << " observer=" << observer_;
}

void HTTPCoroSessionPool::setSslManager() {
  if (tcpConnParams_.sslSessionManager == nullptr &&
      poolParams_.enableSslSessionCaching) {
    tcpConnParams_.sslSessionManager = &sslSessionManager_;
  }
}

HTTPCoroSessionPool::SessionList& HTTPCoroSessionPool::getSessionList(
    HTTPCoroSessionPool::Holder& holder) {
  switch (holder.state_) {
    case Holder::State::Idle:
      return idleSessions_;
    case Holder::State::Available:
      return availableSessions_;
    case Holder::State::Full:
      return fullSessions_;
  }
  folly::assume_unreachable();
}

void HTTPCoroSessionPool::moveToFull(HTTPCoroSessionPool::Holder& holder) {
  XLOG(DBG4) << "Session now full, pool=" << *this
             << " session=" << holder.session;
  removeSession(holder, /*checkForWaiters=*/false);
  addSession(holder, Holder::State::Full);
}

/*
 * Always push to the back and pop from the front (FIFO). This prevents
 * slow servers from being excessively re-used and drawing load to themselves.
 */
void HTTPCoroSessionPool::moveToAvailable(HTTPCoroSessionPool::Holder& holder) {
  XLOG(DBG4) << "Session now available, pool=" << *this
             << " session=" << holder.session;
  // TODO: consider leaving in fullSessions if the pool is over-full somehow?
  removeSession(holder, /*checkForWaiters=*/false);
  addSession(holder, Holder::State::Available);
}

void HTTPCoroSessionPool::moveToIdle(HTTPCoroSessionPool::Holder& holder) {
  XLOG(DBG4) << "Session now idle, pool=" << *this
             << " session=" << holder.session;
  // TODO: consider leaving in fullSessions if the pool is over-full somehow?
  removeSession(holder, /*checkForWaiters=*/false);
  addSession(holder, Holder::State::Idle);
}

void HTTPCoroSessionPool::addSession(HTTPCoroSessionPool::Holder& holder,
                                     Holder::State state) {
  Holder::State prevState = std::exchange(holder.state_, state);
  auto& sessionList = getSessionList(holder);
  sessionList.push_back(holder);
  if (state != Holder::State::Full) {
    signalWaiters(holder.session.numTransactionsAvailable());
  }

  // if either removing or adding an idle session, invoke observer
  if (prevState == Holder::State::Idle || state == Holder::State::Idle) {
    notifyIdleSessionObserver();
  }
}

void HTTPCoroSessionPool::removeSession(HTTPCoroSessionPool::Holder& holder,
                                        bool checkForWaiters) {
  auto& sessionList = getSessionList(holder);
  sessionList.erase(sessionList.iterator_to(holder));

  if (checkForWaiters && !isDraining() && !waiters_.empty() && !full()) {
    XLOG(DBG5) << "Initiating new connection attempt, pool=" << *this;
    connectsInProgress_++;
    XLOG(DBG4) << *this << ":" << serverAddress_
               << " ++connectsInProgress_ = " << connectsInProgress_;
    co_withExecutor(
        eventBase_,
        co_withCancellation(cancellationSource_.getToken(), addNewConnection()))
        .start();
  }
}

bool HTTPCoroSessionPool::shouldAgeOut(
    HTTPCoroSessionPool::Holder& holder) const {
  if (poolParams_.maxAge.count() == 0) {
    return false;
  }

  auto age = secondsSince(holder.session.getStartTime());
  return age >= holder.maxAge_;
}

folly::coro::Task<HTTPCoroSessionPool::GetSessionResult>
HTTPCoroSessionPool::getSessionWithReservation() {
  // This coroutine performs late binding.  It will grab the head of the
  // availableSessions_, or wait for it to become non-empty.
  //
  // It will optionally start a coro to create a new connection, if there is
  // room in this pool.  The new connection will race with existing connections
  // to become available.
  auto start = getCurrentTime();
  const auto& reqToken = co_await folly::coro::co_current_cancellation_token;
  auto poolToken = cancellationSource_.getToken();
  auto mergedToken = folly::cancellation_token_merge(reqToken, poolToken);

  while (!mergedToken.isCancellationRequested()) {
    while (hasAvailableSessions()) {
      auto& holder = availableSessions_.empty() ? idleSessions_.front()
                                                : availableSessions_.front();
      bool supportsMoreTransactions = holder.session.supportsMoreTransactions();
      if (supportsMoreTransactions && !shouldAgeOut(holder)) {
        XLOG(DBG4) << "Session available, pool=" << *this
                   << " returning session=" << holder.session;
        auto& session = holder.session;
        auto maybeReservation = session.reserveRequest();
        if (maybeReservation.hasException()) {
          co_yield co_error(std::move(maybeReservation.exception()));
        }
        co_return GetSessionResult(std::move(*maybeReservation), &session);
      } else if (!supportsMoreTransactions) {
        XLOG(ERR) << "Full session in available!, pool=" << *this
                  << " sess=" << holder.session;
        moveToFull(holder);
      } else {
        XLOG(DBG4) << "Draining too-old session, pool=" << *this
                   << " session=" << holder.session;
        holder.session.initiateDrain();
      }
    }
    if (millisecondsSince(start) > poolParams_.connectTimeout) {
      XLOG(ERR) << "Timed out waiting for session, pool=" << *this;
      co_yield co_error(
          Exception(Exception::Type::Timeout, "Timed out waiting for session"));
    }
    if (!full()) {
      // A thundering herd can occur here for multiplexed protocols.  If the
      // pool is empty, a rush of getSession calls will each start a new
      // connection, even if the first one is multiplexed and can satisfy all
      // of them.
      // TODO: keep an average of the last N max_streams per conn, and delay
      // creating a new connection if a pending one is likely to satisfy this
      XLOG(DBG5) << "Initiating new connection attempt, pool=" << *this;
      connectsInProgress_++;
      XLOG(DBG4) << *this << ":" << serverAddress_
                 << " ++connectsInProgress_ = " << connectsInProgress_;
      co_withExecutor(eventBase_,
                      co_withCancellation(cancellationSource_.getToken(),
                                          addNewConnection()))
          .start();
    }
    Waiter waiter;
    waiters_.push_back(waiter);
    // we may be exceeding the max waiters
    signalWaiters(0);
    XLOG(DBG4) << "No sessions available, waiting, pool=" << *this;
    auto res = co_await co_withCancellation(
        waiter.cancellationSource.getToken(),
        waiter.baton.timedWait(eventBase_, poolParams_.connectTimeout));
    if (res == TimedBaton::Status::cancelled) {
      XLOG_IF(DBG4, !poolToken.isCancellationRequested())
          << "getSession cancelled, pool=" << *this;
      if (waiter.exception) {
        co_yield co_error(std::move(*waiter.exception));
      } else {
        co_yield co_error(Exception(Exception::Type::Cancelled, "Cancelled"));
      }
    }
    if (waiter.listHook_.is_linked()) {
      waiters_.erase(waiters_.iterator_to(waiter));
    }
    // loop around to check, including on timeout
  }

  if (reqToken.isCancellationRequested()) {
    co_yield folly::coro::co_error(
        Exception(Exception::Type::Cancelled, "Cancelled"));
  }

  co_yield co_error(Exception(Exception::Type::Draining, "Pool is draining"));
}

folly::coro::Task<void> HTTPCoroSessionPool::addNewConnection() {
  const auto& cancelToken = co_await folly::coro::co_current_cancellation_token;
  co_await folly::coro::co_safe_point;
  auto g = folly::makeGuard([&, observer = observer_] {
    if (observer) {
      XLOG(DBG4) << "--pendingUpstreamConnections";
      observer->incrementPendingUpstreamConnections(-1);
    }
    if (!cancelToken.isCancellationRequested()) {
      connectsInProgress_--;
      XLOG(DBG4) << *this << ":" << serverAddress_
                 << " --connectsInProgress_ = " << connectsInProgress_;
    }
  });
  if (observer_) {
    XLOG(DBG4) << "++pendingUpstreamConnections, pool=" << *this;
    observer_->incrementPendingUpstreamConnections(1);
  } else {
    XLOG(DBG4) << "observer not set, pool=" << *this;
  }
  folly::exception_wrapper exceptionWrapper;
  XLOG(DBG4) << "Getting session, pool=" << *this;
  for (uint16_t attempt = 0; attempt < poolParams_.maxConnectionAttempts;
       attempt++) {
    folly::Try<HTTPCoroSession*> sessionTry;
    if (quicConnParams_) {
      sessionTry = co_await co_awaitTry(
          HTTPCoroConnector::connect(eventBase_,
                                     serverAddress_,
                                     poolParams_.connectTimeout,
                                     *quicConnParams_,
                                     sessionParams_));
    } else if (proxyPool_) {
      auto proxySession =
          co_await co_awaitTry(proxyPool_->getSessionWithReservation());
      if (proxySession.hasException()) {
        sessionTry.emplaceException(std::move(proxySession.exception()));
      } else {
        co_await folly::coro::co_safe_point;
        sessionTry = co_await co_awaitTry(HTTPCoroConnector::proxyConnect(
            proxySession->session,
            std::move(proxySession->reservation),
            authority_,
            /*connectUnique=*/false,
            poolParams_.connectTimeout,
            tcpConnParams_,
            sessionParams_));
      }
    } else {
      sessionTry = co_await co_awaitTry(
          HTTPCoroConnector::connect(eventBase_,
                                     serverAddress_,
                                     poolParams_.connectTimeout,
                                     tcpConnParams_,
                                     sessionParams_));
    }
    if (cancelToken.isCancellationRequested()) {
      if (!sessionTry.hasException()) {
        sessionTry.value()->initiateDrain();
      }
      co_yield co_error(folly::OperationCancelled{});
    }
    if (sessionTry.hasException()) {
      XLOG(DBG3) << "Failed to get connection for pool=" << *this
                 << " err=" << sessionTry.exception().what()
                 << ", server=" << serverAddress_;
      exceptionWrapper = sessionTry.exception();
      if (overflowed()) {
        XLOG(ERR) << "Too many connections in progress=" << " pending="
                  << connectsInProgress_ << ", idle=" << idleSessions_.size()
                  << ", available=" << availableSessions_.size()
                  << ", full=" << fullSessions_.size();
      }
      if (waiters_.empty()) {
        // No one is waiting
        XLOG(INFO) << "Giving up on a connection attempt, no waiters";
        break;
      }
      XLOG(DBG4) << *this << ":" << serverAddress_
                 << " ++connectsInProgress_ = " << connectsInProgress_;
    } else {
      auto session = *sessionTry;
      XLOG(DBG4) << "Got upstream, pool=" << *this << " session=" << *session;
      auto* holder = new Holder(*session, *this);
      // defaulted available since immediate local use is expected
      addSession(*holder, holder->state_);
      co_return;
    }
  }
  if (!waiters_.empty() && !hasSession() && connectsInProgress_ == 1) {
    XLOG(DBG4) << "Cancelling waiters in pool=" << *this;
    // The last running connect exited in failure, but there are getSession
    // calls waiting.  Cancel them so they return.
    cancelWaiters(Exception(Exception::Type::ConnectFailed,
                            "Connect Failed",
                            std::move(exceptionWrapper)));
  }
}

void HTTPCoroSessionPool::signalWaiters(uint32_t n) {
  XLOG(DBG4) << "signalWaiters n=" << n
             << ", waiters_.size()=" << waiters_.size();
  while (!waiters_.empty() && n-- > 0) {
    // Remove waiters as they are signalled
    waiters_.front().baton.signal();
    waiters_.pop_front();
  }

  // cancel excess waiters if full, until <= maxWaiters
  const bool full = fullSessions_.size() >= poolParams_.maxConnections;
  while (full && (waiters_.size() > poolParams_.maxWaiters)) {
    waiters_.back().cancel(maxWaitersEx());
    waiters_.pop_back();
  }
}

void HTTPCoroSessionPool::Waiter::cancel(Exception ex) {
  exception = std::move(ex);
  cancellationSource.requestCancellation();
}

void HTTPCoroSessionPool::cancelWaiters(Exception ex) {
  for (auto& waiter : waiters_) {
    waiter.cancel(ex);
  }
  waiters_.clear();
}

void HTTPCoroSessionPool::drain() {
  XCHECK(!eventBase_->isRunning() || eventBase_->isInEventBaseThread());
  cancellationSource_.requestCancellation();
  XLOG(DBG4) << *this << ":" << serverAddress_
             << " connectsInProgress_ = " << connectsInProgress_ << " at drain";
  connectsInProgress_ = 0;
  cancelWaiters(Exception(Exception::Type::Draining, "Draining"));
  flush();
  idleSessionObserver_ = nullptr;
}

void HTTPCoroSessionPool::flush() {
  cancelWaiters(Exception(Exception::Type::Draining, "Flushing"));

  // Draining a session will delete the holder and pop it off the list
  while (!fullSessions_.empty()) {
    auto& holder = fullSessions_.front();
    holder.session.initiateDrain();
  }

  while (!availableSessions_.empty()) {
    auto& holder = availableSessions_.front();
    holder.session.initiateDrain();
  }

  while (!idleSessions_.empty()) {
    auto& holder = idleSessions_.front();
    holder.session.initiateDrain();
  }
}

HTTPSessionContextPtr HTTPCoroSessionPool::detachIdleSession() noexcept {
  XLOG(DBG6) << "pool=" << *this;
  if (!hasIdleSessions() || !waiters_.empty()) {
    return {};
  }

  for (auto& holder : idleSessions_) {
    auto& session = holder.session;
    if (session.isDetachable()) {
      XLOG(DBG6) << __func__ << "; found detachable sess=" << session;
      session.detachEvb();
      delete &holder;
      return session.acquireKeepAlive();
    } else {
      XLOG(ERR) << __func__ << "idle session not detachable";
    }
  }

  return {}; // no detachable sessions
}

void HTTPCoroSessionPool::insertIdleSession(
    HTTPSessionContextPtr&& sess) noexcept {
  XLOG(DBG6) << __func__ << "; sess=" << sess.get();
  auto* session = static_cast<HTTPCoroSession*>(sess.get());
  XCHECK_EQ(sess->getEventBase(), eventBase_);
  auto* holder = std::make_unique<Holder>(*session, *this).release();
  // set as available since immediate local use is expected
  addSession(*holder, Holder::State::Available);
}

void HTTPCoroSessionPool::notifyIdleSessionObserver() const noexcept {
  if (idleSessionObserver_ && !isDraining()) {
    idleSessionObserver_->onIdleSessionsChanged(*this);
  }
}

bool HTTPCoroSessionPool::hasSession() const {
  return !idleSessions_.empty() || !availableSessions_.empty() ||
         !fullSessions_.empty();
}

void HTTPCoroSessionPool::describe(std::ostream& os) const {
  os << "server=" << authority_ << ", idle=" << idleSessions_.size()
     << ", available=" << availableSessions_.size()
     << ", full=" << fullSessions_.size()
     << ", connectsInProgress=" << connectsInProgress_
     << ", waiters=" << waiters_.size()
     << ", poolingEnabled_=" << poolingEnabled_;
}

std::ostream& operator<<(std::ostream& os, const HTTPCoroSessionPool& pool) {
  pool.describe(os);
  return os;
}

} // namespace proxygen::coro
