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
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include <folly/Memory.h>
#include <folly/Portability.h>
#include <folly/SharedMutex.h>
#include <folly/Singleton.h>
#include <folly/SocketAddress.h>
#include <folly/Synchronized.h>
#include <folly/TokenBucket.h>
#include <folly/concurrency/memory/PrimaryPtr.h>
#include <folly/coro/AsyncScope.h>
#include <folly/dynamic.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/executors/VirtualExecutor.h>
#include <folly/io/ShutdownSocketSet.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseLocal.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/lang/Badge.h>
#include <folly/logging/xlog.h>
#include <folly/observer/Observer.h>
#include <folly/synchronization/CallOnce.h>

#include <fmt/core.h>

#include <thrift/lib/cpp/concurrency/PosixThreadFactory.h>
#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp/server/TServerEventHandler.h>
#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/HeaderServerChannel.h>
#include <thrift/lib/cpp2/server/AdaptiveConcurrency.h>
#include <thrift/lib/cpp2/server/CPUConcurrencyController.h>
#include <thrift/lib/cpp2/server/ControlServerInterface.h>
#include <thrift/lib/cpp2/server/DecoratorDataRuntime.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/server/MonitoringServerInterface.h>
#include <thrift/lib/cpp2/server/PolledServiceHealth.h>
#include <thrift/lib/cpp2/server/PreprocessFunctions.h>
#include <thrift/lib/cpp2/server/RequestDebugLog.h>
#include <thrift/lib/cpp2/server/RequestsRegistry.h>
#include <thrift/lib/cpp2/server/ResourcePool.h>
#include <thrift/lib/cpp2/server/ResourcePoolSet.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/SecurityServerInterface.h>
#include <thrift/lib/cpp2/server/ServerAttribute.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>
#include <thrift/lib/cpp2/server/ServerInstrumentation.h>
#include <thrift/lib/cpp2/server/ServerModule.h>
#include <thrift/lib/cpp2/server/ServiceHealthPoller.h>
#include <thrift/lib/cpp2/server/StatusServerInterface.h>
#include <thrift/lib/cpp2/server/ThreadManagerLoggingWrapper.h>
#include <thrift/lib/cpp2/server/ThriftServerConfig.h>
#include <thrift/lib/cpp2/server/TransportRoutingHandler.h>
#include <thrift/lib/cpp2/server/metrics/InterceptorMetricCallback.h>
#include <thrift/lib/cpp2/transport/core/ManagedConnectionIf.h>
#include <thrift/lib/cpp2/transport/rocket/RequestPayload.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/AllocatingParserStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_constants.h>
#include <wangle/acceptor/ServerSocketConfig.h>
#include <wangle/acceptor/SharedSSLContextManager.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/ssl/SSLContextConfig.h>
#include <wangle/ssl/TLSCredProcessor.h>
#include <wangle/ssl/TLSInMemoryTicketProcessor.h>

FOLLY_GFLAGS_DECLARE_bool(thrift_abort_if_exceeds_shutdown_deadline);
FOLLY_GFLAGS_DECLARE_string(service_identity);
FOLLY_GFLAGS_DECLARE_bool(disable_legacy_header_routing_handler);

THRIFT_FLAG_DECLARE_bool(dump_snapshot_on_long_shutdown);
THRIFT_FLAG_DECLARE_bool(server_check_unimplemented_extra_interfaces);
THRIFT_FLAG_DECLARE_bool(enable_io_queue_lag_detection);
THRIFT_FLAG_DECLARE_bool(default_sync_max_requests_to_concurrency_limit);
THRIFT_FLAG_DECLARE_bool(default_sync_max_qps_to_execution_rate);

namespace wangle {
class ConnectionManager;
}

namespace apache::thrift {

// Forward declaration of classes
class Cpp2Connection;
class Cpp2Worker;
class ThriftServer;
class ThriftProcessor;
class ThriftQuicServer;
namespace detail {
class ThriftServerInternals;
}
namespace rocket {
class ThriftRocketServerHandler;
class RocketRequestHandler;
} // namespace rocket

enum class SSLPolicy { DISABLED, PERMITTED, REQUIRED };

enum class PSPUpgradePolicy {
  // Server will refuse to negotiate PSP
  DISABLED,

  // Server will always accept PSP upgrade requests. This will result in
  // fatal connection errors if this runs on unsupported hardware.
  ALWAYS,
};

enum class EffectiveTicketSeedStrategy {
  IN_MEMORY,
  IN_MEMORY_WITH_ROTATION,
  FILE
};

using Pipeline =
    wangle::Pipeline<folly::IOBufQueue&, std::unique_ptr<folly::IOBuf>>;

class ThriftTlsConfig : public wangle::CustomConfig {
 public:
  bool enableThriftParamsNegotiation{true};
  bool enableStopTLS{false};
  bool enableStopTLSV2{false};
  folly::Optional<PSPUpgradePolicy> pspUpgradePolicy;
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

  bool hasTicketPathToWatch() const {
    return credProcessor_.hasTicketPathToWatch();
  }

  wangle::TLSTicketKeySeeds initInMemoryTicketSeeds(ThriftServer* server);

 private:
  wangle::TLSCredProcessor credProcessor_;
  std::optional<wangle::TLSInMemoryTicketProcessor> inMemoryTicketProcessor_;
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

using IsOverloadedFunc = folly::Function<bool(
    const transport::THeader::StringToStringMap&, const std::string&) const>;

using GetHeaderHandlerFunc = std::function<void(
    const apache::thrift::transport::THeader*, const folly::SocketAddress*)>;

template <typename T>
class ThriftServerAsyncProcessorFactory : public AsyncProcessorFactory {
 public:
  explicit ThriftServerAsyncProcessorFactory(std::shared_ptr<T> t) : svIf_(t) {}

  std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor() override {
    return std::unique_ptr<apache::thrift::AsyncProcessor>(
        new typename T::ProcessorType(svIf_.get()));
  }

  CreateMethodMetadataResult createMethodMetadata() override {
    return svIf_->T::createMethodMetadata();
  }

  std::vector<ServiceHandlerBase*> getServiceHandlers() override {
    return {svIf_.get()};
  }

