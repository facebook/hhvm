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

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>

#include <folly/Memory.h>
#include <folly/Portability.h>
#include <folly/SocketAddress.h>
#include <folly/Synchronized.h>
#include <folly/VirtualExecutor.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp/server/TServerEventHandler.h>
#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/server/AdaptiveConcurrency.h>
#include <thrift/lib/cpp2/server/CPUConcurrencyController.h>
#include <thrift/lib/cpp2/server/ControlServerInterface.h>
#include <thrift/lib/cpp2/server/MonitoringServerInterface.h>
#include <thrift/lib/cpp2/server/ResourcePool.h>
#include <thrift/lib/cpp2/server/ResourcePoolSet.h>
#include <thrift/lib/cpp2/server/SecurityServerInterface.h>
#include <thrift/lib/cpp2/server/ServerAttribute.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>
#include <thrift/lib/cpp2/server/StatusServerInterface.h>
#include <thrift/lib/cpp2/server/ThreadManagerLoggingWrapper.h>
#include <thrift/lib/cpp2/server/ThriftServerConfig.h>

namespace wangle {
class ConnectionManager;
}

namespace apache {
namespace thrift {

typedef std::function<void(
    folly::EventBase*,
    wangle::ConnectionManager*,
    std::shared_ptr<folly::AsyncTransport>,
    std::unique_ptr<folly::IOBuf>)>
    getHandlerFunc;

typedef std::function<void(
    const apache::thrift::transport::THeader*, const folly::SocketAddress*)>
    GetHeaderHandlerFunc;

using IsOverloadedFunc = folly::Function<bool(
    const transport::THeader::StringToStringMap*, const std::string*) const>;

using PreprocessFunc =
    folly::Function<PreprocessResult(const server::PreprocessParams&) const>;

template <typename T>
class ThriftServerAsyncProcessorFactory : public AsyncProcessorFactory {
 public:
  explicit ThriftServerAsyncProcessorFactory(std::shared_ptr<T> t) {
    svIf_ = t;
  }

  std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor() override {
    return std::unique_ptr<apache::thrift::AsyncProcessor>(
        new typename T::ProcessorType(svIf_.get()));
  }

  std::vector<ServiceHandlerBase*> getServiceHandlers() override {
    return {svIf_.get()};
  }

 private:
  std::shared_ptr<T> svIf_;
};
class BaseThriftServer;
namespace detail {
/**
 * Gets the server's ThriftServerConfig which contains all the static and
 * dynamic Server Attributes
 */
inline ThriftServerConfig& getThriftServerConfig(BaseThriftServer&);
} // namespace detail

/**
 *   Base class for thrift servers using cpp2 style generated code.
 */

class BaseThriftServer : public apache::thrift::concurrency::Runnable,
                         public apache::thrift::server::ServerConfigs {
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
    bool resourcePoolEnabled{false};

    bool resourcePoolEnabledGflag{false};
    bool resourcePoolDisabledGflag{false};

    bool checkComplete{false};

    std::string explain() const;
  };

