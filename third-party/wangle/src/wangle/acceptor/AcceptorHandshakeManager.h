/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/ExceptionWrapper.h>
#include <folly/Optional.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <wangle/acceptor/ManagedConnection.h>
#include <wangle/acceptor/SecureTransportType.h>
#include <wangle/acceptor/TransportInfo.h>
#include <chrono>

namespace wangle {

/**
 * Helper to return a string of the given transport's client IP and port.
 */
std::string describeAddresses(const folly::AsyncTransport* transport);

class Acceptor;

/**
 * AcceptorHandshakeHelper represents the lifecycle of a single asynchronous
 * handshake operation performed on a specific connection.
 *
 * The asynchronous operation is initiated with `AcceptorHandshakeHelper::start`
 *
 * The asynchronous operation completes with an invocation to one either
 *    1) Callback::connectionReady - indicating a successful handshake.
 *    2) Callback::connectionError - indicating a failed handshake.
 *
 * The asynchronous operation may be canceled via an invocation of
 * `AcceptorHandshakeHelper::dropConnection`. `dropConnection()` is guaranteed
 * to synchronously invoke a terminal callback when canceled.
 */
class AcceptorHandshakeHelper : public folly::DelayedDestruction {
 public:
  using UniquePtr = folly::DelayedDestructionUniquePtr<AcceptorHandshakeHelper>;

  /**
   * AcceptorHandshakeHelper::Callback is the result channel of the
   * asynchronous handshake operation.
   *
   * It is guaranteed that exactly *one* of these methods will be called
   * when the asynchronous handshake (1) completes or (2) is canceled.
   */
  class Callback {
   public:
    virtual ~Callback() = default;

    /**
     * Called after handshake has been completed successfully.
     *
     * If sslErr is set, Acceptor::updateSSLStats will be called.
     */
    virtual void connectionReady(
        folly::AsyncTransport::UniquePtr transport,
        std::string nextProtocol,
        SecureTransportType secureTransportType,
        folly::Optional<SSLErrorEnum> sslErr) noexcept = 0;

    /**
     * Called if an error was encountered while performing handshake.
     *
     * If sslErr is set, Acceptor::updateSSLStats will be called.
     */
    virtual void connectionError(
        folly::AsyncTransport* transport,
        folly::exception_wrapper ex,
        folly::Optional<SSLErrorEnum> sslErr) noexcept = 0;
  };

  /**
   * AcceptorHandshakeHelper::start initiates the handshake.
   *
   * The handshake may complete synchronously with respect to the invocation
   * of `start`.
   *
   * Implementations must obey the callback contract as specified in
   * AcceptorHandshakeHelper's documentation.
   *
   * Implementations cannot assume that the caller will keep the object alive
   * if the helper synchronously invokes one of the terminal callbacks. If
   * a handshake implementation can complete synchronously, implementations must
   * ensure that there are no further access to `this` past the callback
   * invocation point, or take a DestructorGuard.
   *
   * @note This can only be invoked at most once per instance of
   *       AcceptorHandshakeHelper.
   *
   * @param  sock     The socket to perform an asynchronous handshake on.
   * @param  callback The completion callback that will be invoked upon
   *                  handshake completion. This must be a valid, non null
   *                  pointer.
   *
   *                  AcceptorHandshakeHelper does not take ownership over
   *                  the supplied callback.
   */
  virtual void start(
      folly::AsyncSSLSocket::UniquePtr sock,
      AcceptorHandshakeHelper::Callback* callback) noexcept = 0;

