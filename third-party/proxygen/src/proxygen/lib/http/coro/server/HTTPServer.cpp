/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/server/HTTPServer.h"
#include "proxygen/lib/http/coro/HTTPFilterFactoryHandler.h"
#include <folly/io/async/EventBaseManager.h>
#include <folly/logging/xlog.h>
#include <proxygen/lib/utils/Time.h>
#include <quic/common/events/FollyQuicEventBase.h>
#include <quic/congestion_control/ServerCongestionControllerFactory.h>
#include <quic/logging/FileQLogger.h>
#include <quic/server/QuicSharedUDPSocketFactory.h>

namespace proxygen::coro {

namespace {

class QuicAcceptorTransportFactory : public quic::QuicServerTransportFactory {
 public:
  explicit QuicAcceptorTransportFactory(HTTPServer& server) : server_(server) {
  }

  quic::QuicServerTransport::Ptr make(
      folly::EventBase* evb,
      std::unique_ptr<quic::FollyAsyncUDPSocketAlias> socket,
      const folly::SocketAddress& /* peerAddr */,
      quic::QuicVersion,
      std::shared_ptr<const fizz::server::FizzServerContext> ctx) noexcept
      override {
    auto transport = quic::QuicHandshakeSocketHolder::makeServerTransport(
        evb, std::move(socket), std::move(ctx), &server_);
    const auto& quicConfig = server_.getConfig().quicConfig;
    if (quicConfig->qlogger && quicConfig->qloggerSampling.isLucky()) {
      transport->setQLogger(quicConfig->qlogger);
    }
    for (auto* quicObserver : quicConfig->observers) {
      transport->addObserver(quicObserver);
    }
    return transport;
  }

 private:
  proxygen::coro::HTTPServer& server_;
};

uint64_t getHTTPSettingValueOrDefault(const SettingsList& settings,
                                      const SettingsId id,
                                      const SettingsValue defaultValue) {
  for (const auto& setting : settings) {
    if (setting.id == id) {
      return setting.value;
    }
  }
  return defaultValue;
}

/**
 * HTTPServerHandler is a wrapper around the HTTPHandler that the user provides
 * via the HTTPServer constructor (i.e. HTTPServer::handler_). It's designed to
 * install a list of filters on the request (before executing user's
 * handleRequest) and response path (after executing user's handleRequest).
 *
 * See HTTPFilterFactoryHandler.h for more details on filter installation.
 */
class HTTPServerHandler : public proxygen::coro::HTTPFilterFactoryHandler {
 public:
  HTTPServerHandler(std::shared_ptr<HTTPHandler> userHandler,
                    HTTPServer::ServerFilterFactoryList filterFactories) {
    // user supplied handler must exist
    CHECK(userHandler);
    setNextHandler(std::move(userHandler));
    for (auto& factory : filterFactories) {
      addFilterFactory(std::move(factory));
    }
  }

