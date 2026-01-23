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

#include <signal.h>

#include <folly/io/async/AsyncServerSocket.h>
#include <thrift/lib/cpp2/server/IOUringUtil.h>

#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/ThriftServerInternals.h>

#include <iostream>
#include <memory>
#include <random>
#include <utility>
#include <variant>

#include <glog/logging.h>
#include <folly/Memory.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Collect.h>
#include <folly/coro/CurrentExecutor.h>
#include <folly/coro/Invoke.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/IOThreadPoolDeadlockDetectorObserver.h>
#include <folly/executors/thread_factory/InitThreadFactory.h>
#include <folly/executors/thread_factory/NamedThreadFactory.h>
#include <folly/executors/thread_factory/PriorityThreadFactory.h>
#include <folly/io/GlobalShutdownSocketSet.h>
#include <folly/portability/Sockets.h>
#include <folly/system/HardwareConcurrency.h>
#include <folly/system/Pid.h>
#include <thrift/lib/cpp/concurrency/InitThreadFactory.h>
#include <thrift/lib/cpp/concurrency/PosixThreadFactory.h>
#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/MultiplexAsyncProcessor.h>
#include <thrift/lib/cpp2/runtime/Init.h>
#include <thrift/lib/cpp2/server/Cpp2Connection.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ExecutorToThreadManagerAdaptor.h>
#include <thrift/lib/cpp2/server/LegacyHeaderRoutingHandler.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>
#include <thrift/lib/cpp2/server/ServerInstrumentation.h>
#include <thrift/lib/cpp2/server/StandardConcurrencyController.h>
#include <thrift/lib/cpp2/server/TMConcurrencyController.h>
#include <thrift/lib/cpp2/server/ThriftProcessor.h>
#include <thrift/lib/cpp2/server/metrics/PendingConnectionsMetrics.h>
#include <thrift/lib/cpp2/transport/core/ManagedConnectionIf.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketRoutingHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <wangle/acceptor/FizzConfigUtil.h>
#include <wangle/acceptor/SharedSSLContextManager.h>

using namespace std::literals::chrono_literals;

FOLLY_GFLAGS_DEFINE_bool(
    thrift_abort_if_exceeds_shutdown_deadline,
    true,
    "Abort the server if failed to drain active requests within deadline");

FOLLY_GFLAGS_DEFINE_string(
    thrift_ssl_policy, "required", "SSL required / permitted / disabled");

FOLLY_GFLAGS_DEFINE_string(
    service_identity,
    "",
    "The name of the service. Associates the service with ACLs and keys");
FOLLY_GFLAGS_DEFINE_bool(
    disable_legacy_header_routing_handler,
    false,
    "Do not register a TransportRoutingHandler that can handle the legacy transports: header, framed, and unframed (default: false)");

THRIFT_FLAG_DEFINE_bool(server_enable_stoptls, false);
THRIFT_FLAG_DEFINE_bool(server_enable_stoptlsv2, false);

THRIFT_FLAG_DEFINE_bool(dump_snapshot_on_long_shutdown, true);

THRIFT_FLAG_DEFINE_bool(server_check_unimplemented_extra_interfaces, true);

THRIFT_FLAG_DEFINE_bool(enable_on_stop_serving, true);

THRIFT_FLAG_DEFINE_bool(enable_io_queue_lag_detection, true);

THRIFT_FLAG_DEFINE_bool(fizz_server_enable_hybrid_kex, false);

THRIFT_FLAG_DEFINE_bool(server_fizz_enable_aegis, false);
THRIFT_FLAG_DEFINE_bool(server_fizz_prefer_psk_ke, false);
THRIFT_FLAG_DEFINE_bool(server_fizz_enable_receiving_dc, false);
THRIFT_FLAG_DEFINE_bool(server_fizz_enable_presenting_dc, false);
THRIFT_FLAG_DEFINE_bool(default_sync_max_requests_to_concurrency_limit, false);
THRIFT_FLAG_DEFINE_bool(default_sync_max_qps_to_execution_rate, false);

namespace apache::thrift::detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(
    apache::thrift::ThriftServer::DumpSnapshotOnLongShutdownResult,
    dumpSnapshotOnLongShutdown) {
  return {folly::makeSemiFuture(folly::unit), 0ms};
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    apache::thrift::ThriftServer::ExtraInterfaces,
    createDefaultExtraInterfaces) {
  return {
      nullptr /* monitoring */,
      nullptr /* status */,
      nullptr /* control */,
      nullptr /* security */};
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    ThriftServer::UnimplementedExtraInterfacesResult,
    serviceHasUnimplementedExtraInterfaces,
    AsyncProcessorFactory& /* service */) {
  return ThriftServer::UnimplementedExtraInterfacesResult::UNRECOGNIZED;
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    folly::observer::Observer<AdaptiveConcurrencyController::Config>,
    makeAdaptiveConcurrencyConfig) {
  return folly::observer::makeStaticObserver(
      AdaptiveConcurrencyController::Config{});
}
} // namespace apache::thrift::detail

namespace {

[[noreturn]] void try_quick_exit(int code) {
#if defined(_GLIBCXX_HAVE_AT_QUICK_EXIT)
  std::quick_exit(code);
#else
  std::exit(code);
#endif
}

} // namespace