  /**
   * AcceptorHandshakeHelper::dropConnection forcibly terminates an
   * in progress handshake.
   *
   * This can only be invoked once.
   *
   * Callers *must* ensure that `dropConnection` is invoked *only if*
   * AcceptorHandshakeHelper::Callback has not been signaled.
   *
   * Implementations *must* ensure that one of the terminal callbacks
   * is invoked *synchronously* with respect to the invocation of
   * `dropConnection()`.
   *
   * Implementations *must not* destroy the helper instance when
   * `AcceptorHandshakeHelper::dropConnection()` is invoked. The caller of
   * this method assumes responsibility for destroying the helper object.
   *
   * @pre `AsyncHandshakeHelper::start` must have been invoked prior to this
   *      call.
   * @post After the call to `dropConnnection()` returns, it is guaranteed
   *       that one of the methods in `Callback` have been invoked.
   * @post After the call to `dropConnection()` returns, the implementation
   *       guarantees that no future callbacks will be delivered.
   *
   * @param reason  The reason that the connection was canceled or dropped.
   */
  virtual void dropConnection(SSLErrorEnum reason = SSLErrorEnum::NO_ERROR) = 0;
};

/**
 * AcceptorHandshakeManager represents the lifecycle of a single asynchronous
 * handshake operation performed on a single connection with an associated
 * deadline for handshake completion.
 *
 * AcceptorHandshakeManager conceptually binds an instance of an
 * `AcceptorHandshakeHelper` together with the `wangle::Acceptor` that had
 * accepted the original connection.
 *
 * Before the handshake is initiated, AcceptorHandshakeManager will register
 * itself with the corresponding `wangle::Acceptor`'s connection manager. When
 * the handshake is completed, AcceptorHandshakeManager will unregister itself
 * with the connection manager.
 *
 * Registration against the connection manager is intended to allow for the
 * wangle::Acceptor to propagate cancellation requests (due to load shedding
 * or graceful shutdown).
 *
 * Lifecycle
 * =========
 * 1. AcceptorHandshakeManager is allocated with naked `new`. It manages its
 *    own allocation lifetime.
 *
 * 2. AcceptorHandshakeManager::start() is invoked at most once.
 *
 * 3a. AcceptorHandshakeManager will signal `wangle::Acceptor` with either
 *    an `sslConnectionReady` or an `sslConnectionError` and delete itself
 *    when one of the following events occur:
 *        i. The handshake completes successfully.
 *       ii. The handshake fails.
 *      iii. The handshake timeout / idle timeout elapses.
 *       iv. AcceptorHandshakeManager::dropConnection() is invoked.
 *
 * It is guaranteed that either one of the following methods:
 *  * `Acceptor::sslConnectionReady`
 *  * `Acceptor::sslConnectionError`
 *
 * are invoked by the AcceptorHandshakeManager's lifetime ends.
 */
class AcceptorHandshakeManager : public ManagedConnection,
                                 public AcceptorHandshakeHelper::Callback {
 public:
  AcceptorHandshakeManager(
      Acceptor* acceptor,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      TransportInfo tinfo)
      : acceptor_(acceptor),
        clientAddr_(clientAddr),
        acceptTime_(acceptTime),
        tinfo_(std::move(tinfo)) {}

  ~AcceptorHandshakeManager() override = default;

  /**
   * AcceptorHandshakeManager::start begins the asynchronous handshake,
   * registers the `AcceptorHandshakeManager` instance against the managing
   * Acceptor's connection manager, and begins the handshake timeout timer.
   *
   * @param sock    The socket to perform the handshake on.
   */
  virtual void start(folly::AsyncSSLSocket::UniquePtr sock) noexcept;

  // timeoutExpired is invoked when the handshake timer/idle timer
  // elapses. If the timer elapses, this indicates that the handshake did not
  // complete in time, and we should treat this as a terminal signal.
  void timeoutExpired() noexcept override;

  void describe(std::ostream& os) const override {
    os << "pending handshake on " << clientAddr_;
  }

  bool isBusy() const override {
    return true;
  }

  void notifyPendingShutdown() override {}

  void closeWhenIdle() override {}

  /**
   * `AcceptorHandshakeManager::dropConnection()` cancels an in progress
   * handshake.
   *
   * @pre  AcceptorHandshakeManager::start() must have been invoked prior
   *       to this call.
   *
   * @param errorMsg  Not used.
   */
  // wangle::ManagedConnection override
  void dropConnection(const std::string& /* errorMsg */ = "") override;

  void dumpConnectionState(uint8_t /* loglevel */) override {}

 private:
  // Invoking this method guarantees a synchronous invocation to one of
  // `connectionReady` or `connectionError`, which will destroy this object.
  void handshakeAborted(SSLErrorEnum reason);

 protected:
  void connectionReady(
      folly::AsyncTransport::UniquePtr transport,
      std::string nextProtocol,
      SecureTransportType secureTransportType,
      folly::Optional<SSLErrorEnum> details) noexcept override;

  void connectionError(
      folly::AsyncTransport* transport,
      folly::exception_wrapper ex,
      folly::Optional<SSLErrorEnum> details) noexcept override;

  std::chrono::milliseconds timeSinceAcceptMs() const;

  virtual void startHelper(folly::AsyncSSLSocket::UniquePtr sock) = 0;

  void startHandshakeTimeout();

  Acceptor* acceptor_;
  folly::SocketAddress clientAddr_;
  std::chrono::steady_clock::time_point acceptTime_;
  TransportInfo tinfo_;
  AcceptorHandshakeHelper::UniquePtr helper_;
};

} // namespace wangle
