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

#include <chrono>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_set>
#include <variant>

#include <folly/container/F14Map.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventHandler.h>
#include <folly/io/async/HHWheelTimer.h>
#include <folly/net/NetworkSocket.h>
#include <thrift/lib/cpp/async/TAsyncSSLSocket.h>
#include <thrift/lib/cpp2/security/FizzPeeker.h>
#include <thrift/lib/cpp2/server/IOWorkerContext.h>
#include <thrift/lib/cpp2/server/MemoryTracker.h>
#include <thrift/lib/cpp2/server/RequestsRegistry.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/peeking/TLSHelper.h>
#include <wangle/acceptor/Acceptor.h>
#include <wangle/acceptor/ConnectionManager.h>
#include <wangle/acceptor/PeekingAcceptorHandshakeHelper.h>

namespace apache {
namespace thrift {

// Forward declaration of classes
class Cpp2Connection;
class ThriftServer;
class ThriftQuicServer;

/**
 * Cpp2Worker drives the actual I/O for ThriftServer connections.
 *
 * The ThriftServer itself accepts incoming connections, then hands off each
 * connection to a Cpp2Worker running in another thread.  There should
 * typically be around one Cpp2Worker thread per core.
 */
class Cpp2Worker : public IOWorkerContext,
                   public wangle::Acceptor,
                   private wangle::PeekingAcceptorHandshakeHelper::PeekCallback,
                   public std::enable_shared_from_this<Cpp2Worker> {
 protected:
  enum { kPeekCount = 9 };
  struct DoNotUse {};

 public:
  /**
   * Cpp2Worker is the actual server object for existing connections.
   * One or more of these should be created by ThriftServer (one per
   * CPU core is recommended).
   *
   * @param server the ThriftServer which created us.
   */
  static std::shared_ptr<Cpp2Worker> create(
      ThriftServer* server,
      folly::EventBase* eventBase = nullptr,
      std::shared_ptr<fizz::server::CertManager> certManager = nullptr,
      std::shared_ptr<wangle::SSLContextManager> ctxManager = nullptr,
      std::shared_ptr<const fizz::server::FizzServerContext> fizzContext =
          nullptr) {
    std::shared_ptr<Cpp2Worker> worker(new Cpp2Worker(server, {}));
    worker->setFizzCertManager(certManager);
    worker->setSSLContextManager(ctxManager);
    worker->construct(server, eventBase, fizzContext);
    return worker;
  }

  static std::shared_ptr<Cpp2Worker> createDummy(
      folly::EventBase* eventBase, ThriftServer* server = nullptr) {
    std::shared_ptr<Cpp2Worker> worker(new Cpp2Worker(server, {}));
    worker->Acceptor::init(nullptr, eventBase);
    if (eventBase) {
      worker->IOWorkerContext::init(*eventBase);
    }
    return worker;
  }

  void init(
      folly::AsyncServerSocket* serverSocket,
      folly::EventBase* eventBase,
      wangle::SSLStats* stats,
      std::shared_ptr<const fizz::server::FizzServerContext> fizzContext)
      override {
    securityProtocolCtxManager_.addPeeker(this);
    Acceptor::init(serverSocket, eventBase, stats, fizzContext);
    IOWorkerContext::init(*eventBase);
  }

  /**
   * Get underlying server.
   *
   * @returns pointer to ThriftServer
   */
  ThriftServer* getServer() const { return server_; }

  const server::ServerConfigs* getServerContext() const override {
    return getServer();
  }

  /**
   * Get a shared_ptr of this Cpp2Worker.
   */
  std::shared_ptr<Cpp2Worker> getWorkerShared() { return shared_from_this(); }

  /**
   * SSL stats hook
   */
  void updateSSLStats(
      const folly::AsyncTransport* sock,
      std::chrono::milliseconds acceptLatency,
      wangle::SSLErrorEnum error,
      const folly::exception_wrapper& ex) noexcept override;

  void handleHeader(
      folly::AsyncTransport::UniquePtr sock,
      const folly::SocketAddress* addr,
      const wangle::TransportInfo& tinfo);

  RequestsRegistry* getRequestsRegistry() const { return requestsRegistry_; }

  bool isStopping() const { return stopping_.load(std::memory_order_relaxed); }

  struct ActiveRequestsDecrement {
    void operator()(Cpp2Worker* worker) {
      if (--worker->activeRequests_ == 0 && worker->isStopping()) {
        worker->stopBaton_.post();
      }
    }
  };
  using ActiveRequestsGuard =
      std::unique_ptr<Cpp2Worker, ActiveRequestsDecrement>;
  ActiveRequestsGuard getActiveRequestsGuard();

  class PerServiceMetadata {
   public:
    explicit PerServiceMetadata(
        AsyncProcessorFactory& processorFactory,
        AsyncProcessorFactory::CreateMethodMetadataResult&& methods,
        std::shared_ptr<AsyncProcessorFactory> processorFactoryStorage = {})
        : processorFactory_(processorFactory), methods_(std::move(methods)) {
      if (processorFactoryStorage) {
        processorFactoryStorageTracker_.emplace(
            std::move(processorFactoryStorage));
      }
    }

    bool expired() const {
      return processorFactoryStorageTracker_ &&
          processorFactoryStorageTracker_->expired();
    }

    /**
     * The service metadata contained an entry for the provided method name.
     * Otherwise, if the metadata is WildcardMethodMetadataMap, then this is a
     * reference to a WildcardMethodMetadata object.
     *
     * This aligns with the contracts of MethodMetadataMap and
     * WildcardMethodMetadataMap.
     */
    struct MetadataFound {
      const AsyncProcessorFactory::MethodMetadata& metadata;
    };
    /**
     * The service metadata did not contain an entry for the provided method
     * name. This should result in an unknown method error.
     */
    struct MetadataNotFound {};

    /**
     * The result type of findMethod() below.
     */
    using FindMethodResult = std::variant<MetadataFound, MetadataNotFound>;
    /**
     * Looks up the provided method name in the metadata map.
     *
     * This returns a valid metadata object per the contract established by
     * AsyncProcessorFactory::createMethodMetadata.
     *
     * This returns MetadataNotFound iff no valid metadata exists. That means
     * that an unknown method error should be sent.
     */
    FindMethodResult findMethod(std::string_view methodName) const;

    /**
     * Extracts the base request context from the service based on the result of
     * findMethod().
     * This returns nullptr iff no metadata was found or createMethodMetadata()
     * is not implemented.
     */
    std::shared_ptr<folly::RequestContext> getBaseContextForRequest(
        const FindMethodResult&) const;

   private:
    AsyncProcessorFactory& processorFactory_;
    std::optional<std::weak_ptr<AsyncProcessorFactory>>
        processorFactoryStorageTracker_;
    AsyncProcessorFactory::CreateMethodMetadataResult methods_;
  };
  /**
   * Gets the per-IO-thread metadata stored per-service. The metadata is lazily
   * created and the same object is returned for subsequent calls that pass the
   * same service.
   */
  PerServiceMetadata& getMetadataForService(
      AsyncProcessorFactory& processorFactory,
      std::shared_ptr<AsyncProcessorFactory> processorFactoryStorage = {})
      const {
    getEventBase()->dcheckIsInEventBaseThread();
    if (auto metadata =
            folly::get_ptr(perServiceMetadata_, &processorFactory)) {
      if (!metadata->expired()) {
        return *metadata;
      }
    }
    // Perform GC
    {
      std::vector<AsyncProcessorFactory*> expired;
      for (const auto& kv : perServiceMetadata_) {
        if (kv.second.expired()) {
          expired.push_back(kv.first);
        }
      }

      for (auto expiredProcessorFactory : expired) {
        perServiceMetadata_.erase(expiredProcessorFactory);
      }
    }
    auto [metadata, _] = perServiceMetadata_.emplace(
        &processorFactory,
        PerServiceMetadata{
            processorFactory,
            processorFactory.createMethodMetadata(),
            std::move(processorFactoryStorage)});
    return metadata->second;
  }

  static void dispatchRequest(
      AsyncProcessor* processor,
      ResponseChannelRequest::UniquePtr request,
      SerializedCompressedRequest&& serializedCompressedRequest,
      const PerServiceMetadata::FindMethodResult& methodMetadataResult,
      protocol::PROTOCOL_TYPES protocolId,
      Cpp2RequestContext* cpp2ReqCtx,
      concurrency::ThreadManager* tm,
      server::ServerConfigs* serverConfigs);

 protected:
  Cpp2Worker(
      ThriftServer* server,
      DoNotUse /* ignored, never call constructor directly */)
      : Acceptor(
            server ? server->getServerSocketConfig()
                   : wangle::ServerSocketConfig()),
        wangle::PeekingAcceptorHandshakeHelper::PeekCallback(kPeekCount),
        server_(server),
        activeRequests_(0) {
    if (server) {
      // Leave enough headroom to close connections ungracefully before the
      // worker join timeout expires.
      constexpr auto kGracefulTimeoutHeadroom = std::chrono::milliseconds{500};
      setGracefulShutdownTimeout(std::max(
          server->getWorkersJoinTimeout() - kGracefulTimeoutHeadroom,
          std::chrono::milliseconds::zero()));
    }
  }

  void construct(
      ThriftServer* server,
      folly::EventBase* eventBase,
      std::shared_ptr<const fizz::server::FizzServerContext> fizzContext) {
    auto observer = std::dynamic_pointer_cast<folly::EventBaseObserver>(
        server_->getObserverShared());
    if (!eventBase) {
      eventBase = folly::EventBaseManager::get()->getEventBase();
    }
    if (server) {
      fizzPeeker_.setTransportOptions(server->getFizzConfig().transportOptions);
    }
    init(nullptr, eventBase, nullptr, fizzContext);
    initRequestsRegistry();

    if (observer) {
      eventBase->add([eventBase, observer = std::move(observer)] {
        eventBase->setObserver(observer);
      });
    }

    // We distribute the memory limit averaged out over all IO workers. This
    // avoids the need to synchronize memory usage counts with other IO threads.
    // folly::AsyncServerSocket hands out connections to IO workers in a
    // round-robin manner so we should expect a roughly uniform distribution of
    // payload sizes.
    ingressMemoryTracker_ = std::make_unique<MemoryTracker>(
        folly::observer::makeObserver([server]() -> size_t {
          return **server->getIngressMemoryLimitObserver() /
              server->getNumIOWorkerThreads();
        }),
        server->getMinPayloadSizeToEnforceIngressMemoryLimitObserver());
    egressMemoryTracker_ = std::make_unique<MemoryTracker>(
        folly::observer::makeObserver([server]() -> size_t {
          return **server->getEgressMemoryLimitObserver() /
              server->getNumIOWorkerThreads();
        }));
  }

  void onNewConnection(
      folly::AsyncTransport::UniquePtr,
      const folly::SocketAddress*,
      const std::string&,
      wangle::SecureTransportType,
      const wangle::TransportInfo&) override;

  virtual std::shared_ptr<folly::AsyncTransport> createThriftTransport(
      folly::AsyncTransport::UniquePtr);

  void markSocketAccepted(folly::AsyncSocket* sock);

  void plaintextConnectionReady(
      folly::AsyncSocket::UniquePtr sock,
      const folly::SocketAddress& clientAddr,
      wangle::TransportInfo& tinfo) override;

  void requestStop();

  // returns false if timed out due to deadline
  bool waitForStop(std::chrono::steady_clock::time_point deadline);

  virtual wangle::AcceptorHandshakeHelper::UniquePtr createSSLHelper(
      const std::vector<uint8_t>& bytes,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      wangle::TransportInfo& tinfo);

  wangle::DefaultToFizzPeekingCallback* getFizzPeeker() override {
    return &fizzPeeker_;
  }

  MemoryTracker& getIngressMemoryTracker() { return *ingressMemoryTracker_; }
  MemoryTracker& getEgressMemoryTracker() { return *egressMemoryTracker_; }

 private:
  void onNewConnectionThatMayThrow(
      folly::AsyncTransport::UniquePtr,
      const folly::SocketAddress*,
      const std::string&,
      wangle::SecureTransportType,
      const wangle::TransportInfo&);

  /// The mother ship.
  ThriftServer* server_;

  FizzPeeker fizzPeeker_;

  // We expect to have one processor factory per InterfaceKind. Using F14NodeMap
  // guarantees reference stability.
  mutable folly::F14NodeMap<AsyncProcessorFactory*, PerServiceMetadata>
      perServiceMetadata_;

  folly::AsyncSocket::UniquePtr makeNewAsyncSocket(
      folly::EventBase* base,
      int fd,
      const folly::SocketAddress* peerAddress) override;

  folly::AsyncSSLSocket::UniquePtr makeNewAsyncSSLSocket(
      const std::shared_ptr<folly::SSLContext>& ctx,
      folly::EventBase* base,
      int fd,
      const folly::SocketAddress* peerAddress) override {
    return folly::AsyncSSLSocket::UniquePtr(
        new apache::thrift::async::TAsyncSSLSocket(
            ctx,
            base,
            folly::NetworkSocket::fromFd(fd),
            true, /* set server */
            true /* defer the security negotiation until sslAccept. */,
            peerAddress));
  }

  void cancelQueuedRequests();

  static void handleServerRequestRejection(
      const ServerRequest& serverRequest, ServerRequestRejection& reject);

  uint32_t activeRequests_;
  RequestsRegistry* requestsRegistry_;
  std::atomic<bool> stopping_{false};
  folly::Baton<> stopBaton_;
  std::unique_ptr<MemoryTracker> ingressMemoryTracker_;
  std::unique_ptr<MemoryTracker> egressMemoryTracker_;
  std::unique_ptr<folly::WorkerProvider> workerProvider_;

  void initRequestsRegistry();

  wangle::AcceptorHandshakeHelper::UniquePtr getHelper(
      const std::vector<uint8_t>& bytes,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      wangle::TransportInfo& tinfo) override;

  bool isPlaintextAllowedOnLoopback() {
    return server_->isPlaintextAllowedOnLoopback();
  }

  SSLPolicy getSSLPolicy() { return server_->getSSLPolicy(); }

  bool shouldPerformSSL(
      const std::vector<uint8_t>& bytes,
      const folly::SocketAddress& clientAddr);

  std::optional<ThriftParametersContext> getThriftParametersContext(
      const folly::SocketAddress& clientAddr);

  static const std::string& errorCodeFromTapplicationException(
      TApplicationException::TApplicationExceptionType exceptionType);

  friend class Cpp2Connection;
  friend class ThriftServer;
  friend class ThriftQuicServer;
  friend class RocketRoutingHandler;
  friend class TestRoutingHandler;
};

} // namespace thrift
} // namespace apache