 private:
  std::shared_ptr<T> svIf_;
};

class ThriftServer;
namespace detail {
/**
 * Gets the server's ThriftServerConfig which contains all the static and
 * dynamic Server Attributes
 */
ThriftServerConfig& getThriftServerConfig(ThriftServer&);
} // namespace detail

/**
 *   This is yet another thrift server.
 *   Uses cpp2 style generated code.
 */

class ThriftServer : public apache::thrift::concurrency::Runnable,
                     public apache::thrift::server::ServerConfigs,
                     public wangle::ServerBootstrap<Pipeline> {
 public:
  struct Metadata {
    std::string configPath;
    std::optional<std::string> serviceFramework;
    std::optional<std::string> serviceFrameworkName;
    std::optional<std::string> wrapper;
    std::optional<std::string> languageFramework;
    std::optional<std::set<std::string>> modules;
    std::optional<std::string> tlsConfigSource;
    std::optional<std::string> serverSourceLocation;

    void addModule(std::string_view name) {
      if (!modules) {
        modules.emplace();
      }

      modules->emplace(name);
    }
  };

  /**
   * Behavior when a thrift client calls server with legacy transport, ie,
   * header. Default behavior is determined by thrift flags
   */
  enum class LegacyTransport : int {
    DEFAULT = 0, // Use thrift flags(server_header_reject_all) to decide whether
                 // to reject or allow header traffic
    DISABLED = 1, // Always reject header traffic
    ALLOWED = 2, // Always allow header traffic
  };

  /**
   * The type of thread manager to create for the server.
   */
  enum class ThreadManagerType : int {
    PRIORITY = 0, //! Use a PriorityThreadManager
    SIMPLE = 1, //! Use a SimpleThreadManager
    PRIORITY_QUEUE = 2, //! Use a PriorityQueueThreadManager
    EXECUTOR_ADAPTER = 3, //! Use ThreadManagerExecutorAdapter
    EXECUTOR = 3 //! Another way to say EXECUTOR_ADAPTER
    // In resource pools mode the executor is used directly, it is only wrapped
    // in the ThreadManagerExecutorAdapter when in ThreadManager mode
  };

  struct RuntimeServerActions {
    bool userSuppliedThreadManager{false};
    bool userSuppliedResourcePools{false};
    bool interactionInService{false};
    bool wildcardMethods{false};
    bool noServiceRequestInfo{false};
    bool activeRequestTrackingDisabled{false};
    bool setPreprocess{false};
    bool setIsOverloaded{false};
    bool resourcePoolFlagSet{false};
    bool codelEnabled{false};
    bool setupThreadManagerBeforeHandler{false};
    std::string executorToThreadManagerUnexpectedFunctionName{};

    bool resourcePoolEnablementLocked{false};
    bool resourcePoolRuntimeRequested{false};
    bool resourcePoolRuntimeDisabled{false};
    std::string resourcePoolRuntimeDisabledReason{};
    bool resourcePoolEnabled{false};

    bool resourcePoolEnabledGflag{false};
    bool resourcePoolDisabledGflag{false};

    bool checkComplete{false};
    bool isProcessorFactoryThriftGenerated{false};

    bool moduleListFinalized{false};

    bool setupThreadManagerCalledByUser{false};
    bool runtimeResourcePoolsChecksCalledByUser{false};

    std::vector<std::pair<std::string, std::string>> toStringPairs() const;
    std::string explain() const;
  };

  /**
   * Set the address(es) to listen on.
   */
  void setAddress(const folly::SocketAddress& address) {
    setAddresses({address});
  }

  void setAddress(folly::SocketAddress&& address) {
    setAddresses({std::move(address)});
  }

  void setAddress(const char* ip, uint16_t port) {
    setAddresses({folly::SocketAddress(ip, port)});
  }

  void setAddress(const std::string& ip, uint16_t port) {
    setAddresses({folly::SocketAddress(ip, port)});
  }

  void setAddresses(std::vector<folly::SocketAddress> addresses) {
    CHECK(!addresses.empty());
    CHECK(configMutable());
    port_.reset();
    addresses_ = std::move(addresses);
  }

  /**
   * Get the address the server is listening on.
   *
   * This should generally only be called after setup() has finished.
   *
   * (The address may be uninitialized until setup() has run.  If called from
   * another thread besides the main server thread, the caller is responsible
   * for providing their own synchronization to ensure that setup() is not
   * modifying the address while they are using it.)
   */
  const folly::SocketAddress& getAddress() const { return addresses_.at(0); }

  const std::vector<folly::SocketAddress>& getAddresses() const {
    return addresses_;
  }

  const std::string getAddressAsString() const {
    return getAddress().isInitialized() ? getAddress().describe()
                                        : std::to_string(port_.value_or(0));
  }

  /**
   * Set the port to listen on.
   */
  void setPort(uint16_t port) {
    CHECK(configMutable());
    port_ = port;
    addresses_.at(0).reset();
  }

  bool isPortSet() {
    if (!getAddress().isInitialized()) {
      return port_.has_value();
    }
    return true;
  }

  /**
   * Get the port.
   */
  uint16_t getPort() {
    auto addr = getAddress();
    if (!addr.isInitialized()) {
      return port_.value_or(0);
    }
    return addr.isFamilyInet() ? addr.getPort() : 0;
  }

  std::shared_ptr<server::TServerEventHandler> getEventHandler() const {
    return eventHandler_;
  }

  /**
   * If a view of the event handlers is needed that does not need to extend
   * their lifetime beyond that of the ThriftServer, this method allows
   * obtaining the raw pointer rather than the more expensive shared_ptr. Since
   * unsynchronized setServerEventHandler / addServerEventHandler /
   * getEventHandler calls are not permitted, use cases that get the handler,
   * inform it of some action, and then discard the handle immediately can use
   * getEventHandlersUnsafe.
   */
  const std::vector<std::shared_ptr<server::TServerEventHandler>>&
  getEventHandlersUnsafe() const {
    return eventHandlers_;
  }

  /**
   * DEPRECATED! Please use addServerEventHandler instead.
   */
  void setServerEventHandler(
      std::shared_ptr<server::TServerEventHandler> eventHandler) {
    if (eventHandler_) {
      eventHandlers_.erase(
          std::find(
              eventHandlers_.begin(), eventHandlers_.end(), eventHandler_));
    }
    eventHandler_ = std::move(eventHandler);
    if (eventHandler_) {
      eventHandlers_.push_back(eventHandler_);
    }
  }

  void addServerEventHandler(
      std::shared_ptr<server::TServerEventHandler> eventHandler) {
    eventHandlers_.push_back(eventHandler);
  }

  void removeServerEventHandler(
      std::shared_ptr<server::TServerEventHandler> eventHandler) {
    eventHandlers_.erase(
        std::remove(eventHandlers_.begin(), eventHandlers_.end(), eventHandler),
        eventHandlers_.end());
  }

  /**
   * Sets the main server interface that exposes user-defined methods.
   */
  void setInterface(std::shared_ptr<AsyncProcessorFactory> iface);

  const std::shared_ptr<apache::thrift::AsyncProcessorFactory>&
  getProcessorFactory() const {
    return cpp2Pfac_;
  }

  concurrency::ThreadManager::ExecutionScope getRequestExecutionScope(
      Cpp2RequestContext* ctx, concurrency::PRIORITY defaultPriority) override {
    if (applicationServerInterface_) {
      return applicationServerInterface_->getRequestExecutionScope(
          ctx, defaultPriority);
    }
    return ServerConfigs::getRequestExecutionScope(ctx, defaultPriority);
  }

  /**
   * Sets the interface that will be used for monitoring connections only.
   */
  void setMonitoringInterface(
      std::shared_ptr<MonitoringServerInterface> iface) {
    CHECK(configMutable());
    monitoringServiceHandler_ = std::move(iface);
  }

  const std::shared_ptr<MonitoringServerInterface>& getMonitoringInterface()
      const {
    return monitoringServiceHandler_;
  }

  /**
   * Sets the interface that will be used for status RPCs only.
   */
  void setStatusInterface(std::shared_ptr<StatusServerInterface> iface) {
    CHECK(configMutable());
    statusServiceHandler_ = std::move(iface);
  }

  const std::shared_ptr<StatusServerInterface>& getStatusInterface() {
    return statusServiceHandler_;
  }

  /**
   * Sets the interface that will be used for control RPCs only.
   */
  void setControlInterface(std::shared_ptr<ControlServerInterface> iface) {
    CHECK(configMutable());
    controlServiceHandler_ = std::move(iface);
  }

  const std::shared_ptr<ControlServerInterface>& getControlInterface() const {
    return controlServiceHandler_;
  }

  /**
   * Sets the interface that will be used for security RPCs only.
   */
  void setSecurityInterface(std::shared_ptr<SecurityServerInterface> iface) {
    CHECK(configMutable());
    securityServiceHandler_ = std::move(iface);
  }

  const std::shared_ptr<SecurityServerInterface>& getSecurityInterface() const {
    return securityServiceHandler_;
  }

  void setGetHeaderHandler(GetHeaderHandlerFunc func) {
    getHeaderHandler_ = func;
  }

  GetHeaderHandlerFunc getGetHeaderHandler() { return getHeaderHandler_; }

  // Get load of the server.
  int64_t getLoad(
      const std::string& counter = "", bool check_custom = true) const final;

  void setGetLoad(std::function<int64_t(const std::string&)> getLoad) {
    getLoad_ = getLoad;
  }

  std::function<int64_t(const std::string&)> getGetLoad() const {
    return getLoad_;
  }

  std::string getLoadInfo(int64_t load) const;

  /**
  -----------------------------------------------------------------
  |                      RESOURCE POOLS BEGIN                     |
  -----------------------------------------------------------------
   */
 public:
  /**
   * Apply various runtime checks to determine whether we can use resource pools
   * in this service. Returns true if resource pools is permitted by runtime
   * checks.
   */
  bool runtimeResourcePoolsChecks();

 private:
  bool runtimeResourcePoolsChecksImpl();

 public:
  /**
   * Ensure that this Thrift Server has ResourcePools set up. If there is
   * already a non-empty ResourcePoolSet, nothing will be done. Otherwise, the
   * default setup of ResourcePools will be created.
   */
  void ensureResourcePools();

  bool resourcePoolEnabled() const override {
    return getRuntimeServerActions().resourcePoolEnabled;
  }

  /**
   * Returns debug information regarding ResourcePool setup on this server.
   **/
  serverdbginfo::ResourcePoolsDbgInfo getResourcePoolsDbgInfo() const;

  /**
   * Get the ResourcePoolSet used by this ThriftServer. There is always one, but
   * it may be empty if ResourcePools are not in use.
   */
  const ResourcePoolSet& resourcePoolSet() const override {
    return resourcePoolSet_;
  }

  /**
   * Get the ResourcePoolSet used by this ThriftServer. There is always one, but
   * it may be empty if ResourcePools are not in use.
   */
  ResourcePoolSet& resourcePoolSet() override { return resourcePoolSet_; }

  // Used to disable resource pool at run time. This
  // should only be used by thrift team.
  void runtimeDisableResourcePoolsDeprecated(
      const std::string& reason = "Unknown") {
    if (runtimeServerActions_.resourcePoolRuntimeDisabled) {
      return;
    }
    if (runtimeServerActions_.resourcePoolEnablementLocked &&
        runtimeServerActions_.resourcePoolEnabled) {
      LOG(FATAL)
          << "Trying to disable ResourcePool after it's locked (enabled)";
      return;
    }
    runtimeServerActions_.resourcePoolRuntimeDisabled = true;
    runtimeServerActions_.resourcePoolRuntimeDisabledReason = reason;
  }

  bool runtimeDisableResourcePoolsSet() {
    return runtimeServerActions_.resourcePoolRuntimeDisabled;
  }

  // Used to enable resource pool at run time. If resource
  // pools cannot be enabled (due to run-time conditions) the
  // process will be aborted during server startup.
  void requireResourcePools() {
    if (runtimeServerActions_.resourcePoolRuntimeRequested) {
      return;
    }
    if (runtimeServerActions_.resourcePoolEnablementLocked &&
        !runtimeServerActions_.resourcePoolEnabled) {
      LOG(FATAL)
          << "Trying to enable ResourcePool after it's locked (disabled)";
      return;
    }
    if (FLAGS_thrift_disable_resource_pools) {
      LOG(FATAL)
          << "--thrift_disable_resource_pools flag set and requireResourcePools() called";
      return;
    }
    runtimeServerActions_.resourcePoolRuntimeRequested = true;
  }

  bool useResourcePools() {
    if (!runtimeServerActions_.resourcePoolEnablementLocked) {
      runtimeServerActions_.resourcePoolEnablementLocked = true;
      bool flagSet = useResourcePoolsFlagsSet();
      bool runtimeRequested =
          runtimeServerActions_.resourcePoolRuntimeRequested;
      bool runtimeDisabled = runtimeServerActions_.resourcePoolRuntimeDisabled;
      runtimeServerActions_.resourcePoolEnabled =
          (flagSet || runtimeRequested) && !runtimeDisabled;

      // Enforce requireResourcePools.
      if (runtimeServerActions_.resourcePoolRuntimeRequested &&
          !runtimeServerActions_.resourcePoolEnabled) {
        LOG(FATAL) << "requireResourcePools() failed "
                   << runtimeServerActions_.explain();
      }
    }

    return runtimeServerActions_.resourcePoolEnabled;
  }

 private:
  /**
   * Ensures no further changes can be made to the ResourcePoolSet.
   */
  void lockResourcePoolSet();

  //! The ResourcePoolsSet used by this ThriftServer (if in ResourcePools
  //! are enabled).
  ResourcePoolSet resourcePoolSet_;
  std::optional<std::string> resourcePoolsOptOutExplanation_;
  /**
  -----------------------------------------------------------------
  |                      RESOURCE POOLS END                       |
  -----------------------------------------------------------------
   */

 public:
  /**
   * Set Thread Manager (for queuing mode).
   * If not set, defaults to the number of worker threads.
   * This is meant to be used as an external API
   *
   * @param threadManager a shared pointer to the thread manager
   */
  void setThreadManager(
      std::shared_ptr<apache::thrift::concurrency::ThreadManager>
          threadManager) {
    setThreadManagerInternal(threadManager);
    if (!THRIFT_FLAG(allow_set_thread_manager_resource_pools)) {
      runtimeDisableResourcePoolsDeprecated("setThreadManager");
    }
    runtimeServerActions_.userSuppliedThreadManager = true;
  }

  /**
   * This an equivalent entry point to setThreadManager. During deprecation
   * there will be places where we have to continue to call setThreadManager
   * from user code - where we decide to migrate them by adding resource pools
   * specific paths to the service code. When we do we'll use
   * setThreadManager_deprecated instead of setThreadManager so we can track
   * when all instances of setThreadManager have been removed.
   */
  void setThreadManager_deprecated(
      std::shared_ptr<apache::thrift::concurrency::ThreadManager>
          threadManager) {
    setThreadManagerInternal(threadManager);
    runtimeDisableResourcePoolsDeprecated("setThreadManager_deprecated");
    runtimeServerActions_.userSuppliedThreadManager = true;
  }

  /**
   * Set Thread Manager from an executor.
   *
   * @param executor folly::Executor to be set as the threadManager
   */
  void setThreadManagerFromExecutor(
      folly::Executor* executor, std::string name = "") {
    if (THRIFT_FLAG(allow_resource_pools_set_thread_manager_from_executor)) {
      setThreadManagerType(ThreadManagerType::EXECUTOR_ADAPTER);
      setThreadManagerExecutor(executor);
    } else {
      concurrency::ThreadManagerExecutorAdapter::Options opts(std::move(name));
      setThreadManagerInternal(
          std::make_shared<concurrency::ThreadManagerExecutorAdapter>(
              folly::getKeepAliveToken(executor), std::move(opts)));
      runtimeDisableResourcePoolsDeprecated("setThreadManagerFromExecutor");
      runtimeServerActions_.userSuppliedThreadManager = true;
    }
  }

  /**
   * Get the executor for the default executor used to execute requests.
   *
   * @return a shared pointer to the executor
   */
  std::shared_ptr<folly::Executor> getHandlerExecutor_deprecated()
      const override {
    if (!runtimeServerActions_.userSuppliedThreadManager &&
        !resourcePoolSet().empty()) {
      return resourcePoolSet()
          .resourcePool(ResourcePoolHandle::defaultAsync())
          .sharedPtrExecutor_deprecated()
          .value();
    }
    std::shared_lock lock(threadManagerMutex_);
    return threadManager_;
  }

  folly::Executor::KeepAlive<> getHandlerExecutorKeepAlive() const override {
    if (!runtimeServerActions_.userSuppliedThreadManager &&
        !resourcePoolSet().empty()) {
      return *resourcePoolSet()
                  .resourcePool(ResourcePoolHandle::defaultAsync())
                  .keepAliveExecutor();
    }
    std::shared_lock lock(threadManagerMutex_);
    return threadManager_.get();
  }

  /**
   * Set the default priority for CPU worker threads. This will only apply when
   * the thread manager type is SIMPLE or PRIORITY_QUEUE.
   */
  void setCPUWorkerThreadPriority(
      concurrency::PosixThreadFactory::THREAD_PRIORITY priority) {
    CHECK(configMutable());
    std::lock_guard lock(threadManagerMutex_);
    CHECK(!threadManager_);
    CHECK(!threadFactory_);
    threadPriority_ = priority;
  }

  /**
   * Set the ThreadFactory that will be used to create worker threads for the
   * service.  If not set, a default factory will be used.  Must be called
   * before the thread manager is started.
   */
  void setThreadFactory(
      std::shared_ptr<concurrency::ThreadFactory> threadFactory) {
    CHECK(configMutable());
    std::lock_guard lock(threadManagerMutex_);
    CHECK(!threadManager_);
    CHECK(!threadPriority_);
    threadFactory_ = std::move(threadFactory);
  }

  /**
   * Set the type of ThreadManager to use for this server.
   */
  void setThreadManagerType(ThreadManagerType threadManagerType) {
    CHECK(configMutable());
    std::lock_guard lock(threadManagerMutex_);
    CHECK(!threadManager_);
    threadManagerType_ = threadManagerType;
  }

  /**
   * Set the size of thread pools when using ThreadManagerType::PRIORITY
   */
  void setThreadManagerPoolSizes(
      const std::array<size_t, concurrency::N_PRIORITIES>& poolSizes) {
    CHECK(configMutable());
    std::lock_guard lock(threadManagerMutex_);
    CHECK(!threadManager_);
    for (std::size_t i = 0; i < concurrency::N_PRIORITIES; ++i) {
      threadManagerPrioritiesAndPoolSizes_[i].second = poolSizes[i];
    }
  }

  /**
   * Set the priority and size of thread pools when using
   * ThreadManagerType::PRIORITY.
   */
  void setThreadManagerPrioritiesAndPoolSizes(
      const std::array<
          std::pair<
              apache::thrift::concurrency::PosixThreadFactory::THREAD_PRIORITY,
              size_t>,
          concurrency::N_PRIORITIES>& poolPrioritiesAndSizes) {
    CHECK(configMutable());
    std::lock_guard lock(threadManagerMutex_);
    CHECK(!threadManager_);
    threadManagerPrioritiesAndPoolSizes_ = poolPrioritiesAndSizes;
  }

  /**
   * Set the executors to use for ThreadManagerType::EXECUTOR_ADAPTER
   */
  void setThreadManagerExecutors(
      std::array<std::shared_ptr<folly::Executor>, concurrency::N_PRIORITIES>
          executors) {
    threadManagerExecutors_ = executors;
  }

  void setThreadManagerExecutor(std::shared_ptr<folly::Executor> executor) {
    threadManagerExecutors_.fill(executor);
  }

  void setThreadManagerExecutor(folly::Executor::KeepAlive<> ka) {
    auto executor = std::make_shared<folly::VirtualExecutor>(std::move(ka));
    threadManagerExecutors_.fill(executor);
  }

  void setThreadFactoryInit(
      std::function<void()>&& threadInitializer,
      std::function<void()>&& threadFinalizer = [] {}) {
    // These must be valid callables
    CHECK(threadInitializer);
    CHECK(threadFinalizer);
    CHECK(configMutable());
    std::lock_guard lock(threadManagerMutex_);
    CHECK(!threadManager_);
    threadInitializer_ = std::move(threadInitializer);
    threadFinalizer_ = std::move(threadFinalizer);
  }

  /**
   * Get Thread Manager (for queuing mode).
   *
   * @return a shared pointer to the thread manager
   */
  std::shared_ptr<concurrency::ThreadManager> getThreadManager_deprecated()
      const override {
    std::shared_lock lock(threadManagerMutex_);
    return tmLoggingWrapper_;
  }

  std::shared_ptr<folly::Executor> getThreadManager() const override {
    std::shared_lock lock(threadManagerMutex_);
    return threadManager_;
  }

  // Define Server behavior to allow or reject header traffic
  void setLegacyTransport(LegacyTransport value) { legacyTransport_ = value; }

  LegacyTransport getLegacyTransport() const { return legacyTransport_; }

  const Metadata& metadata() const { return metadata_; }

  Metadata& metadata() { return metadata_; }

  /**
   * Get the flags used to support migrations and rollouts.
   */
  RuntimeServerActions& getRuntimeServerActions() const {
    return runtimeServerActions_;
  }

  void setAsPrimaryServer() { isPrimaryServer_ = true; }
  bool isPrimaryServer() const { return isPrimaryServer_; }

  /**
   * Set the client identity hook for the server, which will be called in
   * Cpp2ConnContext(). It can be used to cache client identities for each
   * connection. They can be retrieved with Cpp2ConnContext::getPeerIdentities.
   */
  void setClientIdentityHook(ClientIdentityHook func) {
    clientIdentityHook_ = func;
  }

  ClientIdentityHook getClientIdentityHook() { return clientIdentityHook_; }

  /**
   * Returns a reference to the custom allocator used by the server when parsing
   * Thrift frames.
   */
  std::shared_ptr<rocket::ParserAllocatorType> getCustomAllocatorForParser() {
    return customAllocatorForParser_;
  }

  /**
   * Sets the custom allocator used by the server. The allocator is use by the
   * server to allocate memory for the IOBufs when parsing incoming frames.
   *
   * @param customAllocator A unique pointer to the custom allocator. The
   * ThriftServer will take over the ownership
   */
  void setCustomAllocatorForParser(
      std::shared_ptr<rocket::ParserAllocatorType> customParserAllocator) {
    customAllocatorForParser_ = std::move(customParserAllocator);
  }

  void enableInfoLogging() { infoLoggingEnabled_ = true; }

  void disableInfoLogging() { infoLoggingEnabled_ = false; }

  void setCPUConcurrencyController(
      std::shared_ptr<CPUConcurrencyController> controller) {
    cpuConcurrencyController_ = std::move(controller);
  }

 private:
  friend ThriftServerConfig& detail::getThriftServerConfig(ThriftServer&);

  ThriftServerConfig thriftConfig_;

  bool infoLoggingEnabled_{true};

  ServerInterface* applicationServerInterface_{};

  // Explicitly set monitoring service interface handler
  std::shared_ptr<MonitoringServerInterface> monitoringServiceHandler_;

  // Explicitly set status service interface handler
  std::shared_ptr<StatusServerInterface> statusServiceHandler_;

  // Explicitly set control service interface handler
  std::shared_ptr<ControlServerInterface> controlServiceHandler_;

  // Explicitly set security service interface handler
  std::shared_ptr<SecurityServerInterface> securityServiceHandler_;

  std::shared_ptr<server::TServerEventHandler> eventHandler_;
  std::vector<std::shared_ptr<server::TServerEventHandler>> eventHandlers_;

  GetHeaderHandlerFunc getHeaderHandler_;

  // Cpp2 ProcessorFactory.
  // NOTE: cpp2Pfac_ should destruct before the above
  // shared_ptr<TServerEventHandler> fields. In particular,
  // PythonServerEventHandler could own the reference to
  // folly::python::NotificationQueueAsyncioExecutor, thus expecting all other
  // reference to the executor to be released before PythonServerEventHandler
  // destructs. Meanwhile, the cpp2Pfac_ here could hold a reference (e.g.,
  // PythonAsyncProcessorFactory.create in
  // thrift/lib/python/server/python_async_processor.pyx)
  std::shared_ptr<apache::thrift::AsyncProcessorFactory> cpp2Pfac_;

  // TODO: T176242251 we use unique_ptr and just pass raw pointer / reference in
  // rocket's stack. If the object is owned by ThriftServer, then we know it
  // will outlive every RocketServerConnection (and related) objects.
  std::shared_ptr<rocket::ParserAllocatorType> customAllocatorForParser_{
      nullptr};

  // Server behavior to wrt header traffic
  LegacyTransport legacyTransport_{LegacyTransport::DEFAULT};

  Metadata metadata_;

  //! Flags used to track certain actions of thrift servers to help support
  //! migrations and rollouts.
  mutable RuntimeServerActions runtimeServerActions_;

  /**
   * In cases where multiple services are running in the same process, this
   * will be used to indicate which is the primary server.
   */
  bool isPrimaryServer_{false};

  // Notification of various server events. Note that once observer_ has been
  // set, it cannot be set again and will remain alive for (at least) the
  // lifetime of *this.
  folly::Synchronized<std::shared_ptr<server::TServerObserver>> observer_;
  std::atomic<server::TServerObserver*> observerPtr_{nullptr};

  // Interface for instrumenting interceptors
  std::shared_ptr<InterceptorMetricCallback> interceptorMetricCallback_{
      std::make_shared<NoopInterceptorMetricCallback>()};

  //! The type of thread manager to create.
  ThreadManagerType threadManagerType_{ThreadManagerType::PRIORITY};

  //! The thread pool sizes and priorities for priority thread manager.
  //! If any of the pool sizes are set to non-zero then we will use this.
  std::array<
      std::pair<
          apache::thrift::concurrency::PosixThreadFactory::THREAD_PRIORITY,
          size_t>,
      concurrency::N_PRIORITIES>
      threadManagerPrioritiesAndPoolSizes_{
          {{concurrency::PosixThreadFactory::HIGHER_PRI, 0},
           {concurrency::PosixThreadFactory::HIGH_PRI, 0},
           {concurrency::PosixThreadFactory::HIGH_PRI, 0},
           {concurrency::PosixThreadFactory::NORMAL_PRI, 0},
           {concurrency::PosixThreadFactory::LOWER_PRI, 0}}};

  //! Executors to use for ThreadManagerExecutorAdapter.
  std::array<std::shared_ptr<folly::Executor>, concurrency::N_PRIORITIES>
      threadManagerExecutors_;

  /**
   * The thread manager used for sync calls.
   */
  mutable folly::SharedMutex threadManagerMutex_;
  std::shared_ptr<apache::thrift::concurrency::ThreadManager> threadManager_;
  // we need to make the wrapper stick to the server because the users calling
  // getThreadManager are relying on the server to maintain the tm lifetime
  std::shared_ptr<apache::thrift::ThreadManagerLoggingWrapper>
      tmLoggingWrapper_;

  // Thread factory hooks if required. threadInitializer_ will be called when
  // the thread is started before it starts handling requests and
  // threadFinalizer_ will be called before the thread exits.
  std::function<void()> threadInitializer_;
  std::function<void()> threadFinalizer_;

  // If set, the thread factory that should be used to create worker threads.
  std::shared_ptr<concurrency::ThreadFactory> threadFactory_;

  // The default thread priority to use (only applies to SIMPLE or
  // PRIORITY_QUEUE ThreadManagerType and if no threadFactory supplied)
  std::optional<concurrency::PosixThreadFactory::THREAD_PRIORITY>
      threadPriority_;

  AdaptiveConcurrencyController adaptiveConcurrencyController_;

  folly::observer::SimpleObservable<
      std::optional<CPUConcurrencyController::Config>>
      mockCPUConcurrencyControllerConfig_{std::nullopt};
  folly::observer::Observer<CPUConcurrencyController::Config>
  makeCPUConcurrencyControllerConfigInternal();
  std::shared_ptr<CPUConcurrencyController> cpuConcurrencyController_;

  //! The server's listening addresses
  std::vector<folly::SocketAddress> addresses_;

  //! The server's listening port
  std::optional<uint16_t> port_;

  IsOverloadedFunc isOverloaded_;

  PreprocessFunctionSet preprocessFunctions_;

  std::function<int64_t(const std::string&)> getLoad_;

  ClientIdentityHook clientIdentityHook_;

  // This is meant to be used internally
  // We separate setThreadManager and configureThreadManager
  // so that we can have proper logging for the former
  // These APIs will be deprecated eventually when ResourcePool
  // migration is done.
  void setThreadManagerInternal(
      std::shared_ptr<apache::thrift::concurrency::ThreadManager>
          threadManager) {
    CHECK(configMutable());
    std::lock_guard lock(threadManagerMutex_);
    threadManager_ = threadManager;
    tmLoggingWrapper_ = std::make_shared<ThreadManagerLoggingWrapper>(
        threadManager_, this, true, resourcePoolEnabled());
  }

 public:
  void setObserver(const std::shared_ptr<server::TServerObserver>& observer) {
    auto locked = observer_.wlock();
    if (*locked) {
      throw std::logic_error("Server already has an observer installed");
    }
    *locked = observer;
    observerPtr_.store(locked->get());
  }

  server::TServerObserver* getObserver() const final {
    return observerPtr_.load(std::memory_order_relaxed);
  }

  std::shared_ptr<server::TServerObserver> getObserverShared() const {
    return observer_.copy();
  }

  AdaptiveConcurrencyController& getAdaptiveConcurrencyController() final {
    return adaptiveConcurrencyController_;
  }

  const AdaptiveConcurrencyController& getAdaptiveConcurrencyController()
      const final {
    return adaptiveConcurrencyController_;
  }

  const auto& adaptiveConcurrencyController() const {
    return adaptiveConcurrencyController_;
  }

  CPUConcurrencyController* getCPUConcurrencyController() final {
    return cpuConcurrencyController_.get();
  }

  const CPUConcurrencyController* getCPUConcurrencyController() const final {
    return cpuConcurrencyController_.get();
  }

  bool notifyCPUConcurrencyControllerOnRequestLoadShed(
      std::optional<CPUConcurrencyController::Method> method);

  void setMockCPUConcurrencyControllerConfig(
      CPUConcurrencyController::Config config) {
    mockCPUConcurrencyControllerConfig_.setValue(config);
  }

  /**
   * Get the maximum # of requests being processed in handler before overload.
   *
   * @return current setting.
   */
  uint32_t getMaxRequests() const override {
    return adaptiveConcurrencyController_.enabled()
        ? static_cast<uint32_t>(adaptiveConcurrencyController_.getMaxRequests())
        : thriftConfig_.getMaxRequests().get();
  }

  /**
   * Set the maximum # of requests being processed in handler before overload.
   *
   * @param maxRequests new setting for maximum # of active requests.
   */
  void setMaxRequests(uint32_t maxRequests) override {
    thriftConfig_.setMaxRequests(
        folly::observer::makeStaticObserver(std::optional{maxRequests}),
        AttributeSource::OVERRIDE);
  }

  /**
   * Get the maximum number of requests that this server's resource pool may
   * execute concurrently. When the maximum is reached, resource pools will
   * queue requests until they can be processed without exceeding the maximum.
   *
   * @return current setting
   */
  uint32_t getConcurrencyLimit() const override {
    return thriftConfig_.getConcurrencyLimit().get();
  }

  /**
   * Set the maximum number of requests that this server's resource pool may
   * execute concurrently.
   *
   * @param concurrencyLimit new setting for concurrency limit.
   */
  void setConcurrencyLimit(uint32_t concurrencyLimit) override {
    folly::call_once(cancelSetMaxRequestsCallbackHandleFlag_, [this]() {
      setMaxRequestsCallbackHandle.cancel();
    });
    thriftConfig_.setConcurrencyLimit(
        folly::observer::makeStaticObserver(std::optional{concurrencyLimit}),
        AttributeSource::OVERRIDE);
  }

  /**
   * Get the number of CPU (pool) threads
   *
   * @return number of CPU (pool) threads
   */
  size_t getNumCPUWorkerThreads() const {
    return thriftConfig_.getNumCPUWorkerThreads();
  }

  /**
   * Set the number of CPU (pool) threads.
   * Won't be effective if using customized ResourcePool.
   * If set to 0, the number of normal priority threads will be the same as
   * number of CPU cores.
   *
   * @param number of CPU (pool) threads
   */
  void setNumCPUWorkerThreads(
      size_t numCPUWorkerThreads,
      AttributeSource source = AttributeSource::OVERRIDE) {
    CHECK(!threadManager_);
    thriftConfig_.setNumCPUWorkerThreads(
        std::move(numCPUWorkerThreads), source);
  }

  /**
   * Indicate whether it is safe to modify the server config through setters.
   * This roughly corresponds to whether the IO thread pool could be servicing
   * requests.
   *
   * @return true if the configuration can be modified, false otherwise
   */
  bool configMutable() { return !thriftConfig_.isFrozen(); }

  /**
   * Get the prefix for naming the CPU (pool) threads.
   *
   * @return current setting.
   */
  std::string getCPUWorkerThreadName() const {
    return thriftConfig_.getCPUWorkerThreadName();
  }

  /**
   * Set the prefix for naming the CPU (pool) threads. Not set by default.
   * must be called before serve() for it to take effect
   * ignored if setThreadManager() is called.
   *
   * @param cpuWorkerThreadName thread name prefix
   */
  void setCPUWorkerThreadName(
      const std::string& cpuWorkerThreadName,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setCPUWorkerThreadName(cpuWorkerThreadName, source);
  }

  /**
   * Get whether to use in memory ticket seeds.
   *
   * @return true if ticket seeds are stored in memory; false if ticket seeds
   * are read from a file
   */
  bool getUseInMemoryTicketSeeds() const {
    return thriftConfig_.getUseInMemoryTicketSeeds();
  }

  /**
   * Set whether to use in memory ticket seeds.
   *
   * @param useInMemoryTicketSeeds true if ticket seeds are stored in memory;
   * false if ticket seeds are read from a file
   */
  void setUseInMemoryTicketSeeds(
      bool useInMemoryTicketSeeds,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setUseInMemoryTicketSeeds(useInMemoryTicketSeeds, source);
  }

  /**
   * Get the maximum # of connections allowed before overload.
   *
   * @return current setting.
   */
  uint32_t getMaxConnections() const {
    return thriftConfig_.getMaxConnections().get();
  }

  /**
   * Set the maximum # of connections allowed before overload.
   *
   * @param maxConnections new setting for maximum # of connections.
   */
  void setMaxConnections(
      uint32_t maxConnections,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setMaxConnections(
        folly::observer::makeStaticObserver(std::optional{maxConnections}),
        source);
  }

  /**
   * Get the maximum queries per second (QPS) this server is allowed
   * to receive. If we receive more requests than this, we will shed
   * incoming requests until we refresh our token bucket.
   *
   * @return current setting
   */
  uint32_t getMaxQps() const override {
    return thriftConfig_.getMaxQps().get();
  }

  /**
   * Set the maximum queries per second (QPS) this server is allowed
   * to receive.
   *
   * @param maxQps new setting for maximum qps
   */
  void setMaxQps(uint32_t maxQps) override {
    thriftConfig_.setMaxQps(
        folly::observer::makeStaticObserver(std::optional{maxQps}),
        AttributeSource::OVERRIDE);
  }

  /**
   * Get the maximum requests that may begin being processing in a given
   * one-second window before additional requests are queued. Only applies when
   * using TokenBucketConcurrencyController.
   *
   * @return current setting
   */
  uint32_t getExecutionRate() const override {
    return thriftConfig_.getExecutionRate().get();
  }

  /**
   * Set the maximum requests that may begin being processing in a given
   * one-second window before additional requests are queued. Only applies when
   * using TokenBucketConcurrencyController.
   *
   * @param executionRate new setting for execution rate.
   */
  void setExecutionRate(uint32_t executionRate) override {
    folly::call_once(cancelSetMaxQpsCallbackHandleFlag_, [this]() {
      setMaxQpsCallbackHandle.cancel();
    });
    thriftConfig_.setExecutionRate(
        folly::observer::makeStaticObserver(std::optional{executionRate}),
        AttributeSource::OVERRIDE);
  }

  /**
   * Sets the timeout for joining workers
   * @param timeout new setting for timeout for joining requests.
   */
  void setWorkersJoinTimeout(
      std::chrono::seconds timeout,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setWorkersJoinTimeout(std::move(timeout), source);
  }

  /**
   * Get the timeout for joining workers.
   * @return workers joing timeout in seconds
   */
  std::chrono::seconds getWorkersJoinTimeout() const {
    return thriftConfig_.getWorkersJoinTimeout();
  }

  uint64_t getMaxResponseSize() const final {
    return thriftConfig_.getMaxResponseSize().get();
  }

  void setMaxResponseSize(
      uint64_t size, AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setMaxResponseSize(
        folly::observer::makeStaticObserver(std::optional{size}), source);
  }

  bool getUseClientTimeout() const override {
    return thriftConfig_.getUseClientTimeout().get();
  }

  void setUseClientTimeout(
      bool useClientTimeout,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setUseClientTimeout(
        folly::observer::makeStaticObserver(std::optional{useClientTimeout}),
        source);
  }

  /**
   * Get the maximum number of pending connections each io worker thread can
   * hold.
   */
  uint32_t getMaxNumPendingConnectionsPerWorker() const {
    return thriftConfig_.getMaxNumPendingConnectionsPerWorker();
  }
  /**
   * Set the maximum number of pending connections each io worker thread can
   * hold. No new connections will be sent to that io worker thread if there
   * are more than such number of unprocessed connections in that queue. If
   * every io worker thread's queue is full the connection will be dropped.
   */
  void setMaxNumPendingConnectionsPerWorker(
      uint32_t num, AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setMaxNumPendingConnectionsPerWorker(std::move(num), source);
  }

  /** Get maximum number of milliseconds we'll wait for data (0 = infinity).
   *
   *  @return number of milliseconds, or 0 if no timeout set.
   */
  std::chrono::milliseconds getIdleTimeout() const {
    return thriftConfig_.getIdleTimeout();
  }

  /** Don't use: Set maximum number of milliseconds connection can live. Most
   * likely you want to use setIdleTimeout instead..
   *
   *  @param timeout number of milliseconds.
   */
  void setConnectionAgeTimeout(std::chrono::milliseconds timeout) {
    thriftConfig_.setConnectionAgeTimeout(
        std::move(timeout), AttributeSource::OVERRIDE);
  }

  std::chrono::milliseconds getConnectionAgeTimeout() const {
    return thriftConfig_.getConnectionAgeTimeout();
  }

  /** Set maximum number of milliseconds we'll wait for data (0 = infinity).
   *  Note: existing connections are unaffected by this call.
   *
   * WARNING: Idle timeout will not work for Rocket connections unless the
   * rocket_set_idle_connection_timeout Thrift Flag is set to true.
   *
   *  @param timeout number of milliseconds, or 0 to disable timeouts.
   */
  void setIdleTimeout(
      std::chrono::milliseconds timeout,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setIdleTimeout(std::move(timeout), source);
  }

  /**
   * Set the number of IO worker threads
   *
   * @param number of IO worker threads
   */
  void setNumIOWorkerThreads(size_t numIOWorkerThreads) {
    thriftConfig_.setNumIOWorkerThreads(
        std::move(numIOWorkerThreads), AttributeSource::OVERRIDE);
  }

  /**
   * Get the number of IO worker threads
   *
   * @return number of IO worker threads
   */
  size_t getNumIOWorkerThreads() const final {
    return thriftConfig_.getNumIOWorkerThreads();
  }

  bool getEnableCodel() const { return thriftConfig_.getEnableCodel().get(); }

  /**
   * Set the task expire time
   *
   */
  void setTaskExpireTime(
      std::chrono::milliseconds timeout,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setTaskExpireTime(
        folly::observer::makeStaticObserver(std::optional{timeout}), source);
  }

  /**
   * Get the task expire time
   *
   * @return task expire time
   */
  std::chrono::milliseconds getTaskExpireTime() const override {
    return thriftConfig_.getTaskExpireTime().get();
  }

  /**
   * Set the stream starvation time
   *
   */
  void setStreamExpireTime(
      std::chrono::milliseconds timeout,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setStreamExpireTime(
        folly::observer::makeStaticObserver(std::optional{timeout}), source);
  }

  /**
   * If there is no request for the stream for the given time period, then the
   * stream will create timeout error.
   */
  std::chrono::milliseconds getStreamExpireTime() const final {
    return thriftConfig_.getStreamExpireTime().get();
  }

  /**
   * Set the queue timeout to processing timeout percentage. This is to ensure
   * server can load shedding effectively when service is hosting many clients
   * that has different client timeout. If set, Thrift Server will choose the
   * high queue Timeout from this setting and queue timeout from
   * setQueueTimeout() above. Also, notes if client side set queue_timeout
   * explicitly, then server side queuetimeout setting will be ignored.
   */
  virtual void setQueueTimeoutPct(
      uint32_t queueTimeoutPct,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setQueueTimeoutPct(
        folly::observer::makeStaticObserver(std::optional{queueTimeoutPct}),
        source);
  }

  /**
   * Get the time requests are allowed to stay on the queue
   *
   * @return queue timeout
   */
  std::chrono::milliseconds getQueueTimeout() const override {
    return thriftConfig_.getQueueTimeout().get();
  }

  /**
   * Get the queue_timeout_pct.
   *
   * @return queue timeout percent
   */
  uint32_t getQueueTimeoutPct() const override {
    return thriftConfig_.getQueueTimeoutPct().get();
  }

  /**
   * Sets the duration before which new connections waiting on a socket's queue
   * are closed. A value of 0 represents an infinite duration.
   * See `folly::AsyncServerSocket::setQueueTimeout`.
   */
  void setSocketQueueTimeout(
      folly::observer::Observer<std::chrono::nanoseconds> timeout,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setSocketQueueTimeout(
        folly::observer::makeObserver(
            [=]() -> std::optional<std::chrono::milliseconds> {
              return std::chrono::duration_cast<std::chrono::milliseconds>(
                  **timeout);
            }),
        source);
  }

  void setSocketQueueTimeout(
      folly::Optional<std::chrono::nanoseconds> timeout,
      AttributeSource source = AttributeSource::OVERRIDE) {
    if (timeout) {
      thriftConfig_.setSocketQueueTimeout(
          folly::observer::makeStaticObserver(
              std::optional{
                  std::chrono::duration_cast<std::chrono::milliseconds>(
                      *timeout)}),
          source);
    }
  }

  void setSocketQueueTimeout(
      std::chrono::milliseconds timeout,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setSocketQueueTimeout(
        folly::observer::makeStaticObserver(std::optional{timeout}), source);
  }

  /**
   * How long a socket with outbound data will tolerate read inactivity from a
   * client. Clients must read data from their end of the connection before this
   * period expires or the server will drop the connection. The amount of data
   * read by the client is irrelevant. Zero disables the timeout.
   */
  void setSocketWriteTimeout(
      std::chrono::milliseconds timeout,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setSocketWriteTimeout(
        folly::observer::makeStaticObserver(std::optional{timeout}), source);
  }

  std::chrono::milliseconds getSocketWriteTimeout() const {
    return thriftConfig_.getSocketWriteTimeout().get();
  }

  /**
   * Gets an observer representing the socket queue timeout.
   */
  const folly::observer::Observer<std::chrono::nanoseconds>&
  getSocketQueueTimeout() const {
    return thriftConfig_.getSocketQueueTimeout().getObserver();
  }

  /**
   * Gets the current socket queue timeout in milliseconds.
   */
  std::chrono::milliseconds getSocketQueueTimeoutMs() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        thriftConfig_.getSocketQueueTimeout().get());
  }

