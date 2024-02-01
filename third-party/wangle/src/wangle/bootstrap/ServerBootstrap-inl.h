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
#include <folly/SharedMutex.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBaseManager.h>
#include <wangle/acceptor/Acceptor.h>
#include <wangle/acceptor/ManagedConnection.h>
#include <wangle/acceptor/SharedSSLContextManager.h>
#include <wangle/bootstrap/ServerSocketFactory.h>
#include <wangle/channel/Handler.h>
#include <wangle/channel/Pipeline.h>
#include <wangle/ssl/SSLStats.h>

namespace wangle {

class AcceptorException : public std::runtime_error {
 public:
  enum class ExceptionType {
    UNKNOWN = 0,
    TIMED_OUT = 1,
    DROPPED = 2,
    ACCEPT_STOPPED = 3,
    DRAIN_CONN_PCT = 4,
    DROP_CONN_PCT = 5,
    FORCE_STOP = 6,
    INTERNAL_ERROR = 7,
    ACCEPT_PAUSED = 8,
    ACCEPT_RESUMED = 9,
    SHUTDOWN_PENDING = 10, // A graceful shutdown has been scheduled.
  };

  explicit AcceptorException(ExceptionType type)
      : std::runtime_error(""), type_(type), pct_(0.0) {}

  explicit AcceptorException(ExceptionType type, const std::string& message)
      : std::runtime_error(message), type_(type), pct_(0.0) {}

  explicit AcceptorException(
      ExceptionType type,
      const std::string& message,
      double pct)
      : std::runtime_error(message), type_(type), pct_(pct) {}

  ExceptionType getType() const noexcept {
    return type_;
  }
  double getPct() const noexcept {
    return pct_;
  }