namespace apache::thrift {

using namespace apache::thrift::server;
using namespace std;
using apache::thrift::concurrency::PriorityThreadManager;
using apache::thrift::concurrency::Runnable;
using apache::thrift::concurrency::ThreadManager;
using wangle::TLSCredProcessor;

namespace {

folly::Synchronized<std::vector<ThriftServer::IOObserverFactory>>
    ioObserverFactories{};

/**
 * Multiplexes the user-service (set via setInterface) with the
 * monitoring interface (set via setMonitoringInterface).
 */
std::unique_ptr<AsyncProcessorFactory> createDecoratedProcessorFactory(
    std::shared_ptr<AsyncProcessorFactory> processorFactory,
    std::shared_ptr<StatusServerInterface> statusProcessorFactory,
    std::shared_ptr<MonitoringServerInterface> monitoringProcessorFactory,
    std::shared_ptr<ControlServerInterface> controlProcessorFactory,
    std::shared_ptr<SecurityServerInterface> securityProcessorFactory,
    bool shouldCheckForUnimplementedExtraInterfaces) {
  std::vector<std::shared_ptr<AsyncProcessorFactory>> servicesToMultiplex;
  CHECK(processorFactory != nullptr);
  if (statusProcessorFactory != nullptr) {
    servicesToMultiplex.emplace_back(std::move(statusProcessorFactory));
  }
  if (monitoringProcessorFactory != nullptr) {
    servicesToMultiplex.emplace_back(std::move(monitoringProcessorFactory));
  }
  if (controlProcessorFactory != nullptr) {
    servicesToMultiplex.emplace_back(std::move(controlProcessorFactory));
  }
  if (securityProcessorFactory != nullptr) {
    servicesToMultiplex.emplace_back(std::move(securityProcessorFactory));
  }

  const bool shouldPlaceExtraInterfacesInFront =
      shouldCheckForUnimplementedExtraInterfaces &&
      apache::thrift::detail::serviceHasUnimplementedExtraInterfaces(
          *processorFactory) ==
          ThriftServer::UnimplementedExtraInterfacesResult::UNIMPLEMENTED;
  auto userServicePosition = shouldPlaceExtraInterfacesInFront
      ? servicesToMultiplex.end()
      : servicesToMultiplex.begin();
  servicesToMultiplex.insert(userServicePosition, std::move(processorFactory));

  return std::make_unique<MultiplexAsyncProcessorFactory>(
      std::move(servicesToMultiplex));
}
} // namespace

// HACK: To avoid circular header includes, we define these in ThriftServer.h
// instead of AsyncProcessor.h

#if FOLLY_HAS_COROUTINES
folly::coro::CancellableAsyncScope* ServiceHandlerBase::getAsyncScope() {
  return server_->getAsyncScope();
}
#endif

void ServiceHandlerBase::attachServer(ThriftServer& server) {
  server_ = &server;
  serverStopController_.lock()->emplace(server.getStopController());
}

void ServiceHandlerBase::detachServer() {
  server_ = nullptr;
  serverStopController_.lock()->reset();
}

void ServiceHandlerBase::shutdownServer() {
  // shutdownServer should be idempotent -- this means that it can race with
  // detachServer. Thus we should sychronize access to it.
  serverStopController_.withLock([](auto& stopController) {
    if (!stopController.has_value()) {
      return;
    }
    if (auto lockedPtr = stopController->lock()) {
      lockedPtr->stop();
    }
  });
}

TLSCredentialWatcher::TLSCredentialWatcher(ThriftServer* server)
    : credProcessor_() {
  credProcessor_.addCertCallback([server] { server->updateTLSCert(); });
  credProcessor_.addTicketCallback([server](wangle::TLSTicketKeySeeds seeds) {
    server->updateTicketSeeds(std::move(seeds));
  });
}

wangle::TLSTicketKeySeeds TLSCredentialWatcher::initInMemoryTicketSeeds(
    ThriftServer* server) {
  inMemoryTicketProcessor_ = wangle::TLSInMemoryTicketProcessor(
      {[server](wangle::TLSTicketKeySeeds seeds) {
        server->updateTicketSeeds(std::move(seeds));
      }});
  return inMemoryTicketProcessor_->initInMemoryTicketSeeds();
}

ThriftServer::ThriftServer()
    : thriftConfig_(),
      adaptiveConcurrencyController_{
          apache::thrift::detail::makeAdaptiveConcurrencyConfig(),
          thriftConfig_.getMaxRequests().getObserver(),
          detail::getThriftServerConfig(*this)},
      cpuConcurrencyController_{std::make_shared<CPUConcurrencyController>(
          makeCPUConcurrencyControllerConfigInternal(),
          *this,
          detail::getThriftServerConfig(*this))},
      addresses_(1),
      wShutdownSocketSet_(folly::tryGetShutdownSocketSet()),
      lastRequestTime_(
          std::chrono::steady_clock::now().time_since_epoch().count()) {
  tracker_.emplace(instrumentation::kThriftServerTrackerKey, *this);
  initializeDefaults();
}

void ThriftServer::initializeDefaults() {
  if (FLAGS_thrift_ssl_policy == "required") {
    sslPolicy_ = SSLPolicy::REQUIRED;
  } else if (FLAGS_thrift_ssl_policy == "permitted") {
    sslPolicy_ = SSLPolicy::PERMITTED;
  } else if (FLAGS_thrift_ssl_policy == "disabled") {
    sslPolicy_ = SSLPolicy::DISABLED;
  }
  metadata().wrapper = "ThriftServer-cpp";
  auto extraInterfaces = apache::thrift::detail::createDefaultExtraInterfaces();

  // status and monitoring methods should bypass request limit by default

  folly::sorted_vector_set<std::string> methodsBypassMaxRequestsLimit;
  if (extraInterfaces.monitoring) {
    auto monitoringInterfaceMethods =
        extraInterfaces.monitoring->createMethodMetadata();
    auto monitoringMethodsMetadataMap =
        std::get_if<AsyncProcessorFactory::MethodMetadataMap>(
            &monitoringInterfaceMethods);
    CHECK(monitoringMethodsMetadataMap != nullptr)
        << "WildcardMethodMetadataMap is not allowed for the monitoring interface";
    for (const auto& [method, _] : *monitoringMethodsMetadataMap) {
      methodsBypassMaxRequestsLimit.insert(method);
    }
  }
  if (extraInterfaces.status) {
    auto statusInterfaceMethods =
        extraInterfaces.status->createMethodMetadata();
    auto statusMethodsMetadataMethods =
        std::get_if<AsyncProcessorFactory::MethodMetadataMap>(
            &statusInterfaceMethods);
    CHECK(statusMethodsMetadataMethods != nullptr)
        << "WildcardMethodMetadataMap is not allowed for the status interface";
    for (const auto& [method, _] : *statusMethodsMetadataMethods) {
      methodsBypassMaxRequestsLimit.insert(method);
    }
  }
  setInternalMethods(
      std::unordered_set<std::string>(
          methodsBypassMaxRequestsLimit.begin(),
          methodsBypassMaxRequestsLimit.end()));
  thriftConfig_.methodsBypassMaxRequestsLimit_.setDefault(
      std::move(methodsBypassMaxRequestsLimit));

  setMonitoringInterface(std::move(extraInterfaces.monitoring));
  setStatusInterface(std::move(extraInterfaces.status));
  setControlInterface(std::move(extraInterfaces.control));
  setSecurityInterface(std::move(extraInterfaces.security));
  getAdaptiveConcurrencyController().setConfigUpdateCallback(
      [this](auto snapshot) {
        if (snapshot->isEnabled()) {
          THRIFT_SERVER_EVENT(ACC_enabled).log(*this);
        }
      });

  for (const auto& initializer :
       apache::thrift::runtime::getGlobalServerInitializers()) {
    initializer(*this);
  }
}

std::unique_ptr<RequestPileInterface> ThriftServer::makeStandardRequestPile(
    RoundRobinRequestPile::Options options) {
  if (runtimeServerActions_.interactionInService) {
    options = RoundRobinRequestPile::addInternalPriorities(std::move(options));
  }
  return std::make_unique<RoundRobinRequestPile>(std::move(options));
}

ThriftServer::~ThriftServer() {
  tracker_.reset();

  SCOPE_EXIT {
    stopController_.join();
  };

  if (stopWorkersOnStopListening_) {
    // Everything is already taken care of.
    return;
  }
  // If the flag is false, neither i/o nor CPU workers are stopped at this
  // point. Stop them now.
  if (!joinRequestsWhenServerStops_) {
    stopAcceptingAndJoinOutstandingRequests();
  }
  stopCPUWorkers();
  stopWorkers();
}

void ThriftServer::setInterface(std::shared_ptr<AsyncProcessorFactory> iface) {
  CHECK(configMutable());
  if (iface) {
    runtimeServerActions_.isProcessorFactoryThriftGenerated =
        iface->isThriftGenerated();
  }
  cpp2Pfac_ = iface;
  applicationServerInterface_ = nullptr;
  for (auto* serviceHandler : cpp2Pfac_->getServiceHandlers()) {
    if (auto serverInterface = dynamic_cast<ServerInterface*>(serviceHandler)) {
      applicationServerInterface_ = serverInterface;
      break;
    }
  }
  thriftProcessor_.reset(new ThriftProcessor(*this));
}

std::unique_ptr<AsyncProcessor>
ThriftServer::getDecoratedProcessorWithoutEventHandlers() const {
  return static_cast<MultiplexAsyncProcessorFactory&>(
             getDecoratedProcessorFactory())
      .getProcessorWithUnderlyingModifications(
          [](AsyncProcessor& processor) { processor.clearEventHandlers(); });
}

void ThriftServer::useExistingSocket(
    folly::AsyncServerSocket::UniquePtr socket) {
  socket_ = std::move(socket);
}

void ThriftServer::useExistingSockets(const std::vector<int>& socketFds) {
  folly::AsyncServerSocket::UniquePtr socket(new folly::AsyncServerSocket);
  std::vector<folly::NetworkSocket> sockets;
  sockets.reserve(socketFds.size());
  for (auto s : socketFds) {
    sockets.push_back(folly::NetworkSocket::fromFd(s));
  }
  socket->useExistingSockets(sockets);
  useExistingSocket(std::move(socket));
}

void ThriftServer::useExistingSocket(int socket) {
  useExistingSockets({socket});
}

std::vector<int> ThriftServer::getListenSockets() const {
  std::vector<int> sockets;
  for (const auto& socket : getSockets()) {
    auto newsockets = socket->getNetworkSockets();
    sockets.reserve(sockets.size() + newsockets.size());
    for (auto s : newsockets) {
      sockets.push_back(s.toFd());
    }
  }
  return sockets;
}

int ThriftServer::getListenSocket() const {
  std::vector<int> sockets = getListenSockets();
  if (sockets.size() == 0) {
    return -1;
  }

  CHECK(sockets.size() == 1);
  return sockets[0];
}

folly::EventBaseManager* ThriftServer::getEventBaseManager() {
  return eventBaseManager_;
}

ThriftServer::IdleServerAction::IdleServerAction(
    ThriftServer& server,
    folly::HHWheelTimer& timer,
    std::chrono::milliseconds timeout)
    : server_(server), timer_(timer), timeout_(timeout) {
  timer_.scheduleTimeout(this, timeout_);
}

void ThriftServer::IdleServerAction::timeoutExpired() noexcept {
  try {
    const auto lastRequestTime = server_.lastRequestTime();
    const auto elapsed = std::chrono::steady_clock::now() - lastRequestTime;
    if (elapsed >= timeout_) {
      XLOG(INFO) << "shutting down server due to inactivity after "
                 << std::chrono::duration_cast<std::chrono::milliseconds>(
                        elapsed)
                        .count()
                 << "ms";
      server_.stop();
      return;
    }

    timer_.scheduleTimeout(this, timeout_);
  } catch (const std::exception& e) {
    XLOG(ERR) << e.what();
  }
}

std::chrono::steady_clock::time_point ThriftServer::lastRequestTime()
    const noexcept {
  return std::chrono::steady_clock::time_point(
      std::chrono::steady_clock::duration(
          lastRequestTime_.load(std::memory_order_relaxed)));
}

void ThriftServer::touchRequestTimestamp() noexcept {
  if (idleServer_.has_value()) {
    lastRequestTime_.store(
        std::chrono::steady_clock::now().time_since_epoch().count(),
        std::memory_order_relaxed);
  }
}

void ThriftServer::configureIOUring() {
#if FOLLY_HAS_LIBURING
  if (preferIoUring_) {
    XLOG_IF(INFO, infoLoggingEnabled_) << "Preferring io_uring";
    auto b = io_uring_util::validateExecutorSupportsIOUring(ioThreadPool_);

    if (!b) {
      if (!useDefaultIoUringExecutor_) {
        XLOG_IF(INFO, infoLoggingEnabled_)
            << "Configured IOThreadPoolExecutor does not support io_uring, but default not selected. epoll will be used";
        usingIoUring_ = false;
        return;
      }

      XLOG_IF(INFO, infoLoggingEnabled_)
          << "Configured IOThreadPoolExecutor does not support io_uring, "
             "configuring default io_uring IOThreadPoolExecutor pool";
      ioThreadPool_ = io_uring_util::getDefaultIOUringExecutor(
          THRIFT_FLAG(enable_io_queue_lag_detection));
      usingIoUring_ = true;
    } else {
      XLOG_IF(INFO, infoLoggingEnabled_)
          << "Configured IOThreadPoolExecutor supports io_uring";
      usingIoUring_ = true;
    }
  }
#endif
}

class ThriftServer::ConnectionEventCallback
    : public folly::AsyncServerSocket::ConnectionEventCallback {
 public:
  explicit ConnectionEventCallback(const ThriftServer& thriftServer)
      : thriftServer_(thriftServer),
        pendingConnectionsMetrics_(setupPendingConnectionsMetrics()) {}

  void onConnectionAccepted(
      const folly::NetworkSocket,
      const folly::SocketAddress&) noexcept override {}

  void onConnectionAcceptError(const int err) noexcept override {
    THRIFT_CONNECTION_EVENT(server_socket_connection_accept_error)
        .log(thriftServer_, {}, [&] {
          folly::dynamic metadata = folly::dynamic::object;
          metadata["errno"] = err;
          metadata["errstr"] = folly::errnoStr(err);
          return metadata;
        });
  }

  void onConnectionDropped(
      const folly::NetworkSocket /* socket */,
      const folly::SocketAddress& clientAddr,
      const std::string& errorMsg) noexcept override {
    THRIFT_CONNECTION_EVENT(server_socket_connection_dropped)
        .log(thriftServer_, clientAddr, [&] {
          folly::dynamic metadata = folly::dynamic::object;
          metadata["error_msg"] = errorMsg;
          return metadata;
        });
    if (pendingConnectionsMetrics_) {
      pendingConnectionsMetrics_->onConnectionDropped(errorMsg);
    }
  }

  void onConnectionEnqueuedForAcceptorCallback(
      const folly::NetworkSocket,
      const folly::SocketAddress& clientAddr) noexcept override {
    // Increment counters prior to logging Scuba events to mitigate the risk of
    // race conditions between increment and decrement operations.
    if (pendingConnectionsMetrics_) {
      pendingConnectionsMetrics_->onConnectionEnqueuedToIoWorker();
    }

    THRIFT_CONNECTION_EVENT(connection_enqueued_acceptor)
        .log(thriftServer_, clientAddr);
  }

  void onConnectionDequeuedByAcceptorCallback(
      const folly::NetworkSocket,
      const folly::SocketAddress& clientAddr) noexcept override {
    // Increment counters prior to logging Scuba events to mitigate the risk of
    // race conditions between increment and decrement operations.
    if (pendingConnectionsMetrics_) {
      pendingConnectionsMetrics_->onConnectionDequedByIoWorker();
    }

    THRIFT_CONNECTION_EVENT(connection_dequeued_acceptor)
        .log(thriftServer_, clientAddr);
  }

  void onBackoffStarted() noexcept override {}

  void onBackoffEnded() noexcept override {}

  void onBackoffError() noexcept override {}

 private:
  const ThriftServer& thriftServer_;
  std::shared_ptr<PendingConnectionsMetrics> pendingConnectionsMetrics_;

  std::shared_ptr<PendingConnectionsMetrics> setupPendingConnectionsMetrics() {
    return thriftServer_.getObserver()
        ? std::make_shared<PendingConnectionsMetrics>(
              thriftServer_.getObserverShared())
        : nullptr;
  }
};

void ThriftServer::setup() {
  ensureProcessedServiceDescriptionInitialized();

  auto nWorkers = getNumIOWorkerThreads();
  DCHECK_GT(nWorkers, 0u);

  // Initialize event base for this thread
  auto serveEventBase = eventBaseManager_->getEventBase();
  serveEventBase_ = serveEventBase;
  stopController_.set(
      std::make_unique<StopController>(
          folly::badge<ThriftServer>{}, *serveEventBase));
  if (idleServerTimeout_.count() > 0) {
    idleServer_.emplace(*this, serveEventBase->timer(), idleServerTimeout_);
  }
  // Print some libevent stats
  VLOG(1) << "libevent " << folly::EventBase::getLibeventVersion() << " method "
          << serveEventBase->getLibeventMethod();

  switch (getEffectiveTicketSeedStrategy()) {
    case EffectiveTicketSeedStrategy::IN_MEMORY_WITH_ROTATION:
      fizzConfig_.supportedPskModes = {
          fizz::PskKeyExchangeMode::psk_ke,
          fizz::PskKeyExchangeMode::psk_dhe_ke};
      scheduleInMemoryTicketSeeds();
      break;
    case EffectiveTicketSeedStrategy::IN_MEMORY:
    case EffectiveTicketSeedStrategy::FILE:
      break;
  }

  try {
#ifndef _WIN32
    // OpenSSL might try to write to a closed socket if the peer disconnects
    // abruptly, raising a SIGPIPE signal. By default this will terminate the
    // process, which we don't want. Hence we need to handle SIGPIPE specially.
    //
    // We don't use SIG_IGN here as child processes will inherit that handler.
    // Instead, we swallow the signal to enable SIGPIPE in children to behave
    // normally.
    // Furthermore, the signal flags passed below to sigaction prevents
    // SA_RESTART from restarting syscalls after the handler completed. This is
    // important for code using SIGPIPE to interrupt syscalls in other threads.
    // Also pass SA_ONSTACK to prevent using the default stack which causes
    // panics in Go (see https://pkg.go.dev/os/signal).
    struct sigaction sa = {};
    sa.sa_handler = [](int) {};
    sa.sa_flags = SA_ONSTACK;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPIPE, &sa, nullptr);
#endif

    configureIOUring();

    if (!getObserver() && server::observerFactory_) {
      setObserver(server::observerFactory_->getObserver());
    }

    // This may be the second time this is called (if setupThreadManager
    // was called directly by the service code)
    runtimeResourcePoolsChecksImpl();

    setupThreadManagerImpl();

    // Routing handlers may install custom Resource Pools to handle their
    // requests, so they must be added after Resource Pool enablement status is
    // solidified in setupThreadManager().
    addRoutingHandler(
        std::make_unique<apache::thrift::RocketRoutingHandler>(*this));
    if (!FLAGS_disable_legacy_header_routing_handler) {
      addRoutingHandler(
          std::make_unique<apache::thrift::LegacyHeaderRoutingHandler>(*this));
    }

    // Ensure no further changes can be made to the set of ResourcePools
    // currently installed
    lockResourcePoolSet();

    ServerBootstrap::socketConfig.acceptBacklog = getListenBacklog();
    ServerBootstrap::socketConfig.maxNumPendingConnectionsPerWorker =
        getMaxNumPendingConnectionsPerWorker();
    if (reusePort_.value_or(false)) {
      ServerBootstrap::setReusePort(true);
    }
    if (enableTFO_) {
      ServerBootstrap::socketConfig.enableTCPFastOpen = *enableTFO_;
      ServerBootstrap::socketConfig.fastOpenQueueSize = fastOpenQueueSize_;
    }

    ioThreadPool_->addObserver(
        folly::IOThreadPoolDeadlockDetectorObserver::create(
            ioThreadPool_->getName()));
    ioObserverFactories.withRLock([this](auto& factories) {
      for (auto& f : factories) {
        ioThreadPool_->addObserver(
            f(ioThreadPool_->getName(), ioThreadPool_->getThreadIdCollector()));
      }
    });

    // Resize the IO pool
    ioThreadPool_->setNumThreads(nWorkers);
    if (!acceptPool_) {
      acceptPool_ = std::make_shared<folly::IOThreadPoolExecutor>(
          nAcceptors_,
          std::make_shared<folly::NamedThreadFactory>("Acceptor Thread"));
    }

    auto acceptorFactory = acceptorFactory_
        ? acceptorFactory_
        : std::make_shared<DefaultThriftAcceptorFactory>(this);
    if (auto factory = dynamic_cast<wangle::AcceptorFactorySharedSSLContext*>(
            acceptorFactory.get())) {
      sharedSSLContextManager_ = factory->initSharedSSLContextManager();
    }
    ServerBootstrap::childHandler(std::move(acceptorFactory));

    {
      std::unique_lock lock(ioGroupMutex_);
      ServerBootstrap::group(acceptPool_, ioThreadPool_);
    }
    // RemoteAcceptor needs to be initialized with ConnectionEventCallback
    // during address bind, in order to enable it to report connection events
    // back to ThriftServer
    connEventCallback_ = std::make_shared<ConnectionEventCallback>(*this);
    ServerBootstrap::useConnectionEventCallback(connEventCallback_);

    server::DecoratorDataPerRequestBlueprint::Setup decoratorDataSetup;

    // Call decorator and handler beforeStartServing()
    callDecoratorsAndHandlersBeforeStartServing(decoratorDataSetup);

    // Interceptor onStartServing() needs to be called before we bind to the
    // socket, otherwise it is possible for connections to be accepted
    // (and onConnection() called) before onStartServing is called.
    callInterceptorsOnStartServing(decoratorDataSetup);

    decoratorDataPerRequestBlueprint_ =
        std::move(decoratorDataSetup).finalize();

    if (socket_) {
      ServerBootstrap::bind(std::move(socket_));
    } else if (!getAddress().isInitialized()) {
      ServerBootstrap::bind(port_.value_or(0));
    } else {
      for (auto& address : addresses_) {
        ServerBootstrap::bind(address);
      }
    }
    // Update address_ with the address that we are actually bound to.
    // (This is needed if we were supplied a pre-bound socket, or if
    // address_'s port was set to 0, so an ephemeral port was chosen by
    // the kernel.)
    ServerBootstrap::getSockets()[0]->getAddress(&addresses_.at(0));

    // Log the port(s) that the server is listening on
    XLOG_IF(INFO, infoLoggingEnabled_)
        << "ThriftServer listening on address/port: " << getAddressAsString();

    // we enable zerocopy for the server socket if the
    // zeroCopyEnableFunc_ is valid
    bool useZeroCopy = !!zeroCopyEnableFunc_;
    for (auto& socket : getSockets()) {
      auto* evb = socket->getEventBase();
      evb->runImmediatelyOrRunInEventBaseThreadAndWait([&] {
        socket->setShutdownSocketSet(wShutdownSocketSet_);
        socket->setAcceptRateAdjustSpeed(acceptRateAdjustSpeed_);
        socket->setZeroCopy(useZeroCopy);
        socket->setQueueTimeout(getSocketQueueTimeout());
        if (callbackAssignFunc_) {
          socket->setCallbackAssignFunction(callbackAssignFunc_);
        }

        try {
          socket->setTosReflect(tosReflect_);
          socket->setListenerTos(listenerTos_);
        } catch (std::exception const& ex) {
          XLOG(ERR) << "Got exception setting up TOS settings: "
                    << folly::exceptionStr(ex);
        }
      });
    }

    startAdditionalServers();

#if FOLLY_HAS_COROUTINES
    asyncScope_ = std::make_unique<folly::coro::CancellableAsyncScope>();
#endif
    for (auto handler : collectServiceHandlers()) {
      handler->attachServer(*this);
    }

    DCHECK(
        internalStatus_.load(std::memory_order_relaxed) ==
        ServerStatus::NOT_RUNNING);
    // The server is not yet ready for the user's service methods but fine to
    // handle internal methods. See ServerConfigs::getInternalMethods().
    internalStatus_.store(
        ServerStatus::PRE_STARTING, std::memory_order_release);

    // Notify handler of the preStart event
    for (const auto& eventHandler : getEventHandlersUnsafe()) {
      eventHandler->preStart(&addresses_.at(0));
    }

    internalStatus_.store(ServerStatus::STARTING, std::memory_order_release);

    // Called after setup
    callHandlersOnStartServing();

    // After the onStartServing hooks have finished, we are ready to handle
    // requests, at least from the server's perspective.
    internalStatus_.store(ServerStatus::RUNNING, std::memory_order_release);

#if FOLLY_HAS_COROUTINES
    // Set up polling for PolledServiceHealth handlers if Python health polling
    // is not up.
    if (!isServiceHealthPollerDisabled()) {
      DCHECK(!getServiceHealth().has_value());
      auto handlers = collectServiceHandlers<PolledServiceHealth>();
      if (!handlers.empty()) {
        auto poll = ServiceHealthPoller::poll(
            std::move(handlers), getPolledServiceHealthLivenessObserver());
        auto loop = folly::coro::co_invoke(
            [this,
             poll = std::move(poll)]() mutable -> folly::coro::Task<void> {
              while (auto value = co_await poll.next()) {
                co_await folly::coro::co_safe_point;
                cachedServiceHealth_.store(*value, std::memory_order_relaxed);
              }
            });
        asyncScope_->add(
            co_withExecutor(getHandlerExecutorKeepAlive(), std::move(loop)));
      }
    }
#endif
    // Do not allow setters to be called past this point until the IO worker
    // threads have been joined in stopWorkers().
    thriftConfig_.freeze();

    // Notify handler of the preServe event
    for (const auto& eventHandler : getEventHandlersUnsafe()) {
      eventHandler->preServe(&addresses_.at(0));
    }

  } catch (std::exception& ex) {
    // This block allows us to investigate the exception using gdb
    XLOG(ERR) << "Got an exception while setting up the server: " << ex.what();
    handleSetupFailure();
    throw;
  } catch (...) {
    handleSetupFailure();
    throw;
  }

