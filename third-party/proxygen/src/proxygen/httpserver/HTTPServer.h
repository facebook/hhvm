/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/EventBase.h>
#include <proxygen/httpserver/HTTPServerOptions.h>
#include <proxygen/lib/http/codec/HTTPCodecFactory.h>
#include <proxygen/lib/http/session/HTTPSession.h>
#include <proxygen/lib/services/AcceptorConfiguration.h>
#include <thread>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/ssl/SSLContextConfig.h>

namespace proxygen {

class SignalHandler;
class HTTPServerAcceptor;

/**
 * HTTPServer based on proxygen http libraries
 */
class HTTPServer final {
 public:
  /**
   * Protocol identifies the default HTTP protocol implementation that an
   * IPConfig will use for plaintext connections.
   *
   * Connections established through TLS will use the protocol negotiated
   * through ALPN. If no ALPN is given, HTTP/1 (with an option to upgrade
   * to HTTP/2) is used, and this field is ignored.
   */
  enum class Protocol : uint8_t {
    HTTP,
    SPDY,
    HTTP2,
  };

  struct IPConfig {
    IPConfig(folly::SocketAddress a,
             Protocol p,
             std::shared_ptr<HTTPCodecFactory> c = nullptr)
        : address(a), protocol(p), codecFactory(c) {
    }

    folly::SocketAddress address;
    Protocol protocol;
    std::shared_ptr<HTTPCodecFactory> codecFactory;
    std::vector<wangle::SSLContextConfig> sslConfigs;

    /*
     * Sets the initial ticket seeds to use when starting the HTTPServer.
     * Ticket seeds are used to generate the session ticket encryption keys
     * for ticket resumption. When using session tickets, it is important
     * to change them and keep them updated, see updateTicketSeeds to keep
     * seeds up to date.
     */
    folly::Optional<wangle::TLSTicketKeySeeds> ticketSeeds;

    /*
     * Whether to allow an insecure connection on a secure port.
     * This should be used in very few cases where a HTTP server needs to
     * support insecure and secure connections on the same address.
     */
    bool allowInsecureConnectionsOnSecureServer{false};
    bool enableTCPFastOpen{false};
    /**
     * Maximum queue size of pending fast open connections.
     */
    uint32_t fastOpenQueueSize{10000};

    /*
     * Determines if this server does strict checking when loading SSL contexts.
     */
    bool strictSSL{true};

    folly::Optional<folly::SocketOptionMap> acceptorSocketOptions;
  };

  struct AcceptorFactoryConfig {
    AcceptorConfiguration accConfig;
    std::shared_ptr<HTTPCodecFactory> codecFactory;
  };

  /**
   * Create a new HTTPServer
   */
  explicit HTTPServer(HTTPServerOptions options);
  ~HTTPServer();

  /**
   * Configure server to bind to the following addresses.
   * The addresses you bind manually and provide to HTTPServerOptions
   * should be placed at the head of this vector before other addresses.
   *
   * Actual bind happens in `start` function.
   *
   * Can be called from any thread.
   */
  void bind(std::vector<IPConfig>&& addrs);
  void bind(std::vector<IPConfig> const& addrs);

  /**
   * Start HTTPServer.
   *
   * Note this is a blocking call and the current thread will be used to listen
   * for incoming connections. Throws exception if something goes wrong (say
   * somebody else is already listening on that socket).
   *
   * `onSuccess` callback will be invoked from the event loop which shows that
   * all the setup was successfully done.
   *
   * `onError` callback will be invoked if some errors occurs while starting the
   * server instead of throwing exception.
   *
   * `getAcceptorFactory` will be used to get the acceptor factory if it is set.
   * Otherwise, we will create an HTTPAcceptorFactory.
   *
   * `ioExecutor` will be used for for IO threads if it is set. Otherwise, we
   * will create a new IOThreadPoolExectutor for IO threads.
   */
  void start(
      std::function<void()> onSuccess = nullptr,
      std::function<void(std::exception_ptr)> onError = nullptr,
      std::function<std::shared_ptr<wangle::AcceptorFactory>(
          AcceptorFactoryConfig)> getAcceptorFactory = nullptr,
      std::shared_ptr<folly::IOThreadPoolExecutorBase> ioExecutor = nullptr);

  /**
   * Stop listening on bound ports. (Stop accepting new work).
   * It does not wait for pending work to complete.
   * You must still invoke stop() before destroying the server.
   * You do NOT need to invoke this before calling stop().
   * This can be called from any thread, and it is idempotent.
   * However, it may only be called **after** start() has called onSuccess.
   */
  void stopListening();

  /**
   * Stop HTTPServer.
   *
   * Can be called from any thread, but only after start() has called
   * onSuccess.  Server will stop listening for new connections and drop all
   * connections immediately. Before calling stop(), you may want to make sure
   * to properly drain and close on-going requests/connections.
   *
   * TODO: Separate method to do graceful shutdown?
   */
  void stop();

  /**
   * Get the list of addresses server is listening on. Empty if sockets are not
   * bound yet.
   */
  std::vector<IPConfig> addresses() const {
    return addresses_;
  }

  /**
   * Get the sockets the server is currently bound to.
   */
  const std::vector<const folly::AsyncSocketBase*> getSockets() const;

  void setSessionInfoCallback(HTTPSession::InfoCallback* cb) {
    sessionInfoCb_ = cb;
  }

  /**
   * Returns a file descriptor associated with the listening socket
   */
  int getListenSocket() const;

  /**
   * Re-reads the certificate / key pair for all SSL vips on all acceptors
   */
  void updateTLSCredentials();

  /**
   * Updates ticket seeds for the HTTPServer for all the VIPs.
   */
  void updateTicketSeeds(wangle::TLSTicketKeySeeds seeds);

 protected:
  /**
   * Start TCP HTTP server.
   *
   * @param getAcceptorFactory - provides the acceptor factory to use. If it is
   * null we will create an HTTPAcceptorFactory.
   * @param executor - io executor to use for IO threads. If it is null, we will
   * create one to use.
   */
  folly::Expected<folly::Unit, std::exception_ptr> startTcpServer(
      const std::function<std::shared_ptr<wangle::AcceptorFactory>(
          AcceptorFactoryConfig)>& getAcceptorFactory,
      std::shared_ptr<folly::IOThreadPoolExecutorBase> ioExecutor);

 private:
  std::shared_ptr<HTTPServerOptions> options_;

  /**
   * Event base in which we binded server sockets.
   */
  folly::EventBase* mainEventBase_{nullptr};

  /**
   * Optional signal handlers on which we should shutdown server
   */
  std::unique_ptr<SignalHandler> signalHandler_;

  /**
   * Addresses we are listening on
   */
  std::vector<IPConfig> addresses_;
  std::vector<wangle::ServerBootstrap<wangle::DefaultPipeline>> bootstrap_;

  /**
   * Callback for session create/destruction
   */
  HTTPSession::InfoCallback* sessionInfoCb_{nullptr};
};

} // namespace proxygen
