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

#ifndef THRIFT_SERVER_H_
#define THRIFT_SERVER_H_ 1

#include <array>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <map>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include <folly/Memory.h>
#include <folly/Singleton.h>
#include <folly/SocketAddress.h>
#include <folly/TokenBucket.h>
#include <folly/dynamic.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/experimental/PrimaryPtr.h>
#include <folly/experimental/coro/AsyncScope.h>
#include <folly/experimental/observer/Observer.h>
#include <folly/io/ShutdownSocketSet.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseLocal.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/lang/Badge.h>
#include <folly/synchronization/CallOnce.h>
#include <thrift/lib/cpp/concurrency/PosixThreadFactory.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/HeaderServerChannel.h>
#include <thrift/lib/cpp2/server/BaseThriftServer.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/server/PolledServiceHealth.h>
#include <thrift/lib/cpp2/server/PreprocessParams.h>
#include <thrift/lib/cpp2/server/RequestDebugLog.h>
#include <thrift/lib/cpp2/server/RequestsRegistry.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/ServerInstrumentation.h>
#include <thrift/lib/cpp2/server/ServiceHealthPoller.h>
#include <thrift/lib/cpp2/server/TransportRoutingHandler.h>
#include <thrift/lib/cpp2/transport/rocket/PayloadUtils.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_constants.h>
#include <wangle/acceptor/ServerSocketConfig.h>
#include <wangle/acceptor/SharedSSLContextManager.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/ssl/SSLContextConfig.h>
#include <wangle/ssl/TLSCredProcessor.h>

FOLLY_GFLAGS_DECLARE_bool(thrift_abort_if_exceeds_shutdown_deadline);
FOLLY_GFLAGS_DECLARE_string(service_identity);

THRIFT_FLAG_DECLARE_bool(dump_snapshot_on_long_shutdown);
THRIFT_FLAG_DECLARE_bool(server_check_unimplemented_extra_interfaces);
THRIFT_FLAG_DECLARE_bool(enable_io_queue_lag_detection);
THRIFT_FLAG_DECLARE_bool(enforce_queue_concurrency_resource_pools);

namespace apache {
namespace thrift {

// Forward declaration of classes
class Cpp2Connection;
class Cpp2Worker;
class ThriftServer;
class ThriftProcessor;
class ThriftQuicServer;
namespace rocket {
class ThriftRocketServerHandler;
}

enum class SSLPolicy { DISABLED, PERMITTED, REQUIRED };

typedef wangle::Pipeline<folly::IOBufQueue&, std::unique_ptr<folly::IOBuf>>
    Pipeline;

class ThriftTlsConfig : public wangle::CustomConfig {
 public:
  bool enableThriftParamsNegotiation{true};
  bool enableStopTLS{false};
};

class TLSCredentialWatcher {
 public:
  explicit TLSCredentialWatcher(ThriftServer* server);

  void setCertPathsToWatch(std::set<std::string> paths) {
    credProcessor_.setCertPathsToWatch(std::move(paths));
  }

  void setTicketPathToWatch(const std::string& path) {
    credProcessor_.setTicketPathToWatch(path);
  }

 private:
  wangle::TLSCredProcessor credProcessor_;
};

/**
 * State pertaining to stopping a running Thrift server. It is safe to call
 * stop() on this even after the relevant ThriftServer has been destroyed as
 * long as the server's event base is not re-used for something else. This is
 * useful to prevent racing between requests to stop (such as from signal
 * handlers) and ThriftServer's destructor.
 *
 * This class cannot be directly constructed by user code. Instead every
 * ThriftServer owns a stop controller (using folly::PrimaryPtr) and hands out
 * non-owning references to use (folly::PrimaryPtrRef). ThriftServer's
 * destructor will block until all locked references have been released.
 *
 * The user-facing API that makes use of this is ServiceHandler::shutdownServer.
 */
class ThriftServerStopController final {
 public:
  explicit ThriftServerStopController(
      folly::badge<ThriftServer>, folly::EventBase& eventBase)
      : serveEventBase_(eventBase) {}

  void stop();