  THRIFT_SERVER_EVENT(serve).log(*this);
  // This gives us an event in thrift server events when a server is run with
  // DCHECK enabled.
  DCHECK(serverRanWithDCHECK());
}

bool ThriftServer::serverRanWithDCHECK() {
  THRIFT_SERVER_EVENT(dcheck).log(*this);
  return true;
}

void ThriftServer::setupThreadManager() {
  runtimeServerActions_.setupThreadManagerCalledByUser = true;
  THRIFT_SERVER_EVENT(setupThreadManager).log(*this);
  setupThreadManagerImpl();
}

void ThriftServer::setupThreadManagerImpl() {
  if (!setupThreadManagerCalled_) {
    setupThreadManagerCalled_ = true;

    // Try the runtime checks - if it is too early to complete them we will
    // retry on setup()
    runtimeResourcePoolsChecksImpl();

    // Past this point no modification to the enablement of
    // ResourcePool should be made in the same server
    runtimeServerActions_.resourcePoolFlagSet =
        THRIFT_FLAG(experimental_use_resource_pools);

    // Ensure that either the thread manager or resource pools exist.
    if (!useResourcePools()) {
      THRIFT_SERVER_EVENT(serviceNeedsResourcePoolsMigration).log(*this);
      DCHECK(resourcePoolSet().empty());
      // We always need a thread manager for cpp2.
      auto explanation = fmt::format(
          "runtime: {}, thrift flag: {}, enable gflag: {}, disable gflag: {}",
          runtimeServerActions_.explain(),
          THRIFT_FLAG(experimental_use_resource_pools),
          FLAGS_thrift_experimental_use_resource_pools,
          FLAGS_thrift_disable_resource_pools);
      XLOG_IF(INFO, infoLoggingEnabled_)
          << "Using thread manager (resource pools not enabled) on address/port "
          << getAddressAsString() << ": " << explanation;
      if (auto observer = getObserverShared()) {
        observer->resourcePoolsDisabled(explanation);
      }
      if (!threadManager_) {
        std::shared_ptr<apache::thrift::concurrency::ThreadManager>
            threadManager;
        switch (threadManagerType_) {
          case ThreadManagerType::PRIORITY:
            CHECK(!threadPriority_);
            if (std::any_of(
                    std::begin(threadManagerPrioritiesAndPoolSizes_),
                    std::end(threadManagerPrioritiesAndPoolSizes_),
                    [](auto prioritySize) {
                      return prioritySize.second != 0;
                    })) {
              // The priorities were specified using setThreadManagerPoolSizes
              std::array<
                  std::
                      pair<std::shared_ptr<concurrency::ThreadFactory>, size_t>,
                  concurrency::N_PRIORITIES>
                  args;
              for (int i = 0; i < concurrency::N_PRIORITIES; ++i) {
                args[i].first =
                    std::make_shared<concurrency::PosixThreadFactory>(
                        concurrency::PosixThreadFactory::OTHER,
                        threadManagerPrioritiesAndPoolSizes_[i].first);
                args[i].second = threadManagerPrioritiesAndPoolSizes_[i].second;
              }
              threadManager =
                  PriorityThreadManager::newPriorityThreadManager(args);
            } else {
              threadManager = PriorityThreadManager::newPriorityThreadManager(
                  getNumCPUWorkerThreads());
            }
            break;
          case ThreadManagerType::SIMPLE:
            threadManager =
                ThreadManager::newSimpleThreadManager(getNumCPUWorkerThreads());
            break;
          case ThreadManagerType::PRIORITY_QUEUE:
            threadManager = ThreadManager::newPriorityQueueThreadManager(
                getNumCPUWorkerThreads());
            break;
          case ThreadManagerType::EXECUTOR_ADAPTER:
            CHECK(!threadPriority_);
            threadManager =
                std::make_shared<concurrency::ThreadManagerExecutorAdapter>(
                    threadManagerExecutors_);
            break;
        }
        threadManager->enableCodel(getEnableCodel());

        std::shared_ptr<concurrency::ThreadFactory> threadFactory;
        if (threadFactory_) {
          CHECK(!threadPriority_);
          threadFactory = threadFactory_;
        } else if (threadPriority_) {
          auto factory = std::make_shared<concurrency::PosixThreadFactory>();
          factory->setPriority(threadPriority_.value());
          threadFactory = factory;
        }

        if (threadInitializer_ || threadFinalizer_) {
          if (threadFactory) {
            // Wrap the thread factory if initializer/finalizer are specified.
            threadFactory = std::make_shared<concurrency::InitThreadFactory>(
                std::move(threadFactory),
                std::move(threadInitializer_),
                std::move(threadFinalizer_));
          } else {
            XLOG(FATAL)
                << "setThreadInit not supported without setThreadFactory";
          }
        }

        if (threadFactory) {
          threadManager->threadFactory(std::move(threadFactory));
        }

        auto poolThreadName = getCPUWorkerThreadName();
        if (!poolThreadName.empty()) {
          threadManager->setNamePrefix(poolThreadName);
        }
        threadManager->start();
        setThreadManagerInternal(threadManager);
      }

      // Log the case when we tried to use resource pools and disabled it
      // because of run-time holdouts.
      if (runtimeServerActions_.resourcePoolFlagSet) {
        THRIFT_SERVER_EVENT(resourcepoolsruntimedisallowed).log(*this);
      }
    } else {
      auto explanation = fmt::format(
          "thrift flag: {}, enable gflag: {}, disable gflag: {}, runtime actions: {}",
          THRIFT_FLAG(experimental_use_resource_pools),
          FLAGS_thrift_experimental_use_resource_pools,
          FLAGS_thrift_disable_resource_pools,
          runtimeServerActions_.explain());
      XLOG_IF(INFO, infoLoggingEnabled_)
          << "Using resource pools on address/port " << getAddressAsString()
          << ": " << explanation;
      if (auto observer = getObserverShared()) {
        observer->resourcePoolsEnabled(explanation);
      }
      ensureResourcePools();

      // During resource pools roll out we want to track services that get
      // enrolled in the roll out.
      THRIFT_SERVER_EVENT(resourcepoolsenabled).log(*this);
    }
  }

  // Now do setup that we want to do whether we created these resources or the
  // client did.
  if (!resourcePoolSet().empty()) {
    setConcurrencyLimitCallbackHandle =
        detail::getThriftServerConfig(*this)
            .getConcurrencyLimit()
            .getObserver()
            .addCallback(
                [this](const folly::observer::Snapshot<uint32_t>& snapshot) {
                  auto concurrencyLimit = *snapshot;
                  if (auto cc =
                          resourcePoolSet()
                              .resourcePool(ResourcePoolHandle::defaultAsync())
                              .concurrencyController()) {
                    cc.value().get().setExecutionLimitRequests(
                        concurrencyLimit != 0
                            ? concurrencyLimit
                            : std::numeric_limits<
                                  decltype(concurrencyLimit)>::max());
                  }
                });
    if (THRIFT_FLAG(default_sync_max_requests_to_concurrency_limit)) {
      // ThriftServer's maxRequests was historically synced to
      // ConcurrencyController's executionLimitRequests. We are migrating away
      // from this syncing behavior, but gate the functionality behind a flag
      // for services that still rely on the behavior.
      XLOG(WARN)
          << "--default_sync_max_requests_to_concurrency_limit=true. Please "
             "follow this guide to disable this. "
             "https://www.internalfb.com/wiki/"
             "Thrift_how_to_disable_sync_max_requests_to_concurrency_limit/";
      setMaxRequestsCallbackHandle =
          detail::getThriftServerConfig(*this)
              .getMaxRequests()
              .getObserver()
              .addCallback([this](
                               const folly::observer::Snapshot<uint32_t>&
                                   snapshot) {
                auto maxRequests = *snapshot;
                if (auto cc =
                        resourcePoolSet()
                            .resourcePool(ResourcePoolHandle::defaultAsync())
                            .concurrencyController()) {
                  if (folly::test_once(
                          cancelSetMaxRequestsCallbackHandleFlag_)) {
                    return;
                  }
                  cc.value().get().setExecutionLimitRequests(
                      maxRequests != 0
                          ? maxRequests
                          : std::numeric_limits<decltype(maxRequests)>::max());
                }
              });
    }
    setExecutionRateCallbackHandle =
        detail::getThriftServerConfig(*this)
            .getExecutionRate()
            .getObserver()
            .addCallback([this](
                             const folly::observer::Snapshot<uint32_t>&
                                 snapshot) {
              auto executionRate = *snapshot;
              if (auto cc =
                      resourcePoolSet()
                          .resourcePool(ResourcePoolHandle::defaultAsync())
                          .concurrencyController()) {
                cc.value().get().setQpsLimit(
                    executionRate != 0
                        ? executionRate
                        : std::numeric_limits<decltype(executionRate)>::max());
              }
            });
    if (THRIFT_FLAG(default_sync_max_qps_to_execution_rate)) {
      // ThriftServer's maxQps was historically synced to
      // ConcurrencyController's qpsLimit. This syncing behavior will be
      // removed, but for now we sync by default. We are migrating away from
      // this syncing behavior, but gate the functionality behind a flag for
      // services that still rely on the behavior.
      XLOG(WARN) << "--default_sync_max_qps_to_execution_rate=true. Please "
                    "follow this guide to disable this. "
                    "https://www.internalfb.com/wiki/"
                    "Thrift_how_to_disable_sync_max_qps_to_execution_rate/";
      setMaxQpsCallbackHandle =
          detail::getThriftServerConfig(*this)
              .getMaxQps()
              .getObserver()
              .addCallback(
                  [this](const folly::observer::Snapshot<uint32_t>& snapshot) {
                    auto maxQps = *snapshot;
                    if (auto cc = resourcePoolSet()
                                      .resourcePool(
                                          ResourcePoolHandle::defaultAsync())
                                      .concurrencyController()) {
                      if (folly::test_once(
                              cancelSetMaxQpsCallbackHandleFlag_)) {
                        return;
                      }
                      cc.value().get().setQpsLimit(
                          maxQps != 0
                              ? maxQps
                              : std::numeric_limits<decltype(maxQps)>::max());
                    }
                  });
    }
    // Create an adapter so calls to getThreadManager_deprecated will work
    // when we are using resource pools
    if (!threadManager_ &&
        resourcePoolSet().hasResourcePool(ResourcePoolHandle::defaultAsync())) {
      auto extm =
          std::make_shared<ExecutorToThreadManagerAdaptor>(resourcePoolSet());
      setThreadManagerInternal(extm);
    }
  } else {
    threadManager_->setExpireCallback([&](std::shared_ptr<Runnable> r) {
      EventTask* task = dynamic_cast<EventTask*>(r.get());
      if (task) {
        task->expired();
      }
    });
    threadManager_->setCodelCallback([&](std::shared_ptr<Runnable>) {
      auto observer = getObserver();
      if (observer) {
        if (getEnableCodel()) {
          observer->queueTimeout();
        }
      }
    });
  }
}