  // Used to disable resource pool at run time. This
  // should only be used by thrift team.
  void runtimeDisableResourcePoolsDeprecated() {
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

  /**
   * Get the flags used to support migrations and rollouts.
   */
  RuntimeServerActions& getRuntimeServerActions() const {
    return runtimeServerActions_;
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
  // Cpp2 ProcessorFactory.
  std::shared_ptr<apache::thrift::AsyncProcessorFactory> cpp2Pfac_;

  ServerInterface* applicationServerInterface_{};

  // Explicitly set monitoring service interface handler
  std::shared_ptr<MonitoringServerInterface> monitoringServiceHandler_;

  // Explicitly set status service interface handler
  std::shared_ptr<StatusServerInterface> statusServiceHandler_;

  // Explicitly set control service interface handler
  std::shared_ptr<ControlServerInterface> controlServiceHandler_;

  // Explicitly set security service interface handler
  std::shared_ptr<SecurityServerInterface> securityServiceHandler_;

  // Server behavior to wrt header traffic
  LegacyTransport legacyTransport_{LegacyTransport::DEFAULT};

  Metadata metadata_;

  std::shared_ptr<server::TServerEventHandler> eventHandler_;
  std::vector<std::shared_ptr<server::TServerEventHandler>> eventHandlers_;

  friend ThriftServerConfig& detail::getThriftServerConfig(BaseThriftServer&);

 protected:
  ThriftServerConfig thriftConfig_;

  /**
   * In cases where multiple services are running in the same process, this
   * will be used to indicate which is the primary server.
   */
  bool isPrimaryServer_{false};

 private:
  AdaptiveConcurrencyController adaptiveConcurrencyController_;

  folly::observer::SimpleObservable<
      std::optional<CPUConcurrencyController::Config>>
      mockCPUConcurrencyControllerConfig_{std::nullopt};
  folly::observer::Observer<CPUConcurrencyController::Config>
  makeCPUConcurrencyControllerConfigInternal();
  CPUConcurrencyController cpuConcurrencyController_;

 public:
  void setAsPrimaryServer() { isPrimaryServer_ = true; }
  bool isPrimaryServer() const { return isPrimaryServer_; }

  void setMockCPUConcurrencyControllerConfig(
      CPUConcurrencyController::Config config) {
    mockCPUConcurrencyControllerConfig_.setValue(config);
  }

 protected:
  //! The server's listening addresses
  std::vector<folly::SocketAddress> addresses_;

  //! The server's listening port
  std::optional<uint16_t> port_;

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

  // Thread factory hooks if required. threadInitializer_ will be called when
  // the thread is started before it starts handling requests and
  // threadFinalizer_ will be called before the thread exits.
  std::function<void()> threadInitializer_;
  std::function<void()> threadFinalizer_;

  //! The ResourcePoolsSet used by this ThriftServer (if in ResourcePools
  //! are enabled).
  ResourcePoolSet resourcePoolSet_;

  //! Flags used to track certain actions of thrift servers to help support
  //! migrations and rollouts.
  mutable RuntimeServerActions runtimeServerActions_;

  /**
   * The thread manager used for sync calls.
   */
  mutable std::mutex threadManagerMutex_;
  std::shared_ptr<apache::thrift::concurrency::ThreadManager> threadManager_;
  // we need to make the wrapper stick to the server because the users calling
  // getThreadManager are relying on the server to maintatin the tm lifetime
  std::shared_ptr<apache::thrift::ThreadManagerLoggingWrapper>
      tmLoggingWrapper_;

  // If set, the thread factory that should be used to create worker threads.
  std::shared_ptr<concurrency::ThreadFactory> threadFactory_;

  // The default thread priority to use (only applies to SIMPLE or
  // PRIORITY_QUEUE ThreadManagerType and if no threadFactory supplied)
  std::optional<concurrency::PosixThreadFactory::THREAD_PRIORITY>
      threadPriority_;

  // Notification of various server events. Note that once observer_ has been
  // set, it cannot be set again and will remain alive for (at least) the
  // lifetime of *this.
  folly::Synchronized<std::shared_ptr<server::TServerObserver>> observer_;
  std::atomic<server::TServerObserver*> observerPtr_{nullptr};

  IsOverloadedFunc isOverloaded_;
  PreprocessFunc preprocess_;
  std::function<int64_t(const std::string&)> getLoad_;

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
    mutable std::mutex mutex_;
    float errorThreshold_;
    float dropThreshold_;
    float disconnectThreshold_;
  };

  // Unlike FailureInjection, this is cumulative and thread-safe
  CumulativeFailureInjection failureInjection_;

  InjectedFailure maybeInjectFailure() const {
    return failureInjection_.test();
  }

  // This is meant to be used internally
  // We separate setThreadManager and configureThreadManager
  // so that we can have proper logging for the former
  // These APIs will be deprecated eventually when ResourcePool
  // migration is done.
  void setThreadManagerInternal(
      std::shared_ptr<apache::thrift::concurrency::ThreadManager>
          threadManager) {
    CHECK(configMutable());
    std::lock_guard<std::mutex> lock(threadManagerMutex_);
    threadManager_ = threadManager;
    tmLoggingWrapper_ =
        std::make_shared<ThreadManagerLoggingWrapper>(threadManager_, this);
  }

  getHandlerFunc getHandler_;
  GetHeaderHandlerFunc getHeaderHandler_;

  ClientIdentityHook clientIdentityHook_;

  BaseThriftServer();

  explicit BaseThriftServer(const ThriftServerInitialConfig& initialConfig);
  ~BaseThriftServer() override {}

 public:
  std::shared_ptr<server::TServerEventHandler> getEventHandler() const {
    return eventHandler_;
  }

  /**
   * If a view of the event handlers is needed that does not need to extend
   * their lifetime beyond that of the BaseThriftServer, this method allows
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
      eventHandlers_.erase(std::find(
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

  /**
   * Indicate whether it is safe to modify the server config through setters.
   * This roughly corresponds to whether the IO thread pool could be servicing
   * requests.
   *
   * @return true if the configuration can be modified, false otherwise
   */
  bool configMutable() { return !thriftConfig_.isFrozen(); }

  /**
   * Set the default priority for CPU worker threads. This will only apply when
   * the thread manager type is SIMPLE or PRIORITY_QUEUE.
   */
  void setCPUWorkerThreadPriority(
      concurrency::PosixThreadFactory::THREAD_PRIORITY priority) {
    CHECK(configMutable());
    std::lock_guard<std::mutex> lock(threadManagerMutex_);
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
    std::lock_guard<std::mutex> lock(threadManagerMutex_);
    CHECK(!threadManager_);
    CHECK(!threadPriority_);
    threadFactory_ = std::move(threadFactory);
  }

  /**
   * Set the type of ThreadManager to use for this server.
   */
  void setThreadManagerType(ThreadManagerType threadManagerType) {
    CHECK(configMutable());
    std::lock_guard<std::mutex> lock(threadManagerMutex_);
    CHECK(!threadManager_);
    threadManagerType_ = threadManagerType;
  }

  /**
   * Set the size of thread pools when using ThreadManagerType::PRIORITY
   */
  void setThreadManagerPoolSizes(
      const std::array<size_t, concurrency::N_PRIORITIES>& poolSizes) {
    CHECK(configMutable());
    std::lock_guard<std::mutex> lock(threadManagerMutex_);
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
    std::lock_guard<std::mutex> lock(threadManagerMutex_);
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
    std::lock_guard<std::mutex> lock(threadManagerMutex_);
    CHECK(!threadManager_);
    threadInitializer_ = std::move(threadInitializer);
    threadFinalizer_ = std::move(threadFinalizer);
  }

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
  void setCPUWorkerThreadName(const std::string& cpuWorkerThreadName) {
    thriftConfig_.setCPUWorkerThreadName(
        cpuWorkerThreadName, AttributeSource::OVERRIDE);
  }

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
      runtimeDisableResourcePoolsDeprecated();
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
    runtimeDisableResourcePoolsDeprecated();
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
      runtimeDisableResourcePoolsDeprecated();
      runtimeServerActions_.userSuppliedThreadManager = true;
    }
  }

