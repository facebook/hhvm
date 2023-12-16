/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/httpserver/HTTPServer.h>

#include <folly/executors/thread_factory/NamedThreadFactory.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/system/ThreadName.h>
#include <proxygen/httpserver/HTTPServerAcceptor.h>
#include <proxygen/httpserver/SignalHandler.h>
#include <proxygen/httpserver/filters/CompressionFilter.h>
#include <proxygen/httpserver/filters/RejectConnectFilter.h>
#include <wangle/ssl/SSLContextManager.h>

using folly::EventBaseManager;
using folly::IOThreadPoolExecutor;
using folly::IOThreadPoolExecutorBase;
using folly::ThreadPoolExecutor;

namespace proxygen {

class AcceptorFactory : public wangle::AcceptorFactory {
 public:
  AcceptorFactory(std::shared_ptr<HTTPServerOptions> options,
                  std::shared_ptr<HTTPCodecFactory> codecFactory,
                  AcceptorConfiguration config,
                  HTTPSession::InfoCallback* sessionInfoCb)
      : options_(options),
        codecFactory_(codecFactory),
        config_(config),
        sessionInfoCb_(sessionInfoCb) {
  }
  std::shared_ptr<wangle::Acceptor> newAcceptor(
      folly::EventBase* eventBase) override {
    auto acc = std::shared_ptr<HTTPServerAcceptor>(
        HTTPServerAcceptor::make(config_, *options_, codecFactory_).release());
    if (sessionInfoCb_) {
      acc->setSessionInfoCallback(sessionInfoCb_);
    }
    acc->init(nullptr, eventBase);
    return acc;
  }

 private:
  std::shared_ptr<HTTPServerOptions> options_;
  std::shared_ptr<HTTPCodecFactory> codecFactory_;
  AcceptorConfiguration config_;
  HTTPSession::InfoCallback* sessionInfoCb_;
};

HTTPServer::HTTPServer(HTTPServerOptions options)
    : options_(std::make_shared<HTTPServerOptions>(std::move(options))) {

  if (options_->threads == 0) {
    options_->threads = std::thread::hardware_concurrency();
  }

  // Insert a filter to fail all the CONNECT request, if required
  if (!options_->supportsConnect) {
    options_->handlerFactories.insert(
        options_->handlerFactories.begin(),
        std::make_unique<RejectConnectFilterFactory>());
  }

  // Add Content Compression filter (gzip and maybe zstd), if needed. Should be
  // final filter
  if (options_->enableContentCompression) {
    CompressionFilterFactory::Options opts;
    opts.minimumCompressionSize = options_->contentCompressionMinimumSize;
    opts.zlibCompressionLevel = options_->contentCompressionLevel;
    opts.compressibleContentTypes = options_->contentCompressionTypes;
    opts.enableGzip = options_->enableGzipCompression;
    if (options_->enableZstdCompression) {
      opts.enableZstd = options_->enableZstdCompression;
      opts.independentChunks = options_->useZstdIndependentChunks;
      opts.zstdCompressionLevel = options_->zstdContentCompressionLevel;
    }
    options_->handlerFactories.insert(
        options_->handlerFactories.begin(),
        std::make_unique<CompressionFilterFactory>(opts));
  }
}

HTTPServer::~HTTPServer() {
  CHECK(!mainEventBase_) << "Forgot to stop() server?";
}

void HTTPServer::bind(std::vector<IPConfig>&& addrs) {
  addresses_ = std::move(addrs);
}

void HTTPServer::bind(std::vector<IPConfig> const& addrs) {
  addresses_ = addrs;
}

class HandlerCallbacks : public IOThreadPoolExecutorBase::IOObserver {
 public:
  explicit HandlerCallbacks(std::shared_ptr<HTTPServerOptions> options)
      : options_(options) {
  }

  void registerEventBase(folly::EventBase& evb) override {
    evb.runInEventBaseThread([&evb, this]() {
      for (auto& factory : options_->handlerFactories) {
        factory->onServerStart(&evb);
      }
    });
  }

  void unregisterEventBase(folly::EventBase& evb) override {
    evb.runInEventBaseThread([this]() {
      for (auto& factory : options_->handlerFactories) {
        factory->onServerStop();
      }
    });
  }