  /**
   * Set the listen backlog. Refer to the comment on listenBacklog_ member for
   * details.
   */
  void setListenBacklog(
      int listenBacklog, AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setListenBacklog(std::move(listenBacklog), source);
  }

  /**
   * Get the listen backlog.
   *
   * @return listen backlog.
   */
  int getListenBacklog() const { return thriftConfig_.getListenBacklog(); }

  void setMethodsBypassMaxRequestsLimit(
      const std::vector<std::string>& methods,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setMethodsBypassMaxRequestsLimit(methods, source);
  }

  const folly::sorted_vector_set<std::string>&
  getMethodsBypassMaxRequestsLimit() const {
    return thriftConfig_.getMethodsBypassMaxRequestsLimit();
  }

  /**
   * Return the maximum memory usage by each debug payload.
   */
  uint64_t getMaxDebugPayloadMemoryPerRequest() const {
    return thriftConfig_.getMaxDebugPayloadMemoryPerRequest();
  }

  /**
   * Set the maximum memory usage by each debug payload.
   */
  void setMaxDebugPayloadMemoryPerRequest(
      uint64_t limit, AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setMaxDebugPayloadMemoryPerRequest(std::move(limit), source);
  }

  /**
   * Return the maximum memory usage by each worker to keep track of debug
   * payloads.
   */
  uint64_t getMaxDebugPayloadMemoryPerWorker() const {
    return thriftConfig_.getMaxDebugPayloadMemoryPerWorker();
  }