namespace {
struct PoolNames {
  std::string_view name;
  std::string_view suffix;
};

constexpr std::array<PoolNames, concurrency::N_PRIORITIES> kPoolNames = {
    {{"HIGH_IMPORTANT", "HI"},
     {"HIGH", "H"},
     {"IMPORTANT", "I"},
     {"NORMAL", "N"},
     {"BEST_EFFORT", "BE"}}};

std::string getThreadNameForPriority(
    const std::string& cpuWorkerThreadName, concurrency::PRIORITY priority) {
  return fmt::format(
      "{}.{}", cpuWorkerThreadName, kPoolNames.at(priority).suffix);
}
} // namespace

void ThriftServer::ensureResourcePoolsDefaultPrioritySetup(
    std::vector<concurrency::PRIORITY> allocated) {
  static_assert(concurrency::HIGH_IMPORTANT == 0);
  for (std::size_t i = concurrency::HIGH_IMPORTANT;
       i < concurrency::N_PRIORITIES;
       ++i) {
    if (std::none_of(
            std::cbegin(allocated),
            std::cend(allocated),
            [=](concurrency::PRIORITY p) { return p == i; })) {
      // We expect that at least the normal priority has been set up.
      CHECK_NE(i, concurrency::NORMAL);

      auto name = getThreadNameForPriority(
          getCPUWorkerThreadName(), concurrency::PRIORITY(i));
      std::shared_ptr<folly::ThreadFactory> factory =
          std::make_shared<folly::NamedThreadFactory>(name);
      // 2 is the default for the priorities other than NORMAL.
      auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(
          2, ResourcePool::kPreferredExecutorNumPriorities, std::move(factory));
      apache::thrift::RoundRobinRequestPile::Options options;
      auto requestPile = makeStandardRequestPile(std::move(options));
      auto concurrencyController = makeStandardConcurrencyController(
          *requestPile.get(), *executor.get());
      resourcePoolSet().addResourcePool(
          kPoolNames.at(i).name,
          std::move(requestPile),
          executor,
          std::move(concurrencyController),
          concurrency::PRIORITY(i));
    }
  }
}

// Return true if the runtime checks pass and using resource pools is an option.
bool ThriftServer::runtimeResourcePoolsChecks() {
  runtimeServerActions_.runtimeResourcePoolsChecksCalledByUser = true;
  THRIFT_SERVER_EVENT(runtimeResourcePoolsChecks).log(*this);
  return runtimeResourcePoolsChecksImpl();
}

bool ThriftServer::runtimeResourcePoolsChecksImpl() {
  runtimeServerActions_.resourcePoolEnabledGflag =
      FLAGS_thrift_experimental_use_resource_pools;
  runtimeServerActions_.resourcePoolDisabledGflag =
      FLAGS_thrift_disable_resource_pools;
  if (runtimeDisableResourcePoolsSet()) {
    // No need to check if we've already set this.
    XLOG_IF(INFO, infoLoggingEnabled_)
        << "runtimeResourcePoolsChecks() returns false because of runtimeDisableResourcePoolsSet()";
    return false;
  }
  // This can be called multiple times - only run it to completion once
  // but note below that it can exit early.
  if (runtimeServerActions_.checkComplete) {
    return !runtimeDisableResourcePoolsSet();
  }
  // If this is called too early we can't run our other checks.
  if (!getProcessorFactory()) {
    runtimeServerActions_.setupThreadManagerBeforeHandler = true;
    if (runtimeServerActions_.resourcePoolRuntimeRequested) {
      // If this is called early - because the service calls
      // setupThreadManager() directly before setInterface() and
      // requireResourcePools() was called then do not disable resource pools
      // yet. runtimeResourcePoolsChecks() will be called again later on in
      // setup() and if it calls runtimeDisableResourcePoolsDeprecated() at that
      // time that will become a fatal error which is what we want (that can
      // only be triggered by a requireResourcePools() call in the server code).
      XLOG_IF(INFO, infoLoggingEnabled_)
          << "It's too early to call runtimeResourcePoolsChecks(), returning True for now";
      return true;
    }
    runtimeDisableResourcePoolsDeprecated("setupThreadManagerBeforeHandler");
  } else {
    // Initialize decorated processor factory, if not done yet.
    ensureDecoratedProcessorFactoryInitialized();

    // Check whether there are any wildcard services.
    auto methodMetadata = getDecoratedProcessorFactory().createMethodMetadata();

    if (auto* methodMetadataMap =
            std::get_if<AsyncProcessorFactory::MethodMetadataMap>(
                &methodMetadata)) {
      for (const auto& methodToMetadataPtr : *methodMetadataMap) {
        const auto& metadata = *methodToMetadataPtr.second;
        if (metadata.executorType ==
                AsyncProcessorFactory::MethodMetadata::ExecutorType::UNKNOWN ||
            metadata.interactionType ==
                AsyncProcessorFactory::MethodMetadata::InteractionType::
                    UNKNOWN ||
            !metadata.rpcKind || !metadata.priority) {
          // Disable resource pools if there is no service request info
          XLOG_IF(INFO, infoLoggingEnabled_)
              << "Resource pools disabled. Incomplete metadata";
          runtimeServerActions_.noServiceRequestInfo = true;
          runtimeDisableResourcePoolsDeprecated("IncompleteMetadata");
        }
        if (metadata.interactionType ==
            AsyncProcessorFactory::MethodMetadata::InteractionType::
                INTERACTION_V1) {
          // We've found an interaction in this service. We use
          // `interactionInService` to decide if we need to add internal
          // priorities to the default RoundRobinRequestPile.
          runtimeServerActions_.interactionInService = true;
        }
      }
    } else { // we can only have WildcardMethodMetadataMap here
      auto* wildcardMap =
          std::get_if<AsyncProcessorFactory::WildcardMethodMetadataMap>(
              &methodMetadata);

      DCHECK(wildcardMap);

      bool wildcardFlagEnabled =
          THRIFT_FLAG(allow_resource_pools_for_wildcards) ||
          FLAGS_thrift_allow_resource_pools_for_wildcards;
      bool validExecutor = wildcardMap->wildcardMetadata->executorType !=
          AsyncProcessorFactory::MethodMetadata::ExecutorType::UNKNOWN;

      // if a wildcard is not providing valid executor type
      // we should not turn on resource pool for it
      if (!wildcardFlagEnabled || !validExecutor) {
        XLOG_IF(INFO, infoLoggingEnabled_)
            << "Resource pools disabled. Wildcard methods";
        runtimeServerActions_.wildcardMethods = true;
        runtimeDisableResourcePoolsDeprecated("WildcardMethods");
      }
    }
  }

  if (isActiveRequestsTrackingDisabled()) {
    // Record this but don't disable. Managed in configuration instead.
    runtimeServerActions_.activeRequestTrackingDisabled = true;
  }

  runtimeServerActions_.checkComplete = true;

  if (runtimeDisableResourcePoolsSet()) {
    XLOG_IF(INFO, infoLoggingEnabled_)
        << "runtimeResourcePoolsChecks() returns false because of runtimeDisableResourcePoolsSet()";
    return false;
  }
  return true;
}