  ~HTTPServerHandler() override = default;
};

} // namespace

HTTPServer::~HTTPServer() {
  XCHECK(!eventBase_.isRunning());
}

void HTTPServer::start(
    std::function<void()> onSuccess,
    std::function<void(std::exception_ptr)> onError,
    std::shared_ptr<folly::ThreadPoolExecutor::Observer> observer) {
  folly::EventBaseManager::get()->setEventBase(&eventBase_, false);
  if (config_.numIOThreads == 0) {
    config_.numIOThreads = std::thread::hardware_concurrency();
  }
  folly::IOThreadPoolExecutor::Options options;
  options.setWaitForAll(true);
  folly::IOThreadPoolExecutor threadPool(
      config_.numIOThreads,
      config_.numIOThreads,
      std::make_shared<folly::NamedThreadFactory>("HTTPServerIO"),
      folly::EventBaseManager::get(),
      options);
  auto threadObserver = std::make_shared<InternalThreadObserver>(this);
  threadPool.addObserver(threadObserver);
  if (observer) {
    threadPool.addObserver(observer);
  }

  // if filterFactories not empty, wrap user supplied handler with server
  // handler that installs the filterFactories
  if (!config_.filterFactories.empty()) {
    handler_ = std::make_shared<HTTPServerHandler>(std::move(handler_),
                                                   config_.filterFactories);
  }

  try {
    // Ensures all evb threads are running
    auto evbs = threadPool.getAllEventBases();
    if (config_.quicConfig) {
      startQuic(evbs);
    } else {
      startTcp(evbs);
    }
  } catch (const std::exception& ex) {
    XLOG(ERR) << "Initialization encountered an error=" << ex.what();
    if (onError) {
      onError(std::current_exception());
    } else {
      // Rather than swallowing the exception silently, raise it accordingly.
      throw;
    }
    return;
  }
  run(std::move(onSuccess));
  // Blocks until all IO Threads have terminated and joined
  threadPool.stop();
  state_ = State::STOPPED;
}

void HTTPServer::startQuic(const KeepAliveEventBaseVec& keepAliveEvbs) {
  std::vector<folly::EventBase*> evbs{keepAliveEvbs.size(), nullptr};
  std::transform(keepAliveEvbs.begin(),
                 keepAliveEvbs.end(),
                 evbs.begin(),
                 [](const folly::Executor::KeepAlive<folly::EventBase>& evb) {
                   return evb.get();
                 });
  createQuicServer(evbs);
  quicServer_->initialize(config_.socketConfig.bindAddress, evbs, true);
  quicServer_->waitUntilInitialized();
  quicServer_->start();

  // Wait for start to install read callback for all listening sockets
  std::atomic<size_t> started = evbs.size();
  folly::Baton baton;
  for (auto evb : evbs) {
    evb->runInEventBaseThread([&] {
      if (--started == 0) {
        baton.post();
      }
    });
  }
  baton.wait();
}

HTTPCoroAcceptor* HTTPServer::createAcceptor(
    folly::EventBase* evb,
    std::shared_ptr<const AcceptorConfiguration> acceptorConfig) {
  auto [it, _] = acceptors_.try_emplace(evb, std::list<HTTPCoroAcceptor>());
  return &(*it->second.emplace(it->second.end(),
                               std::move(acceptorConfig),
                               handler_,
                               &config_.newConnectionFilter));
}

HTTPCoroAcceptor* FOLLY_NULLABLE
HTTPServer::getQuicAcceptor(folly::EventBase* evb) {
  auto it = acceptors_.find(evb);
  if (it == acceptors_.end()) {
    return nullptr;
  }
  // For now, the quic implementation can still only support one acceptor.
  XCHECK_EQ(it->second.size(), 1UL);
  return &it->second.front();
}

void HTTPServer::startTcp(const KeepAliveEventBaseVec& keepAliveEvbs) {
  std::vector<SocketAcceptorConfig> socketAcceptorConfigs;
  if (socketAcceptorConfigFactoryFn_) {
    XLOG(DBG4) << "Using custom socket acceptor config factory";
    socketAcceptorConfigs = socketAcceptorConfigFactoryFn_(eventBase_, config_);
    for (auto& socketAcceptorConfig : socketAcceptorConfigs) {
      socketAcceptorConfig.socket->startAccepting();
    }
  } else {
    auto serverSocket = folly::AsyncServerSocket::UniquePtr(
        new folly::AsyncServerSocket(&eventBase_));
    try {
      serverSocket->setReusePortEnabled(setReusePortSocketOption_);
      if (config_.preboundSocket.has_value()) {
        serverSocket->useExistingSocket(
            folly::NetworkSocket::fromFd(config_.preboundSocket.value()));
      } else {
        serverSocket->bind(config_.socketConfig.bindAddress);
      }
      serverSocket->listen(config_.socketConfig.acceptBacklog);
      serverSocket->startAccepting();
    } catch (const std::exception& ex) {
      XLOG(ERR) << "Failed to setup server socket ex=" << ex.what();
      throw;
    }
    socketAcceptorConfigs.push_back({
        .socket = std::move(serverSocket),
        .acceptorConfig = toAcceptorConfig(config_),
    });
  }
  for (auto& socketAcceptorConfig : socketAcceptorConfigs) {
    for (auto& evb : keepAliveEvbs) {
      createAcceptor(evb.get(), socketAcceptorConfig.acceptorConfig)
          ->init(socketAcceptorConfig.socket.get(), evb.get());
    }
    serverSockets_.emplace_back(std::move(socketAcceptorConfig.socket));
  }
}

void HTTPServer::run(std::function<void()> onSuccess) {
  XLOG(DBG4) << __func__;
  XCHECK_EQ(state_, State::UNINIT);
  state_ = State::RUNNING;
  for (auto sig : config_.shutdownOnSignals) {
    signalHandler_.registerSignalHandler(sig);
  }
  if (onSuccess) {
    eventBase_.runInLoop([onSuccess = std::move(onSuccess)] { onSuccess(); });
  }
  eventBase_.loop();
  XLOG(DBG4) << __func__ << " exit";
}

void HTTPServer::createQuicServer(const std::vector<folly::EventBase*>& evbs) {
  std::shared_ptr<fizz::server::FizzServerContext> fizzCtx;
  auto acceptorConfig = toAcceptorConfig(config_);
  for (auto evb : evbs) {
    auto acceptor = createAcceptor(evb, acceptorConfig);
    acceptor->init(nullptr, evb);
    nRunningAcceptors_++;
    acceptor->setOnConnectionDrainedFn([this] {
      if (--nRunningAcceptors_ == 0) {
        quicServer_->shutdown();
      }
    });
    if (!fizzCtx) {
      fizzCtx = acceptor->recreateFizzContext();
    }
  }
  CHECK(config_.quicConfig) << "QuicConfig must be set";
  quicServer_ =
      quic::QuicServer::createQuicServer(config_.quicConfig->transportSettings);
  quicServer_->setBindV6Only(false);
  quicServer_->setCongestionControllerFactory(
      std::make_shared<quic::ServerCongestionControllerFactory>());
  quicServer_->setQuicServerTransportFactory(
      std::make_unique<QuicAcceptorTransportFactory>(*this));
  quicServer_->setQuicUDPSocketFactory(
      std::make_unique<quic::QuicSharedUDPSocketFactory>());
  quicServer_->setHealthCheckToken("health");
  if (!config_.quicConfig->quicVersions.empty()) {
    quicServer_->setSupportedVersion(config_.quicConfig->quicVersions);
  }
  configureFizzServerContext(fizzCtx);
  quicServer_->setFizzContext(fizzCtx);
  if (config_.quicConfig->rateLimitPerThread) {
    quicServer_->setRateLimit(
        [rateLimitPerThread =
             config_.quicConfig->rateLimitPerThread.value()]() {
          return rateLimitPerThread;
        },
        std::chrono::seconds(1));
  }
  if (config_.quicConfig->statsFactory) {
    quicServer_->setTransportStatsCallbackFactory(
        std::move(config_.quicConfig->statsFactory));
  }
  if (config_.quicConfig->ccFactory) {
    quicServer_->setCongestionControllerFactory(
        std::move(config_.quicConfig->ccFactory));
  }
  quicServer_->setConnectionIdVersion(quic::ConnectionIdVersion::V2);
  quicServer_->setHostId(hostId_);
}

void HTTPServer::configureFizzServerContext(
    std::shared_ptr<fizz::server::FizzServerContext> serverCtx) {
  serverCtx->setSupportedAlpns(config_.quicConfig->supportedAlpns);
  serverCtx->setAlpnMode(fizz::server::AlpnMode::Required);
  serverCtx->setSendNewSessionTicket(false);
  serverCtx->setEarlyDataFbOnly(false);
  serverCtx->setVersionFallbackEnabled(false);

  fizz::server::ClockSkewTolerance tolerance;
  tolerance.before = std::chrono::minutes(-5);
  tolerance.after = std::chrono::minutes(5);

  std::shared_ptr<fizz::server::ReplayCache> replayCache =
      std::make_shared<fizz::server::AllowAllReplayReplayCache>();

  serverCtx->setEarlyDataSettings(true, tolerance, std::move(replayCache));
}

void HTTPServer::onQuicTransportReady(
    std::shared_ptr<quic::QuicSocket> quicSocket) {
  wangle::TransportInfo tinfo;
  tinfo.acceptTime = getCurrentTime();
  tinfo.appProtocol =
      std::make_shared<std::string>(quicSocket->getAppProtocol().value_or(""));
  tinfo.secure = true;
  // TODO: fill in other tinfo
  auto acceptor =
      getQuicAcceptor(quicSocket->getEventBase()
                          ->getTypedEventBase<quic::FollyQuicEventBase>()
                          ->getBackingEventBase());
  XCHECK(acceptor) << "QuicSocket in foreign EventBase";
  acceptor->onNewConnection(std::move(quicSocket), std::move(tinfo));
}

void HTTPServer::drain() {
  XLOG(DBG4) << __func__;
  if (state_ == State::RUNNING) {
    state_ = State::DRAINING;
    eventBase_.runImmediatelyOrRunInEventBaseThread(
        [this] { globalDrainImpl(); });
    for (auto& it : acceptors_) {
      for (auto& acceptor : it.second) {
        if (auto evb = acceptor.getEventBaseKeepalive()) {
          evb->runImmediatelyOrRunInEventBaseThread(
              [this, &acceptor] { drainImpl(acceptor); });
        } // else the acceptor already drained, maybe called after
          // drain/forceStop
      }
    }
    eventBase_.runImmediatelyOrRunInEventBaseThread(
        [this] { unregisterSignalHandlers(); });
  }
}

void HTTPServer::globalDrainImpl() {
  XLOG(DBG4) << __func__;
  for (const auto& serverSocket : serverSockets_) {
    XCHECK(serverSocket);
    serverSocket->stopAccepting();
  }
  if (quicServer_) {
    quicServer_->rejectNewConnections([]() { return true; });
  }
}

void HTTPServer::unregisterSignalHandlers() {
  XLOG(DBG4) << __func__;
  for (auto sig : config_.shutdownOnSignals) {
    signalHandler_.unregisterSignalHandler(sig);
  }
}

void HTTPServer::drainImpl(HTTPCoroAcceptor& acceptor) {
  XLOG(DBG4) << __func__;
  if (quicServer_) {
    acceptor.stopAcceptingQuic();
  }
}

void HTTPServer::forceStop() {
  XLOG(DBG4) << __func__;
  auto state = state_.load();
  if (state == State::RUNNING) {
    state_ = state = State::DRAINING;
    folly::ExecutorKeepAlive keepAlive(&eventBase_);
    eventBase_.runImmediatelyOrRunInEventBaseThread([this] {
      for (const auto& serverSocket : serverSockets_) {
        XCHECK(serverSocket);
        serverSocket->stopAccepting();
      }
    });
    eventBase_.runImmediatelyOrRunInEventBaseThread(
        [this] { unregisterSignalHandlers(); });
  }
  if (state == State::DRAINING) {
    if (quicServer_) {
      quicServer_->shutdown();
    }
    for (auto& it : acceptors_) {
      for (auto& acceptor : it.second) {
        if (acceptor.getEventBaseKeepalive()) {
          acceptor.forceStop();
        } // else, the acceptor already drained
      }
    }
  }
}

std::shared_ptr<const AcceptorConfiguration> HTTPServer::toAcceptorConfig(
    const Config& config) {
  auto accConfig = std::make_shared<AcceptorConfiguration>();
  wangle::ServerSocketConfig* serverSocketConfig = accConfig.get();
  *serverSocketConfig = config.socketConfig;
  // TODO: write timeout
  accConfig->egressSettings = config.sessionConfig.settings;
  accConfig->transactionIdleTimeout = config.sessionConfig.streamReadTimeout;
  accConfig->receiveSessionWindowSize = config.sessionConfig.connFlowControl;
  accConfig->receiveStreamWindowSize =
      getHTTPSettingValueOrDefault(config.sessionConfig.settings,
                                   SettingsId::INITIAL_WINDOW_SIZE,
                                   config.sessionConfig.streamFlowControl);
  accConfig->plaintextProtocol = config.plaintextProtocol;
  accConfig->forceHTTP1_0_to_1_1 = true;
  accConfig->connectionIdleTimeout = config.sessionConfig.connIdleTimeout;
  return accConfig;
}

void HTTPServer::setHostId(uint32_t hostId) {
  hostId_ = hostId;
  if (quicServer_) {
    // quicServer_ has been initialized. need to update hostId in
    // quicServer_
    eventBase_.runImmediatelyOrRunInEventBaseThreadAndWait(
        [this] { quicServer_->setHostId(hostId_); });
  }
}

} // namespace proxygen::coro