  /**
   * Set the maximum memory usage by each worker to keep track of debug
   * payloads.
   */
  void setMaxDebugPayloadMemoryPerWorker(
      uint64_t limit, AttributeSource source = AttributeSource::OVERRIDE) {
    // setStaticAttribute(
    //     maxDebugPayloadMemoryPerWorker_, std::move(limit), source);
    thriftConfig_.setMaxDebugPayloadMemoryPerWorker(std::move(limit), source);
  }

  /**
   * Return the maximum memory usage by each worker to keep track of debug
   * payloads.
   */
  uint16_t getMaxFinishedDebugPayloadsPerWorker() const {
    return thriftConfig_.getMaxFinishedDebugPayloadsPerWorker();
  }

  /**
   * Set the maximum memory usage by each worker to keep track of debug
   * payloads.
   */
  void setMaxFinishedDebugPayloadsPerWorker(
      uint16_t limit, AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setMaxFinishedDebugPayloadsPerWorker(
        std::move(limit), source);
  }

  /**
   * Set write batching interval
   */
  void setWriteBatchingInterval(
      std::chrono::milliseconds interval,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setWriteBatchingInterval(
        folly::observer::makeStaticObserver(std::optional{interval}), source);
  }

  /**
   * Get write batching interval
   */
  std::chrono::milliseconds getWriteBatchingInterval() const {
    return thriftConfig_.getWriteBatchingInterval().get();
  }