  /**
   * Get Thread Manager (for queuing mode).
   *
   * @return a shared pointer to the thread manager
   */
  std::shared_ptr<concurrency::ThreadManager> getThreadManager_deprecated()
      const override {
    std::lock_guard<std::mutex> lock(threadManagerMutex_);
    return tmLoggingWrapper_;
  }

  std::shared_ptr<folly::Executor> getThreadManager() const override {
    std::lock_guard<std::mutex> lock(threadManagerMutex_);
    return threadManager_;
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
          .sharedPtrExecutor()
          .value();
    }
    std::lock_guard<std::mutex> lock(threadManagerMutex_);
    return threadManager_;
  }

  folly::Executor::KeepAlive<> getHandlerExecutorKeepAlive() const override {
    if (!runtimeServerActions_.userSuppliedThreadManager &&
        !resourcePoolSet().empty()) {
      return resourcePoolSet()
          .resourcePool(ResourcePoolHandle::defaultAsync())
          .sharedPtrExecutor()
          .value()
          .get();
    }
    std::lock_guard<std::mutex> lock(threadManagerMutex_);
    return threadManager_.get();
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
  void setMaxConnections(uint32_t maxConnections) {
    thriftConfig_.setMaxConnections(
        folly::observer::makeStaticObserver(std::optional{maxConnections}),
        AttributeSource::OVERRIDE);
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
   * Sets the timeout for joining workers
   * @param timeout new setting for timeout for joining requests.
   */
  void setWorkersJoinTimeout(std::chrono::seconds timeout) {
    thriftConfig_.setWorkersJoinTimeout(
        std::move(timeout), AttributeSource::OVERRIDE);
  }

  /**
   * Get the timeout for joining workers.
   * @return workers joing timeout in seconds
   */
  std::chrono::seconds getWorkersJoinTimeout() const {
    return thriftConfig_.getWorkersJoinTimeout();
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

  uint64_t getMaxResponseSize() const final {
    return thriftConfig_.getMaxResponseSize().get();
  }

  void setMaxResponseSize(uint64_t size) {
    thriftConfig_.setMaxResponseSize(
        folly::observer::makeStaticObserver(std::optional{size}),
        AttributeSource::OVERRIDE);
  }

  bool getUseClientTimeout() const override {
    return thriftConfig_.getUseClientTimeout().get();
  }

  void setUseClientTimeout(bool useClientTimeout) {
    thriftConfig_.setUseClientTimeout(
        folly::observer::makeStaticObserver(std::optional{useClientTimeout}),
        AttributeSource::OVERRIDE);
  }

  // Get load of the server.
  int64_t getLoad(
      const std::string& counter = "", bool check_custom = true) const final;
  virtual std::string getLoadInfo(int64_t load) const;

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

  AdaptiveConcurrencyController& getAdaptiveConcurrencyController() final {
    return adaptiveConcurrencyController_;
  }

  const AdaptiveConcurrencyController& getAdaptiveConcurrencyController()
      const final {
    return adaptiveConcurrencyController_;
  }

  CPUConcurrencyController& getCPUConcurrencyController() final {
    return cpuConcurrencyController_;
  }

  const CPUConcurrencyController& getCPUConcurrencyController() const final {
    return cpuConcurrencyController_;
  }

  std::shared_ptr<server::TServerObserver> getObserverShared() const {
    return observer_.copy();
  }

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
  void setMaxNumPendingConnectionsPerWorker(uint32_t num) {
    thriftConfig_.setMaxNumPendingConnectionsPerWorker(
        std::move(num), AttributeSource::OVERRIDE);
  }

  /**
   * Get the number of connections dropped by the AsyncServerSocket
   */
  virtual uint64_t getNumDroppedConnections() const = 0;

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
   *  @param timeout number of milliseconds, or 0 to disable timeouts.
   */
  void setIdleTimeout(std::chrono::milliseconds timeout) {
    thriftConfig_.setIdleTimeout(std::move(timeout), AttributeSource::OVERRIDE);
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

  /**
   * Set the number of CPU (pool) threads.
   * Only valid if you do not also set a threadmanager. This controls the number
   * of normal priority threads; the Thrift thread manager can create additional
   * threads for other priorities.
   * If set to 0, the number of normal priority threads will be the same as
   * number of CPU cores.
   *
   * @param number of CPU (pool) threads
   */
  void setNumCPUWorkerThreads(size_t numCPUWorkerThreads) {
    CHECK(!threadManager_);
    thriftConfig_.setNumCPUWorkerThreads(
        std::move(numCPUWorkerThreads), AttributeSource::OVERRIDE);
  }

  /**
   * Get the number of CPU (pool) threads
   *
   * @return number of CPU (pool) threads
   */
  size_t getNumCPUWorkerThreads() const {
    return thriftConfig_.getNumCPUWorkerThreads();
  }

  bool getEnableCodel() const { return thriftConfig_.getEnableCodel().get(); }

  /**
   * Sets the main server interface that exposes user-defined methods.
   */
  void setInterface(std::shared_ptr<AsyncProcessorFactory> iface) {
    setProcessorFactory(std::move(iface));
  }

  /**
   * DEPRECATED! Use setInterface instead.
   */
  virtual void setProcessorFactory(
      std::shared_ptr<AsyncProcessorFactory> pFac) {
    CHECK(configMutable());
    cpp2Pfac_ = pFac;
    applicationServerInterface_ = nullptr;
    for (auto* serviceHandler : cpp2Pfac_->getServiceHandlers()) {
      if (auto serverInterface =
              dynamic_cast<ServerInterface*>(serviceHandler)) {
        applicationServerInterface_ = serverInterface;
        break;
      }
    }
  }

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

  size_t getNumTypedInterceptors() const final {
    return applicationServerInterface_
        ? applicationServerInterface_->getNumTypedInterceptors()
        : 0;
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

  /**
   * Set the task expire time
   *
   */
  void setTaskExpireTime(std::chrono::milliseconds timeout) {
    thriftConfig_.setTaskExpireTime(
        folly::observer::makeStaticObserver(std::optional{timeout}),
        AttributeSource::OVERRIDE);
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
  void setStreamExpireTime(std::chrono::milliseconds timeout) {
    thriftConfig_.setStreamExpireTime(
        folly::observer::makeStaticObserver(std::optional{timeout}),
        AttributeSource::OVERRIDE);
  }

  /**
   * If there is no request for the stream for the given time period, then the
   * stream will create timeout error.
   */
  std::chrono::milliseconds getStreamExpireTime() const final {
    return thriftConfig_.getStreamExpireTime().get();
  }

  /**
   * Set the time requests are allowed to stay on the queue.
   * Note, queuing is an indication that your server cannot keep
   * up with load, and realtime systems should not queue. Only
   * override this if you do heavily batched requests.
   */
  virtual void setQueueTimeout(std::chrono::milliseconds timeout) {
    thriftConfig_.setQueueTimeout(
        folly::observer::makeStaticObserver(std::optional{timeout}),
        AttributeSource::OVERRIDE);
  }

  /**
   * Set the queue timeout to processing timeout percentage. This is to ensure
   * server can load shedding effectively when service is hosting many clients
   * that has different client timeout. If set, Thrift Server will choose the
   * high queue Timeout from this setting and queue timeout from
   * setQueueTimeout() above. Also, notes if client side set queue_timeout
   * explicitly, then server side queuetimeout setting will be ignored.
   */
  virtual void setQueueTimeoutPct(uint32_t queueTimeoutPct) {
    thriftConfig_.setQueueTimeoutPct(
        folly::observer::makeStaticObserver(std::optional{queueTimeoutPct}),
        AttributeSource::OVERRIDE);
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
      folly::observer::Observer<std::chrono::nanoseconds> timeout) {
    thriftConfig_.setSocketQueueTimeout(
        folly::observer::makeObserver(
            [=]() -> std::optional<std::chrono::milliseconds> {
              return std::chrono::duration_cast<std::chrono::milliseconds>(
                  **timeout);
            }),
        AttributeSource::OVERRIDE);
  }

  void setSocketQueueTimeout(
      folly::Optional<std::chrono::nanoseconds> timeout) {
    if (timeout) {
      thriftConfig_.setSocketQueueTimeout(
          folly::observer::makeStaticObserver(std::optional{
              std::chrono::duration_cast<std::chrono::milliseconds>(*timeout)}),
          AttributeSource::OVERRIDE);
    }
  }

  void setSocketQueueTimeout(std::chrono::nanoseconds timeout) {
    thriftConfig_.setSocketQueueTimeout(
        folly::observer::makeStaticObserver(std::optional{
            std::chrono::duration_cast<std::chrono::milliseconds>(timeout)}),
        AttributeSource::OVERRIDE);
  }

  /**
   * How long a socket with outbound data will tolerate read inactivity from a
   * client. Clients must read data from their end of the connection before this
   * period expires or the server will drop the connection. The amount of data
   * read by the client is irrelevant. Zero disables the timeout.
   */
  void setSocketWriteTimeout(std::chrono::milliseconds timeout) {
    thriftConfig_.setSocketWriteTimeout(
        folly::observer::makeStaticObserver(std::optional{timeout}),
        AttributeSource::OVERRIDE);
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

  /**
   * Set the listen backlog. Refer to the comment on listenBacklog_ member for
   * details.
   */
  void setListenBacklog(int listenBacklog) {
    thriftConfig_.setListenBacklog(
        std::move(listenBacklog), AttributeSource::OVERRIDE);
  }

  /**
   * Get the listen backlog.
   *
   * @return listen backlog.
   */
  int getListenBacklog() const { return thriftConfig_.getListenBacklog(); }

  // Do not try to access ThreadManager in this function as
  // ThreadManagers are being deprecated from thrift server
  // e.g. don't call getThreadManager() inside this
  [[deprecated("Use setPreprocess instead")]] virtual void setIsOverloaded(
      IsOverloadedFunc isOverloaded) {
    isOverloaded_ = std::move(isOverloaded);
    runtimeServerActions_.setIsOverloaded = true;
    LOG(INFO) << "thrift server: isOverloaded() set.";
  }

  // Do not try to access ThreadManager in this function as
  // ThreadManagers are being deprecated from thrift server
  // e.g. don't call getThreadManager() inside this
  virtual void setPreprocess(PreprocessFunc preprocess) {
    preprocess_ = std::move(preprocess);
    runtimeServerActions_.setPreprocess = true;
    LOG(INFO) << "thrift server: preprocess() set.";
  }

  void setMethodsBypassMaxRequestsLimit(
      const std::vector<std::string>& methods) {
    thriftConfig_.setMethodsBypassMaxRequestsLimit(
        methods, AttributeSource::OVERRIDE);
  }

  const folly::sorted_vector_set<std::string>&
  getMethodsBypassMaxRequestsLimit() const {
    return thriftConfig_.getMethodsBypassMaxRequestsLimit();
  }

  void setGetLoad(std::function<int64_t(const std::string&)> getLoad) {
    getLoad_ = getLoad;
  }

  std::function<int64_t(const std::string&)> getGetLoad() const {
    return getLoad_;
  }

  /**
   * Set failure injection parameters.
   */
  virtual void setFailureInjection(FailureInjection fi) {
    failureInjection_.set(fi);
  }

  void setGetHandler(getHandlerFunc func) { getHandler_ = func; }

  getHandlerFunc getGetHandler() { return getHandler_; }

  void setGetHeaderHandler(GetHeaderHandlerFunc func) {
    getHeaderHandler_ = func;
  }

  GetHeaderHandlerFunc getGetHeaderHandler() { return getHeaderHandler_; }

  /**
   * Set the client identity hook for the server, which will be called in
   * Cpp2ConnContext(). It can be used to cache client identities for each
   * connection. They can be retrieved with Cpp2ConnContext::getPeerIdentities.
   */
  void setClientIdentityHook(ClientIdentityHook func) {
    clientIdentityHook_ = func;
  }

  ClientIdentityHook getClientIdentityHook() { return clientIdentityHook_; }

  virtual void serve() = 0;

  virtual void stop() = 0;

  // This API is intended to stop listening on the server
  // socket and stop accepting new connection first while
  // still letting the established connections to be
  // processed on the server.
  virtual void stopListening() = 0;

  // Allows running the server as a Runnable thread
  void run() override { serve(); }

  /**
   * Return the maximum memory usage by each debug payload.
   */
  uint64_t getMaxDebugPayloadMemoryPerRequest() const {
    return thriftConfig_.getMaxDebugPayloadMemoryPerRequest();
  }

  /**
   * Set the maximum memory usage by each debug payload.
   */
  void setMaxDebugPayloadMemoryPerRequest(uint64_t limit) {
    thriftConfig_.setMaxDebugPayloadMemoryPerRequest(
        std::move(limit), AttributeSource::OVERRIDE);
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
  void setMaxDebugPayloadMemoryPerWorker(uint64_t limit) {
    // setStaticAttribute(
    //     maxDebugPayloadMemoryPerWorker_, std::move(limit), source);
    thriftConfig_.setMaxDebugPayloadMemoryPerWorker(
        std::move(limit), AttributeSource::OVERRIDE);
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
  void setMaxFinishedDebugPayloadsPerWorker(uint16_t limit) {
    thriftConfig_.setMaxFinishedDebugPayloadsPerWorker(
        std::move(limit), AttributeSource::OVERRIDE);
  }

  /**
   * Set write batching interval
   */
  void setWriteBatchingInterval(std::chrono::milliseconds interval) {
    thriftConfig_.setWriteBatchingInterval(
        folly::observer::makeStaticObserver(std::optional{interval}),
        AttributeSource::OVERRIDE);
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
  void setWriteBatchingSize(size_t batchingSize) {
    thriftConfig_.setWriteBatchingSize(
        folly::observer::makeStaticObserver(std::optional{batchingSize}),
        AttributeSource::OVERRIDE);
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
  void setWriteBatchingByteSize(size_t batchingByteSize) {
    thriftConfig_.setWriteBatchingByteSize(
        folly::observer::makeStaticObserver(std::optional{batchingByteSize}),
        AttributeSource::OVERRIDE);
  }

  /**
   * Get write batching byte size
   */
  size_t getWriteBatchingByteSize() const {
    return thriftConfig_.getWriteBatchingByteSize().get();
  }

  const Metadata& metadata() const { return metadata_; }

  Metadata& metadata() { return metadata_; }

  /**
   * Ingress memory is the total memory used for receiving inflight requests.
   * If the memory limit is hit, the connection along with the violating request
   * will be closed
   */
  void setIngressMemoryLimit(size_t ingressMemoryLimit) {
    thriftConfig_.setIngressMemoryLimit(
        folly::observer::makeStaticObserver(std::optional{ingressMemoryLimit}),
        AttributeSource::OVERRIDE);
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
  void setEgressMemoryLimit(size_t max) {
    thriftConfig_.setEgressMemoryLimit(
        folly::observer::makeStaticObserver(std::optional{max}),
        AttributeSource::OVERRIDE);
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
      size_t minPayloadSizeToEnforceIngressMemoryLimit) {
    thriftConfig_.setMinPayloadSizeToEnforceIngressMemoryLimit(
        folly::observer::makeStaticObserver(
            std::optional{minPayloadSizeToEnforceIngressMemoryLimit}),
        AttributeSource::OVERRIDE);
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
  void setEgressBufferBackpressureThreshold(size_t thresholdInBytes) {
    thriftConfig_.setEgressBufferBackpressureThreshold(
        folly::observer::makeStaticObserver(std::optional{thresholdInBytes}),
        AttributeSource::OVERRIDE);
  }

  double getEgressBufferRecoveryFactor() const {
    return thriftConfig_.getEgressBufferRecoveryFactor().get();
  }

  /**
   * When egress buffer backpressure is enabled, resume normal operation once
   * egress buffer size falls below this factor of the threshold.
   */
  void setEgressBufferRecoveryFactor(double recoveryFactor) {
    thriftConfig_.setEgressBufferRecoveryFactor(
        folly::observer::makeStaticObserver(std::optional{recoveryFactor}),
        AttributeSource::OVERRIDE);
  }

  folly::observer::Observer<std::chrono::milliseconds>
  getPolledServiceHealthLivenessObserver() const {
    return thriftConfig_.getPolledServiceHealthLiveness().getObserver();
  }

  void setPolledServiceHealthLiveness(std::chrono::milliseconds liveness) {
    thriftConfig_.setPolledServiceHealthLiveness(
        folly::observer::makeStaticObserver(std::optional{liveness}),
        AttributeSource::OVERRIDE);
  }

  const auto& adaptiveConcurrencyController() const {
    return adaptiveConcurrencyController_;
  }

  // Define Server behavior to allow or reject header traffic
  void setLegacyTransport(LegacyTransport value) { legacyTransport_ = value; }

  LegacyTransport getLegacyTransport() const { return legacyTransport_; }

  const folly::SocketOptionMap& getPerConnectionSocketOptions() const {
    return thriftConfig_.getPerConnectionSocketOptions().get();
  }
  void setPerConnectionSocketOptions(folly::SocketOptionMap options) {
    thriftConfig_.setPerConnectionSocketOptions(
        folly::observer::makeStaticObserver(std::optional{options}),
        AttributeSource::OVERRIDE);
  }

  bool resourcePoolEnabled() const override {
    return getRuntimeServerActions().resourcePoolEnabled;
  }

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

  const ThriftServerConfig& getThriftServerConfig() const {
    return thriftConfig_;
  }
};

namespace detail {
inline ThriftServerConfig& getThriftServerConfig(BaseThriftServer& server) {
  return server.thriftConfig_;
}
} // namespace detail

} // namespace thrift
} // namespace apache