 protected:
  const ExceptionType type_;
  // the percentage of connections to be drained or dropped during the shutdown
  const double pct_;
};

template <typename Pipeline>
class ServerAcceptor : public Acceptor,
                       public wangle::InboundHandler<AcceptPipelineType> {
 public:
  using OnDataAvailableParams =
      folly::AsyncUDPSocket::ReadCallback::OnDataAvailableParams;

  class ServerConnection : public wangle::ManagedConnection,
                           public wangle::PipelineManager {
   public:
    explicit ServerConnection(typename Pipeline::Ptr pipeline)
        : pipeline_(std::move(pipeline)) {
      pipeline_->setPipelineManager(this);
    }

    explicit ServerConnection(
        typename Pipeline::Ptr pipeline,
        folly::SocketAddress peerAddress)
        : pipeline_(std::move(pipeline)), peerAddress_(std::move(peerAddress)) {
      pipeline_->setPipelineManager(this);
    }

    void timeoutExpired() noexcept override {
      auto ew = folly::make_exception_wrapper<AcceptorException>(
          AcceptorException::ExceptionType::TIMED_OUT, "timeout");
      pipeline_->readException(ew);
    }

    void describe(std::ostream&) const override {}
    bool isBusy() const override {
      return true;
    }

    void notifyPendingShutdown() override {
      if (enableNotifyPendingShutdown_) {
        auto ew = folly::make_exception_wrapper<AcceptorException>(
            AcceptorException::ExceptionType::SHUTDOWN_PENDING,
            "shutdown_pending");
        pipeline_->readException(ew);
      }
    }

    void closeWhenIdle() override {}
    void dropConnection(const std::string& /* errorMsg */ = "") override {
      auto ew = folly::make_exception_wrapper<AcceptorException>(
          AcceptorException::ExceptionType::DROPPED, "dropped");
      pipeline_->readException(ew);
    }
    void dumpConnectionState(uint8_t /* loglevel */) override {}

    void deletePipeline(wangle::PipelineBase* p) override {
      CHECK(p == pipeline_.get());
      destroy();
    }

    void init() {
      pipeline_->transportActive();
    }

    void refreshTimeout() override {
      resetTimeout();
    }

    void setNotifyPendingShutdown(bool isEnabled) {
      enableNotifyPendingShutdown_ = isEnabled;
    }

    [[nodiscard]] const folly::SocketAddress& getPeerAddress()
        const noexcept override {
      return peerAddress_;
    }

   private:
    ~ServerConnection() override {
      pipeline_->setPipelineManager(nullptr);
    }
    typename Pipeline::Ptr pipeline_;
    bool enableNotifyPendingShutdown_{false};
    folly::SocketAddress peerAddress_;
  };

  explicit ServerAcceptor(
      std::shared_ptr<AcceptPipelineFactory> acceptPipelineFactory,
      std::shared_ptr<PipelineFactory<Pipeline>> childPipelineFactory,
      const ServerSocketConfig& accConfig)
      : Acceptor(accConfig),
        acceptPipelineFactory_(acceptPipelineFactory),
        childPipelineFactory_(childPipelineFactory) {}

  void init(
      folly::AsyncServerSocket* serverSocket,
      folly::EventBase* eventBase,
      SSLStats* stats = nullptr,
      std::shared_ptr<const fizz::server::FizzServerContext> fizzCtx =
          nullptr) override {
    Acceptor::init(serverSocket, eventBase, stats, fizzCtx);

    acceptPipeline_ = acceptPipelineFactory_->newPipeline(this);

    if (childPipelineFactory_) {
      // This means a custom AcceptPipelineFactory was not passed in via
      // pipeline() and we're using the DefaultAcceptPipelineFactory.
      // Add the default inbound handler here.
      acceptPipeline_->addBack(this);
    }
    acceptPipeline_->finalize();
  }

  void setNotifyPendingShutdown(bool isEnabled) {
    enableNotifyPendingShutdown_ = isEnabled;
  }

  void read(Context*, AcceptPipelineType conn) override {
    if (conn.type() != typeid(ConnInfo&)) {
      return;
    }

    auto connInfo = boost::get<ConnInfo&>(conn);
    folly::AsyncTransport::UniquePtr transport(connInfo.sock);

    // Setup local and remote addresses
    auto tInfoPtr = std::make_shared<TransportInfo>(connInfo.tinfo);
    tInfoPtr->localAddr =
        std::make_shared<folly::SocketAddress>(accConfig_.bindAddress);
    transport->getLocalAddress(tInfoPtr->localAddr.get());
    tInfoPtr->remoteAddr =
        std::make_shared<folly::SocketAddress>(*connInfo.clientAddr);
    tInfoPtr->appProtocol =
        std::make_shared<std::string>(connInfo.nextProtoName);

    auto pipeline = childPipelineFactory_->newPipeline(
        std::shared_ptr<folly::AsyncTransport>(
            transport.release(), folly::DelayedDestruction::Destructor()));
    pipeline->setTransportInfo(tInfoPtr);
    auto connection =
        new ServerConnection(std::move(pipeline), *connInfo.clientAddr);
    connection->setNotifyPendingShutdown(enableNotifyPendingShutdown_);
    Acceptor::addConnection(connection);
    connection->init();
  }

  // Null implementation to terminate the call in this handler
  // and suppress warnings
  void readEOF(Context*) override {}
  void readException(Context*, folly::exception_wrapper) override {}

  /* See Acceptor::onNewConnection for details */
  void onNewConnection(
      folly::AsyncTransport::UniquePtr transport,
      const folly::SocketAddress* clientAddr,
      const std::string& nextProtocolName,
      SecureTransportType secureTransportType,
      const TransportInfo& tinfo) override {
    ConnInfo connInfo = {
        transport.release(),
        clientAddr,
        nextProtocolName,
        secureTransportType,
        tinfo};
    acceptPipeline_->read(connInfo);
  }

  // notify the acceptors in the acceptPipeline to drain & drop conns
  void acceptStopped() noexcept override {
    auto ew = folly::make_exception_wrapper<AcceptorException>(
        AcceptorException::ExceptionType::ACCEPT_STOPPED,
        "graceful shutdown timeout");

    acceptPipeline_->readException(ew);
    Acceptor::acceptStopped();
  }

  void drainConnections(double pct) noexcept override {
    auto ew = folly::make_exception_wrapper<AcceptorException>(
        AcceptorException::ExceptionType::DRAIN_CONN_PCT,
        "draining some connections",
        pct);

    acceptPipeline_->readException(ew);
    Acceptor::drainConnections(pct);
  }

  void dropConnections(double pct) noexcept override {
    auto ew = folly::make_exception_wrapper<AcceptorException>(
        AcceptorException::ExceptionType::DROP_CONN_PCT,
        "dropping some connections",
        pct);

    acceptPipeline_->readException(ew);
    Acceptor::dropConnections(pct);
  }

  void dropEstablishedConnections(
      double pct,
      const std::function<bool(ManagedConnection*)>& filter) noexcept override {
    auto ew = folly::make_exception_wrapper<AcceptorException>(
        AcceptorException::ExceptionType::DROP_CONN_PCT,
        "dropping some established connections",
        pct);

    acceptPipeline_->readException(ew);
    Acceptor::dropEstablishedConnections(pct, filter);
  }

  void dropIdleConnectionsBasedOnTimeout(
      std::chrono::milliseconds targetIdleTimeMs,
      const std::function<void(size_t)>& droppedConnectionsCB) override {
    auto ew = folly::make_exception_wrapper<AcceptorException>(
        AcceptorException::ExceptionType::DROP_CONN_PCT,
        "dropping idle connections");

    acceptPipeline_->readException(ew);
    Acceptor::dropIdleConnectionsBasedOnTimeout(
        targetIdleTimeMs, droppedConnectionsCB);
  }

  void forceStop() noexcept override {
    auto ew = folly::make_exception_wrapper<AcceptorException>(
        AcceptorException::ExceptionType::FORCE_STOP, "hard shutdown timeout");

    acceptPipeline_->readException(ew);
    Acceptor::forceStop();
  }

  // UDP thunk
  void onDataAvailable(
      std::shared_ptr<folly::AsyncUDPSocket> socket,
      const folly::SocketAddress& addr,
      std::unique_ptr<folly::IOBuf> buf,
      bool /* truncated */,
      OnDataAvailableParams /* params */) noexcept override {
    acceptPipeline_->read(
        AcceptPipelineType(make_tuple(buf.release(), socket, addr)));
  }

  void onConnectionAdded(const ManagedConnection*) override {
    acceptPipeline_->read(ConnEvent::CONN_ADDED);
  }

  void onConnectionRemoved(const ManagedConnection*) override {
    acceptPipeline_->read(ConnEvent::CONN_REMOVED);
  }

  void sslConnectionError(const folly::exception_wrapper& ex) override {
    acceptPipeline_->readException(ex);
    Acceptor::sslConnectionError(ex);
  }

 protected:
  std::shared_ptr<AcceptPipeline> acceptPipeline_;
  bool enableNotifyPendingShutdown_{false};

 private:
  std::shared_ptr<AcceptPipelineFactory> acceptPipelineFactory_;
  std::shared_ptr<PipelineFactory<Pipeline>> childPipelineFactory_;
};

template <typename Pipeline>
class ServerAcceptorFactory : public AcceptorFactory {
 public:
  explicit ServerAcceptorFactory(
      std::shared_ptr<AcceptPipelineFactory> acceptPipelineFactory,
      std::shared_ptr<PipelineFactory<Pipeline>> childPipelineFactory,
      const ServerSocketConfig& accConfig)
      : acceptPipelineFactory_(acceptPipelineFactory),
        childPipelineFactory_(childPipelineFactory),
        accConfig_(accConfig) {}