  /**
   * Set write batching size. Ignored if write batching interval is not set.
   */
  void setWriteBatchingSize(
      size_t batchingSize, AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setWriteBatchingSize(
        folly::observer::makeStaticObserver(std::optional{batchingSize}),
        source);
  }

  /**
   * Get write batching size
   */
  size_t getWriteBatchingSize() const {
    return thriftConfig_.getWriteBatchingSize().get();
  }

  /**
   * Set write batching byte size. Ignored if write batching interval is not
   * set.
   */
  void setWriteBatchingByteSize(
      size_t batchingByteSize,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setWriteBatchingByteSize(
        folly::observer::makeStaticObserver(std::optional{batchingByteSize}),
        source);
  }

  /**
   * Get write batching byte size
   */
  size_t getWriteBatchingByteSize() const {
    return thriftConfig_.getWriteBatchingByteSize().get();
  }

  /**
   * Ingress memory is the total memory used for receiving inflight requests.
   * If the memory limit is hit, the connection along with the violating request
   * will be closed
   */
  void setIngressMemoryLimit(
      size_t ingressMemoryLimit,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setIngressMemoryLimit(
        folly::observer::makeStaticObserver(std::optional{ingressMemoryLimit}),
        source);
  }

  size_t getIngressMemoryLimit() const {
    return thriftConfig_.getIngressMemoryLimit().get();
  }