 private:
  folly::EventBase& serveEventBase_;
  folly::once_flag stopped_;
};

/**
 *   This is yet another thrift server.
 *   Uses cpp2 style generated code.
 */

class ThriftServer : public apache::thrift::BaseThriftServer,
                     public wangle::ServerBootstrap<Pipeline> {
 private:
  void configureIOUring();

  //! SSL context
  std::optional<folly::observer::Observer<wangle::SSLContextConfig>>
      sslContextObserver_;
  std::optional<wangle::TLSTicketKeySeeds> ticketSeeds_;

  std::optional<bool> reusePort_;
  std::optional<bool> enableTFO_;
  uint32_t fastOpenQueueSize_{10000};

  std::optional<wangle::SSLCacheOptions> sslCacheOptions_;
  wangle::FizzConfig fizzConfig_;
  ThriftTlsConfig thriftTlsConfig_;

  // Security negotiation settings
  SSLPolicy sslPolicy_{SSLPolicy::REQUIRED};
  bool strictSSL_ = false;
  // whether we allow plaintext connections from loopback in REQUIRED mode
  bool allowPlaintextOnLoopback_ = false;

  // If true, then falls back to the corresponding THRIFT_FLAG.
  // If false, then the check is bypassed even if the THRIFT_FLAG is set.
  // This allows a hard-coded opt-out of the check for services where it would
  // not be useful, e.g. non-C++ languages.
  bool allowCheckUnimplementedExtraInterfaces_ = true;

  bool preferIoUring_ = false;
  bool useDefaultIoUringExecutor_ = false;
  bool usingIoUring_ = false;

  std::weak_ptr<folly::ShutdownSocketSet> wShutdownSocketSet_;

  //! Listen socket
  folly::AsyncServerSocket::UniquePtr socket_;

  // evb->worker eventbase local
  folly::EventBaseLocal<Cpp2Worker*> evbToWorker_;

  struct IdleServerAction : public folly::HHWheelTimer::Callback {
    IdleServerAction(
        ThriftServer& server,
        folly::HHWheelTimer& timer,
        std::chrono::milliseconds timeout);

    void timeoutExpired() noexcept override;

    ThriftServer& server_;
    folly::HHWheelTimer& timer_;
    std::chrono::milliseconds timeout_;
  };

  //! The folly::EventBase currently driving serve().  NULL when not serving.
  std::atomic<folly::EventBase*> serveEventBase_{nullptr};
  std::optional<IdleServerAction> idleServer_;
  std::chrono::milliseconds idleServerTimeout_ = std::chrono::milliseconds(0);
  std::optional<std::chrono::milliseconds> sslHandshakeTimeout_;
  std::atomic<std::chrono::steady_clock::duration::rep> lastRequestTime_;
  // Token bucket used to load shed requests when we've exceeded `getMaxQps()`.
  folly::DynamicTokenBucket qpsTokenBucket_;

  // Includes non-request events in Rocket. Only bumped if idleTimeout set.
  std::chrono::steady_clock::time_point lastRequestTime() const noexcept;
  void touchRequestTimestamp() noexcept;

  //! Manager of per-thread EventBase objects.
  folly::EventBaseManager* eventBaseManager_ = folly::EventBaseManager::get();

  // Creates the default ThriftIO IOThreadPoolExecutor
  static std::shared_ptr<folly::IOThreadPoolExecutorBase> createIOThreadPool();

  //! IO thread pool. Drives Cpp2Workers.
  std::shared_ptr<folly::IOThreadPoolExecutorBase> ioThreadPool_ =
      createIOThreadPool();

  /**
   * The speed for adjusting connection accept rate.
   * 0 for disabling auto adjusting connection accept rate.
   */
  double acceptRateAdjustSpeed_ = 0.0;

  /**
   * Acceptors accept and process incoming connections.  The acceptor factory
   * helps create acceptors.
   */
  std::shared_ptr<wangle::AcceptorFactory> acceptorFactory_;
  std::shared_ptr<wangle::SharedSSLContextManager> sharedSSLContextManager_;
  class ConnectionEventCallback;
  std::unique_ptr<ConnectionEventCallback> connEventCallback_;

  void handleSetupFailure(void);

  void updateCertsToWatch();

  bool stopWorkersOnStopListening_ = true;
  bool joinRequestsWhenServerStops_{true};

  folly::AsyncWriter::ZeroCopyEnableFunc zeroCopyEnableFunc_;

  folly::AsyncServerSocket::CallbackAssignFunction callbackAssignFunc_;

  std::shared_ptr<folly::IOThreadPoolExecutorBase> acceptPool_;
  int nAcceptors_ = 1;
  uint16_t socketMaxReadsPerEvent_{16};

  mutable std::mutex ioGroupMutex_;

  std::shared_ptr<folly::IOThreadPoolExecutorBase> getIOGroupSafe() const {
    std::lock_guard<std::mutex> lock(ioGroupMutex_);
    return getIOGroup();
  }

  void stopWorkers();
  void stopCPUWorkers();
  void stopAcceptingAndJoinOutstandingRequests();

  void callOnStartServing();
  void callOnStopRequested();

  void ensureProcessedServiceDescriptionInitialized();

  bool serverRanWithDCHECK();

  // hook inheriting classes can use to start any additional servers
  virtual void startAdditionalServers() {}

#if FOLLY_HAS_COROUTINES
  std::unique_ptr<folly::coro::CancellableAsyncScope> asyncScope_;
#endif

  folly::Synchronized<std::optional<TLSCredentialWatcher>> tlsCredWatcher_{};

  std::unique_ptr<ThriftProcessor> thriftProcessor_;
  std::vector<std::unique_ptr<TransportRoutingHandler>> routingHandlers_;

  friend class Cpp2Connection;
  friend class Cpp2Worker;
  friend class rocket::ThriftRocketServerHandler;
  friend class ThriftQuicServer;

  bool tosReflect_{false};
  uint32_t listenerTos_{0};

  std::optional<instrumentation::ServerTracker> tracker_;

  bool quickExitOnShutdownTimeout_ = false;

  bool setupThreadManagerCalled_ = false;

 protected:
  folly::observer::CallbackHandle getSSLCallbackHandle();
  folly::observer::CallbackHandle setMaxRequestsCallbackHandle{};
  folly::observer::CallbackHandle setMaxQpsCallbackHandle{};

 public:
  /**
   * The goal of this enum is to capture every state the server goes through in
   * its lifecycle. Notice how the lifecycle is actually a cycle - after the
   * server stops, it returns to its initial state of NOT_RUNNING.
   *
   * NOTE: For the restrictions regarding only allowing internal methods - these
   * do not apply if getRejectRequestsUntilStarted() is false.
   */
  enum class ServerStatus {
    /**
     * The server is not running. Either:
     *   1. The server was never started. Or,
     *   2. The server was stopped and there are outstanding requests
     *      were drained.
     */
    NOT_RUNNING = 0,
    /**
     * The server is about to start and is executing
     * TServerEventHandler::preStart hooks. If getRejectRequestsUntilStarted()
     * is true, the server only responds to internal methods. See
     * ServerConfigs::getInternalMethods.
     */
    PRE_STARTING,
    /**
     * The preStart hooks are done executing and
     * ServiceHandler::semifuture_onStartServing hooks are executing. If
     * getRejectRequestsUntilStarted() is true, the server only responds to
     * internal methods.
     */
    STARTING,
    /**
     * The service is healthy and ready to handle traffic.
     */
    RUNNING,
    /**
     * The server is preparing to stop and
     * ServiceHandler::semifuture_onStopRequested hooks are still executing.
     */
    PRE_STOPPING,
    /**
     * The server is about to stop and no new connections are accepted.
     * Existing connections are unaffected.
     * ServiceHandler::semifuture_onStopRequested hooks have finished executing.
     */
    STOPPING,
    /**
     * Outstanding requests are being joined. New requests are rejected.
     */
    DRAINING_UNTIL_STOPPED,
  };

  ServerStatus getServerStatus() const {
    auto status = internalStatus_.load(std::memory_order_acquire);
    if (status == ServerStatus::RUNNING && !getEnabled()) {
      // Even if the server is capable of serving, the user might have
      // explicitly disabled the service at startup, in which case the server
      // only responds to internal methods.
      return ServerStatus::STARTING;
    }
    return status;
  }

#if FOLLY_HAS_COROUTINES
  using ServiceHealth = PolledServiceHealth::ServiceHealth;

  std::optional<ServiceHealth> getServiceHealth() const {
    auto health = cachedServiceHealth_.load(std::memory_order_relaxed);
    return health == ServiceHealth{} ? std::nullopt
                                     : std::make_optional(health);
  }
#endif

  RequestHandlingCapability shouldHandleRequests() const override {
    auto status = getServerStatus();
    switch (status) {
      case ServerStatus::RUNNING:
        return RequestHandlingCapability::ALL;
      case ServerStatus::NOT_RUNNING:
        // The server can be in the NOT_RUNNING state and still have open
        // connections, for example, if useExistingSocket is called with a
        // socket that is already listening.
        [[fallthrough]];
      case ServerStatus::PRE_STARTING:
      case ServerStatus::STARTING:
        return getRejectRequestsUntilStarted()
            ? RequestHandlingCapability::INTERNAL_METHODS_ONLY
            : RequestHandlingCapability::ALL;
      case ServerStatus::PRE_STOPPING:
      case ServerStatus::STOPPING:
        // When the server is stopping, we close the sockets for new
        // connections. However, existing connections should be unaffected.
        return RequestHandlingCapability::ALL;
      case ServerStatus::DRAINING_UNTIL_STOPPED:
      default:
        return RequestHandlingCapability::NONE;
    }
  }

 private:
  /**
   * Thrift server's view of the currently running service. This status
   * represents the source of truth for the status reported by the server.
   */
  std::atomic<ServerStatus> internalStatus_{ServerStatus::NOT_RUNNING};
#if FOLLY_HAS_COROUTINES
  /**
   * Thrift server's latest view of the running service's reported health.
   */
  std::atomic<ServiceHealth> cachedServiceHealth_{};
#endif

  struct ProcessedServiceDescription {
    ProcessedModuleSet modules;
    std::unique_ptr<AsyncProcessorFactory> decoratedProcessorFactory;
  };
  std::unique_ptr<ProcessedServiceDescription> processedServiceDescription_;

  /**
   * Collects service handlers of the current service of a specific type.
   */
  template <
      typename TServiceHandler = ServiceHandlerBase,
      typename = std::enable_if_t<
          std::is_base_of_v<ServiceHandlerBase, TServiceHandler>>>
  std::vector<TServiceHandler*> collectServiceHandlers() const {
    if constexpr (std::is_same_v<TServiceHandler, ServiceHandlerBase>) {
      return getDecoratedProcessorFactory().getServiceHandlers();
    }
    std::vector<TServiceHandler*> matchedServiceHandlers;
    for (auto* serviceHandler :
         getDecoratedProcessorFactory().getServiceHandlers()) {
      if (auto matched = dynamic_cast<TServiceHandler*>(serviceHandler)) {
        matchedServiceHandlers.push_back(matched);
      }
    }
    return matchedServiceHandlers;
  }

 public:
  ThriftServer();

  explicit ThriftServer(const ThriftServerInitialConfig& initialConfig);

 private:
  // method to encapsulate the default setup needed for the construction of
  // ThriftServer. Should be called in all ctors not calling the default ctor
  void initializeDefaults();

  std::unique_ptr<RequestPileInterface> makeStandardRequestPile(
      RoundRobinRequestPile::Options options);

 public:
  ~ThriftServer() override;

  /**
   * Set the thread pool used to drive the server's IO threads. Note that the
   * pool's thread factory will be overridden - if you'd like to use your own,
   * set it afterwards via ThriftServer::setIOThreadFactory(). If the given
   * thread pool has one or more allocated threads, the number of workers will
   * be set to this number. Use ThreadServer::setNumIOWorkerThreads() to set
   * it afterwards if you want to change the number of works.
   *
   * @param the new thread pool
   */
  void setIOThreadPool(
      std::shared_ptr<folly::IOThreadPoolExecutorBase> ioThreadPool) {
    CHECK(configMutable());
    ioThreadPool_ = ioThreadPool;

    if (ioThreadPool_->numThreads() > 0) {
      setNumIOWorkerThreads(ioThreadPool_->numThreads());
    }
  }

  /**
   * Doing any blocking work on this executor will cause the server to
   * stall servicing requests. Be careful about using this executor for anything
   * other than its main purpose.
   */
  std::shared_ptr<folly::IOThreadPoolExecutorBase> getIOThreadPool() {
    return ioThreadPool_;
  }

  /**
   * Set the thread factory that will be used to create the server's IO
   * threads.
   *
   * @param the new thread factory
   */
  void setIOThreadFactory(
      std::shared_ptr<folly::NamedThreadFactory> threadFactory) {
    CHECK(configMutable());
    ioThreadPool_->setThreadFactory(threadFactory);
  }

  /**
   * Add a IOObserverFactory which will be used to construct a
   * ThreadPoolExecutor::Observer and attached to the ioThreadPool_ in setup().
   * The same factory will be used to create an observer instance for each
   * Thrift Server (if there is more than one)
   *
   * NOTE: Must be called before setup() in order to take effect
   */
  using IOObserverFactory =
      folly::Function<std::shared_ptr<folly::ThreadPoolExecutor::Observer>(
          std::string, folly::WorkerProvider*) const>;
  static void addIOThreadPoolObserverFactory(IOObserverFactory factory);

  /**
   * Set the prefix for naming the worker threads. "Cpp2Worker" by default.
   * must be called before serve() for it to take effect
   *
   * @param cpp2WorkerThreadName net thread name prefix
   */
  void setCpp2WorkerThreadName(const std::string& cpp2WorkerThreadName) {
    CHECK(configMutable());
    auto factory = ioThreadPool_->getThreadFactory();
    CHECK(factory);
    auto namedFactory =
        std::dynamic_pointer_cast<folly::NamedThreadFactory>(factory);
    CHECK(namedFactory);
    namedFactory->setNamePrefix(cpp2WorkerThreadName);
  }

  // if overloaded, returns applicable overloaded exception code.
  folly::Optional<server::ServerConfigs::ErrorCodeAndMessage> checkOverload(
      const transport::THeader::StringToStringMap* readHeaders = nullptr,
      const std::string* = nullptr) final;

  // returns descriptive error if application is unable to process request
  PreprocessResult preprocess(
      const server::PreprocessParams& params) const final;

  std::string getLoadInfo(int64_t load) const override;

  /*
   * Use a ZeroCopyEnableFunc to decide when to use zerocopy mode
   * Ex: use zerocopy when the IOBuf chain exceeds a certain thresold
   * setZeroCopyEnableFunc([threshold](const std::unique_ptr<folly::IOBuf>& buf)
   * { return (buf->computeChainDataLength() > threshold);});
   */
  void setZeroCopyEnableFunc(folly::AsyncWriter::ZeroCopyEnableFunc func) {
    zeroCopyEnableFunc_ = std::move(func);
  }

  const folly::AsyncWriter::ZeroCopyEnableFunc& getZeroCopyEnableFunc() const {
    return zeroCopyEnableFunc_;
  }

  void setCallbackAssignFunc(
      folly::AsyncServerSocket::CallbackAssignFunction func) {
    callbackAssignFunc_ = std::move(func);
  }

  const folly::AsyncServerSocket::CallbackAssignFunction&
  getCallbackAssignFunc() const {
    return callbackAssignFunc_;
  }

  void setAcceptExecutor(
      std::shared_ptr<folly::IOThreadPoolExecutorBase> pool) {
    acceptPool_ = pool;
  }

  /**
   * Generally the acceptor should not do any work other than
   * accepting connections, so use this with care.
   */
  std::shared_ptr<folly::IOThreadPoolExecutorBase> getAcceptExecutor() {
    return acceptPool_;
  }

  void setNumAcceptThreads(int numAcceptThreads) {
    CHECK(!acceptPool_);
    nAcceptors_ = numAcceptThreads;
  }

  /**
   * Set the SSLContextConfig on the thrift server.
   */
  void setSSLConfig(std::shared_ptr<wangle::SSLContextConfig> context) {
    CHECK(configMutable());
    if (context) {
      setSSLConfig(folly::observer::makeObserver(
          [context = std::move(context)]() { return *context; }));
    }
    updateCertsToWatch();
  }

  /**
   * Set the SSLContextConfig on the thrift server. Note that the thrift server
   * keeps an observer on the SSLContextConfig. Whenever the SSLContextConfig
   * has an update, the observer callback would reset SSLContextConfig on all
   * acceptors.
   */
  void setSSLConfig(
      folly::observer::Observer<wangle::SSLContextConfig> contextObserver) {
    sslContextObserver_ = folly::observer::makeObserver(
        [observer = std::move(contextObserver),
         tlsRevocationObserver = enableTLSCertRevocation(),
         tlsRevocationEnforcementObserver = enforceTLSCertRevocation(),
         hybridKexObserver = enableHybridKex(),
         aegisObserver = enableAegis()]() {
          (void)**tlsRevocationObserver;
          (void)**tlsRevocationEnforcementObserver;
          (void)**hybridKexObserver;
          (void)**aegisObserver;
          auto context = **observer;
          context.isDefault = true;
          context.alpnAllowMismatch = false;
          return context;
        });
  }

  void setFizzConfig(wangle::FizzConfig config) { fizzConfig_ = config; }

  const wangle::FizzConfig& getFizzConfig() const { return fizzConfig_; }

  void setThriftConfig(ThriftTlsConfig thriftConfig) {
    thriftTlsConfig_ = thriftConfig;
  }

  void setSSLCacheOptions(wangle::SSLCacheOptions options) {
    sslCacheOptions_ = std::move(options);
  }

  void setTicketSeeds(wangle::TLSTicketKeySeeds seeds) { ticketSeeds_ = seeds; }

  /**
   * Set the ssl handshake timeout.
   */
  void setSSLHandshakeTimeout(
      std::optional<std::chrono::milliseconds> timeout) {
    sslHandshakeTimeout_ = timeout;
  }

  const std::optional<std::chrono::milliseconds>& getSSLHandshakeTimeout()
      const {
    return sslHandshakeTimeout_;
  }

  /**
   * Stops the Thrift server if it's idle for the given time.
   */
  void setIdleServerTimeout(std::chrono::milliseconds timeout) {
    idleServerTimeout_ = timeout;
  }

  /**
   * Configures maxReadsPerEvent for accepted connections, see
   * `folly::AsyncSocket::setMaxReadsPerEvent` for more details.
   */
  void setSocketMaxReadsPerEvent(uint16_t socketMaxReadsPerEvent) {
    socketMaxReadsPerEvent_ = socketMaxReadsPerEvent;
  }

  void updateTicketSeeds(wangle::TLSTicketKeySeeds seeds);

  void updateTLSCert();

  void setPreferIoUring(bool b) { preferIoUring_ = b; }

  bool preferIoUring() const { return preferIoUring_; }

  bool usingIoUring() const { return usingIoUring_; }

  void setUseDefaultIoUringExecutor(bool b) { useDefaultIoUringExecutor_ = b; }

  bool useDefaultIoUringExecutor() const { return useDefaultIoUringExecutor_; }

  /**
   * Tells the thrift server to update ticket seeds with the contents of the
   * file ticketPath when modified and initialized the seeds with the contents
   * of the file ticketPath. The seed file previously being watched will no
   * longer be watched.  This is not thread safe.
   */
  void watchTicketPathForChanges(const std::string& ticketPath);

  void setFastOpenOptions(bool enableTFO, uint32_t fastOpenQueueSize) {
    enableTFO_ = enableTFO;
    fastOpenQueueSize_ = fastOpenQueueSize;
  }

  std::optional<bool> getTFOEnabled() { return enableTFO_; }

  void setReusePort(bool reusePort) { reusePort_ = reusePort; }

  std::optional<bool> getReusePort() { return reusePort_; }

  const std::optional<folly::observer::Observer<wangle::SSLContextConfig>>&
  getSSLConfig() const {
    return sslContextObserver_;
  }

  const std::optional<wangle::TLSTicketKeySeeds>& getTicketSeeds() const {
    return ticketSeeds_;
  }

  const std::optional<wangle::SSLCacheOptions>& getSSLCacheOptions() const {
    return sslCacheOptions_;
  }

  wangle::ServerSocketConfig getServerSocketConfig() {
    wangle::ServerSocketConfig config;
    if (sslContextObserver_.has_value()) {
      config.sslContextConfigs.push_back(*sslContextObserver_->getSnapshot());
    }
    if (sslCacheOptions_) {
      config.sslCacheOptions = *sslCacheOptions_;
    }
    config.connectionIdleTimeout = getIdleTimeout();
    config.connectionAgeTimeout = getConnectionAgeTimeout();
    config.acceptBacklog = getListenBacklog();
    if (ticketSeeds_) {
      config.initialTicketSeeds = *ticketSeeds_;
    }
    if (enableTFO_) {
      config.enableTCPFastOpen = *enableTFO_;
      config.fastOpenQueueSize = fastOpenQueueSize_;
    }
    if (sslHandshakeTimeout_) {
      config.sslHandshakeTimeout = *sslHandshakeTimeout_;
    } else if (getIdleTimeout() == std::chrono::milliseconds::zero()) {
      // make sure a handshake that takes too long doesn't kill the connection
      config.sslHandshakeTimeout = std::chrono::milliseconds::zero();
    }
    // By default, we set strictSSL to false. This means the server will start
    // even if cert/key is missing as it may become available later
    config.strictSSL = getStrictSSL() || getSSLPolicy() == SSLPolicy::REQUIRED;
    config.fizzConfig = fizzConfig_;
    config.customConfigMap["thrift_tls_config"] =
        std::make_shared<ThriftTlsConfig>(thriftTlsConfig_);
    config.socketMaxReadsPerEvent = socketMaxReadsPerEvent_;

    config.useZeroCopy = !!zeroCopyEnableFunc_;
    config.preferIoUring = preferIoUring_;
    return config;
  }

  /**
   * Use the provided socket rather than binding to address_.  The caller must
   * call ::bind on this socket, but should not call ::listen.
   *
   * NOTE: ThriftServer takes ownership of this 'socket' so if binding fails
   *       we destroy this socket, while cleaning itself up. So, 'accept' better
   *       work the first time :)
   */
  void useExistingSocket(int socket);
  void useExistingSockets(const std::vector<int>& sockets);
  void useExistingSocket(folly::AsyncServerSocket::UniquePtr socket);

  /**
   * Return the file descriptor(s) associated with the listening socket
   */
  int getListenSocket() const;
  std::vector<int> getListenSockets() const;

  /**
   * Get the ThriftServer's main event base.
   *
   * @return a pointer to the EventBase.
   */
  folly::EventBase* getServeEventBase() const { return serveEventBase_; }

  /**
   * Get the EventBaseManager used by this server.  This can be used to find
   * or create the EventBase associated with any given thread, including any
   * new threads created by clients.  This may be called from any thread.
   *
   * @return a pointer to the EventBaseManager.
   */
  folly::EventBaseManager* getEventBaseManager();
  const folly::EventBaseManager* getEventBaseManager() const {
    return const_cast<ThriftServer*>(this)->getEventBaseManager();
  }

  SSLPolicy getSSLPolicy() const { return sslPolicy_; }

  void setSSLPolicy(SSLPolicy policy) { sslPolicy_ = policy; }

  void setStrictSSL(bool strictSSL) { strictSSL_ = strictSSL; }

  bool getStrictSSL() { return strictSSL_; }

  void setAllowPlaintextOnLoopback(bool allow) {
    allowPlaintextOnLoopback_ = allow;
  }

  bool isPlaintextAllowedOnLoopback() const {
    return allowPlaintextOnLoopback_;
  }

  void setAllowCheckUnimplementedExtraInterfaces(bool allow) {
    allowCheckUnimplementedExtraInterfaces_ = allow;
  }

  bool isCheckUnimplementedExtraInterfacesAllowed() const {
    return allowCheckUnimplementedExtraInterfaces_;
  }

  static folly::observer::Observer<bool> enableStopTLS();

  static folly::observer::Observer<bool> enableTLSCertRevocation();
  static folly::observer::Observer<bool> enforceTLSCertRevocation();

#if FOLLY_HAS_COROUTINES
  /**
   * Get CancellableAsyncScope that will be maintained by the Thrift Server.
   * Cancellation is requested when the server is stopping.
   * Returns nullptr, before server setup and after server stops.
   */
  folly::coro::CancellableAsyncScope* getAsyncScope() {
    return asyncScope_.get();
  }

  /**
   * Get the global CancellableAsyncScope, it is usally the AsyncScope
   * associated with the global server Cancellation is requested when the
   * global server is stopping.
   */
  static folly::coro::CancellableAsyncScope& getGlobalAsyncScope();
#endif

  /**
   * Checks if a global server is set
   */
  static bool isGlobalServerSet();

  static void setGlobalServer(ThriftServer* server);

  void setAcceptorFactory(
      const std::shared_ptr<wangle::AcceptorFactory>& acceptorFactory) {
    acceptorFactory_ = acceptorFactory;
  }

  /**
   * Get the speed of adjusting connection accept rate.
   */
  double getAcceptRateAdjustSpeed() const { return acceptRateAdjustSpeed_; }

  /**
   * Set the speed of adjusting connection accept rate.
   */
  void setAcceptRateAdjustSpeed(double speed) {
    CHECK(configMutable());
    acceptRateAdjustSpeed_ = speed;
  }

  /**
   * Enable/Disable TOS reflection on the server socket
   */
  void setTosReflect(bool enable) { tosReflect_ = enable; }

  /**
   * Get TOS reflection setting for the server socket
   */
  bool getTosReflect() const override { return tosReflect_; }

  /**
   * Set default TOS for listener/accepted connections
   */
  void setListenerTos(uint32_t tos) { listenerTos_ = tos; }

  /**
   * Get default TOS for listener socket
   */
  uint32_t getListenerTos() const override { return listenerTos_; }

  /**
   * Get the number of connections dropped by the AsyncServerSocket
   */
  uint64_t getNumDroppedConnections() const override;

  /**
   * Clear all the workers.
   */
  void clearWorkers() { ioThreadPool_->join(); }

  /**
   * Set whether to stop io workers when stopListening() is called (we do stop
   * them by default).
   */
  void setStopWorkersOnStopListening(bool stopWorkers) {
    CHECK(configMutable());
    stopWorkersOnStopListening_ = stopWorkers;
  }

  /**
   * Get whether to stop io workers when stopListening() is called.
   */
  bool getStopWorkersOnStopListening() const {
    return stopWorkersOnStopListening_;
  }

  /**
   * If stopWorkersOnStopListening is disabled, then enabling
   * leakOutstandingRequestsWhenServerStops permits thriftServer->serve() to
   * return before all outstanding requests are joined.
   */
  void leakOutstandingRequestsWhenServerStops(bool leak) {
    CHECK(configMutable());
    joinRequestsWhenServerStops_ = !leak;
  }

  /**
   * Call this to complete initialization
   */
  void setup();

  /**
   * Create and start the default thread manager unless it already exists.
   */
  void setupThreadManager();

  /**
   * Apply various runtime checks to determine whether we can use resource pools
   * in this service. Returns true if resource pools is permitted by runtime
   * checks.
   */
  bool runtimeResourcePoolsChecks();

  /**
   * Adds resource pools for any priorities not specified in allocated to this
   * server.
   */
  void ensureResourcePoolsDefaultPrioritySetup(
      std::vector<concurrency::PRIORITY> allocated = {concurrency::NORMAL});

  /**
   * Ensure that this Thrift Server has ResourcePools set up. If there is
   * already a non-empty ResourcePoolSet, nothing will be done. Otherwise, the
   * default setup of ResourcePools will be created.
   */
  void ensureResourcePools();

  /**
   * Kill the workers and wait for listeners to quit
   */
  void cleanUp();

  void setQueueTimeout(std::chrono::milliseconds timeout) override final {
    THRIFT_SERVER_EVENT(call.setQueueTimeout).log(*this, [timeout]() {
      return folly::dynamic::object("timeout_ms", timeout.count());
    });
    BaseThriftServer::setQueueTimeout(timeout);
  }

  [[deprecated("Use setPreprocess instead")]] void setIsOverloaded(
      IsOverloadedFunc isOverloaded) override final {
    THRIFT_SERVER_EVENT(call.setIsOverloaded).log(*this);
    BaseThriftServer::setIsOverloaded(std::move(isOverloaded));
  }

  void setPreprocess(PreprocessFunc preprocess) override final {
    THRIFT_SERVER_EVENT(call.setPreprocess).log(*this);
    BaseThriftServer::setPreprocess(std::move(preprocess));
  }

  /**
   * One stop solution:
   *
   * starts worker threads, enters accept loop; when
   * the accept loop exits, shuts down and joins workers.
   */
  void serve() override;

  /**
   * Call this to stop the server, if started by serve()
   *
   * This (asynchronously) causes the main serve() function to stop listening
   * for new connections, close existing connections, shut down the worker
   * threads, and then return.
   *
   * NOTE: that this function may not be safe to call multiple times (such as
   * from a signal handler) because a previous call may initiate a sequence of
   * events leading to the destruction of this object.
   * Instead you should use StopController (see getStopController()) which lets
   * you guard against destruction (by way of folly::PrimaryPtr).
   */
  void stop() override;

  using StopController = ThriftServerStopController;

 private:
  folly::PrimaryPtr<StopController> stopController_{
      std::unique_ptr<StopController>{}};

 public:
  folly::PrimaryPtrRef<StopController> getStopController() {
    return stopController_.ref();
  }

  /**
   * Call this to stop listening on the server port.
   *
   * This causes the main serve() function to stop listening for new
   * connections while still allows the worker threads to process
   * existing connections. stop() still needs to be called to clear
   * up the worker threads.
   */
  void stopListening() override;

  const std::vector<std::unique_ptr<TransportRoutingHandler>>*
  getRoutingHandlers() const {
    return &routingHandlers_;
  }

  void addRoutingHandler(
      std::unique_ptr<TransportRoutingHandler> routingHandler) {
    routingHandlers_.push_back(std::move(routingHandler));
  }

  void clearRoutingHandlers() { routingHandlers_.clear(); }

  /**
   * Returns a reference to the processor that is used by custom transports
   */
  apache::thrift::ThriftProcessor* getThriftProcessor() {
    return thriftProcessor_.get();
  }

  const std::vector<std::shared_ptr<folly::AsyncServerSocket>> getSockets()
      const {
    std::vector<std::shared_ptr<folly::AsyncServerSocket>> serverSockets;
    for (auto& socket : ServerBootstrap::getSockets()) {
      serverSockets.push_back(
          std::dynamic_pointer_cast<folly::AsyncServerSocket>(socket));
    }
    return serverSockets;
  }

  /**
   * Sets an explicit AsyncProcessorFactory and sets the ThriftProcessor
   * to use for custom transports
   */
  virtual void setProcessorFactory(
      std::shared_ptr<AsyncProcessorFactory> pFac) override;

  /**
   * Returns an AsyncProcessorFactory that wraps the user-provided service and
   * additionally handles Thrift-internal methods as well (such as the
   * monitoring interface).
   *
   * This is the factory that all transports should use to handle requests.
   *
   * Logically, this is an apache::thrift::MultiplexAsyncProcessorFactory with
   * the following composition:
   *
   *    ┌────────────────────────┐
   *    │      User Service      │
   *    │ (setProcessorFactory)  │  │
   *    └────────────────────────┘  │
   *                                │
   *    ┌────────────────────────┐  │
   *    │    Status Interface    │  │
   *    │  (setStatusInterface)  │  │
   *    └────────────────────────┘  │
   *                                │   Method
   *    ┌────────────────────────┐  │ precedence
   *    │  Monitoring Interface  │  │
   *    │(setMonitoringInterface)│  │
   *    └────────────────────────┘  │
   *                                │
   *    ┌────────────────────────┐  │
   *    │   Control Interface    │  ▼
   *    │ (setControlInterface)  │
   *    └────────────────────────┘
   */
  AsyncProcessorFactory& getDecoratedProcessorFactory() const {
    CHECK(processedServiceDescription_)
        << "Server must be set up before calling this method";
    return *processedServiceDescription_->decoratedProcessorFactory;
  }

  /**
   * Returns an AsyncProcessor from getDecoratedProcessorFactory() without any
   * application-specific event handlers installed on the underlying processors.
   * This is useful, for example, in InterfaceKind::MONITORING where
   * application-specific checks (such as ACL checks) should be bypassed.
   */
  std::unique_ptr<AsyncProcessor> getDecoratedProcessorWithoutEventHandlers()
      const;

  /**
   * Gets TProcessorEventHandler instances that are scoped to this ThriftServer
   * instance. These include a snapshot of the set of globally registered
   * TProcessorEventHandlers at the moment the server started running.
   */
  const std::vector<std::shared_ptr<TProcessorEventHandler>>&
  getLegacyEventHandlers() const override {
    CHECK(processedServiceDescription_)
        << "Server must be set up before calling this method";
    return processedServiceDescription_->modules.coalescedLegacyEventHandlers;
  }

  /**
   * A struct containing all "extra" internal interfaces that the service
   * multiplexes behind the main user-defined interface.
   *
   * See ThriftServer::getDecoratedProcessorFactory.
   */
  struct ExtraInterfaces {
    // See ThriftServer::setMonitoringInterface.
    std::shared_ptr<MonitoringServerInterface> monitoring;
    // See ThriftServer::setStatusInterface.
    std::shared_ptr<StatusServerInterface> status;
    // See ThriftServer::setControlInterface
    std::shared_ptr<ControlServerInterface> control;
    // See ThriftServer::setSecurityInterface
    std::shared_ptr<SecurityServerInterface> security;
  };

  // ThriftServer by defaults uses a global ShutdownSocketSet, so all socket's
  // FDs are registered there. But in some tests you might want to simulate 2
  // ThriftServer running in different processes, so their ShutdownSocketSet are
  // different. In that case server should have their own SSS each so shutting
  // down FD from one doesn't interfere with shutting down sockets for the
  // other.
  void replaceShutdownSocketSet(
      const std::shared_ptr<folly::ShutdownSocketSet>& newSSS);

  static folly::observer::Observer<std::list<std::string>>
  defaultNextProtocols();

  bool getQuickExitOnShutdownTimeout() const {
    return quickExitOnShutdownTimeout_;
  }

  void setQuickExitOnShutdownTimeout(bool quickExitOnShutdownTimeout) {
    quickExitOnShutdownTimeout_ = quickExitOnShutdownTimeout;
  }

  static folly::observer::Observer<bool> enableHybridKex();

  static folly::observer::Observer<bool> enableAegis();

  /**
   * For each request debug stub, a snapshot information can be constructed to
   * persist some transitent states about the corresponding request.
   */
  class RequestSnapshot {
   public:
    explicit RequestSnapshot(const RequestsRegistry::DebugStub& stub)
        : methodName_(stub.getMethodName()),
          creationTimestamp_(stub.getTimestamp()),
          finishedTimestamp_(stub.getFinished()),
          protoId_(stub.getProtoId()),
          peerAddress_(*stub.getPeerAddress()),
          localAddress_(*stub.getLocalAddress()),
          rootRequestContextId_(stub.getRootRequestContextId()),
          reqId_(RequestsRegistry::getRequestId(rootRequestContextId_)),
          reqDebugLog_(collectRequestDebugLog(stub)) {
      auto requestPayload = rocket::unpack<RequestPayload>(stub.clonePayload());
      payload_ = std::move(*requestPayload->payload);
      auto& metadata = requestPayload->metadata;
      if (metadata.otherMetadata()) {
        headers_ = std::move(*requestPayload->metadata.otherMetadata());
      }
      clientId_ = metadata.clientId().to_optional();
      serviceTraceMeta_ = metadata.serviceTraceMeta().to_optional();
      auto req = stub.getRequest();
      DCHECK(
          req != nullptr || finishedTimestamp_.time_since_epoch().count() != 0);
      startedProcessing_ = req == nullptr ? true : stub.getStartedProcessing();
    }

    const std::string& getMethodName() const { return methodName_; }

    std::chrono::steady_clock::time_point getCreationTimestamp() const {
      return creationTimestamp_;
    }

    std::chrono::steady_clock::time_point getFinishedTimestamp() const {
      return finishedTimestamp_;
    }

    intptr_t getRootRequestContextId() const { return rootRequestContextId_; }

    const std::string& getRequestId() const { return reqId_; }

    bool getStartedProcessing() const { return startedProcessing_; }

    /**
     * Returns empty IOBuff if payload is not present.
     */
    const folly::IOBuf& getPayload() const { return payload_; }

    const transport::THeader::StringToStringMap& getHeaders() const {
      return headers_;
    }

    protocol::PROTOCOL_TYPES getProtoId() const { return protoId_; }

    const folly::SocketAddress& getLocalAddress() const {
      return localAddress_;
    }
    const folly::SocketAddress& getPeerAddress() const { return peerAddress_; }

    const std::vector<std::string>& getDebugLog() const { return reqDebugLog_; }

    const auto& clientId() const { return clientId_; }
    auto& clientId() { return clientId_; }

    const auto& serviceTraceMeta() const { return serviceTraceMeta_; }
    auto& serviceTraceMeta() { return serviceTraceMeta_; }

   private:
    const std::string methodName_;
    const std::chrono::steady_clock::time_point creationTimestamp_;
    const std::chrono::steady_clock::time_point finishedTimestamp_;
    const protocol::PROTOCOL_TYPES protoId_;
    folly::IOBuf payload_;
    transport::THeader::StringToStringMap headers_;
    std::optional<std::string> clientId_;
    std::optional<std::string> serviceTraceMeta_;
    folly::SocketAddress peerAddress_;
    folly::SocketAddress localAddress_;
    intptr_t rootRequestContextId_;
    const std::string reqId_;
    const std::vector<std::string> reqDebugLog_;
    bool startedProcessing_;
  };

  struct ServerIOMemory {
    size_t ingress;
    size_t egress;
  };
  /**
   * Returns structure highlighting the ingress and egress memory usage in
   * thrift server
   */
  folly::SemiFuture<ServerIOMemory> getUsedIOMemory();

  struct ConnectionSnapshot {
    size_t numActiveRequests{0};
    size_t numPendingWrites{0};
    std::chrono::steady_clock::time_point creationTime;
  };
  using RequestSnapshots = std::vector<RequestSnapshot>;
  using ConnectionSnapshots =
      std::unordered_map<folly::SocketAddress, ConnectionSnapshot>;
  struct ServerSnapshot {
    RecentRequestCounter::Values recentCounters;
    RequestSnapshots requests;
    ConnectionSnapshots connections;
    ServerIOMemory memory;
  };
  struct SnapshotOptions {
    std::chrono::microseconds connectionsAgeMax;
  };
  folly::SemiFuture<ServerSnapshot> getServerSnapshot() {
    return getServerSnapshot(SnapshotOptions{});
  }
  folly::SemiFuture<ServerSnapshot> getServerSnapshot(
      const SnapshotOptions& options);

  /**
   * If shutdown does not complete within the configured worker join timeout,
   * then we schedule a task to dump the server's state to disk for
   * investigation.
   *
   * The implementor of the dumping logic should provide the the task as well
   * as an appropriate timeout -- we do not want to indefinitely block shutdown
   * in case the task deadlocks.
   */
  struct DumpSnapshotOnLongShutdownResult {
    folly::SemiFuture<folly::Unit> task;
    std::chrono::milliseconds timeout;
  };

  enum class UnimplementedExtraInterfacesResult {
    /**
     * The method is completely unrecognized by the service.
     */
    UNRECOGNIZED,
    /**
     * Extra interfaces are implemented directly by the service.
     */
    IMPLEMENTED,
    /**
     * Extra interfaces are left unimplemented but recognized by the service.
     */
    UNIMPLEMENTED,
  };

  struct NewConnectionContext {
    std::shared_ptr<AsyncProcessorFactory> processorFactory;
  };

  static folly::Optional<NewConnectionContext> extractNewConnectionContext(
      folly::AsyncTransport& sock);

  void acceptConnection(
      folly::NetworkSocket fd,
      const folly::SocketAddress& clientAddr,
      folly::AsyncServerSocket::AcceptCallback::AcceptInfo info,
      std::shared_ptr<AsyncProcessorFactory> processor);

 private:
  using ServerBootstrap::acceptConnection;
};

template <typename AcceptorClass, typename SharedSSLContextManagerClass>
class ThriftAcceptorFactory : public wangle::AcceptorFactorySharedSSLContext {
 public:
  ThriftAcceptorFactory(ThriftServer* server) : server_(server) {}