void ThriftServer::ensureResourcePools() {
  auto resourcePoolSupplied = !resourcePoolSet().empty();
  if (resourcePoolSupplied) {
    XLOG_IF(INFO, infoLoggingEnabled_)
        << "Resource pools supplied: " << resourcePoolSet().size();
  }

  if (!resourcePoolSet().hasResourcePool(ResourcePoolHandle::defaultSync())) {
    // Ensure there is a sync resource pool.
    resourcePoolSet().setResourcePool(
        ResourcePoolHandle::defaultSync(),
        /*requestPile=*/nullptr,
        /*executor=*/nullptr,
        /*concurrencyController=*/nullptr);
  }

  // The user provided the resource pool. Use it as is.
  if (resourcePoolSupplied) {
    if (!resourcePoolSet().hasResourcePool(
            ResourcePoolHandle::defaultAsync())) {
      XLOG_IF(INFO, infoLoggingEnabled_)
          << "Default async pool is NOT supplied, creating a default async pool";
      auto threadFactory = [this]() -> std::shared_ptr<folly::ThreadFactory> {
        auto prefix = getThreadNameForPriority(
            getCPUWorkerThreadName(), concurrency::PRIORITY::NORMAL);
        auto factory = std::make_shared<folly::PriorityThreadFactory>(
            std::make_shared<folly::NamedThreadFactory>(prefix),
            concurrency::PosixThreadFactory::Impl::toPthreadPriority(
                concurrency::PosixThreadFactory::kDefaultPolicy,
                concurrency::PosixThreadFactory::NORMAL_PRI));
        if (threadInitializer_ || threadFinalizer_) {
          return std::make_shared<folly::InitThreadFactory>(
              std::move(factory),
              std::move(threadInitializer_),
              std::move(threadFinalizer_));
        }
        return factory;
      };

      auto requestPile =
          makeStandardRequestPile(RoundRobinRequestPile::Options());

      auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(
          getNumCPUWorkerThreads(),
          ResourcePool::kPreferredExecutorNumPriorities,
          threadFactory());

      auto concurrencyController = makeStandardConcurrencyController(
          *requestPile.get(), *executor.get());

      resourcePoolSet().setResourcePool(
          ResourcePoolHandle::defaultAsync(),
          std::move(requestPile),
          executor,
          std::move(concurrencyController));
    }

    runtimeServerActions_.userSuppliedResourcePools = true;
    return;
  }

  if (threadManager_) {
    apache::thrift::RoundRobinRequestPile::Options options;
    auto requestPile = makeStandardRequestPile(std::move(options));
    auto concurrencyController =
        std::make_unique<apache::thrift::TMConcurrencyController>(
            *requestPile, *threadManager_);
    resourcePoolSet().setResourcePool(
        ResourcePoolHandle::defaultAsync(),
        std::move(requestPile),
        threadManager_,
        std::move(concurrencyController),
        concurrency::PRIORITY::NORMAL);
    return;
  }

  if (threadManagerType_ == ThreadManagerType::EXECUTOR_ADAPTER) {
    CHECK(!threadPriority_);
    for (std::size_t i = 0; i < concurrency::N_PRIORITIES; ++i) {
      auto executor = threadManagerExecutors_[i];
      if (!executor) {
        // If no executor provided for this priority create one.
        executor = std::make_shared<folly::CPUThreadPoolExecutor>(
            i == concurrency::PRIORITY::NORMAL ? folly::available_concurrency()
                                               : 2,
            ResourcePool::kPreferredExecutorNumPriorities);
      }
      apache::thrift::RoundRobinRequestPile::Options options;
      auto requestPile = makeStandardRequestPile(std::move(options));
      auto concurrencyController = makeStandardConcurrencyController(
          *requestPile.get(), *executor.get());
      if (i == concurrency::PRIORITY::NORMAL) {
        resourcePoolSet().setResourcePool(
            ResourcePoolHandle::defaultAsync(),
            std::move(requestPile),
            executor,
            std::move(concurrencyController),
            concurrency::PRIORITY::NORMAL);
      } else {
        std::string name("EW-pri-");
        name += std::to_string(i);
        resourcePoolSet().addResourcePool(
            name,
            std::move(requestPile),
            executor,
            std::move(concurrencyController),
            concurrency::PRIORITY(i));
      }
    }
  } else {
    struct Pool {
      concurrency::PosixThreadFactory::THREAD_PRIORITY threadPriority;
      size_t numThreads;
      std::optional<ResourcePoolHandle> handle;
      concurrency::PRIORITY thriftPriority;
    };

    std::vector<Pool> pools;

    switch (threadManagerType_) {
      case ThreadManagerType::PRIORITY: {
        CHECK(!threadPriority_);
        Pool priorityPools[] = {
            {concurrency::PosixThreadFactory::HIGHER_PRI,
             2,
             std::nullopt,
             concurrency::HIGH_IMPORTANT},
            {concurrency::PosixThreadFactory::HIGH_PRI,
             2,
             std::nullopt,
             concurrency::HIGH},
            {concurrency::PosixThreadFactory::HIGH_PRI,
             2,
             std::nullopt,
             concurrency::IMPORTANT},
            {concurrency::PosixThreadFactory::NORMAL_PRI,
             getNumCPUWorkerThreads(),
             ResourcePoolHandle::defaultAsync(),
             concurrency::NORMAL},
            {concurrency::PosixThreadFactory::LOWER_PRI,
             2,
             std::nullopt,
             concurrency::BEST_EFFORT}};
        if (std::any_of(
                std::begin(threadManagerPrioritiesAndPoolSizes_),
                std::end(threadManagerPrioritiesAndPoolSizes_),
                [](const auto& prioritySize) {
                  return prioritySize.second != 0;
                })) {
          // The priorities were specified using setThreadManagerPoolSizes
          for (std::size_t i = 0; i < std::size(priorityPools); ++i) {
            priorityPools[i].numThreads =
                threadManagerPrioritiesAndPoolSizes_.at(i).second;
            priorityPools[i].threadPriority =
                threadManagerPrioritiesAndPoolSizes_.at(i).first;
          }
        }
        std::copy(
            std::begin(priorityPools),
            std::end(priorityPools),
            std::back_inserter(pools));
        break;
      }
      case ThreadManagerType::SIMPLE: {
        pools.push_back(
            Pool{
                threadPriority_.value_or(
                    concurrency::PosixThreadFactory::NORMAL_PRI),
                getNumCPUWorkerThreads(),
                ResourcePoolHandle::defaultAsync(),
                concurrency::NORMAL});
        break;
      }
      case ThreadManagerType::PRIORITY_QUEUE: {
        pools.push_back(
            Pool{
                threadPriority_.value_or(
                    concurrency::PosixThreadFactory::NORMAL_PRI),
                getNumCPUWorkerThreads(),
                ResourcePoolHandle::defaultAsync(),
                concurrency::NORMAL});
        break;
      }
      default: {
        XLOG(FATAL) << "Unexpected ThreadMangerType:"
                    << int(threadManagerType_);
      }
    }
    for (const auto& pool : pools) {
      auto name = getThreadNameForPriority(
          getCPUWorkerThreadName(), pool.thriftPriority);
      std::shared_ptr<folly::ThreadFactory> factory =
          std::make_shared<folly::PriorityThreadFactory>(
              std::make_shared<folly::NamedThreadFactory>(name),
              concurrency::PosixThreadFactory::Impl::toPthreadPriority(
                  concurrency::PosixThreadFactory::kDefaultPolicy,
                  pool.threadPriority));
      if (threadInitializer_ || threadFinalizer_) {
        factory = std::make_shared<folly::InitThreadFactory>(
            std::move(factory),
            std::move(threadInitializer_),
            std::move(threadFinalizer_));
      }
      auto executor = std::make_shared<folly::CPUThreadPoolExecutor>(
          pool.numThreads,
          ResourcePool::kPreferredExecutorNumPriorities,
          std::move(factory));
      apache::thrift::RoundRobinRequestPile::Options options;
      if (threadManagerType_ == ThreadManagerType::PRIORITY_QUEUE) {
        options.setNumPriorities(concurrency::N_PRIORITIES);
        options.setPileSelectionFunction(options.getDefaultPileSelectionFunc());
      }
      auto requestPile = makeStandardRequestPile(std::move(options));
      auto concurrencyController = makeStandardConcurrencyController(
          *requestPile.get(), *executor.get());
      if (pool.handle) {
        resourcePoolSet().setResourcePool(
            ResourcePoolHandle::defaultAsync(),
            std::move(requestPile),
            executor,
            std::move(concurrencyController),
            pool.thriftPriority);
      } else {
        resourcePoolSet().addResourcePool(
            kPoolNames.at(pool.thriftPriority).suffix,
            std::move(requestPile),
            executor,
            std::move(concurrencyController),
            pool.thriftPriority);
      }
    }
  }
}

void ThriftServer::lockResourcePoolSet() {
  // After this point there should be no further changes to resource pools. We
  // lock whether or not we are actually using them so that the checks on
  // resourcePoolSet().empty() can be efficient.
  resourcePoolSet().lock();

  if (!resourcePoolSet().empty()) {
    XLOG_IF(INFO, infoLoggingEnabled_)
        << "Resource pools configured: " << resourcePoolSet().size();

    auto descriptions = resourcePoolSet().poolsDescriptions();
    if (auto observer = getObserverShared()) {
      observer->resourcePoolsInitialized(descriptions);
    }

    size_t count{0};
    for (auto description : descriptions) {
      VLOG(1) << fmt::format("Resource pool [{}]: {}", count++, description);
    }
  }
}

/**
 * Loop and accept incoming connections.
 */
void ThriftServer::serve() {
  setup();
  SCOPE_EXIT {
    this->cleanUp();
  };

  auto sslContextConfigCallbackHandle = sslContextObserver_
      ? getSSLCallbackHandle()
      : folly::observer::CallbackHandle{};

  eventBaseManager_->getEventBase()->loopForever();
}

void ThriftServer::cleanUp() {
  // tlsCredWatcher_ uses a background thread that needs to be joined
  // prior to any further writes to ThriftServer members.
  tlsCredWatcher_.withWLock([](auto& credWatcher) { credWatcher.reset(); });

  // It is users duty to make sure that setup() call
  // should have returned before doing this cleanup
  idleServer_.reset();
  serveEventBase_ = nullptr;
  stopController_.join();
  stopListening();

  // Stop the routing handlers.
  for (auto& handler : routingHandlers_) {
    handler->stopListening();
  }

  if (stopWorkersOnStopListening_) {
    // Wait on the i/o worker threads to actually stop
    stopWorkers();
  } else if (joinRequestsWhenServerStops_) {
    stopAcceptingAndJoinOutstandingRequests();
  }

  for (auto handler : getProcessorFactory()->getServiceHandlers()) {
    handler->detachServer();
  }

  // Now clear all the handlers
  routingHandlers_.clear();

  // Clear the service description so that it's re-created if the server
  // is restarted.
  processedServiceDescription_.reset();
  decoratedProcessorFactory_.reset();
}

uint64_t ThriftServer::getNumDroppedConnections() const {
  uint64_t droppedConns = 0;
  for (auto& socket : getSockets()) {
    droppedConns += socket->getNumDroppedConnections();
  }
  return droppedConns;
}

void ThriftServerStopController::stop() {
  folly::call_once(stopped_, [&] { serveEventBase_.terminateLoopSoon(); });
}

void ThriftServer::stop() {
  if (auto s = stopController_.lock()) {
    s->stop();
  }
}

void ThriftServer::stopListening() {
  // We have to make sure stopListening() is not called twice when both
  // stopListening() and cleanUp() are called
  {
    auto expected = ServerStatus::RUNNING;
    if (!internalStatus_.compare_exchange_strong(
            expected,
            ServerStatus::PRE_STOPPING,
            std::memory_order_release,
            std::memory_order_relaxed)) {
      // stopListening() was called earlier
      DCHECK(
          expected == ServerStatus::PRE_STOPPING ||
          expected == ServerStatus::STOPPING ||
          expected == ServerStatus::DRAINING_UNTIL_STOPPED ||
          expected == ServerStatus::NOT_RUNNING);
      return;
    }
  }

#if FOLLY_HAS_COROUTINES
  asyncScope_->requestCancellation();
#endif
  callOnStopRequested();

  {
    auto sockets = getSockets();
    folly::Baton<> done;
    SCOPE_EXIT {
      done.wait();
    };
    std::shared_ptr<folly::Baton<>> doneGuard(
        &done, [](folly::Baton<>* done) { done->post(); });

    for (auto& socket : sockets) {
      // Stop accepting new connections
      auto eb = socket->getEventBase();
      eb->runInEventBaseThread([socket = std::move(socket), doneGuard] {
        socket->pauseAccepting();
        // unset connection event callback
        socket->setConnectionEventCallback(nullptr);
      });
    }
  }

  if (stopWorkersOnStopListening_) {
    stopAcceptingAndJoinOutstandingRequests();
    stopCPUWorkers();
  }
}

void ThriftServer::stopWorkers() {
  ServerBootstrap::stop();
  ServerBootstrap::join();
  thriftConfig_.unfreeze();
}

void ThriftServer::stopAcceptingAndJoinOutstandingRequests() {
  {
    auto expected = ServerStatus::PRE_STOPPING;
    if (!internalStatus_.compare_exchange_strong(
            expected,
            ServerStatus::STOPPING,
            std::memory_order_release,
            std::memory_order_relaxed)) {
      // stopListening() was called earlier
      DCHECK(
          expected == ServerStatus::STOPPING ||
          expected == ServerStatus::DRAINING_UNTIL_STOPPED ||
          expected == ServerStatus::NOT_RUNNING);
      return;
    }
  }

  internalStatus_.store(
      ServerStatus::DRAINING_UNTIL_STOPPED, std::memory_order_release);

  forEachWorker([&](wangle::Acceptor* acceptor) {
    if (auto worker = dynamic_cast<Cpp2Worker*>(acceptor)) {
      worker->requestStop();
    }
  });
  // tlsCredWatcher_ uses a background thread that needs to be joined prior
  // to any further writes to ThriftServer members.
  tlsCredWatcher_.withWLock([](auto& credWatcher) { credWatcher.reset(); });
  sharedSSLContextManager_ = nullptr;

  {
    auto sockets = getSockets();
    folly::Baton<> done;
    SCOPE_EXIT {
      done.wait();
    };
    std::shared_ptr<folly::Baton<>> doneGuard(
        &done, [](folly::Baton<>* done) { done->post(); });

    for (auto& socket : sockets) {
      // We should have already paused accepting new connections. This just
      // closes the sockets once and for all.
      auto eb = socket->getEventBase();
      eb->runInEventBaseThread([socket = std::move(socket), doneGuard] {
        // This will also cause the workers to stop
        socket->stopAccepting();
      });
    }
  }

  auto joinDeadline =
      std::chrono::steady_clock::now() + getWorkersJoinTimeout();
  bool dumpSnapshotFlag = THRIFT_FLAG(dump_snapshot_on_long_shutdown);

  forEachWorker([&](wangle::Acceptor* acceptor) {
    if (auto worker = dynamic_cast<Cpp2Worker*>(acceptor)) {
      if (!worker->waitForStop(joinDeadline)) {
        // Before we crash, let's dump a snapshot of the server.

        // We create the CPUThreadPoolExecutor outside the if block so that it
        // doesn't wait for our task to complete when exiting the block even
        // after the timeout expires as we can't cancel the task
        folly::CPUThreadPoolExecutor dumpSnapshotExecutor{1};
        if (dumpSnapshotFlag) {
          // The IO threads may be deadlocked in which case we won't be able
          // to dump snapshots. It still shouldn't block shutdown
          // indefinitely.
          auto dumpSnapshotResult =
              apache::thrift::detail::dumpSnapshotOnLongShutdown();
          try {
            std::move(dumpSnapshotResult.task)
                .via(folly::getKeepAliveToken(dumpSnapshotExecutor))
                .get(dumpSnapshotResult.timeout);
          } catch (...) {
            XLOG(ERR) << "Failed to dump server snapshot on long shutdown: "
                      << folly::exceptionStr(folly::current_exception());
          }
        }

        constexpr auto msgTemplate =
            "Could not drain active requests within allotted deadline. "
            "Deadline value: {} secs. {} because undefined behavior is possible. "
            "Underlying reasons could be either requests that have never "
            "terminated, long running requests, or long queues that could "
            "not be fully processed.";
        if (quickExitOnShutdownTimeout_) {
          XLOG(ERR) << fmt::format(
              msgTemplate,
              getWorkersJoinTimeout().count(),
              "quick_exiting (no coredump)");
          // similar to abort but without generating a coredump
          try_quick_exit(124);
        }
        if (FLAGS_thrift_abort_if_exceeds_shutdown_deadline) {
          XLOG(FATAL) << fmt::format(
              msgTemplate, getWorkersJoinTimeout().count(), "Aborting");
        }
      }
    }
  });

  internalStatus_.store(ServerStatus::NOT_RUNNING, std::memory_order_release);
}