  folly::observer::Observer<size_t> getIngressMemoryLimitObserver() const {
    return thriftConfig_.getIngressMemoryLimit().getObserver();
  }

  /**
   * Limit the amount of memory available for inflight responses, meaning
   * responses that are queued on the server pending delivery to clients. This
   * limit, divided by the number of IO threads, determines the effective egress
   * limit of a connection. Once the per-connection limit is reached, a
   * connection is dropped immediately and all outstanding responses are
   * discarded.
   */
  void setEgressMemoryLimit(
      size_t max, AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setEgressMemoryLimit(
        folly::observer::makeStaticObserver(std::optional{max}), source);
  }

  size_t getEgressMemoryLimit() const {
    return thriftConfig_.getEgressMemoryLimit().get();
  }

  folly::observer::Observer<size_t> getEgressMemoryLimitObserver() const {
    return thriftConfig_.getEgressMemoryLimit().getObserver();
  }

  /**
   * Connection close will only be enforced and triggered on those requests with
   * size greater or equal than this attribute
   */
  void setMinPayloadSizeToEnforceIngressMemoryLimit(
      size_t minPayloadSizeToEnforceIngressMemoryLimit,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setMinPayloadSizeToEnforceIngressMemoryLimit(
        folly::observer::makeStaticObserver(
            std::optional{minPayloadSizeToEnforceIngressMemoryLimit}),
        source);
  }

  size_t getMinPayloadSizeToEnforceIngressMemoryLimit() const {
    return thriftConfig_.getMinPayloadSizeToEnforceIngressMemoryLimit().get();
  }

  folly::observer::Observer<size_t>
  getMinPayloadSizeToEnforceIngressMemoryLimitObserver() const {
    return thriftConfig_.getMinPayloadSizeToEnforceIngressMemoryLimit()
        .getObserver();
  }

  size_t getEgressBufferBackpressureThreshold() const {
    return thriftConfig_.getEgressBufferBackpressureThreshold().get();
  }

  /**
   * Apply backpressure to all stream generators of a connection when combined
   * allocation size of inflight writes for that connection exceeds the
   * threshold.
   */
  void setEgressBufferBackpressureThreshold(
      size_t thresholdInBytes,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setEgressBufferBackpressureThreshold(
        folly::observer::makeStaticObserver(std::optional{thresholdInBytes}),
        source);
  }

  double getEgressBufferRecoveryFactor() const {
    return thriftConfig_.getEgressBufferRecoveryFactor().get();
  }

  /**
   * When egress buffer backpressure is enabled, resume normal operation once
   * egress buffer size falls below this factor of the threshold.
   */
  void setEgressBufferRecoveryFactor(
      double recoveryFactor,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setEgressBufferRecoveryFactor(
        folly::observer::makeStaticObserver(std::optional{recoveryFactor}),
        source);
  }

  folly::observer::Observer<std::chrono::milliseconds>
  getPolledServiceHealthLivenessObserver() const {
    return thriftConfig_.getPolledServiceHealthLiveness().getObserver();
  }

  void setPolledServiceHealthLiveness(
      std::chrono::milliseconds liveness,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setPolledServiceHealthLiveness(
        folly::observer::makeStaticObserver(std::optional{liveness}), source);
  }

  const folly::SocketOptionMap& getPerConnectionSocketOptions() const {
    return thriftConfig_.getPerConnectionSocketOptions().get();
  }
  void setPerConnectionSocketOptions(
      folly::SocketOptionMap options,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setPerConnectionSocketOptions(
        folly::observer::makeStaticObserver(std::optional{options}), source);
  }

  void setResetConnCtxUserDataOnClose(
      bool value, AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setResetConnCtxUserDataOnClose(value, source);
  }

  /**
   * Calls the twin function getTaskExpireTimeForRequest with the
   * clientQueueTimeoutMs and clientTimeoutMs fields retrieved from the THeader.
   */
  bool getTaskExpireTimeForRequest(
      const apache::thrift::transport::THeader& header,
      std::chrono::milliseconds& queueTimeout,
      std::chrono::milliseconds& taskTimeout) const;

  /**
   * A task has two timeouts:
   *
   * If the task hasn't started processing the request by the time the soft
   * timeout has expired, we should throw the task away.
   *
   * However, if the task has started processing the request by the time the
   * soft timeout has expired, we shouldn't expire the task until the hard
   * timeout has expired.
   *
   * The soft timeout protects the server from starting to process too many
   * requests.  The hard timeout protects us from sending responses that
   * are never read.
   *
   * @returns whether or not the soft and hard timeouts are different
   */
  bool getTaskExpireTimeForRequest(
      std::chrono::milliseconds clientQueueTimeoutMs,
      std::chrono::milliseconds clientTimeoutMs,
      std::chrono::milliseconds& queueTimeout,
      std::chrono::milliseconds& taskTimeout) const final;

  const ThriftServerConfig& getThriftServerConfig() const {
    return thriftConfig_;
  }

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

  // setMaxRequestsCallbackHandle should be cancelled, unsyncing maxRequests
  // from resource pools, when setConcurrencyLimit is explicitly called.
  folly::once_flag cancelSetMaxRequestsCallbackHandleFlag_;

