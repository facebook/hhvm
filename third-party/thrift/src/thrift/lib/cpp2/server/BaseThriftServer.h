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
#include <folly/SharedMutex.h>
#include <folly/SocketAddress.h>
#include <folly/Synchronized.h>
#include <folly/VirtualExecutor.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp/server/TServerEventHandler.h>
#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/server/ControlServerInterface.h>
#include <thrift/lib/cpp2/server/MonitoringServerInterface.h>
#include <thrift/lib/cpp2/server/SecurityServerInterface.h>
#include <thrift/lib/cpp2/server/ServerAttribute.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>
#include <thrift/lib/cpp2/server/ServerModule.h>
#include <thrift/lib/cpp2/server/StatusServerInterface.h>
#include <thrift/lib/cpp2/server/ThriftServerConfig.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/AllocatingParserStrategy.h>

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
ThriftServerConfig& getThriftServerConfig(BaseThriftServer&);
} // namespace detail

/**
 *   Base class for thrift servers using cpp2 style generated code.
 */

class BaseThriftServer : public apache::thrift::concurrency::Runnable,
                         public apache::thrift::server::ServerConfigs {
 public:
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

  /**
   * Get the flags used to support migrations and rollouts.
   */
  RuntimeServerActions& getRuntimeServerActions() const {
    return runtimeServerActions_;
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

  std::shared_ptr<server::TServerEventHandler> eventHandler_;
  std::vector<std::shared_ptr<server::TServerEventHandler>> eventHandlers_;

  // TODO: T176242251 we use unique_ptr and just pass raw pointer / reference in
  // rocket's stack. If the object is owned by ThriftServer, then we know it
  // will outlive every RocketServerConnection (and related) objects.
  std::shared_ptr<rocket::ParserAllocatorType> customAllocatorForParser_{
      nullptr};

  friend ThriftServerConfig& detail::getThriftServerConfig(BaseThriftServer&);

 protected:
  ThriftServerConfig thriftConfig_;

  /**
   * In cases where multiple services are running in the same process, this
   * will be used to indicate which is the primary server.
   */
  bool isPrimaryServer_{false};

 public:
  void setAsPrimaryServer() { isPrimaryServer_ = true; }
  bool isPrimaryServer() const { return isPrimaryServer_; }

 protected:
  //! The server's listening addresses
  std::vector<folly::SocketAddress> addresses_;

  //! The server's listening port
  std::optional<uint16_t> port_;

  //! Flags used to track certain actions of thrift servers to help support
  //! migrations and rollouts.
  mutable RuntimeServerActions runtimeServerActions_;

  // Notification of various server events. Note that once observer_ has been
  // set, it cannot be set again and will remain alive for (at least) the
  // lifetime of *this.
  folly::Synchronized<std::shared_ptr<server::TServerObserver>> observer_;
  std::atomic<server::TServerObserver*> observerPtr_{nullptr};

  PreprocessFunc preprocess_;
  std::function<int64_t(const std::string&)> getLoad_;

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

  void removeServerEventHandler(
      std::shared_ptr<server::TServerEventHandler> eventHandler) {
    eventHandlers_.erase(
        std::remove(eventHandlers_.begin(), eventHandlers_.end(), eventHandler),
        eventHandlers_.end());
  }

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
   * BaseThriftServer will take over the ownership
   */
  void setCustomAllocatorForParser(
      std::shared_ptr<rocket::ParserAllocatorType> customParserAllocator) {
    customAllocatorForParser_ = std::move(customParserAllocator);
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
  void setMaxNumPendingConnectionsPerWorker(
      uint32_t num, AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setMaxNumPendingConnectionsPerWorker(std::move(num), source);
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
          folly::observer::makeStaticObserver(std::optional{
              std::chrono::duration_cast<std::chrono::milliseconds>(*timeout)}),
          source);
    }
  }

  void setSocketQueueTimeout(
      std::chrono::nanoseconds timeout,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setSocketQueueTimeout(
        folly::observer::makeStaticObserver(std::optional{
            std::chrono::duration_cast<std::chrono::milliseconds>(timeout)}),
        source);
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

  // Do not try to access ThreadManager in this function as
  // ThreadManagers are being deprecated from thrift server
  // e.g. don't call getThreadManager() inside this
  virtual void setPreprocess(PreprocessFunc preprocess) {
    preprocess_ = std::move(preprocess);
    runtimeServerActions_.setPreprocess = true;
    LOG(INFO) << "thrift server: preprocess() set.";
  }

  void setMethodsBypassMaxRequestsLimit(
      const std::vector<std::string>& methods,
      AttributeSource source = AttributeSource::OVERRIDE) {
    thriftConfig_.setMethodsBypassMaxRequestsLimit(methods, source);
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

  const ThriftServerConfig& getThriftServerConfig() const {
    return thriftConfig_;
  }

 protected:
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
  };
  static ProcessedModuleSet processModulesSpecification(ModulesSpecification&&);

 public:
  void addModule(std::unique_ptr<ServerModule> module) {
    CHECK(configMutable());
    auto name = module->getName();
    if (unprocessedModulesSpecification_.names.count(name)) {
      throw std::invalid_argument("Duplicate module name");
    }
    unprocessedModulesSpecification_.infos.emplace_back(
        ModulesSpecification::Info{std::move(module), std::move(name)});
  }
};

namespace detail {
inline ThriftServerConfig& getThriftServerConfig(BaseThriftServer& server) {
  return server.thriftConfig_;
}
} // namespace detail

} // namespace thrift
} // namespace apache