AsyncProcessorFactory& ThriftServer::getDecoratedProcessorFactory() const {
  CHECK(decoratedProcessorFactory_)
      << "Server must be set up before calling this method";
  return *decoratedProcessorFactory_;
}

void ThriftServer::ensureDecoratedProcessorFactoryInitialized() {
  DCHECK(getProcessorFactory().get());
  if (decoratedProcessorFactory_ == nullptr) {
    decoratedProcessorFactory_ = createDecoratedProcessorFactory(
        getProcessorFactory(),
        getStatusInterface(),
        getMonitoringInterface(),
        getControlInterface(),
        getSecurityInterface(),
        isCheckUnimplementedExtraInterfacesAllowed() &&
            THRIFT_FLAG(server_check_unimplemented_extra_interfaces));
  }
}

void ThriftServer::ensureProcessedServiceDescriptionInitialized() {
  ensureDecoratedProcessorFactoryInitialized();
  if (processedServiceDescription_ == nullptr) {
    auto modules = processModulesSpecification(
        std::exchange(unprocessedModulesSpecification_, {}));
    processedServiceDescription_ =
        ProcessedServiceDescription::createAndActivate(
            *this, ProcessedServiceDescription{std::move(modules)});
    runtimeServerActions_.moduleListFinalized = true;
  }
}

void ThriftServer::callInterceptorsOnStartServing(
    server::DecoratorDataPerRequestBlueprint::Setup& decoratorDataSetup) {
#if FOLLY_HAS_COROUTINES
  ServiceInterceptorBase::InitParams initParams;
  initParams.serviceSchema =
      decoratedProcessorFactory_->getServiceSchemaNodes();
  auto decoratorDataHandleFactory = decoratorDataSetup.getHandleFactory();
  initParams.decoratorDataHandleFactory = &decoratorDataHandleFactory;
  std::vector<folly::coro::Task<void>> tasks;
  for (const auto& interceptor : getServiceInterceptors()) {
    tasks.emplace_back(interceptor->co_onStartServing(initParams));
  }
  folly::coro::blockingWait(folly::coro::collectAllRange(std::move(tasks)));
#endif // FOLLY_HAS_COROUTINES
}

void ThriftServer::callDecoratorsAndHandlersBeforeStartServing(
    server::DecoratorDataPerRequestBlueprint::Setup& decoratorDataSetup) {
  auto decoratorDataHandleFactory = decoratorDataSetup.getHandleFactory();
  auto handlerList = collectServiceHandlers();
  for (auto handler : handlerList) {
    auto decorators = handler->fbthrift_getDecorators();
    for (auto& decorator : decorators) {
      ServiceMethodDecoratorBase::BeforeStartServingParams decoratorInitParams{
          &decoratorDataHandleFactory};
      decorator.get().onBeforeStartServing(decoratorInitParams);
    }
  }
  std::vector<folly::SemiFuture<folly::Unit>> futures;
  futures.reserve(handlerList.size());
  for (auto handler : handlerList) {
    futures.emplace_back(handler->semifuture_onBeforeStartServing(
        ServiceHandlerBase::BeforeStartServingParams{
            &decoratorDataHandleFactory}));
  }
  auto results =
      folly::collectAll(futures).via(getHandlerExecutorKeepAlive()).get();
  for (auto& result : results) {
    if (result.hasException()) {
      XLOG(FATAL) << "Exception thrown by beforeStartServing(): "
                  << folly::exceptionStr(result.exception());
    }
  }
}

void ThriftServer::callHandlersOnStartServing() {
  auto handlerList = collectServiceHandlers();
  // Exception is handled in setup()
  std::vector<folly::SemiFuture<folly::Unit>> futures;
  futures.reserve(handlerList.size());
  for (auto handler : handlerList) {
    futures.emplace_back(
        folly::makeSemiFuture().deferValue([handler](folly::Unit) {
          return handler->semifuture_onStartServing();
        }));
  }
  auto results =
      folly::collectAll(futures).via(getHandlerExecutorKeepAlive()).get();
  for (auto& result : results) {
    if (result.hasException()) {
      XLOG(FATAL) << "Exception thrown by onStartServing(): "
                  << folly::exceptionStr(result.exception());
    }
  }
}

void ThriftServer::callOnStopRequested() {
  auto handlerList = collectServiceHandlers();
  std::vector<folly::SemiFuture<folly::Unit>> futures;
  futures.reserve(handlerList.size());
  for (auto handler : handlerList) {
    futures.emplace_back(
        THRIFT_FLAG(enable_on_stop_serving)
            ? folly::makeSemiFuture().deferValue([handler](folly::Unit) {
                return handler->semifuture_onStopRequested();
              })
            : handler->semifuture_onStopRequested());
  }
  auto results =
      folly::collectAll(futures).via(getHandlerExecutorKeepAlive()).get();
  for (auto& result : results) {
    if (result.hasException()) {
      XLOG(FATAL) << "Exception thrown by onStopRequested(): "
                  << folly::exceptionStr(result.exception());
    }
  }
}

namespace {
ThriftServer* globalServer = nullptr;
}

#if FOLLY_HAS_COROUTINES
folly::coro::CancellableAsyncScope& ThriftServer::getGlobalAsyncScope() {
  DCHECK(globalServer);
  auto asyncScope = globalServer->getAsyncScope();
  DCHECK(asyncScope);
  return *asyncScope;
}
#endif

/* static */ void ThriftServer::setGlobalServer(ThriftServer* server) {
  globalServer = server;
}

/* static */ bool ThriftServer::isGlobalServerSet() {
  return globalServer != nullptr;
}

void ThriftServer::stopCPUWorkers() {
  // Wait for any tasks currently running on the task queue workers to
  // finish, then stop the task queue workers. Have to do this now, so
  // there aren't tasks completing and trying to write to i/o thread
  // workers after we've stopped the i/o workers.
  if (useResourcePools()) {
    resourcePoolSet().stopAndJoin();
  } else {
    CHECK(threadManager_);
    threadManager_->join();
  }
#if FOLLY_HAS_COROUTINES
  if (asyncScope_) {
    // Wait for tasks running on AsyncScope to join
    folly::coro::blockingWait(asyncScope_->joinAsync());
  }
  cachedServiceHealth_.store(ServiceHealth{}, std::memory_order_relaxed);
#endif

  // Notify handler of the postStop event
  for (const auto& eventHandler : getEventHandlersUnsafe()) {
    eventHandler->postStop();
  }

#if FOLLY_HAS_COROUTINES
  asyncScope_.reset();
#endif
}

void ThriftServer::handleSetupFailure(void) {
  ServerBootstrap::stop();

  // avoid crash on stop()
  idleServer_.reset();
  serveEventBase_ = nullptr;
  stopController_.join();
}