  virtual void enableSharedSSLContext(bool enable) {
    if (enable) {
      sharedSSLContextManager_ =
          std::make_shared<SharedSSLContextManagerImpl<FizzConfigUtil>>(
              accConfig_);
    }
  }

  std::shared_ptr<Acceptor> newAcceptor(folly::EventBase* base) override {
    auto acceptor = std::make_shared<ServerAcceptor<Pipeline>>(
        acceptPipelineFactory_, childPipelineFactory_, accConfig_);

    if (sharedSSLContextManager_) {
      acceptor->setFizzCertManager(sharedSSLContextManager_->getCertManager());
      acceptor->setSSLContextManager(
          sharedSSLContextManager_->getContextManager());
      acceptor->init(
          nullptr, base, nullptr, sharedSSLContextManager_->getFizzContext());
      sharedSSLContextManager_->addAcceptor(acceptor);
    } else {
      acceptor->init(nullptr, base, nullptr);
    }
    return acceptor;
  }

  std::shared_ptr<SharedSSLContextManager> getSharedSSLContextManager() const {
    return sharedSSLContextManager_;
  }

 private:
  std::shared_ptr<AcceptPipelineFactory> acceptPipelineFactory_;
  std::shared_ptr<PipelineFactory<Pipeline>> childPipelineFactory_;
  ServerSocketConfig accConfig_;
  std::shared_ptr<SharedSSLContextManager> sharedSSLContextManager_;
};

class ServerWorkerPool : public folly::IOThreadPoolExecutorBase::IOObserver {
 public:
  explicit ServerWorkerPool(
      std::shared_ptr<AcceptorFactory> acceptorFactory,
      std::shared_ptr<std::vector<std::shared_ptr<folly::AsyncSocketBase>>>
          sockets,
      std::shared_ptr<ServerSocketFactory> socketFactory)
      : workers_(std::make_shared<WorkerMap>()),
        acceptorFactory_(acceptorFactory),
        sockets_(sockets),
        socketFactory_(socketFactory) {}

  template <typename F>
  void forEachWorker(F&& f) const;

  template <typename F>
  void forRandomWorker(F&& f) const;

  void registerEventBase(folly::EventBase& evb) override;
  void unregisterEventBase(folly::EventBase& evb) override;

 private:
  using WorkerMap =
      std::vector<std::pair<folly::EventBase*, std::shared_ptr<Acceptor>>>;
  using Mutex = folly::SharedMutexReadPriority;

  std::shared_ptr<WorkerMap> workers_;
  mutable Mutex workersMutex_;
  std::shared_ptr<AcceptorFactory> acceptorFactory_;
  std::shared_ptr<std::vector<std::shared_ptr<folly::AsyncSocketBase>>>
      sockets_;
  std::shared_ptr<ServerSocketFactory> socketFactory_;
};

template <typename F>
void ServerWorkerPool::forEachWorker(F&& f) const {
  std::shared_lock holder(workersMutex_);
  for (const auto& kv : *workers_) {
    f(kv.second.get());
  }
}

template <typename F>
void ServerWorkerPool::forRandomWorker(F&& f) const {
  std::shared_lock holder(workersMutex_);
  DCHECK(workers_->size());
  f((*workers_)[folly::Random::rand32(workers_->size())].second.get());
}

class DefaultAcceptPipelineFactory : public AcceptPipelineFactory {
 public:
  typename AcceptPipeline::Ptr newPipeline(Acceptor*) override {
    return AcceptPipeline::create();
  }
};

} // namespace wangle