 private:
  std::shared_ptr<HTTPServerOptions> options_;
};

folly::Expected<folly::Unit, std::exception_ptr> HTTPServer::startTcpServer(
    std::shared_ptr<wangle::AcceptorFactory> inputAcceptorFactory,
    std::shared_ptr<folly::IOThreadPoolExecutorBase> ioExecutor) {
  auto accExe = std::make_shared<IOThreadPoolExecutor>(1);
  if (!ioExecutor) {
    ioExecutor = std::make_shared<IOThreadPoolExecutor>(
        options_->threads,
        std::make_shared<folly::NamedThreadFactory>("HTTPSrvExec"));
  }
  auto exeObserver = std::make_shared<HandlerCallbacks>(options_);
  // Observer has to be set before bind(), so onServerStart() callbacks run
  ioExecutor->addObserver(exeObserver);

  try {
    FOR_EACH_RANGE(i, 0, addresses_.size()) {
      auto accConfig = HTTPServerAcceptor::makeConfig(addresses_[i], *options_);
      // If user specified an acceptor factory to use, we will use it.
      // Otherwise, we create one for each address.
      auto acceptorFactory = inputAcceptorFactory;
      if (!acceptorFactory) {
        auto codecFactory = addresses_[i].codecFactory;
        acceptorFactory = std::make_shared<AcceptorFactory>(
            options_, codecFactory, accConfig, sessionInfoCb_);
      }
      bootstrap_.push_back(wangle::ServerBootstrap<wangle::DefaultPipeline>());
      bootstrap_[i].childHandler(acceptorFactory);
      bootstrap_[i].useZeroCopy(options_->useZeroCopy);
      if (accConfig.enableTCPFastOpen) {
        // We need to do this because wangle's bootstrap has 2 acceptor configs
        // and the socketConfig gets passed to the SocketFactory. The number of
        // configs should really be one, and when that happens, we can remove
        // this code path.
        bootstrap_[i].socketConfig.enableTCPFastOpen = true;
        bootstrap_[i].socketConfig.fastOpenQueueSize =
            accConfig.fastOpenQueueSize;
      }
      bootstrap_[i].group(accExe, ioExecutor);
      if (accConfig.reusePort) {
        bootstrap_[i].setReusePort(true);
      }
      if (options_->preboundSockets_.size() > i) {
        bootstrap_[i].bind(std::move(options_->preboundSockets_[i]));
      } else {
        bootstrap_[i].bind(addresses_[i].address);
      }
    }
  } catch (const std::exception&) {
    stop();

    return folly::makeUnexpected(std::current_exception());
  }

  return folly::unit;
}

void HTTPServer::start(
    std::function<void()> onSuccess,
    std::function<void(std::exception_ptr)> onError,
    std::shared_ptr<wangle::AcceptorFactory> acceptorFactory,
    std::shared_ptr<folly::IOThreadPoolExecutorBase> ioExecutor) {
  mainEventBase_ = EventBaseManager::get()->getEventBase();

  auto tcpStarted = startTcpServer(acceptorFactory, ioExecutor);
  if (tcpStarted.hasError()) {
    if (onError) {
      onError(tcpStarted.error());
      return;
    }
    std::rethrow_exception(tcpStarted.error());
  }

  // Install signal handler if required
  if (!options_->shutdownOn.empty()) {
    signalHandler_ = std::make_unique<SignalHandler>(this);
    signalHandler_->install(options_->shutdownOn);
  }

  // Start the main event loop.
  if (onSuccess) {
    mainEventBase_->runInLoop([onSuccess(std::move(onSuccess))]() {
      // IMPORTANT: Since we may be racing with stop(), we must assume that
      // mainEventBase_ can become null the moment that onSuccess is called,
      // so this **has** to be queued to run from inside loopForever().
      onSuccess();
    });
  }
  mainEventBase_->loopForever();
}

void HTTPServer::stopListening() {
  for (auto& bootstrap : bootstrap_) {
    bootstrap.stop();
  }
}

void HTTPServer::stop() {
  stopListening();

  for (auto& bootstrap : bootstrap_) {
    bootstrap.join();
  }

  if (signalHandler_) {
    signalHandler_.reset();
  }

  if (mainEventBase_) {
    // This HTTPServer object may be destoyed by the main thread once
    // terminateLoopSoon() is called, so terminateLoopSoon() should be the last
    // operation here.
    std::exchange(mainEventBase_, nullptr)->terminateLoopSoon();
  }
}

const std::vector<const folly::AsyncSocketBase*> HTTPServer::getSockets()
    const {

  std::vector<const folly::AsyncSocketBase*> sockets;
  FOR_EACH_RANGE(i, 0, bootstrap_.size()) {
    auto& bootstrapSockets = bootstrap_[i].getSockets();
    FOR_EACH_RANGE(j, 0, bootstrapSockets.size()) {
      sockets.push_back(bootstrapSockets[j].get());
    }
  }

  return sockets;
}

int HTTPServer::getListenSocket() const {
  if (bootstrap_.size() == 0) {
    return -1;
  }

  auto& bootstrapSockets = bootstrap_[0].getSockets();
  if (bootstrapSockets.size() == 0) {
    return -1;
  }

  auto serverSocket =
      std::dynamic_pointer_cast<folly::AsyncServerSocket>(bootstrapSockets[0]);
  auto socketFds = serverSocket->getNetworkSockets();
  if (socketFds.size() == 0) {
    return -1;
  }

  return socketFds[0].toFd();
}

void HTTPServer::updateTLSCredentials() {
  for (auto& bootstrap : bootstrap_) {
    bootstrap.forEachWorker([&](wangle::Acceptor* acceptor) {
      if (!acceptor || !acceptor->isSSL()) {
        return;
      }
      auto evb = acceptor->getEventBase();
      if (!evb) {
        return;
      }
      evb->runInEventBaseThread(
          [acceptor] { acceptor->resetSSLContextConfigs(); });
    });
  }
}

void HTTPServer::updateTicketSeeds(wangle::TLSTicketKeySeeds seeds) {
  for (auto& bootstrap : bootstrap_) {
    bootstrap.forEachWorker([&](wangle::Acceptor* acceptor) {
      if (!acceptor || !acceptor->isSSL()) {
        return;
      }
      auto evb = acceptor->getEventBase();
      if (!evb) {
        return;
      }
      evb->runInEventBaseThread([acceptor, seeds] {
        acceptor->setTLSTicketSecrets(
            seeds.oldSeeds, seeds.currentSeeds, seeds.newSeeds);
      });
    });
  }
}

} // namespace proxygen
