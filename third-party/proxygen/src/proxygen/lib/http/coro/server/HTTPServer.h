/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/filters/ServerFilterFactory.h"
#include "proxygen/lib/http/coro/server/HTTPCoroAcceptor.h"
#include <folly/container/F14Map.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/async/AsyncSignalHandler.h>
#include <folly/logging/xlog.h>
#include <proxygen/lib/sampling/Sampling.h>
#include <quic/server/QuicHandshakeSocketHolder.h>
#include <quic/server/QuicServer.h>
#include <signal.h>
#include <wangle/acceptor/ServerSocketConfig.h>

namespace proxygen::coro {

using folly::IOThreadPoolExecutor;
using folly::ThreadPoolExecutor;

class HTTPServer : public quic::QuicHandshakeSocketHolder::Callback {
 public:
  struct QuicConfig {
    std::vector<quic::QuicVersion> quicVersions;
    std::vector<std::string> supportedAlpns{"h3", "h3-fb-05"};
    quic::TransportSettings transportSettings;
    folly::Optional<int64_t> rateLimitPerThread;

    std::shared_ptr<quic::QLogger> qlogger{nullptr};
    Sampling qloggerSampling{1.0};
    std::unique_ptr<quic::QuicTransportStatsCallbackFactory> statsFactory{
        nullptr};
    std::shared_ptr<quic::CongestionControllerFactory> ccFactory{nullptr};
    std::vector<quic::QuicSocket::Observer*> observers;
  };

  struct SessionConfig {
    SettingsList settings{{SettingsId::MAX_HEADER_LIST_SIZE, 32 * 1024},
                          {SettingsId::HEADER_TABLE_SIZE, 4096},
                          {SettingsId::MAX_FRAME_SIZE, 16384},
                          {SettingsId::MAX_CONCURRENT_STREAMS, 100}};
    uint32_t maxConcurrentOutgoingStreams{100};
    // streamFlowControl sets the stream fc window in h2 and stream read buffer
    // limit in h1&h3 – SettingsId::INITIAL_WINDOW_SIZE in SettingsList above
    // overrides this value.
    size_t streamFlowControl{256 * 1024};
    size_t connFlowControl{20 * 256 * 1024};
    std::chrono::milliseconds streamReadTimeout{std::chrono::seconds(10)};
    std::chrono::milliseconds connIdleTimeout{std::chrono::seconds(10)};
    std::chrono::milliseconds writeTimeout{std::chrono::seconds(5)};
  };

  using NewConnectionFilter = HTTPCoroAcceptor::NewConnectionFilter;
  using ServerFilterFactoryList =
      std::vector<std::shared_ptr<ServerFilterFactory>>;
  struct Config {
    wangle::ServerSocketConfig socketConfig;
    std::optional<QuicConfig> quicConfig;
    // Allow taking in an existing socket. If provided together with
    // socketConfig.bindAddress, this is preferred
    // TODO(T198199559): Right now supported for TCP server only
    std::optional<int> preboundSocket;
    std::string plaintextProtocol;
    SessionConfig sessionConfig;
    size_t numIOThreads{1};
    std::vector<int> shutdownOnSignals{SIGINT};
    NewConnectionFilter newConnectionFilter;
    ServerFilterFactoryList filterFactories;
  };

  static wangle::SSLContextConfig getDefaultTLSConfig() {
    wangle::SSLContextConfig defaultConfig;
    defaultConfig.clientVerification =
        folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
    defaultConfig.sslVersion = folly::SSLContext::TLSv1;
    defaultConfig.isDefault = true;
    defaultConfig.setNextProtocols({"h2", "http/1.1"});
    return defaultConfig;
  }

  struct SocketAcceptorConfig {
    folly::AsyncServerSocket::UniquePtr socket;
    std::shared_ptr<const AcceptorConfiguration> acceptorConfig;
  };
  using SocketAcceptorConfigFactoryFn =
      std::function<std::vector<SocketAcceptorConfig>(
          folly::EventBase&, const HTTPServer::Config&)>;