  std::shared_ptr<wangle::SharedSSLContextManager>
  initSharedSSLContextManager() {
    if constexpr (!std::is_same<SharedSSLContextManagerClass, void>::value) {
      sharedSSLContextManager_ = std::make_shared<SharedSSLContextManagerClass>(
          server_->getServerSocketConfig());
    }
    return sharedSSLContextManager_;
  }

  std::shared_ptr<wangle::Acceptor> newAcceptor(folly::EventBase* eventBase) {
    if (!sharedSSLContextManager_) {
      return AcceptorClass::create(server_, eventBase);
    }
    auto acceptor = AcceptorClass::create(
        server_,
        eventBase,
        sharedSSLContextManager_->getCertManager(),
        sharedSSLContextManager_->getContextManager(),
        sharedSSLContextManager_->getFizzContext());
    sharedSSLContextManager_->addAcceptor(acceptor);
    return acceptor;
  }

 protected:
  ThriftServer* server_;
};
using DefaultThriftAcceptorFactory = ThriftAcceptorFactory<Cpp2Worker, void>;

using DefaultThriftAcceptorFactorySharedSSLContext = ThriftAcceptorFactory<
    Cpp2Worker,
    wangle::SharedSSLContextManagerImpl<wangle::FizzConfigUtil>>;

namespace detail {

THRIFT_PLUGGABLE_FUNC_DECLARE(
    apache::thrift::ThriftServer::DumpSnapshotOnLongShutdownResult,
    dumpSnapshotOnLongShutdown);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    apache::thrift::ThriftServer::ExtraInterfaces,
    createDefaultExtraInterfaces);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    ThriftServer::UnimplementedExtraInterfacesResult,
    serviceHasUnimplementedExtraInterfaces,
    AsyncProcessorFactory& service);

} // namespace detail

} // namespace thrift
} // namespace apache

#endif // #ifndef THRIFT_SERVER_H_