void ThriftServer::updateTicketSeeds(wangle::TLSTicketKeySeeds seeds) {
  if (sharedSSLContextManager_) {
    sharedSSLContextManager_->updateTLSTicketKeys(seeds);
  } else {
    forEachWorker([&](wangle::Acceptor* acceptor) {
      if (!acceptor) {
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

void ThriftServer::updateTLSCert() {
  if (sharedSSLContextManager_) {
    sharedSSLContextManager_->reloadSSLContextConfigs();
  } else {
    forEachWorker([&](wangle::Acceptor* acceptor) {
      if (!acceptor) {
        return;
      }
      auto evb = acceptor->getEventBase();
      if (!evb) {
        return;
      }
      evb->runInEventBaseThread(
          [acceptor] { acceptor->reloadSSLContextConfigs(); });
    });
  }
}

void ThriftServer::updateCertsToWatch() {
  std::set<std::string> certPaths;
  if (sslContextObserver_.has_value()) {
    auto sslContext = *sslContextObserver_->getSnapshot();
    if (!sslContext.certificates.empty()) {
      const auto& cert = sslContext.certificates[0];
      certPaths.insert(cert.certPath);
      certPaths.insert(cert.keyPath);
      certPaths.insert(cert.passwordPath);
    }

    for (auto& caPath : sslContext.clientCAFiles) {
      certPaths.insert(caPath);
    }
    if (sslContext.clientCAFiles.empty()) {
      certPaths.insert(sslContext.clientCAFile);
    }
    for (auto& dc : sslContext.delegatedCredentials) {
      certPaths.insert(dc.combinedCertPath);
    }
  }
  tlsCredWatcher_.withWLock([this, &certPaths](auto& credWatcher) {
    if (!credWatcher) {
      credWatcher.emplace(this);
    }
    credWatcher->setCertPathsToWatch(std::move(certPaths));
  });
}

void ThriftServer::watchTicketPathForChanges(const std::string& ticketPath) {
  auto seeds = TLSCredProcessor::processTLSTickets(ticketPath);
  if (seeds) {
    setTicketSeeds(*seeds);
    // Only start watching if we successfully read seeds
    tlsCredWatcher_.withWLock([this, &ticketPath](auto& credWatcher) {
      if (!credWatcher) {
        credWatcher.emplace(this);
      }
      credWatcher->setTicketPathToWatch(ticketPath);
    });
  }
  // If seeds is null, do nothing - rely on in-memory seeds with rotation
}

EffectiveTicketSeedStrategy ThriftServer::getEffectiveTicketSeedStrategy()
    const {
  bool readFromFile = tlsCredWatcher_.withRLock([](auto& credWatcher) {
    return credWatcher && credWatcher->hasTicketPathToWatch();
  });
  if (readFromFile) {
    return EffectiveTicketSeedStrategy::FILE;
  }
  return EffectiveTicketSeedStrategy::IN_MEMORY_WITH_ROTATION;
}

void ThriftServer::scheduleInMemoryTicketSeeds() {
  XLOG_IF(INFO, infoLoggingEnabled_)
      << "Using randomly generated TLS ticket keys.";
  wangle::TLSTicketKeySeeds seeds;
  tlsCredWatcher_.withWLock([this, &seeds](auto& credWatcher) {
    if (!credWatcher) {
      credWatcher.emplace(this);
    }
    seeds = credWatcher->initInMemoryTicketSeeds(this);
  });
  setTicketSeeds(std::move(seeds));
}

PreprocessResult ThriftServer::preprocess(
    const PreprocessParams& params) const {
  if (getMethodsBypassMaxRequestsLimit().contains(params.method)) {
    return {};
  }

  return preprocessFunctions_.run(params);
}

folly::Optional<OverloadResult> ThriftServer::checkOverload(
    const transport::THeader::StringToStringMap& readHeaders,
    const std::string& method) {
  if (UNLIKELY(
          isOverloaded_ &&
          !getMethodsBypassMaxRequestsLimit().contains(method) &&
          isOverloaded_(readHeaders, method))) {
    return OverloadResult{
        kAppOverloadedErrorCode,
        fmt::format(
            "Host {} is load shedding due to custom isOverloaded() callback.",
            getAddressAsString()),
        LoadShedder::CUSTOM};
  }

  // If active request tracking is disabled, skip max requests enforcement here.
  if (!isActiveRequestsTrackingDisabled()) {
    if (auto maxRequests = getMaxRequests(); maxRequests > 0 &&
        !getMethodsBypassMaxRequestsLimit().contains(method) &&
        static_cast<uint32_t>(getActiveRequests()) >= maxRequests) {
      LoadShedder loadShedder = LoadShedder::MAX_REQUESTS;
      if (notifyCPUConcurrencyControllerOnRequestLoadShed(
              CPUConcurrencyController::Method::MAX_REQUESTS)) {
        loadShedder = LoadShedder::CPU_CONCURRENCY_CONTROLLER;
      } else if (getAdaptiveConcurrencyController().enabled()) {
        loadShedder = LoadShedder::ADAPTIVE_CONCURRENCY_CONTROLLER;
      }
      return OverloadResult{
          kOverloadedErrorCode,
          "load shedding due to max request limit",
          loadShedder};
    }
  }

  if (auto maxQps = getMaxQps(); maxQps > 0 &&
      FLAGS_thrift_server_enforces_qps_limit &&
      !getMethodsBypassMaxRequestsLimit().contains(method) &&
      !qpsTokenBucket_.consume(1.0, maxQps, maxQps)) {
    LoadShedder loadShedder = LoadShedder::MAX_QPS;
    if (notifyCPUConcurrencyControllerOnRequestLoadShed(
            CPUConcurrencyController::Method::MAX_QPS)) {
      loadShedder = LoadShedder::CPU_CONCURRENCY_CONTROLLER;
    }
    return OverloadResult{
        kOverloadedErrorCode, "load shedding due to qps limit", loadShedder};
  }

  return {};
}

int64_t ThriftServer::getLoad(
    const std::string& counter, bool check_custom) const {
  if (check_custom && getLoad_) {
    return getLoad_(counter);
  }

  const auto activeRequests = getActiveRequests();

  if (VLOG_IS_ON(1)) {
    FB_LOG_EVERY_MS(INFO, 1000 * 10) << getLoadInfo(activeRequests);
  }

  return activeRequests;
}

std::string ThriftServer::getLoadInfo(int64_t load) const {
  auto ioGroup = getIOGroupSafe();
  auto workerFactory = ioGroup != nullptr
      ? std::dynamic_pointer_cast<folly::NamedThreadFactory>(
            ioGroup->getThreadFactory())
      : nullptr;

  if (!workerFactory) {
    return "";
  }

  std::stringstream stream;

  stream << workerFactory->getNamePrefix() << " load is: " << load
         << "% requests, " << getActiveRequests() << " active reqs";

  return stream.str();
}

void ThriftServer::replaceShutdownSocketSet(
    const std::shared_ptr<folly::ShutdownSocketSet>& newSSS) {
  wShutdownSocketSet_ = newSSS;
}

folly::SemiFuture<ThriftServer::ServerIOMemory>
ThriftServer::getUsedIOMemory() {
  // WorkerIOMemory looks the same as the server, except they are unaggregated
  using WorkerIOMemory = ServerIOMemory;
  std::vector<folly::SemiFuture<WorkerIOMemory>> tasks;

  std::shared_lock ioGroupLock(ioGroupMutex_);

  forEachWorker([&tasks](wangle::Acceptor* acceptor) {
    auto worker = dynamic_cast<Cpp2Worker*>(acceptor);
    if (!worker) {
      return;
    }
    auto fut = folly::via(worker->getEventBase(), [worker]() {
      auto& ingressMemTracker = worker->getIngressMemoryTracker();
      auto& egressMemTracker = worker->getEgressMemoryTracker();
      return WorkerIOMemory{
          ingressMemTracker.getUsage(), egressMemTracker.getUsage()};
    });
    tasks.emplace_back(std::move(fut));
  });

  return folly::collect(tasks.begin(), tasks.end())
      .deferValue(
          [ioGroupLock = std::move(ioGroupLock)](
              std::vector<WorkerIOMemory> workerIOMems) -> ServerIOMemory {
            ServerIOMemory ret{0, 0};
            // Sum all ingress and egress usages
            for (const auto& workerIOMem : workerIOMems) {
              ret.ingress += workerIOMem.ingress;
              ret.egress += workerIOMem.egress;
            }
            return ret;
          });
}

folly::SemiFuture<ThriftServer::ServerSnapshot> ThriftServer::getServerSnapshot(
    const SnapshotOptions& options) {
  // WorkerSnapshots look the same as the server, except they are unaggregated
  using WorkerSnapshot = ServerSnapshot;
  std::vector<folly::SemiFuture<WorkerSnapshot>> tasks;
  const auto snapshotTime = std::chrono::steady_clock::now();

  forEachWorker([&tasks, snapshotTime, options](wangle::Acceptor* acceptor) {
    auto worker = dynamic_cast<Cpp2Worker*>(acceptor);
    if (!worker) {
      return;
    }
    auto fut =
        folly::via(worker->getEventBase(), [worker, snapshotTime, options]() {
          auto reqRegistry = worker->getRequestsRegistry();
          DCHECK(reqRegistry);
          RequestSnapshots requestSnapshots;
          if (reqRegistry != nullptr) {
            for (const auto& stub : reqRegistry->getActive()) {
              requestSnapshots.emplace_back(stub);
            }
            for (const auto& stub : reqRegistry->getFinished()) {
              requestSnapshots.emplace_back(stub);
            }
          }

          std::unordered_map<folly::SocketAddress, ConnectionSnapshot>
              connectionSnapshots;
          // ConnectionManager can be nullptr if the worker didn't have any
          // open connections during shutdown
          if (auto connectionManager = worker->getConnectionManager()) {
            connectionManager->forEachConnection([&](wangle::ManagedConnection*
                                                         wangleConnection) {
              if (auto managedConnection =
                      dynamic_cast<ManagedConnectionIf*>(wangleConnection)) {
                auto numActiveRequests =
                    managedConnection->getNumActiveRequests();
                auto numPendingWrites =
                    managedConnection->getNumPendingWrites();
                auto creationTime = managedConnection->getCreationTime();
                auto minCreationTime = snapshotTime - options.connectionsAgeMax;
                if (numActiveRequests > 0 || numPendingWrites > 0 ||
                    creationTime > minCreationTime) {
                  connectionSnapshots.emplace(
                      managedConnection->getPeerAddress(),
                      ConnectionSnapshot{
                          numActiveRequests, numPendingWrites, creationTime});
                }
              }
            });
          }

          ServerIOMemory serverIOMemory;
          serverIOMemory.ingress = worker->getIngressMemoryTracker().getUsage();
          serverIOMemory.egress = worker->getEgressMemoryTracker().getUsage();

          return WorkerSnapshot{
              worker->getRequestsRegistry()->getRequestCounter().get(),
              std::move(requestSnapshots),
              std::move(connectionSnapshots),
              std::move(serverIOMemory)};
        });
    tasks.emplace_back(std::move(fut));
  });

  return folly::collect(tasks.begin(), tasks.end())
      .deferValue(
          [](std::vector<WorkerSnapshot> workerSnapshots) -> ServerSnapshot {
            ServerSnapshot ret{};

            // Sum all request and connection counts and memory usages
            size_t numRequests = 0;
            size_t numConnections = 0;
            for (const auto& workerSnapshot : workerSnapshots) {
              for (uint64_t i = 0; i < ret.recentCounters.size(); ++i) {
                ret.recentCounters[i].arrivalCount +=
                    workerSnapshot.recentCounters[i].arrivalCount;
                ret.recentCounters[i].activeCount +=
                    workerSnapshot.recentCounters[i].activeCount;
                ret.recentCounters[i].overloadCount +=
                    workerSnapshot.recentCounters[i].overloadCount;
              }
              numRequests += workerSnapshot.requests.size();
              numConnections += workerSnapshot.connections.size();
              ret.memory.ingress += workerSnapshot.memory.ingress;
              ret.memory.egress += workerSnapshot.memory.egress;
            }

            // Move all RequestSnapshots, ServerIOMemory and
            // ConnectionSnapshots to ServerSnapshot
            ret.requests.reserve(numRequests);
            ret.connections.reserve(numConnections);
            for (auto& workerSnapshot : workerSnapshots) {
              auto& requests = workerSnapshot.requests;
              std::move(
                  requests.begin(),
                  requests.end(),
                  std::back_inserter(ret.requests));

              auto& connections = workerSnapshot.connections;
              std::move(
                  connections.begin(),
                  connections.end(),
                  std::inserter(ret.connections, ret.connections.end()));
            }
            return ret;
          });
}

folly::observer::Observer<std::list<std::string>>
ThriftServer::defaultNextProtocols() {
  return folly::observer::makeStaticObserver(
      std::make_shared<std::list<std::string>>(std::list<std::string>{
          "rs",
          "thrift",
          "h2",
          // "http" is not a legit specifier but need to include it for
          // legacy.  Thrift's HTTP2RoutingHandler uses this, and clients
          // may be sending it.
          "http",
          // Many clients still send http/1.1 which is handled by the
          // default handler.
          "http/1.1"}));
}

folly::observer::Observer<bool> ThriftServer::enableStopTLS() {
  return THRIFT_FLAG_OBSERVE(server_enable_stoptls);
}

folly::observer::Observer<bool> ThriftServer::enableStopTLSV2() {
  return THRIFT_FLAG_OBSERVE(server_enable_stoptlsv2);
}

folly::observer::Observer<bool> ThriftServer::enableReceivingDelegatedCreds() {
  return THRIFT_FLAG_OBSERVE(server_fizz_enable_receiving_dc);
}

folly::observer::CallbackHandle ThriftServer::getSSLCallbackHandle() {
  auto originalPid = folly::get_cached_pid();

  return sslContextObserver_->addCallback([&, originalPid](auto ssl) {
    // Because we are posting to an EventBase, we need to ensure that this
    // observer callback is not executing on a fork()'d child.
    //
    // The scenario this can happen is if:
    //  1) The FlagsBackend observer implementation persists in the child.
    //  (e.g.
    //     a custom atfork handler reinitializes and resubscribes to updates)
    //  2) A thrift handler fork()s (e.g. Python thrift server which
    //     uses concurrent.futures.ProcessPoolExecutor)
    //  3) A flag is changed that causes an update to sslContextObserver
    if (folly::get_cached_pid() != originalPid) {
      LOG(WARNING)
          << "Ignoring SSLContext update triggered by observer in forked process.";
      return;
    }
    if (sharedSSLContextManager_) {
      sharedSSLContextManager_->updateSSLConfigAndReloadContexts(*ssl);
    } else {
      // "this" needed due to
      // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67274
      this->forEachWorker([&](wangle::Acceptor* acceptor) {
        auto evb = acceptor->getEventBase();
        if (!evb) {
          return;
        }

        evb->runInEventBaseThread(
            [acceptor, ssl] { acceptor->resetSSLContextConfigs({*ssl}); });
      });
    }
    this->updateCertsToWatch();
  });
}

void ThriftServer::addIOThreadPoolObserverFactory(
    ThriftServer::IOObserverFactory factory) {
  ioObserverFactories.wlock()->push_back(std::move(factory));
}

/* static */ std::shared_ptr<folly::IOThreadPoolExecutorBase>
ThriftServer::createIOThreadPool() {
  return std::make_shared<folly::IOThreadPoolExecutor>(
      0,
      std::make_shared<folly::NamedThreadFactory>("ThriftIO"),
      folly::EventBaseManager::get(),
      folly::IOThreadPoolExecutor::Options().setEnableThreadIdCollection(
          THRIFT_FLAG(enable_io_queue_lag_detection)));
}

namespace {
struct NewConnectionContextHolder
    : public folly::AsyncSocket::LegacyLifecycleObserver {
  explicit NewConnectionContextHolder(ThriftServer::NewConnectionContext c)
      : ctx(std::move(c)) {}

  void observerAttach(folly::AsyncSocket*) noexcept override {}
  void observerDetach(folly::AsyncSocket*) noexcept override { delete this; }
  void destroy(folly::AsyncSocket*) noexcept override { delete this; }
  void close(folly::AsyncSocket*) noexcept override {}

  ThriftServer::NewConnectionContext ctx;
};
} // namespace

void ThriftServer::acceptConnection(
    folly::NetworkSocket fd,
    const folly::SocketAddress& clientAddr,
    folly::AsyncServerSocket::AcceptCallback::AcceptInfo info,
    std::shared_ptr<AsyncProcessorFactory> processor) {
  this->acceptConnection(
      fd,
      clientAddr,
      std::move(info),
      new NewConnectionContextHolder(
          ThriftServer::NewConnectionContext{std::move(processor)}));
}

folly::Optional<ThriftServer::NewConnectionContext>
ThriftServer::extractNewConnectionContext(folly::AsyncTransport& transport) {
  if (auto sock = transport.getUnderlyingTransport<folly::AsyncSocket>()) {
    for (auto observer : sock->getLifecycleObservers()) {
      if (auto ctxHolder =
              dynamic_cast<NewConnectionContextHolder*>(observer)) {
        auto ctx = std::move(ctxHolder->ctx);
        sock->removeLifecycleObserver(observer);
        return ctx;
      }
    }
  }
  return folly::none;
}

folly::observer::Observer<bool> ThriftServer::enableHybridKex() {
  return THRIFT_FLAG_OBSERVE(fizz_server_enable_hybrid_kex);
}

folly::observer::Observer<bool> ThriftServer::enableAegis() {
  return THRIFT_FLAG_OBSERVE(server_fizz_enable_aegis);
}

folly::observer::Observer<bool> ThriftServer::preferPskKe() {
  return THRIFT_FLAG_OBSERVE(server_fizz_prefer_psk_ke);
}

folly::observer::Observer<bool>
ThriftServer::enablePresentingDelegatedCredentials() {
  return THRIFT_FLAG_OBSERVE(server_fizz_enable_presenting_dc);
}

serverdbginfo::ResourcePoolsDbgInfo ThriftServer::getResourcePoolsDbgInfo()
    const {
  if (!resourcePoolEnabled()) {
    serverdbginfo::ResourcePoolsDbgInfo info;
    info.enabled() = false;
    return info;
  }

  serverdbginfo::ResourcePoolsDbgInfo info;
  info.enabled() = true;
  info.resourcePools() = {};

  resourcePoolSet().forEachResourcePool([&](ResourcePool* resourcePool) {
    if (!resourcePool) {
      return;
    }

    info.resourcePools()->push_back(resourcePool->getDbgInfo());
  });

  return info;
}

folly::observer::Observer<CPUConcurrencyController::Config>
ThriftServer::makeCPUConcurrencyControllerConfigInternal() {
  return folly::observer::makeObserver(
      [mockConfig = mockCPUConcurrencyControllerConfig_.getObserver(),
       config = detail::makeCPUConcurrencyControllerConfig(
           this)]() -> CPUConcurrencyController::Config {
        if (auto config_2 = **mockConfig) {
          return *config_2;
        }
        return **config;
      });
}

void ThriftServer::CumulativeFailureInjection::set(const FailureInjection& fi) {
  CHECK_GE(fi.errorFraction, 0);
  CHECK_GE(fi.dropFraction, 0);
  CHECK_GE(fi.disconnectFraction, 0);
  CHECK_LE(fi.errorFraction + fi.dropFraction + fi.disconnectFraction, 1);

  std::lock_guard lock(mutex_);
  errorThreshold_ = fi.errorFraction;
  dropThreshold_ = errorThreshold_ + fi.dropFraction;
  disconnectThreshold_ = dropThreshold_ + fi.disconnectFraction;
  empty_.store((disconnectThreshold_ == 0), std::memory_order_relaxed);
}

ThriftServer::InjectedFailure ThriftServer::CumulativeFailureInjection::test()
    const {
  if (empty_.load(std::memory_order_relaxed)) {
    return InjectedFailure::NONE;
  }

  static folly::ThreadLocalPtr<std::mt19937> rng;
  if (!rng) {
    rng.reset(new std::mt19937(folly::randomNumberSeed()));
  }

  std::uniform_real_distribution<float> dist(0, 1);
  float val = dist(*rng);

  std::shared_lock lock(mutex_);
  if (val <= errorThreshold_) {
    return InjectedFailure::ERROR;
  } else if (val <= dropThreshold_) {
    return InjectedFailure::DROP;
  } else if (val <= disconnectThreshold_) {
    return InjectedFailure::DISCONNECT;
  }
  return InjectedFailure::NONE;
}

std::vector<std::pair<std::string, std::string>>
ThriftServer::RuntimeServerActions::toStringPairs() const {
  std::vector<std::pair<std::string, std::string>> result;

  // clang-format off
  result.emplace_back("userSuppliedThreadManager",          userSuppliedThreadManager ? "true" : "false");
  result.emplace_back("userSuppliedResourcePools",          userSuppliedResourcePools ? "true" : "false");
  result.emplace_back("interactionInService",               interactionInService ? "true" : "false");
  result.emplace_back("wildcardMethods",                    wildcardMethods ? "true" : "false");
  result.emplace_back("noServiceRequestInfo",               noServiceRequestInfo ? "true" : "false");
  result.emplace_back("activeRequestTrackingDisabled",      activeRequestTrackingDisabled ? "true" : "false");
  result.emplace_back("setPreprocess",                      setPreprocess ? "true" : "false");
  result.emplace_back("setIsOverloaded",                    setIsOverloaded ? "true" : "false");
  result.emplace_back("resourcePoolFlagSet",                resourcePoolFlagSet ? "true" : "false");
  result.emplace_back("codelEnabled",                       codelEnabled ? "true" : "false");
  result.emplace_back("setupThreadManagerBeforeHandler",    setupThreadManagerBeforeHandler ? "true" : "false");
  result.emplace_back("resourcePoolEnablementLocked",       resourcePoolEnablementLocked ? "true" : "false");
  result.emplace_back("resourcePoolRuntimeRequested",       resourcePoolRuntimeRequested ? "true" : "false");
  result.emplace_back("resourcePoolRuntimeDisabled",        resourcePoolRuntimeDisabled ? "true" : "false");
  result.emplace_back("resourcePoolRuntimeDisabledReason",  resourcePoolRuntimeDisabledReason);
  result.emplace_back("resourcePoolEnabled",                resourcePoolEnabled ? "true" : "false");
  result.emplace_back("resourcePoolEnabledGflag",           resourcePoolEnabledGflag ? "true" : "false");
  result.emplace_back("resourcePoolDisabledGflag",          resourcePoolDisabledGflag ? "true" : "false");
  result.emplace_back("checkComplete",                      checkComplete ? "true" : "false");
  result.emplace_back("isProcessorFactoryThriftGenerated",  isProcessorFactoryThriftGenerated ? "true" : "false");
  result.emplace_back("executorToThreadManagerUnexpectedFunctionName", executorToThreadManagerUnexpectedFunctionName);
  // clang-format on

  return result;
}

std::string ThriftServer::RuntimeServerActions::explain() const {
  std::string result;
  result = std::string(userSuppliedThreadManager ? "setThreadManager, " : "") +
      (userSuppliedResourcePools ? "userSuppliedResourcePools, " : "") +
      (interactionInService ? "interactionInService, " : "") +
      (wildcardMethods ? "wildcardMethods, " : "") +
      (noServiceRequestInfo ? "noServiceRequestInfo, " : "") +
      (activeRequestTrackingDisabled ? "activeRequestTrackingDisabled, " : "") +
      (setPreprocess ? "setPreprocess, " : "") +
      (setIsOverloaded ? "setIsOverloaded, " : "") +
      (codelEnabled ? "codelEnabled, " : "") +
      (setupThreadManagerBeforeHandler ? "setupThreadManagerBeforeHandler, "
                                       : "") +
      (!resourcePoolFlagSet ? "thriftFlagNotSet, " : "");
  return result;
}

namespace {

void checkModuleNameIsValid(const std::string& moduleName) {
  CHECK(!moduleName.empty()) << "Module name cannot be empty";
  CHECK(moduleName.find_first_of(".,") == std::string::npos)
      << "The character '.' is not allowed in module names - got: "
      << moduleName;
}

} // namespace

/* static */ ThriftServer::ProcessedModuleSet
ThriftServer::processModulesSpecification(ModulesSpecification&& specs) {
  ProcessedModuleSet result;
#if FOLLY_HAS_COROUTINES
  class ServiceInterceptorsCollector {
   public:
    void addModule(const ModulesSpecification::Info& moduleSpec) {
      checkModuleNameIsValid(moduleSpec.name);
      std::vector<std::shared_ptr<ServiceInterceptorBase>> serviceInterceptors =
          moduleSpec.module->getServiceInterceptors();

      std::vector<std::shared_ptr<ServiceInterceptorBase>> result;
      for (auto& interceptor : serviceInterceptors) {
        interceptor->setModuleName(moduleSpec.name);
        auto qualifiedNameStr = interceptor->getQualifiedName().get();
        if (seenNames_.find(qualifiedNameStr) != seenNames_.end()) {
          throw std::logic_error(
              fmt::format(
                  "Duplicate ServiceInterceptor: {}", qualifiedNameStr));
        }
        seenNames_.insert(qualifiedNameStr);
        result.emplace_back(std::move(interceptor));
      }
      interceptorsByModule_.emplace_back(std::move(result));
    }

    std::vector<std::shared_ptr<ServiceInterceptorBase>> coalesce() && {
      if constexpr (folly::kIsDebug) {
        // Our contract guarantees ordering of interceptors within a module, but
        // not across modules. In debug mode, let's shuffle order across modules
        // so that tests break if someone is relying on such ordering.
        // The quality of the RNG is not important here.
        std::shuffle(
            interceptorsByModule_.begin(),
            interceptorsByModule_.end(),
            folly::Random::DefaultGenerator());
      }

      std::vector<std::shared_ptr<ServiceInterceptorBase>> result;
      for (auto& interceptors : interceptorsByModule_) {
        result.insert(
            result.end(),
            std::make_move_iterator(interceptors.begin()),
            std::make_move_iterator(interceptors.end()));
      }
      return result;
    }

   private:
    std::vector<std::vector<std::shared_ptr<ServiceInterceptorBase>>>
        interceptorsByModule_;
    std::unordered_set<std::string_view> seenNames_;
  };
#else
  class ServiceInterceptorsCollector {
   public:
    void addModule(const ModulesSpecification::Info&) {}
    std::vector<std::shared_ptr<ServiceInterceptorBase>> coalesce() && {
      return {};
    }
  };
#endif // FOLLY_HAS_COROUTINES
  ServiceInterceptorsCollector serviceInterceptorsCollector;

  for (auto& info : specs.infos) {
    std::vector<std::shared_ptr<TProcessorEventHandler>> legacyEventHandlers =
        info.module->getLegacyEventHandlers();
    std::vector<std::shared_ptr<server::TServerEventHandler>>
        legacyServerEventHandlers = info.module->getLegacyServerEventHandlers();

    result.coalescedLegacyEventHandlers.insert(
        result.coalescedLegacyEventHandlers.end(),
        std::make_move_iterator(legacyEventHandlers.begin()),
        std::make_move_iterator(legacyEventHandlers.end()));
    result.coalescedLegacyServerEventHandlers.insert(
        result.coalescedLegacyServerEventHandlers.end(),
        std::make_move_iterator(legacyServerEventHandlers.begin()),
        std::make_move_iterator(legacyServerEventHandlers.end()));

    serviceInterceptorsCollector.addModule(info);

    result.modules.emplace_back(std::move(info));
  }
  result.coalescedServiceInterceptors =
      std::move(serviceInterceptorsCollector).coalesce();

  return result;
}

bool ThriftServer::getTaskExpireTimeForRequest(
    const apache::thrift::transport::THeader& requestHeader,
    std::chrono::milliseconds& queueTimeout,
    std::chrono::milliseconds& taskTimeout) const {
  return getTaskExpireTimeForRequest(
      requestHeader.getClientQueueTimeout(),
      requestHeader.getClientTimeout(),
      queueTimeout,
      taskTimeout);
}

bool ThriftServer::getTaskExpireTimeForRequest(
    std::chrono::milliseconds clientQueueTimeoutMs,
    std::chrono::milliseconds clientTimeoutMs,
    std::chrono::milliseconds& queueTimeout,
    std::chrono::milliseconds& taskTimeout) const {
  taskTimeout = getTaskExpireTime();

  // Client side always take precedence in deciding queue_timetout.
  queueTimeout = clientQueueTimeoutMs;
  if (queueTimeout == std::chrono::milliseconds(0)) {
    queueTimeout = getQueueTimeout();
  }
  auto useClientTimeout = getUseClientTimeout() && clientTimeoutMs.count() >= 0;
  // If queue timeout was set to 0 explicitly, this request has opt-out of queue
  // timeout.
  if (queueTimeout != std::chrono::milliseconds(0)) {
    auto queueTimeoutPct = getQueueTimeoutPct();
    // If queueTimeoutPct was set, we use it to calculate another queue timeout
    // based on client timeout. And then use the max of the explicite setting
    // and inferenced queue timeout.
    if (queueTimeoutPct > 0 && queueTimeoutPct < 100 && useClientTimeout) {
      queueTimeout =
          max(queueTimeout,
              std::chrono::milliseconds(
                  (clientTimeoutMs.count() * queueTimeoutPct / 100)));
    }
  }

  if (taskTimeout != std::chrono::milliseconds(0) && useClientTimeout) {
    // we add 10% to the client timeout so that the request is much more likely
    // to timeout on the client side than to read the timeout from the server
    // as a TApplicationException (which can be confusing)
    taskTimeout =
        std::chrono::milliseconds((uint32_t)(clientTimeoutMs.count() * 1.1));
  }
  // Queue timeout shouldn't be greater than task timeout
  if (taskTimeout < queueTimeout &&
      taskTimeout != std::chrono::milliseconds(0)) {
    queueTimeout = taskTimeout;
  }
  return queueTimeout != taskTimeout;
}

bool ThriftServer::notifyCPUConcurrencyControllerOnRequestLoadShed(
    std::optional<CPUConcurrencyController::Method> method) {
  auto* cpuConcurrencyControllerPtr = getCPUConcurrencyController();
  return cpuConcurrencyControllerPtr != nullptr &&
      cpuConcurrencyControllerPtr->requestShed(method);
}

const std::vector<std::string> ThriftServer::getInstalledServerModuleNames()
    const noexcept {
  std::vector<std::string> moduleNames;
  if (processedServiceDescription_) {
    std::transform(
        processedServiceDescription_->modules.modules.begin(),
        processedServiceDescription_->modules.modules.end(),
        std::back_inserter(moduleNames),
        [](const auto& moduleInfo) { return moduleInfo.name; });
  }
  return moduleNames;
}

server::DecoratorDataPerRequestBlueprint&
ThriftServer::getDecoratorDataPerRequestBlueprint() {
  return decoratorDataPerRequestBlueprint_.value();
}

} // namespace apache::thrift