  HTTPServer(Config config, std::shared_ptr<HTTPHandler> handler)
      : HTTPServer(std::move(config), std::move(handler), nullptr) {
  }

  HTTPServer(Config config,
             std::shared_ptr<HTTPHandler> handler,
             SocketAcceptorConfigFactoryFn fn)
      : config_(std::move(config)),
        handler_(std::move(handler)),
        socketAcceptorConfigFactoryFn_(std::move(fn)) {
  }

  ~HTTPServer() override;

  const Config& getConfig() const {
    return config_;
  }

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
   * `observer` will be added as a thread pool observer.
   */
  void start(
      std::function<void()> onSuccess = nullptr,
      std::function<void(std::exception_ptr)> onError = nullptr,
      std::shared_ptr<folly::ThreadPoolExecutor::Observer> observer = nullptr);

  /**
   * Get the address the server is listening on. Empty if sockets are not
   * bound yet. Almost every use cases of HttpServer only support one socket,
   * so we keep this function for backwards compatibility as we migrate to
   * support implementations that require multiple sockets.
   */
  std::optional<folly::SocketAddress> address() const {
    if (!serverSockets_.empty()) {
      XCHECK_EQ(serverSockets_.size(), 1UL)
          << "Attempt to use address() on an implementation that supports "
             "multiple addresses";
      folly::SocketAddress address;
      serverSockets_.front()->getAddress(&address);
      return address;
    } else if (quicServer_) {
      return quicServer_->getAddress();
    } else {
      return std::nullopt;
    }
  }

  /**
   * Returns all addresses the server is listening on - useful for
   * implementations that support multiple sockets, but can still be
   * called by single socket implementations too.
   */
  std::vector<folly::SocketAddress> addresses() const {
    std::vector<folly::SocketAddress> addresses;
    if (quicServer_) {
      addresses.emplace_back(quicServer_->getAddress());
    } else {
      for (auto& socket : serverSockets_) {
        addresses.emplace_back(socket->getAddress());
      }
    }
    return addresses;
  }

  /**
   * Stop listening on bound ports. (Stop accepting new work).
   * It does not wait for pending work to complete.
   * This can be called from any thread, and it is idempotent.
   * However, it may only be called **after** start() has called onSuccess.
   */
  void drain();

  /**
   * Stop HTTPServer.
   *
   * Can be called from any thread, but only after start() has called
   * onSuccess.  Server will stop listening for new connections and drop all
   * connections immediately. It's preferable to call drain() before
   * forceStop().
   */
  void forceStop();

  void enableSoReusePort() {
    setReusePortSocketOption_ = true;
  }

  void setHostId(uint32_t hostId);

  /**
   * Interface for receiving callback when server threads are started or
   * stopped. The server does not guarantee thread-safety for the
   * callbacks.
   */
  class Observer {
   public:
    virtual ~Observer() = default;
    virtual void onThreadStart(folly::EventBase*) {
    }
    virtual void onThreadStop(folly::EventBase*) {
    }
  };

  void addObserver(Observer* observer) {
    CHECK_NOTNULL(observer);
    observers_.insert(observer);
  }

  void removeObserver(Observer* observer) {
    observers_.erase(observer);
  }

 private:
  using KeepAliveEventBaseVec =
      std::vector<folly::Executor::KeepAlive<folly::EventBase>>;

