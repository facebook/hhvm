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

#include <wangle/acceptor/ConnectionManager.h>
#include <wangle/acceptor/FizzAcceptorHandshakeHelper.h>
#include <wangle/acceptor/LoadShedConfiguration.h>
#include <wangle/acceptor/SSLAcceptorHandshakeHelper.h>
#include <wangle/acceptor/SecureTransportType.h>
#include <wangle/acceptor/SecurityProtocolContextManager.h>
#include <wangle/acceptor/ServerSocketConfig.h>
#include <wangle/acceptor/TLSPlaintextPeekingCallback.h>

#include <wangle/acceptor/TransportInfo.h>
#include <wangle/ssl/SSLCacheProvider.h>
#include <wangle/ssl/SSLStats.h>

#include <folly/ExceptionWrapper.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/AsyncUDPServerSocket.h>
#include <chrono>

namespace wangle {

class AcceptObserver;
class ManagedConnection;
class SecurityProtocolContextManager;
class SSLContextManager;

/**
 * An abstract acceptor for TCP-based network services.
 *
 * There is one acceptor object per thread for each listening socket.  When a
 * new connection arrives on the listening socket, it is accepted by one of the
 * acceptor objects.  From that point on the connection will be processed by
 * that acceptor's thread.
 *
 * The acceptor will call the abstract onNewConnection() method to create
 * a new ManagedConnection object for each accepted socket.  The acceptor
 * also tracks all outstanding connections that it has accepted.
 */
class Acceptor : public folly::AsyncServerSocket::AcceptCallback,
                 public wangle::ConnectionManager::Callback,
                 public folly::AsyncUDPServerSocket::Callback {
 public:
  enum class State : uint32_t {
    kInit, // not yet started
    kRunning, // processing requests normally
    kDraining, // processing outstanding conns, but not accepting new ones
    kDone, // no longer accepting, and all connections finished
  };

  explicit Acceptor(const ServerSocketConfig& accConfig);
  ~Acceptor() override;

  /**
   * Supply an SSL cache provider
   * @note Call this before init()
   */
  virtual void setSSLCacheProvider(
      const std::shared_ptr<SSLCacheProvider>& cacheProvider) {
    cacheProvider_ = cacheProvider;
  }

  /**
   * Supply a fizz cert manager for use.
   * If not set before init(), one will be created.
   */
  virtual void setFizzCertManager(
      std::shared_ptr<fizz::server::CertManager> fizzCertManager) {
    fizzCertManager_ = fizzCertManager;
  }

  /**
   * Supply an SSLContextManager for use.
   * If not set before init(), one will be created.
   */
  virtual void setSSLContextManager(
      std::shared_ptr<SSLContextManager> contextManager) {
    sslCtxManager_ = contextManager;
  }

  /**
   * Initialize the Acceptor to run in the specified EventBase
   * thread, receiving connections from the specified AsyncServerSocket.
   *
   * This method will be called from the AsyncServerSocket's primary thread,
   * not the specified EventBase thread.
   */
  virtual void init(
      folly::AsyncServerSocket* serverSocket,
      folly::EventBase* eventBase,
      SSLStats* stats = nullptr,
      std::shared_ptr<const fizz::server::FizzServerContext> fizzContext =
          nullptr);

  /**
   * Recreates ssl configs, re-reads certs
   */
  virtual void resetSSLContextConfigs(
      std::shared_ptr<fizz::server::CertManager> certManager = nullptr,
      std::shared_ptr<SSLContextManager> ctxManager = nullptr,
      std::shared_ptr<const fizz::server::FizzServerContext> fizzContext =
          nullptr);

  SSLContextManager* getSSLContextManager() const {
    return sslCtxManager_.get();
  }

  /**
   * Sets TLS ticket secrets to use, or updates previously set secrets.
   */
  virtual void setTLSTicketSecrets(
      const std::vector<std::string>& oldSecrets,
      const std::vector<std::string>& currentSecrets,
      const std::vector<std::string>& newSecrets);

  /**
   * Return the number of outstanding connections in this service instance.
   */
  uint32_t getNumConnections() const {
    return downstreamConnectionManager_
        ? (uint32_t)downstreamConnectionManager_->getNumConnections()
        : 0;
  }

  /**
   * Access the Acceptor's event base.
   */
  virtual folly::EventBase* getEventBase() const {
    return base_;
  }

  /**
   * Access the Acceptor's downstream (client-side) ConnectionManager
   */
  virtual wangle::ConnectionManager* getConnectionManager() {
    return downstreamConnectionManager_.get();
  }

  /**
   * Invoked when a new ManagedConnection is created.
   *
   * This allows the Acceptor to track the outstanding connections,
   * for tracking timeouts and for ensuring that all connections have been
   * drained on shutdown.
   */
  void addConnection(wangle::ManagedConnection* connection);

  /**
   * Get this acceptor's current state.
   */
  State getState() const {
    return state_;
  }

  /**
   * Get the current connection timeout.
   */
  std::chrono::milliseconds getConnTimeout() const;

  /**
   * Returns the name of this VIP.
   *
   * Will return an empty string if no name has been configured.
   */
  const std::string& getName() const {
    return accConfig_.name;
  }

  /**
   * Returns the ssl handshake connection timeout of this VIP
   */
  std::chrono::milliseconds getSSLHandshakeTimeout() const {
    return accConfig_.sslHandshakeTimeout;
  }

  /**
   * Time after startDrainingAllConnections() or acceptStopped() during which
   * new requests on connections owned by the downstream
   * ConnectionManager will be processed normally.
   */
  void setGracefulShutdownTimeout(std::chrono::milliseconds gracefulShutdown) {
    gracefulShutdownTimeout_ = gracefulShutdown;
  }

  std::chrono::milliseconds getGracefulShutdownTimeout() const {
    return gracefulShutdownTimeout_;
  }

  /**
   * Force the acceptor to drop all connections and stop processing.
   *
   * This function may be called from any thread.  The acceptor will not
   * necessarily stop before this function returns: the stop will be scheduled
   * to run in the acceptor's thread.
   */
  virtual void forceStop();

  bool isSSL() const {
    return accConfig_.isSSL();
  }

  const ServerSocketConfig& getConfig() const {
    return accConfig_;
  }

  /**
   * Called right when the TCP connection has been accepted, before processing
   * the first HTTP bytes (HTTP) or the SSL handshake (HTTPS)
   */
  virtual void onDoneAcceptingConnection(
      int fd,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      const AcceptInfo& info,
      folly::AsyncSocket::LegacyLifecycleObserver* observer) noexcept;

  /**
   * Begins either processing HTTP bytes (HTTP) or the SSL handshake (HTTPS)
   */
  void processEstablishedConnection(
      int fd,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      TransportInfo& tinfo,
      folly::AsyncSocket::LegacyLifecycleObserver* observer = nullptr) noexcept;

  /**
   * Creates and starts the handshake manager.
   */
  virtual void startHandshakeManager(
      folly::AsyncSSLSocket::UniquePtr sslSock,
      Acceptor* acceptor,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      TransportInfo& tinfo) noexcept;

  /**
   * Starts draining all open connections of their outstanding transactions
   * asynchronously. When a connection's transaction count reaches zero, the
   * connection closes.
   */
  virtual void startDrainingAllConnections();

  /**
   * Drain defined percentage of connections.
   */
  virtual void drainConnections(double pctToDrain);

  /**
   * Drop all connections.
   *
   * forceStop() schedules dropAllConnections() to be called in the acceptor's
   * thread.
   */
  void dropAllConnections();

  /**
   * Force-drop "pct" (0.0 to 1.0) of remaining client connections,
   * regardless of whether they are busy or idle.
   *
   * Note: unlike dropAllConnections(),
   * this function can be called from any thread.
   */
  virtual void dropConnections(double pctToDrop);

  /**
   * Drops "pct" (0.0 to 1.0) of the connections active connections.
   * Connection will be dropped only if filter (callback) returns true
   */
  virtual void dropEstablishedConnections(
      double pctToDrop,
      const std::function<bool(ManagedConnection*)>& filter);

  /**
   * Drops every idle for which idle time is less then *timeout*
   *
   * @param timeout - idle time threshold.
   * @param droppedConnectionsCB - Callback will be invoked for dropped
   *   connections, having number of dropped connection as input.
   *
   */
  virtual void dropIdleConnectionsBasedOnTimeout(
      std::chrono::milliseconds targetIdleTimeMs,
      const std::function<void(size_t)>& droppedConnectionsCB);

  /**
   * Wrapper for connectionReady() that can be overridden by
   * subclasses to deal with plaintext connections.
   */
  virtual void plaintextConnectionReady(
      folly::AsyncSocket::UniquePtr sock,
      const folly::SocketAddress& clientAddr,
      TransportInfo& tinfo);

  /**
   * Process a connection that is to ready to receive L7 traffic.
   * This method is called immediately upon accept for plaintext
   * connections and upon completion of SSL handshaking or resumption
   * for SSL connections.
   */
  void connectionReady(
      folly::AsyncTransport::UniquePtr sock,
      const folly::SocketAddress& clientAddr,
      const std::string& nextProtocolName,
      SecureTransportType secureTransportType,
      TransportInfo& tinfo);

  /**
   * Wrapper for connectionReady() that decrements the count of
   * pending SSL connections. This should normally not be overridden.
   */
  virtual void sslConnectionReady(
      folly::AsyncTransport::UniquePtr sock,
      const folly::SocketAddress& clientAddr,
      const std::string& nextProtocol,
      SecureTransportType secureTransportType,
      TransportInfo& tinfo);

  /**
   * Notification callback for SSL handshake failures.
   */
  virtual void sslConnectionError(const folly::exception_wrapper& ex);

  /**
   * Hook for subclasses to record stats about SSL connection establishment.
   *
   * sock may be nullptr.
   */
  virtual void updateSSLStats(
      const folly::AsyncTransport* /*sock*/,
      std::chrono::milliseconds /*acceptLatency*/,
      SSLErrorEnum /*error*/,
      const folly::exception_wrapper& /*ex*/) noexcept {}

  /**
   * Adds observer for accept events.
   *
   * Can be used to install socket observers and instrumentation without
   * changing / interfering with application-specific acceptor logic.
   *
   * @param observer     Observer to add (implements AcceptObserver).
   */
  virtual void addAcceptObserver(AcceptObserver* observer) {
    observerList_.add(observer);
  }

  /**
   * Remove observer for accept events.
   *
   * @param observer     Observer to remove.
   * @return             Whether observer found and removed from list.
   */
  virtual bool removeAcceptObserver(AcceptObserver* observer) {
    return observerList_.remove(observer);
  }

  /**
   * Create a FizzServerContext from the TLS settings presently in this
   * Acceptor.  This context is a suitable starting point for enabling QUIC
   * via mvfst.
   */
  std::shared_ptr<fizz::server::FizzServerContext> recreateFizzContext();

  /**
   * Hook for checking allowlisted client addresses
   */
  virtual bool isPeerAddressAllowlisted(const folly::SocketAddress&);

 protected:
  using OnDataAvailableParams =
      folly::AsyncUDPSocket::ReadCallback::OnDataAvailableParams;

  /**
   * Our event loop.
   *
   * Probably needs to be used to pass to a ManagedConnection
   * implementation. Also visible in case a subclass wishes to do additional
   * things w/ the event loop (e.g. in attach()).
   */
  folly::EventBase* base_{nullptr};

  /**
   * Hook for subclasses to drop newly accepted connections prior
   * to handshaking.
   */
  virtual bool canAccept(const folly::SocketAddress&);

  /**
   * Invoked when a new connection is created. This is where application starts
   * processing a new downstream connection.
   *
   * NOTE: Application should add the new connection to
   *       downstreamConnectionManager so that it can be garbage collected after
   *       certain period of idleness.
   *
   * @param sock                the socket connected to the client
   * @param address             the address of the client
   * @param nextProtocolName    the name of the L6 or L7 protocol to be
   *                              spoken on the connection, if known (e.g.,
   *                              from TLS NPN during secure connection setup),
   *                              or an empty string if unknown
   * @param secureTransportType the name of the secure transport type that was
   *                            requested by the client.
   */
  virtual void onNewConnection(
      folly::AsyncTransport::UniquePtr /*sock*/,
      const folly::SocketAddress* /*address*/,
      const std::string& /*nextProtocolName*/,
      SecureTransportType /*secureTransportType*/,
      const TransportInfo& /*tinfo*/) {}

  void onListenStarted() noexcept override {}
  void onListenStopped() noexcept override {}
  void onDataAvailable(
      std::shared_ptr<folly::AsyncUDPSocket> /*socket*/,
      const folly::SocketAddress&,
      std::unique_ptr<folly::IOBuf>,
      bool,
      OnDataAvailableParams) noexcept override {}

  virtual folly::AsyncSocket::UniquePtr makeNewAsyncSocket(
      folly::EventBase* base,
      int fd,
      const folly::SocketAddress* peerAddress) {
    return folly::AsyncSocket::UniquePtr(new folly::AsyncSocket(
        base, folly::NetworkSocket::fromFd(fd), 0, peerAddress));
  }

  virtual folly::AsyncSSLSocket::UniquePtr makeNewAsyncSSLSocket(
      const std::shared_ptr<folly::SSLContext>& ctx,
      folly::EventBase* base,
      int fd,
      const folly::SocketAddress* peerAddress) {
    return folly::AsyncSSLSocket::UniquePtr(new folly::AsyncSSLSocket(
        ctx,
        base,
        folly::NetworkSocket::fromFd(fd),
        true, /* set server */
        true /* defer the security negotiation until sslAccept */,
        peerAddress));
  }

  /**
   * onConnectionsDrained() will be called once all connections have been
   * drained while the acceptor is stopping.
   *
   * Subclasses can override this method to perform any subclass-specific
   * cleanup.
   */
  virtual void onConnectionsDrained() {}

  // AsyncServerSocket::AcceptCallback methods
  void connectionAccepted(
      folly::NetworkSocket fdNetworkSocket,
      const folly::SocketAddress& clientAddr,
      AcceptInfo /* info */) noexcept override;

 public:
  void acceptConnection(
      folly::NetworkSocket fdNetworkSocket,
      const folly::SocketAddress& clientAddr,
      AcceptInfo /* info */,
      folly::AsyncSocket::LegacyLifecycleObserver* lifeCycleObserver) noexcept;

 protected:
  // TODO(T81599451): Remove the 'using' statement below after
  // eliminating the old AcceptCallback::acceptError callback
  using folly::AsyncServerSocket::AcceptCallback::acceptError;
  void acceptError(const std::exception& ex) noexcept override;
  void acceptStopped() noexcept override;

  // ConnectionManager::Callback methods
  void onEmpty(const wangle::ConnectionManager& cm) override;
  void onConnectionAdded(const ManagedConnection*) override {}
  void onConnectionRemoved(const ManagedConnection*) override {}

  const ServerSocketConfig accConfig_;

  // Helper function to initialize downstreamConnectionManager_
  virtual void initDownstreamConnectionManager(folly::EventBase* eventBase);
  std::string getPskContext();
  virtual DefaultToFizzPeekingCallback* getFizzPeeker() {
    return &defaultFizzPeeker_;
  }
  virtual std::shared_ptr<fizz::server::FizzServerContext> createFizzContext();
  virtual std::shared_ptr<fizz::server::TicketCipher> createFizzTicketCipher(
      const TLSTicketKeySeeds& seeds,
      std::shared_ptr<fizz::Factory> factory,
      std::shared_ptr<fizz::server::CertManager> certManager,
      folly::Optional<std::string> pskContext);

  virtual std::unique_ptr<fizz::server::CertManager> createFizzCertManager();

  /**
   * Socket options to apply to the client socket
   */
  folly::SocketOptionMap socketOptions_;

  std::shared_ptr<SSLContextManager> sslCtxManager_;

  /**
   * Stores peekers for different security protocols.
   */
  SecurityProtocolContextManager securityProtocolCtxManager_;

  TLSPlaintextPeekingCallback tlsPlaintextPeekingCallback_;
  DefaultToSSLPeekingCallback defaultPeekingCallback_;
  DefaultToFizzPeekingCallback defaultFizzPeeker_;

  wangle::ConnectionManager::UniquePtr downstreamConnectionManager_;

  std::shared_ptr<SSLCacheProvider> cacheProvider_;

 private:
  /**
   * This is an intentionally non-virtual method that base acceptors will use
   * that is invoked right before the transport is passed to the application.
   *
   * This function is an infallible method that is designed to alter the
   * transport to reflect settings that are managed by wangle.
   */
  folly::AsyncTransport::UniquePtr transformTransport(
      folly::AsyncTransport::UniquePtr sock);

  void transitionToDrained() {
    state_ = State::kDone;
    onConnectionsDrained();
  }

  TLSTicketKeySeeds ticketSecrets_;
  std::shared_ptr<fizz::server::CertManager> fizzCertManager_{nullptr};

  // Forbidden copy constructor and assignment opererator
  Acceptor(Acceptor const&) = delete;
  Acceptor& operator=(Acceptor const&) = delete;

  void checkIfDrained();

  State state_{State::kInit};
  uint64_t numPendingSSLConns_{0};

  bool forceShutdownInProgress_{false};
  std::chrono::milliseconds gracefulShutdownTimeout_{5000};

  // Wrapper around list of AcceptObservers to handle cleanup on destruction
  class AcceptObserverList {
   public:
    explicit AcceptObserverList(Acceptor* acceptor);

    /**
     * Destructor, triggers observerDetach for any attached observers.
     */
    ~AcceptObserverList();

    /**
     * Add observer and trigger observerAttach.
     */
    void add(AcceptObserver* observer);

    /**
     * Remove observer and trigger observerDetach.
     */
    bool remove(AcceptObserver* observer);

    /**
     * Get reference to vector containing observers.
     */
    const std::vector<AcceptObserver*>& getAll() const {
      return observers_;
    }

   private:
    Acceptor* acceptor_{nullptr};
    std::vector<AcceptObserver*> observers_;
  };

  // List of AcceptObservers
  AcceptObserverList observerList_;
};

class AcceptorFactory {
 public:
  virtual std::shared_ptr<Acceptor> newAcceptor(folly::EventBase*) = 0;
  virtual ~AcceptorFactory() = default;
};

} // namespace wangle
