/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include "proxygen/lib/http/coro/client/HTTPCoroConnector.h"
#include "proxygen/lib/http/coro/client/HTTPSessionFactory.h"
#include <folly/logging/xlog.h>

#include <folly/IntrusiveList.h>

namespace proxygen::coro {

class HTTPCoroSessionPool {
 private:
  class Holder : private LifecycleObserver {
   public:
    Holder(HTTPCoroSession& inSession, HTTPCoroSessionPool& pool);
    Holder(const Holder&) = delete;
    Holder& operator=(const Holder&) = delete;
    ~Holder() override;

    HTTPCoroSession& session;

   private:
    void onTransactionAttached(const HTTPCoroSession& session) override;
    void onTransactionDetached(const HTTPCoroSession& session) override;
    void onSettingsOutgoingStreamsFull(const HTTPCoroSession& sess) override;
    void onSettingsOutgoingStreamsNotFull(const HTTPCoroSession& sess) override;
    void onDrainStarted(const HTTPCoroSession& session) override;
    void onDestroy(const HTTPCoroSession& session) override;
    void onDeactivateConnection(const HTTPCoroSession& session) override;

    HTTPCoroSessionPool& pool_;
    enum State : uint8_t { Idle = 0, Available, Full } state_{Available};
    folly::SafeIntrusiveListHook listHook_;
    // To make IntrusiveList work
    friend class HTTPCoroSessionPool;
    std::chrono::seconds maxAge_;
  };
  using SessionList = folly::CountedIntrusiveList<Holder, &Holder::listHook_>;

 public:
  struct PoolParams {
    uint32_t maxConnections{100};
    uint8_t maxConnectionAttempts{3};
    bool enableSslSessionCaching{true};
    std::chrono::milliseconds connectTimeout{std::chrono::seconds(2)};
    std::chrono::seconds maxAge{std::chrono::minutes(10)};
    uint16_t maxWaiters{std::numeric_limits<uint16_t>::max()};
  };

  class Observer {
   public:
    void incrementPendingUpstreamConnections(int count) {
      onPendingUpstreamConnectionIncrement(count);
    }

   protected:
    ~Observer() = default;

   private:
    virtual void onPendingUpstreamConnectionIncrement(int count) = 0;
  };

  static PoolParams defaultPoolParams() {
    static PoolParams defaultParams;
    return defaultParams;
  }

  HTTPCoroSessionPool(folly::EventBase* evb,
                      const std::string& server,
                      uint16_t port,
                      PoolParams poolParams = defaultPoolParams(),
                      HTTPCoroConnector::ConnectionParams connParams =
                          HTTPCoroConnector::defaultConnectionParams(),
                      HTTPCoroConnector::SessionParams sessionParams =
                          HTTPCoroConnector::defaultSessionParams(),
                      bool allowNameLookup = false,
                      Observer* observer = nullptr);

  HTTPCoroSessionPool(folly::EventBase* evb,
                      folly::SocketAddress socketAddress,
                      PoolParams poolParams = defaultPoolParams(),
                      HTTPCoroConnector::ConnectionParams connParams =
                          HTTPCoroConnector::defaultConnectionParams(),
                      HTTPCoroConnector::SessionParams sessionParams =
                          HTTPCoroConnector::defaultSessionParams(),
                      Observer* observer = nullptr);

  HTTPCoroSessionPool(folly::EventBase* evb,
                      const std::string& server,
                      uint16_t port,
                      std::shared_ptr<HTTPCoroSessionPool> proxyPool,
                      PoolParams poolParams = defaultPoolParams(),
                      HTTPCoroConnector::ConnectionParams connParams =
                          HTTPCoroConnector::defaultConnectionParams(),
                      HTTPCoroConnector::SessionParams sessionParams =
                          HTTPCoroConnector::defaultSessionParams(),
                      Observer* observer = nullptr);

  HTTPCoroSessionPool(
      folly::EventBase* evb,
      const std::string& server,
      uint16_t port,
      PoolParams poolParams,
      std::shared_ptr<const HTTPCoroConnector::QuicConnectionParams> connParams,
      HTTPCoroConnector::SessionParams sessionParams =
          HTTPCoroConnector::defaultSessionParams(),
      bool allowNameLookup = false,
      Observer* observer = nullptr);

  HTTPCoroSessionPool(const HTTPCoroSessionPool& other) = delete;
  HTTPCoroSessionPool& operator=(const HTTPCoroSessionPool& other) = delete;
  HTTPCoroSessionPool(HTTPCoroSessionPool&& goner) = delete;
  HTTPCoroSessionPool& operator=(HTTPCoroSessionPool&& goner) = delete;

  virtual ~HTTPCoroSessionPool() {
    if (!isDraining()) {
      drain();
    }
    XCHECK_EQ(waiters_.size(), 0UL);
    XCHECK_EQ(connectsInProgress_, 0UL);
    XCHECK(idleSessions_.empty());
    XCHECK(availableSessions_.empty());
    XCHECK(fullSessions_.empty());
  }

  [[nodiscard]] bool isSecure() const {
    return quicConnParams_ ||
           tcpConnParams_.fizzContextAndVerifier.fizzContext ||
           tcpConnParams_.sslContext;
  }

  void setMaxConnectionAttempts(uint8_t maxConnectionAttempts) {
    poolParams_.maxConnectionAttempts =
        std::max<uint8_t>(1u, maxConnectionAttempts);
  }

  void setPoolingEnabled(bool poolingEnabled) {
    poolingEnabled_ = poolingEnabled;
  }

  void setMaxConnections(uint32_t maxConnections) {
    // does not clear existing sessions
    poolParams_.maxConnections = std::max(1u, maxConnections);
    poolingEnabled_ = maxConnections > 0;
  }