  // setMaxQpsCallbackHandle should be cancelled, unsyncing maxQps from resource
  // pools, when setExecutionRate is explicitly called.
  folly::once_flag cancelSetMaxQpsCallbackHandleFlag_;

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
  std::shared_ptr<ConnectionEventCallback> connEventCallback_;

  void handleSetupFailure(void);

  void updateCertsToWatch();

  bool stopWorkersOnStopListening_ = true;
  bool joinRequestsWhenServerStops_{true};

  folly::AsyncWriter::ZeroCopyEnableFunc zeroCopyEnableFunc_;

  folly::AsyncServerSocket::CallbackAssignFunction callbackAssignFunc_;

  std::shared_ptr<folly::IOThreadPoolExecutorBase> acceptPool_;
  int nAcceptors_ = 1;
  uint16_t socketMaxReadsPerEvent_{16};

  mutable folly::SharedMutex ioGroupMutex_;

  std::shared_ptr<folly::IOThreadPoolExecutorBase> getIOGroupSafe() const {
    std::shared_lock lock(ioGroupMutex_);
    return getIOGroup();
  }

  void stopWorkers();
  void stopCPUWorkers();
  void stopAcceptingAndJoinOutstandingRequests();

  void callInterceptorsOnStartServing(
      server::DecoratorDataPerRequestBlueprint::Setup& decoratorDataSetup);
  void callDecoratorsAndHandlersBeforeStartServing(
      server::DecoratorDataPerRequestBlueprint::Setup& decoratorDataSetup);
  void callHandlersOnStartServing();
  void callOnStopRequested();

  void ensureDecoratedProcessorFactoryInitialized();
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
  friend class rocket::RocketRequestHandler;
  friend class rocket::RefactoredThriftRocketServerHandler;
  friend class ThriftQuicServer;

  bool tosReflect_{false};
  uint32_t listenerTos_{0};

  std::optional<instrumentation::ServerTracker> tracker_;

  bool quickExitOnShutdownTimeout_ = false;

  bool setupThreadManagerCalled_ = false;

 protected:
  folly::observer::CallbackHandle getSSLCallbackHandle();
  folly::observer::CallbackHandle setConcurrencyLimitCallbackHandle{};
  folly::observer::CallbackHandle setMaxRequestsCallbackHandle{};
  folly::observer::CallbackHandle setMaxQpsCallbackHandle{};
  folly::observer::CallbackHandle setExecutionRateCallbackHandle{};

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
     * ServerConfigs::getInternalMethods. Once the server enters this state,
     * we have finished calling onBeforeStartServing / co_onBeforeStartServing
     * for decorators and handlers and co_onStartServing for interceptors
     * TODO(ezou) rename interceptors onStartServing to co_onBeforeStartServing
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
  /**
   * Tracks whether the C++ health poller has been disabled due to Python
   * services directly setting their health status.
   */
  bool disabledPoller_{false};
#endif

  struct ModulesSpecification {
    struct Info {
      std::unique_ptr<ServerModule> module;
      std::string name;
    };
    std::vector<Info> infos;
    std::unordered_set<std::string> names;
  } unprocessedModulesSpecification_;

  struct ProcessedModuleSet {
    std::vector<ModulesSpecification::Info> modules;
    /**
     * Event handlers from all modules coalesced into one list.
     */
    std::vector<std::shared_ptr<TProcessorEventHandler>>
        coalescedLegacyEventHandlers;
    std::vector<std::shared_ptr<server::TServerEventHandler>>
        coalescedLegacyServerEventHandlers;
    std::vector<std::shared_ptr<ServiceInterceptorBase>>
        coalescedServiceInterceptors;
  };
  static ProcessedModuleSet processModulesSpecification(ModulesSpecification&&);

  struct ProcessedServiceDescription {
    ProcessedModuleSet modules;

    explicit ProcessedServiceDescription(ProcessedModuleSet moduleSet)
        : modules(std::move(moduleSet)) {}

    ProcessedServiceDescription(ProcessedServiceDescription&& modules) =
        default;
    ProcessedServiceDescription& operator=(ProcessedServiceDescription&&) =
        default;

    /**
     * TServerEventHandler objects are added by mutating the ThriftServer
     * instance via addServerEventHandler. So we use RAII to ensure that they
     * are appropriately removed.
     */
    class Deleter : private std::default_delete<ProcessedServiceDescription> {
     public:
      Deleter() noexcept = default;
      explicit Deleter(ThriftServer& server) : server_(&server) {}

      void operator()(ProcessedServiceDescription* ptr) const {
        SCOPE_EXIT {
          std::default_delete<ProcessedServiceDescription>::operator()(ptr);
        };
        if (server_ == nullptr || ptr == nullptr) {
          return;
        }
        for (auto eventHandler :
             ptr->modules.coalescedLegacyServerEventHandlers) {
          server_->removeServerEventHandler(std::move(eventHandler));
        }
      }

     private:
      ThriftServer* server_{nullptr};
    };

    using UniquePtr = std::unique_ptr<ProcessedServiceDescription, Deleter>;

    static UniquePtr createAndActivate(
        ThriftServer& server, ProcessedServiceDescription object) {
      auto ptr = UniquePtr(
          new ProcessedServiceDescription(std::move(object)), Deleter(server));
      for (auto serverEventHandler :
           ptr->modules.coalescedLegacyServerEventHandlers) {
        server.addServerEventHandler(std::move(serverEventHandler));
      }
      return ptr;
    }
  };
  std::unique_ptr<AsyncProcessorFactory> decoratedProcessorFactory_;
  ProcessedServiceDescription::UniquePtr processedServiceDescription_{nullptr};

 public:
  void addModule(std::unique_ptr<ServerModule> module) {
    CHECK(configMutable());

    auto name = module->getName();
    if (runtimeServerActions_.setupThreadManagerCalledByUser ||
        runtimeServerActions_.runtimeResourcePoolsChecksCalledByUser) {
      THRIFT_SERVER_EVENT(addModuleAfterSetup).log(*this, [=]() {
        return folly::dynamic::object("module_name", name);
      });
    }

    THRIFT_SERVER_EVENT(addModule).log(
        *this, [=]() { return folly::dynamic::object("module_name", name); });

    if (unprocessedModulesSpecification_.names.count(name)) {
      throw std::invalid_argument(
          fmt::format("Duplicate module name: {}", name));
    }
    unprocessedModulesSpecification_.infos.emplace_back(
        ModulesSpecification::Info{std::move(module), std::move(name)});
  }

  bool hasModule(const std::string_view name) const noexcept {
    CHECK(processedServiceDescription_)
        << "Server must be set up before calling this method";
    for (const auto& moduleInfo :
         processedServiceDescription_->modules.modules) {
      if (moduleInfo.name == name) {
        return true;
      }
    }
    return false;
  }

  const std::vector<std::string> getInstalledServerModuleNames() const noexcept;

 private:
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

 private:
  // method to encapsulate the default setup needed for the construction of
  // ThriftServer. Should be called in all ctors not calling the default ctor
  void initializeDefaults();

  std::unique_ptr<RequestPileInterface> makeStandardRequestPile(
      RoundRobinRequestPile::Options options);

  void scheduleInMemoryTicketSeeds();

  folly::SocketOptionMap socketOptions_;

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
  folly::Optional<OverloadResult> checkOverload(
      const transport::THeader::StringToStringMap& readHeaders,
      const std::string& method) final;

  // returns descriptive error if application is unable to process request
  PreprocessResult preprocess(
      const server::PreprocessParams& params) const final;

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

  // Set socket options to be applied to all server sockets
  void setSocketOptions(const folly::SocketOptionMap& options) {
    socketOptions_ = options;
  }