  std::shared_ptr<const AcceptorConfiguration> toAcceptorConfig(
      const Config& config);
  HTTPCoroAcceptor* createAcceptor(
      folly::EventBase* evb,
      std::shared_ptr<const AcceptorConfiguration> acceptorConfig);
  HTTPCoroAcceptor* FOLLY_NULLABLE getQuicAcceptor(folly::EventBase* evb);
  void startTcp(const KeepAliveEventBaseVec& evbs);
  void startQuic(const KeepAliveEventBaseVec& vbs);
  void createQuicServer(const std::vector<folly::EventBase*>& evbs);
  void configureFizzServerContext(
      std::shared_ptr<fizz::server::FizzServerContext> fizzCtx);
  void onQuicTransportReady(
      std::shared_ptr<quic::QuicSocket> quicSocket) override;
  void onConnectionSetupError(std::shared_ptr<quic::QuicSocket>,
                              quic::QuicError error) override {
    XLOG(ERR) << "Failed to accept QUIC connection: " << error.message;
  }
  void run(std::function<void()> onSuccess);
  void globalDrainImpl();
  void unregisterSignalHandlers();
  void drainImpl(HTTPCoroAcceptor& acceptor);

  Config config_;
  std::shared_ptr<HTTPHandler> handler_;
  folly::F14FastSet<Observer*> observers_;
  folly::EventBase eventBase_;
  std::vector<folly::AsyncServerSocket::UniquePtr> serverSockets_;
  bool setReusePortSocketOption_{false};
  folly::F14NodeMap<folly::EventBase*, std::list<HTTPCoroAcceptor>> acceptors_;
  std::shared_ptr<quic::QuicServer> quicServer_;
  SocketAcceptorConfigFactoryFn socketAcceptorConfigFactoryFn_{nullptr};
  enum class State : uint8_t {
    UNINIT = 0,
    RUNNING = 1,
    DRAINING = 2,
    STOPPED = 3
  };
  std::atomic<State> state_{State::UNINIT};
  std::atomic<size_t> nRunningAcceptors_{0};
  uint32_t hostId_{0};
  friend std::ostream& operator<<(std::ostream& os,
                                  const HTTPServer::State& state);

  template <typename T, typename... Args>
  void deliverObserverEvent(T callbackFn, Args&&... args) {
    auto obsClone = observers_;

    for (auto& obs : obsClone) {
      if (std::find(obsClone.begin(), obsClone.end(), obs) != obsClone.end()) {
        (obs->*callbackFn)(std::forward<Args>(args)...);
      }
    }
  }

  class InternalThreadObserver
      : public folly::IOThreadPoolExecutorBase::IOObserver {
   public:
    explicit InternalThreadObserver(HTTPServer* server) : server_(server) {
    }

    void registerEventBase(folly::EventBase& evb) override {
      evb.runInEventBaseThread([this, &evb]() {
        server_->deliverObserverEvent(&HTTPServer::Observer::onThreadStart,
                                      &evb);
        for (auto& filterFactory : server_->config_.filterFactories) {
          filterFactory->onServerStart(&evb);
        }
      });
    }

    void unregisterEventBase(folly::EventBase& evb) override {
      evb.runInEventBaseThread([this, &evb]() {
        server_->deliverObserverEvent(&HTTPServer::Observer::onThreadStop,
                                      &evb);
        for (auto& filterFactory : server_->config_.filterFactories) {
          filterFactory->onServerStop();
        }
      });
    }

   private:
    HTTPServer* server_;
  };
  friend class ThreadObserver;

  class SignalHandler : public folly::AsyncSignalHandler {
   public:
    explicit SignalHandler(HTTPServer& server)
        : folly::AsyncSignalHandler(&server.eventBase_), server_(server) {
    }

    void signalReceived(int signum) noexcept override {
      XLOG(DBG2) << "Received signal " << signum << ", initiating drain";
      server_.drain();
    }

   private:
    HTTPServer& server_;
  };
  SignalHandler signalHandler_{*this};
};

inline std::ostream& operator<<(std::ostream& os,
                                const HTTPServer::State& state) {
  switch (state) {
    case HTTPServer::State::UNINIT:
      os << "UNINIT";
      break;
    case HTTPServer::State::RUNNING:
      os << "RUNNING";
      break;
    case HTTPServer::State::DRAINING:
      os << "DRAINING";
      break;
    case HTTPServer::State::STOPPED:
      os << "STOPPED";
      break;
  }
  return os;
}

} // namespace proxygen::coro