  void setSessionParams(HTTPCoroConnector::SessionParams sessionParams) {
    sessionParams_ = sessionParams;
  }

  void setMaxAge(std::chrono::seconds maxAge) {
    poolParams_.maxAge = maxAge;
  }

  bool full() const {
    return idleSessions_.size() + availableSessions_.size() +
               fullSessions_.size() + connectsInProgress_ >=
           poolParams_.maxConnections;
  }

  bool hasAvailableSessions() const {
    return availableSessions_.size() + idleSessions_.size();
  }

  bool hasIdleSessions() const {
    return idleSessions_.size();
  }

  [[nodiscard]] bool overflowed() const {
    return idleSessions_.size() + availableSessions_.size() +
               fullSessions_.size() + connectsInProgress_ >
           poolParams_.maxConnections;
  }

  [[nodiscard]] bool empty() const {
    return !hasSession() && waiters_.empty() && connectsInProgress_ == 0;
  }

  class Exception : public std::runtime_error {
   public:
    enum class Type { ConnectFailed, Timeout, Cancelled, Draining, MaxWaiters };
    explicit Exception(Type t,
                       const std::string& msg,
                       folly::exception_wrapper ex = folly::exception_wrapper())
        : std::runtime_error(folly::to<std::string>(
              msg, ex ? std::string(": " + ex.what()) : std::string())),
          type(t),
          connectException(std::move(ex)) {
    }

    Type type;
    // Set when Type == ConnectFailed
    folly::exception_wrapper connectException;
  };

  using GetSessionResult = HTTPSessionFactory::GetSessionResult;
  virtual folly::coro::Task<GetSessionResult> getSessionWithReservation();

  void drain();

  void flush();

  void describe(std::ostream& os) const;

  [[nodiscard]] folly::EventBase* getEventBase() const {
    return eventBase_;
  }

  void setConnParams(const HTTPCoroConnector::ConnectionParams& connParams) {
    tcpConnParams_ = connParams;
    setSslManager();
  }

  void setQuicConnectionParams(
      std::shared_ptr<const HTTPCoroConnector::QuicConnectionParams>
          quicConnParams) {
    if (!quicConnParams_) {
      return; // ignored for non-QUIC pools
    }
    quicConnParams_ = std::move(quicConnParams);
  }

  /**
   * Observers are notified whenever an idle session is added & removed. The
   * pool is passed in as an argument so callbacks can check if the pool has
   * idle sessions via ::hasIdleSessions
   */
  class IdleSessionObserverIf {
   public:
    IdleSessionObserverIf() = default;
    virtual ~IdleSessionObserverIf() = default;
    virtual void onIdleSessionsChanged(
        const HTTPCoroSessionPool& pool) noexcept = 0;
  };

 protected:
  void insertIdleSession(HTTPSessionContextPtr&& sess) noexcept;
  HTTPSessionContextPtr detachIdleSession() noexcept;

  void setIdleSessionObserver(IdleSessionObserverIf* obs) {
    idleSessionObserver_ = obs;
  }

 private:
  SessionList& getSessionList(HTTPCoroSessionPool::Holder& holder);
  void moveToFull(HTTPCoroSessionPool::Holder& holder);
  void moveToAvailable(HTTPCoroSessionPool::Holder& holder);
  void moveToIdle(HTTPCoroSessionPool::Holder& holder);
  void addSession(HTTPCoroSessionPool::Holder& holder, Holder::State state);
  void removeSession(HTTPCoroSessionPool::Holder& holder, bool checkForWaiters);
  void notifyIdleSessionObserver() const noexcept;
  bool shouldAgeOut(HTTPCoroSessionPool::Holder& holder) const;
  folly::coro::Task<void> addNewConnection();
  void signalWaiters(uint32_t n);
  void cancelWaiters(Exception ex);
  bool hasSession() const;
  void setSslManager();
  bool isDraining() const {
    return cancellationSource_.isCancellationRequested();
  }

  folly::EventBase* eventBase_{nullptr};
  Observer* observer_{nullptr};
  folly::SocketAddress serverAddress_;
  std::string authority_;
  PoolParams poolParams_;
  std::shared_ptr<HTTPCoroSessionPool> proxyPool_;
  HTTPCoroConnector::ConnectionParams tcpConnParams_;
  std::shared_ptr<const HTTPCoroConnector::QuicConnectionParams>
      quicConnParams_;
  HTTPCoroConnector::SessionParams sessionParams_;
  SessionList idleSessions_;
  SessionList availableSessions_;
  SessionList fullSessions_;
  folly::CancellationSource cancellationSource_;
  uint64_t connectsInProgress_{0};
  struct Waiter {
    detail::CancellableBaton baton;
    folly::CancellationSource cancellationSource;
    folly::Optional<Exception> exception;
    folly::SafeIntrusiveListHook listHook_;
    void cancel(Exception ex);
  };
  using WaiterList = folly::CountedIntrusiveList<Waiter, &Waiter::listHook_>;
  WaiterList waiters_;
  class SslSessionManager : public HTTPCoroConnector::SslSessionManagerIf {
    void onNewSslSession(SslSessionPtr session) noexcept override {
      sslSession = std::move(session);
    }
    SslSessionPtr getSslSession() noexcept override {
      return sslSession;
    }

   private:
    std::shared_ptr<folly::ssl::SSLSession> sslSession{nullptr};
  } sslSessionManager_;
  bool poolingEnabled_{true};
  IdleSessionObserverIf* idleSessionObserver_{nullptr};
};

std::ostream& operator<<(std::ostream& os, const HTTPCoroSessionPool& pool);
} // namespace proxygen::coro