  // Get current socket options
  const folly::SocketOptionMap& getSocketOptions() const {
    return socketOptions_;
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
      setSSLConfig(
          folly::observer::makeObserver(
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
         hybridKexObserver = enableHybridKex(),
         aegisObserver = enableAegis(),
         pskModeObserver = preferPskKe(),
         dcReceiveObserver = enableReceivingDelegatedCreds(),
         dcObserver = enablePresentingDelegatedCredentials()]() {
          (void)**hybridKexObserver;
          (void)**aegisObserver;
          (void)**pskModeObserver;
          (void)**dcReceiveObserver;
          (void)**dcObserver;
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

  /* In memory ticket seeds should not be scheduled if watchTicketPathForChange
   * has already been called. Otherwise we should schedule in memory ticket
   * seeds */
  EffectiveTicketSeedStrategy getEffectiveTicketSeedStrategy() const;

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

  std::shared_ptr<wangle::ServerSocketConfig> getServerSocketConfig() {
    auto config = std::make_shared<wangle::ServerSocketConfig>();
    if (sslContextObserver_.has_value()) {
      config->sslContextConfigs.push_back(*sslContextObserver_->getSnapshot());
    }
    if (sslCacheOptions_) {
      config->sslCacheOptions = *sslCacheOptions_;
    }
    config->connectionIdleTimeout = getIdleTimeout();
    config->connectionAgeTimeout = getConnectionAgeTimeout();
    config->acceptBacklog = getListenBacklog();
    if (ticketSeeds_) {
      config->initialTicketSeeds = *ticketSeeds_;
    }
    if (enableTFO_) {
      config->enableTCPFastOpen = *enableTFO_;
      config->fastOpenQueueSize = fastOpenQueueSize_;
    }
    if (sslHandshakeTimeout_) {
      config->sslHandshakeTimeout = *sslHandshakeTimeout_;
    } else if (getIdleTimeout() == std::chrono::milliseconds::zero()) {
      // make sure a handshake that takes too long doesn't kill the connection
      config->sslHandshakeTimeout = std::chrono::milliseconds::zero();
    }
    // By default, we set strictSSL to false. This means the server will start
    // even if cert/key is missing as it may become available later
    config->strictSSL = getStrictSSL() || getSSLPolicy() == SSLPolicy::REQUIRED;
    config->fizzConfig = fizzConfig_;
    config->customConfigMap["thrift_tls_config"] =
        std::make_shared<ThriftTlsConfig>(thriftTlsConfig_);
    config->socketMaxReadsPerEvent = socketMaxReadsPerEvent_;

    // Apply socket options if any are set
    if (!socketOptions_.empty()) {
      config->setSocketOptions(socketOptions_);
    }
    config->useZeroCopy = !!zeroCopyEnableFunc_;
    config->preferIoUring = preferIoUring_;
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

  static folly::observer::Observer<bool> enableStopTLSV2();

  static folly::observer::Observer<PSPUpgradePolicy> pspUpgradePolicy();

  static folly::observer::Observer<bool> enableReceivingDelegatedCreds();

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
  uint64_t getNumDroppedConnections() const;

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

 private:
  void setupThreadManagerImpl();

 public:
  /**
   * Create and start the default thread manager unless it already exists.
   */
  void setupThreadManager();

  /**
   * Adds resource pools for any priorities not specified in allocated to this
   * server.
   */
  void ensureResourcePoolsDefaultPrioritySetup(
      std::vector<concurrency::PRIORITY> allocated = {concurrency::NORMAL});

  /**
   * Kill the workers and wait for listeners to quit
   */
  void cleanUp();

  void setQueueTimeout(std::chrono::milliseconds timeout) {
    THRIFT_SERVER_EVENT(call.setQueueTimeout).log(*this, [timeout]() {
      return folly::dynamic::object("timeout_ms", timeout.count());
    });
    thriftConfig_.setQueueTimeout(
        folly::observer::makeStaticObserver(std::optional{timeout}),
        AttributeSource::OVERRIDE);
  }

  [[deprecated("Use addPreprocessFunc instead")]] void setIsOverloaded(
      IsOverloadedFunc isOverloaded) {
    THRIFT_SERVER_EVENT(call.setIsOverloaded).log(*this);
    isOverloaded_ = std::move(isOverloaded);
    runtimeServerActions_.setIsOverloaded = true;
    XLOG_IF(INFO, infoLoggingEnabled_) << "thrift server: isOverloaded() set.";
  }

  // Do not try to access ThreadManager in this function as
  // ThreadManagers are being deprecated from thrift server
  // e.g. don't call getThreadManager() inside this
  [[deprecated("Use addPreprocessFunc instead")]] void setPreprocess(
      PreprocessFunc preprocess) {
    THRIFT_SERVER_EVENT(call.setPreprocess).log(*this);

    preprocessFunctions_.deprecatedSet(std::move(preprocess));

    runtimeServerActions_.setPreprocess = true;
    XLOG_IF(INFO, infoLoggingEnabled_) << "setPreprocess() call";
  }

  void addPreprocessFunc(const std::string& name, PreprocessFunc preprocess) {
    THRIFT_SERVER_EVENT(call.addPreprocess).log(*this);

    preprocessFunctions_.add(name, std::move(preprocess));

    // TODO(sazonovk): Should there be a separate boolean for addPreprocess?
    runtimeServerActions_.setPreprocess = true;
    XLOG_IF(INFO, infoLoggingEnabled_) << "addPreprocessFunc() call";
  }

  /**
   * One stop solution:
   *
   * starts worker threads, enters accept loop; when
   * the accept loop exits, shuts down and joins workers.
   */
  virtual void serve();

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
  virtual void stop();

  using StopController = ThriftServerStopController;

  // Allows running the server as a Runnable thread
  void run() override { serve(); }

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
  virtual void stopListening();

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
   *    │     (setInterface)     │  │
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
  AsyncProcessorFactory& getDecoratedProcessorFactory() const;

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
   * Gets all ServiceInterceptors installed on this ThriftServer instance via
   * addModule().
   */
  const std::vector<std::shared_ptr<ServiceInterceptorBase>>&
  getServiceInterceptors() const override {
    if (auto* description = processedServiceDescription_.get()) {
      return description->modules.coalescedServiceInterceptors;
    }
    static const folly::Indestructible<
        std::vector<std::shared_ptr<ServiceInterceptorBase>>>
        kEmpty;
    return kEmpty;
  }

  /**
   * Get the InterceptorMetricCallback instance installed on this ThriftServer
   * instance
   */
  InterceptorMetricCallback& getInterceptorMetricCallback() const override {
    DCHECK(interceptorMetricCallback_);
    return *interceptorMetricCallback_;
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

 private:
  bool allowDebugInterface_{true};
  bool allowMonitoringInterface_{true};
  bool allowProfilingInterface_{true};

  void allowDebugInterface(bool value) { allowDebugInterface_ = value; }

  bool allowDebugInterface() const { return allowDebugInterface_; }

  void allowMonitoringInterface(bool value) {
    allowMonitoringInterface_ = value;
  }

  bool allowMonitoringInterface() const { return allowMonitoringInterface_; }

  void allowProfilingInterface(bool value) { allowProfilingInterface_ = value; }

  bool allowProfilingInterface() const { return allowProfilingInterface_; }

#if FOLLY_HAS_COROUTINES
  /**
   * Disable the service health poller. This should be called by Python services
   * before serve() is called if they want to manage their own health status.
   */
  void disableServiceHealthPoller() { disabledPoller_ = true; }

  /**
   * Set the service health for Python services.
   * This allows Python services to directly set their health status.
   * The service health poller must already be disabled before calling this
   * method.
   */
  void setServiceHealth(ServiceHealth health) {
    CHECK(isServiceHealthPollerDisabled())
        << "Service health poller must be disabled before setting service health. "
           "Call disableServiceHealthPoller() first.";
    cachedServiceHealth_.store(health, std::memory_order_relaxed);
  }

  /**
   * Check if the service health poller has been disabled due to Python services
   * directly setting their health status.
   */
  bool isServiceHealthPollerDisabled() const { return disabledPoller_; }
#endif

 public:
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

  static folly::observer::Observer<bool> preferPskKe();

  static folly::observer::Observer<bool> enablePresentingDelegatedCredentials();

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
      auto requestPayload =
          rocket::PayloadSerializer::getInstance()->unpack<RequestPayload>(
              stub.clonePayload(), false /* decodeMetadataUsingBinary */);
      payload_ = std::move(*requestPayload->payload);
      auto& metadata = requestPayload->metadata;
      if (metadata.otherMetadata()) {
        headers_ = std::move(*requestPayload->metadata.otherMetadata());
      }
      clientId_ = metadata.clientId().to_optional();
      tenantId_ = metadata.tenantId().to_optional();
      rpcPriority_ = metadata.priority().to_optional();

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

    const auto& tenantId() const { return tenantId_; }
    auto& tenantId() { return tenantId_; }

    const auto& rpcPriority() const { return rpcPriority_; }
    auto& rpcPriority() { return rpcPriority_; }

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
    std::optional<std::string> tenantId_;
    std::optional<RpcPriority> rpcPriority_;
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
    std::vector<InteractionInfo> interactions;
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

 public:
  struct FailureInjection {
    FailureInjection()
        : errorFraction(0), dropFraction(0), disconnectFraction(0) {}

    // Cause a fraction of requests to fail
    float errorFraction;

    // Cause a fraction of requests to be dropped (and presumably time out
    // on the client)
    float dropFraction;

    // Cause a fraction of requests to cause the channel to be disconnected,
    // possibly failing other requests as well.
    float disconnectFraction;

    bool operator==(const FailureInjection& other) const {
      return errorFraction == other.errorFraction &&
          dropFraction == other.dropFraction &&
          disconnectFraction == other.disconnectFraction;
    }

    bool operator!=(const FailureInjection& other) const {
      return !(*this == other);
    }
  };

  /**
   * Set failure injection parameters.
   */
  void setFailureInjection(FailureInjection fi) { failureInjection_.set(fi); }

 protected:
  enum class InjectedFailure { NONE, ERROR, DROP, DISCONNECT };

  class CumulativeFailureInjection {
   public:
    CumulativeFailureInjection()
        : empty_(true),
          errorThreshold_(0),
          dropThreshold_(0),
          disconnectThreshold_(0) {}

    InjectedFailure test() const;

    void set(const FailureInjection& fi);

   private:
    std::atomic<bool> empty_;
    mutable folly::SharedMutex mutex_;
    float errorThreshold_;
    float dropThreshold_;
    float disconnectThreshold_;
  };

  // Unlike FailureInjection, this is cumulative and thread-safe
  CumulativeFailureInjection failureInjection_;

  InjectedFailure maybeInjectFailure() const {
    return failureInjection_.test();
  }

 private:
  // This stores the decorator data state. DecoratorDataRuntimeSetup is
  // represents the startup state. Once the server is ready to start serving,
  // it transistions to DecoratorDataRuntime
  std::optional<server::DecoratorDataPerRequestBlueprint>
      decoratorDataPerRequestBlueprint_;

 public:
  server::DecoratorDataPerRequestBlueprint&
  getDecoratorDataPerRequestBlueprint() override;

  friend class detail::ThriftServerInternals;
};

template <typename AcceptorClass, typename SharedSSLContextManagerClass>
class ThriftAcceptorFactory : public wangle::AcceptorFactorySharedSSLContext {
 public:
  ThriftAcceptorFactory(ThriftServer* server) : server_(server) {}

  std::shared_ptr<wangle::SharedSSLContextManager> initSharedSSLContextManager()
      override {
    if constexpr (!std::is_same<SharedSSLContextManagerClass, void>::value) {
      sharedSSLContextManager_ = std::make_shared<SharedSSLContextManagerClass>(
          server_->getServerSocketConfig());
    }
    return sharedSSLContextManager_;
  }

  std::shared_ptr<wangle::Acceptor> newAcceptor(
      folly::EventBase* eventBase) override {
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

inline ThriftServerConfig& getThriftServerConfig(ThriftServer& server) {
  return server.thriftConfig_;
}

} // namespace detail

} // namespace apache::thrift

#endif // #ifndef THRIFT_SERVER_H_
