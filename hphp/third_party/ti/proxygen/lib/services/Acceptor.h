// Copyright 2004-present Facebook.  All rights reserved.
#pragma once
#include "thrift/lib/cpp/async/TAsyncServerSocket.h"
#include "thrift/lib/cpp/async/TAsyncTimeoutSet.h"

#include <event.h>
#include <chrono>

#include "ti/proxygen/lib/services/AcceptorConfiguration.h"
#include "ti/proxygen/lib/services/ConnectionManager.h"
#include "ti/proxygen/lib/transport/TransportInfo.h"


namespace apache { namespace thrift {
namespace async {
class TAsyncTransport;
}
namespace transport {
class TSocketAddress;
}
}}

namespace facebook { namespace logging {
class Logger;
}}

namespace facebook { namespace stats {
class ExportedStatMap;
}}

namespace facebook { namespace proxygen {

class ManagedConnection;

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
 *
 * The following are the state transitions of the service:
 *
 * - The service starts in the kIdle state after its constructor is called.
 *
 * - The state changes from kIdle to kAttached in attach(), and the service
 *   starts to accept connections.
 *
 * - When detach() is called in the kAttached state, the service stops
 *   accepting new connections. If there are outstanding connections
 *   (i.e., numConnections_ > 0), the state changes to kDetaching; otherwise,
 *   the state changes to kIdle, the cleanup service is detached from
 *   the event base, and the event base is set to nullptr.
 *
 * - When detach() is called in the kDetaching state, if there are no more
 *   outstanding connections, the state changes to kIdle, the
 *   cleanup service is detached from the event base, and the event base is
 *   set to nullptr; otherswise, the service remains in the kDetaching state
 *   and waits for the outstanding requests to either complete or time out.
 *
 * Note that the detach() method is overridden in the subclass
 * HTTPReverseProxyAcceptor, which maintains additional state in its
 * ProxyHealthCheck member to handle shutdown without dropping
 * requests. See comments on class ShutdownFromThread for details.
 */
class Acceptor :
  public apache::thrift::async::TAsyncServerSocket::AcceptCallback,
  public ConnectionManager::Callback {
 public:

  enum class State : uint32_t {
    kInit,  // not yet started
    kRunning, // processing requests normally
    kDraining, // processing outstanding conns, but not accepting new ones
    kDone,  // no longer accepting, and all connections finished
  };

  explicit Acceptor(const AcceptorConfiguration& accConfig);
  virtual ~Acceptor();

  /**
   * Initialize the Acceptor to run in the specified TEventBase
   * thread, receiving connections from the specified TAsyncServerSocket.
   *
   * This method will be called from the TAsyncServerSocket's primary thread,
   * not the specified TEventBase thread.
   */
  virtual void init(apache::thrift::async::TAsyncServerSocket *serverSocket,
                    apache::thrift::async::TEventBase *eventBase);

  /**
   * Return the number of outstanding connections in this service instance.
   */
  uint32_t getNumConnections() const {
    return downstreamConnectionManager_ ?
        downstreamConnectionManager_->getNumConnections() : 0;
  }

  /**
   * Access the Acceptor's event base.
   */
  apache::thrift::async::TEventBase* getEventBase() { return base_; }

  /**
   * Access the Acceptor's downstream (client-side) ConnectionManager
   */
  virtual ConnectionManager* getConnectionManager() {
    return downstreamConnectionManager_.get();
  }

  /**
   * Access the Acceptor's upstream (server-side) ConnectionManager
   */
  virtual ConnectionManager* getUpstreamConnectionManager() {
    return upstreamConnectionManager_.get();
  }

  /**
   * Access the general-purpose timeout manager for transactions.
   */
  virtual apache::thrift::async::TAsyncTimeoutSet* getTransactionTimeoutSet() {
    return transactionTimeouts_.get();
  }

  virtual apache::thrift::async::TAsyncTimeoutSet* getTcpEventsTimeoutSet() {
    return tcpEventsTimeouts_.get();
  }

  /**
   * Invoked when a new ManagedConnection is created.
   *
   * This allows the Acceptor to track the outstanding connections,
   * for tracking timeouts and for ensuring that all connections have been
   * drained on shutdown.
   */
  void addConnection(ManagedConnection* connection);

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
   * Returns true if this proxy is internal to facebook
   */
  bool isInternal() const {
    return accConfig_.getInternal();
  }

  /**
   * Returns the name of this VIP.
   *
   * Will return an empty string if no name has been configured.
   */
  const std::string &getName() const {
    return accConfig_.getName();
  }

  /**
   * Force the acceptor to drop all connections and stop processing.
   *
   * This function may be called from any thread.  The acceptor will not
   * necessarily stop before this function returns: the stop will be scheduled
   * to run in the acceptor's thread.
   */
  virtual void forceStop();

  /**
   * Flush any counters you have now
   */
  virtual void flushStats();

 protected:
  /**
   * Our event loop.
   *
   * Probably needs to be used to pass to a ManagedConnection
   * implementation. Also visible in case a subclass wishes to do additional
   * things w/ the event loop (e.g. in attach()).
   */
  apache::thrift::async::TEventBase *base_{nullptr};

  /**
   * Hook for subclasses to drop newly accepted connections prior
   * to HTTP handshaking.
   */
  virtual bool canAccept(const apache::thrift::transport::TSocketAddress&);

  /**
   * Invoked when a new connection is created. This is where application starts
   * processing a new downstream connection.
   *
   * NOTE: Application should add the new connection to
   *       downstreamConnectionManager so that it can be garbage collected after
   *       certain period of idleness.
   *
   * @param sock              the socket connected to the client
   * @param address           the address of the client
   * @param nextProtocolName  the name of the L6 or L7 protocol to be
   *                            spoken on the connection, if known (e.g.,
   *                            from TLS NPN during secure connection setup),
   *                            or an empty string if unknown
   */
  virtual void onNewConnection(
      apache::thrift::async::TAsyncSocket::UniquePtr sock,
      const apache::thrift::transport::TSocketAddress* address,
      const std::string& nextProtocolName,
      const TransportInfo& tinfo) = 0;

  /**
   * Drop all connections.
   *
   * forceStop() schedules dropAllConnections() to be called in the acceptor's
   * thread.
   */
  void dropAllConnections();

  /**
   * Close any connections that don't have a transaction outstanding.
   */
  void closeIdleConnections();

  /**
   * onConnectionsDrained() will be called once all connections have been
   * drained while the acceptor is stopping.
   *
   * Subclasses can override this method to perform any subclass-specific
   * cleanup.
   */
  virtual void onConnectionsDrained() {}

  // TAsyncServerSocket::AcceptCallback methods
  void connectionAccepted(int fd,
      const apache::thrift::transport::TSocketAddress& clientAddr)
      noexcept;
  void acceptError(const std::exception& ex) noexcept;
  void acceptStopped() noexcept;

  // ConnectionManager::Callback methods
  void onEmpty(const ConnectionManager& cm);
  void onConnectionAdded(const ConnectionManager& cm) {}
  void onConnectionRemoved(const ConnectionManager& cm) {}

  /**
   * Process a connection that is to ready to receive L7 traffic.
   * This method is called immediately upon accept for plaintext
   * connections and upon completion of SSL handshaking or resumption
   * for SSL connections.
   */
   void connectionReady(
      apache::thrift::async::TAsyncSocket::UniquePtr sock,
      const apache::thrift::transport::TSocketAddress& clientAddr,
      const std::string& nextProtocolName,
      TransportInfo& tinfo);

 protected:
  const AcceptorConfiguration accConfig_;

  /**
   * Socket options to apply to the client socket
   */
  apache::thrift::async::TAsyncSocket::OptionMap socketOptions_;

 private:

  // Forbidden copy constructor and assignment opererator
  Acceptor(Acceptor const &) = delete;
  Acceptor& operator=(Acceptor const &) = delete;

  void checkDrained();

  State state_{State::kInit};

  std::unique_ptr<ConnectionManager> downstreamConnectionManager_;
  std::unique_ptr<ConnectionManager> upstreamConnectionManager_;
  apache::thrift::async::TAsyncTimeoutSet::UniquePtr transactionTimeouts_;
  apache::thrift::async::TAsyncTimeoutSet::UniquePtr tcpEventsTimeouts_;

  bool forceShutdownInProgress_{false};
};

}} // facebook::proxygen
