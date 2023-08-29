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

#include <fcntl.h>
#include <signal.h>

#include <thrift/lib/cpp2/server/IOUringUtil.h>

#include <thrift/lib/cpp2/server/ThriftServer.h>

#include <iostream>
#include <random>
#include <utility>
#include <variant>

#include <glog/logging.h>
#include <folly/Conv.h>
#include <folly/Memory.h>
#include <folly/ScopeGuard.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/IOThreadPoolDeadlockDetectorObserver.h>
#include <folly/executors/thread_factory/InitThreadFactory.h>
#include <folly/executors/thread_factory/NamedThreadFactory.h>
#include <folly/executors/thread_factory/PriorityThreadFactory.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/CurrentExecutor.h>
#include <folly/experimental/coro/Invoke.h>
#include <folly/io/GlobalShutdownSocketSet.h>
#include <folly/portability/Sockets.h>
#include <folly/system/Pid.h>
#include <thrift/lib/cpp/concurrency/InitThreadFactory.h>
#include <thrift/lib/cpp/concurrency/PosixThreadFactory.h>
#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/MultiplexAsyncProcessor.h>
#include <thrift/lib/cpp2/server/Cpp2Connection.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ExecutorToThreadManagerAdaptor.h>
#include <thrift/lib/cpp2/server/LoggingEvent.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>
#include <thrift/lib/cpp2/server/ServerInstrumentation.h>
#include <thrift/lib/cpp2/server/StandardConcurrencyController.h>
#include <thrift/lib/cpp2/server/TMConcurrencyController.h>
#include <thrift/lib/cpp2/server/ThriftProcessor.h>
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
    thrift_ssl_policy, "disabled", "SSL required / permitted / disabled");

FOLLY_GFLAGS_DEFINE_string(
    service_identity,
    "",
    "The name of the service. Associates the service with ACLs and keys");

THRIFT_FLAG_DEFINE_bool(server_alpn_prefer_rocket, true);
THRIFT_FLAG_DEFINE_bool(server_enable_stoptls, false);
THRIFT_FLAG_DEFINE_bool(enable_mrl_check_for_thrift_server, false);
THRIFT_FLAG_DEFINE_bool(enforce_mrl_check_for_thrift_server, false);

THRIFT_FLAG_DEFINE_bool(dump_snapshot_on_long_shutdown, true);

THRIFT_FLAG_DEFINE_bool(server_check_unimplemented_extra_interfaces, true);

THRIFT_FLAG_DEFINE_bool(enable_on_stop_serving, true);

THRIFT_FLAG_DEFINE_bool(enable_io_queue_lag_detection, true);

THRIFT_FLAG_DEFINE_bool(enforce_queue_concurrency_resource_pools, false);

THRIFT_FLAG_DEFINE_bool(fizz_server_enable_hybrid_kex, false);

THRIFT_FLAG_DEFINE_bool(server_fizz_enable_aegis, false);

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
      nullptr /* monitoring */, nullptr /* status */, nullptr /* control */};
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    ThriftServer::UnimplementedExtraInterfacesResult,
    serviceHasUnimplementedExtraInterfaces,
    AsyncProcessorFactory& /* service */) {
  return ThriftServer::UnimplementedExtraInterfacesResult::UNRECOGNIZED;
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

namespace apache {
namespace thrift {

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
 * Multiplexes the user-service (set via setProcessorFactory) with the
 * monitoring interface (set via setMonitoringInterface).
 */
std::unique_ptr<AsyncProcessorFactory> createDecoratedProcessorFactory(
    std::shared_ptr<AsyncProcessorFactory> processorFactory,
    std::shared_ptr<StatusServerInterface> statusProcessorFactory,
    std::shared_ptr<MonitoringServerInterface> monitoringProcessorFactory,
    std::shared_ptr<ControlServerInterface> controlProcessorFactory,
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

ThriftServer::ThriftServer()
    : BaseThriftServer(),
      wShutdownSocketSet_(folly::tryGetShutdownSocketSet()),
      lastRequestTime_(
          std::chrono::steady_clock::now().time_since_epoch().count()) {
  tracker_.emplace(instrumentation::kThriftServerTrackerKey, *this);
  initializeDefaults();
}

ThriftServer::ThriftServer(const ThriftServerInitialConfig& initialConfig)
    : BaseThriftServer(initialConfig),
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
  setInternalMethods(std::unordered_set<std::string>(
      methodsBypassMaxRequestsLimit.begin(),
      methodsBypassMaxRequestsLimit.end()));
  thriftConfig_.methodsBypassMaxRequestsLimit_.setDefault(
      std::move(methodsBypassMaxRequestsLimit));

  setMonitoringInterface(std::move(extraInterfaces.monitoring));
  setStatusInterface(std::move(extraInterfaces.status));
  setControlInterface(std::move(extraInterfaces.control));
  getAdaptiveConcurrencyController().setConfigUpdateCallback(
      [this](auto snapshot) {
        if (snapshot->isEnabled()) {
          THRIFT_SERVER_EVENT(ACC_enabled).log(*this);
        }
      });
}

ThriftServer::~ThriftServer() {
  tracker_.reset();

  SCOPE_EXIT { stopController_.join(); };

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

void ThriftServer::setProcessorFactory(
    std::shared_ptr<AsyncProcessorFactory> pFac) {
  CHECK(configMutable());
  BaseThriftServer::setProcessorFactory(pFac);
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
      LOG(INFO) << "shutting down server due to inactivity after "
                << std::chrono::duration_cast<std::chrono::milliseconds>(
                       elapsed)
                       .count()
                << "ms";
      server_.stop();
      return;
    }

    timer_.scheduleTimeout(this, timeout_);
  } catch (const std::exception& e) {
    LOG(ERROR) << e.what();
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
#ifdef HAS_IO_URING
  if (preferIoUring_) {
    VLOG(1) << "Preferring io_uring";
    auto b = io_uring_util::validateExecutorSupportsIOUring(ioThreadPool_);

    if (!b) {
      if (!useDefaultIoUringExecutor_) {
        VLOG(1)
            << "Configured IOThreadPoolExecutor does not support io_uring, but default not selected. epoll will be used";
        usingIoUring_ = false;
        return;
      }

      VLOG(1) << "Configured IOThreadPoolExecutor does not support io_uring, "
                 "configuring default io_uring IOThreadPoolExecutor pool";
      ioThreadPool_ = io_uring_util::getDefaultIOUringExecutor(
          THRIFT_FLAG(enable_io_queue_lag_detection));
      usingIoUring_ = true;
    } else {
      VLOG(1) << "Configured IOThreadPoolExecutor supports io_uring";
      usingIoUring_ = true;
    }
  }
#endif
}

void ThriftServer::setup() {
  ensureDecoratedProcessorFactoryInitialized();

  auto nWorkers = getNumIOWorkerThreads();
  DCHECK_GT(nWorkers, 0u);

  addRoutingHandler(
      std::make_unique<apache::thrift::RocketRoutingHandler>(*this));

  // Initialize event base for this thread
  auto serveEventBase = eventBaseManager_->getEventBase();
  serveEventBase_ = serveEventBase;
  stopController_.set(std::make_unique<StopController>(
      folly::badge<ThriftServer>{}, *serveEventBase));
  if (idleServerTimeout_.count() > 0) {
    idleServer_.emplace(*this, serveEventBase->timer(), idleServerTimeout_);
  }
  // Print some libevent stats
  VLOG(1) << "libevent " << folly::EventBase::getLibeventVersion() << " method "
          << serveEventBase->getLibeventMethod();

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
    runtimeResourcePoolsChecks();

    setupThreadManager();

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
      std::lock_guard<std::mutex> lock(ioGroupMutex_);
      ServerBootstrap::group(acceptPool_, ioThreadPool_);
    }
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
          socket->setCallbackAssignFunction(std::move(callbackAssignFunc_));
        }

        try {
          socket->setTosReflect(tosReflect_);
          socket->setListenerTos(listenerTos_);
        } catch (std::exception const& ex) {
          LOG(ERROR) << "Got exception setting up TOS settings: "
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
    callOnStartServing();

    // After the onStartServing hooks have finished, we are ready to handle
    // requests, at least from the server's perspective.
    internalStatus_.store(ServerStatus::RUNNING, std::memory_order_release);

#if FOLLY_HAS_COROUTINES
    // Set up polling for PolledServiceHealth handlers if necessary
    {
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
            std::move(loop).scheduleOn(getHandlerExecutorKeepAlive()));
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
    LOG(ERROR) << "Got an exception while setting up the server: " << ex.what();
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
  if (!setupThreadManagerCalled_) {
    setupThreadManagerCalled_ = true;

    // Try the runtime checks - if it is too early to complete them we will
    // retry on setup()
    runtimeResourcePoolsChecks();

    // Past this point no modification to the enablement of
    // ResourcePool should be made in the same server
    runtimeServerActions_.resourcePoolFlagSet =
        THRIFT_FLAG(experimental_use_resource_pools);

    // Ensure that either the thread manager or resource pools exist.
    if (!useResourcePools()) {
      DCHECK(resourcePoolSet().empty());
      // We always need a threadmanager for cpp2.
      auto explanation = fmt::format(
          "runtime: {}, thrift flag: {}, enable gflag: {}, disable gflag: {}",
          runtimeServerActions_.explain(),
          THRIFT_FLAG(experimental_use_resource_pools),
          FLAGS_thrift_experimental_use_resource_pools,
          FLAGS_thrift_disable_resource_pools);
      LOG(INFO)
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
            LOG(FATAL)
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
          "thrift flag: {}, enable gflag: {}, dsiable gflag: {}",
          THRIFT_FLAG(experimental_use_resource_pools),
          FLAGS_thrift_experimental_use_resource_pools,
          FLAGS_thrift_disable_resource_pools);
      LOG(INFO) << "Using resource pools on address/port "
                << getAddressAsString() << ": " << explanation;
      if (auto observer = getObserverShared()) {
        observer->resourcePoolsEnabled(explanation);
      }

      LOG(INFO) << "QPS limit will be enforced by "
                << (FLAGS_thrift_server_enforces_qps_limit
                        ? "the thrift server"
                        : "the concurrency controller");

      ensureResourcePools();

      // During resource pools roll out we want to track services that get
      // enrolled in the roll out.
      THRIFT_SERVER_EVENT(resourcepoolsenabled).log(*this);
    }
  }

  // Now do setup that we want to do whether we created these resources or the
  // client did.
  if (!resourcePoolSet().empty()) {
    // Keep concurrency controller in sync with max requests for now.
    setMaxRequestsCallbackHandle =
        detail::getThriftServerConfig(*this)
            .getMaxRequests()
            .getObserver()
            .addCallback([this](folly::observer::Snapshot<uint32_t> snapshot) {
              auto maxRequests = *snapshot;
              resourcePoolSet()
                  .resourcePool(ResourcePoolHandle::defaultAsync())
                  .concurrencyController()
                  .value()
                  .get()
                  .setExecutionLimitRequests(
                      maxRequests != 0
                          ? maxRequests
                          : std::numeric_limits<decltype(maxRequests)>::max());
            });
    setMaxQpsCallbackHandle =
        detail::getThriftServerConfig(*this)
            .getMaxQps()
            .getObserver()
            .addCallback([this](folly::observer::Snapshot<uint32_t> snapshot) {
              auto maxQps = *snapshot;
              resourcePoolSet()
                  .resourcePool(ResourcePoolHandle::defaultAsync())
                  .concurrencyController()
                  .value()
                  .get()
                  .setQpsLimit(
                      maxQps != 0
                          ? maxQps
                          : std::numeric_limits<decltype(maxQps)>::max());
            });
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
        } else {
          observer->shadowQueueTimeout();
        }
      }
    });
  }

  // After this point there should be no further changes to resource pools. We
  // lock whether or not we are actually using them so that the checks on
  // resourcePoolSet().empty() can be efficient.
  resourcePoolSet().lock();

  if (!resourcePoolSet().empty()) {
    LOG(INFO) << "Resource pools (" << resourcePoolSet().size()
              << "): " << resourcePoolSet().describe();

    auto descriptions = resourcePoolSet().poolsDescriptions();
    if (auto observer = getObserverShared()) {
      observer->resourcePoolsInitialized(descriptions);
    }

    size_t count{0};
    for (auto description : descriptions) {
      LOG(INFO) << fmt::format("Resource pool [{}]: {}", count++, description);
    }
  }
  if (FLAGS_thrift_server_enforces_qps_limit) {
    LOG(INFO) << "QPS limit will be enforced by Thrift Server";
  } else {
    LOG(INFO) << "QPS limit will be enforced by Resource Pool";
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
      auto requestPile =
          std::make_unique<apache::thrift::RoundRobinRequestPile>(
              std::move(options));
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
  runtimeServerActions_.resourcePoolEnabledGflag =
      FLAGS_thrift_experimental_use_resource_pools;
  runtimeServerActions_.resourcePoolDisabledGflag =
      FLAGS_thrift_disable_resource_pools;
  if (runtimeDisableResourcePoolsSet()) {
    // No need to check if we've already set this.
    LOG(INFO)
        << "runtimeResourcePoolsChecks() returns false because of runtimeDisableResourcePoolsSet()";
    return false;
  }
  // This can be called multiple times - only run it to completion once
  // but note below that it can exit early.
  if (runtimeServerActions_.checkComplete) {
    auto result = !runtimeDisableResourcePoolsSet();
    LOG(INFO)
        << "runtimeResourcePoolsChecks() is aleady completed and result is "
        << result;
    return result;
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
      LOG(INFO)
          << "It's too early to call runtimeResourcePoolsChecks(), returning True for now";
      return true;
    }
    runtimeDisableResourcePoolsDeprecated();
  } else {
    // Need to set this up now to check.
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
          LOG(INFO) << "Resource pools disabled. Incomplete metadata";
          runtimeServerActions_.noServiceRequestInfo = true;
          runtimeDisableResourcePoolsDeprecated();
        }
        if (!THRIFT_FLAG(enable_resource_pools_for_interaction) &&
            metadata.interactionType ==
                AsyncProcessorFactory::MethodMetadata::InteractionType::
                    INTERACTION_V1) {
          // We've found an interaction in this service. Mark it is incompatible
          // with resource pools
          LOG(INFO) << "Resource pools disabled. Interaction on request "
                    << methodToMetadataPtr.first;
          runtimeServerActions_.interactionInService = true;
          runtimeDisableResourcePoolsDeprecated();
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
        LOG(INFO) << "Resource pools disabled. Wildcard methods";
        runtimeServerActions_.wildcardMethods = true;
        runtimeDisableResourcePoolsDeprecated();
      }
    }
  }

  if (isActiveRequestsTrackingDisabled()) {
    // Record this but don't disable. Managed in configuration instead.
    runtimeServerActions_.activeRequestTrackingDisabled = true;
  }

  runtimeServerActions_.checkComplete = true;

  if (runtimeDisableResourcePoolsSet()) {
    LOG(INFO)
        << "runtimeResourcePoolsChecks() returns false because of runtimeDisableResourcePoolsSet()";
    return false;
  }
  LOG(INFO) << "Resource pools check complete - allowed";
  return true;
}

void ThriftServer::ensureResourcePools() {
  auto resourcePoolSupplied = !resourcePoolSet().empty();
  if (resourcePoolSupplied) {
    LOG(INFO) << "Resource pools supplied: " << resourcePoolSet().size();
  }

  if (!resourcePoolSet().hasResourcePool(ResourcePoolHandle::defaultSync())) {
    LOG(INFO) << "Creating a default sync pool";
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
      LOG(INFO)
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

      auto requestPile = std::make_unique<RoundRobinRequestPile>(
          RoundRobinRequestPile::Options());

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
    auto requestPile = std::make_unique<apache::thrift::RoundRobinRequestPile>(
        std::move(options));
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
            i == concurrency::PRIORITY::NORMAL
                ? std::thread::hardware_concurrency()
                : 2,
            ResourcePool::kPreferredExecutorNumPriorities);
      }
      apache::thrift::RoundRobinRequestPile::Options options;
      auto requestPile =
          std::make_unique<apache::thrift::RoundRobinRequestPile>(
              std::move(options));
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
        pools.push_back(Pool{
            threadPriority_.value_or(
                concurrency::PosixThreadFactory::NORMAL_PRI),
            getNumCPUWorkerThreads(),
            ResourcePoolHandle::defaultAsync(),
            concurrency::NORMAL});
        break;
      }
      case ThreadManagerType::PRIORITY_QUEUE: {
        pools.push_back(Pool{
            threadPriority_.value_or(
                concurrency::PosixThreadFactory::NORMAL_PRI),
            getNumCPUWorkerThreads(),
            ResourcePoolHandle::defaultAsync(),
            concurrency::NORMAL});
        break;
      }
      default: {
        LOG(FATAL) << "Unexpected ThreadMangerType:" << int(threadManagerType_);
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
      auto requestPile =
          std::make_unique<apache::thrift::RoundRobinRequestPile>(
              std::move(options));
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

/**
 * Loop and accept incoming connections.
 */
void ThriftServer::serve() {
  setup();
  SCOPE_EXIT { this->cleanUp(); };

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
    SCOPE_EXIT { done.wait(); };
    std::shared_ptr<folly::Baton<>> doneGuard(
        &done, [](folly::Baton<>* done) { done->post(); });

    for (auto& socket : sockets) {
      // Stop accepting new connections
      auto eb = socket->getEventBase();
      eb->runInEventBaseThread([socket = std::move(socket), doneGuard] {
        socket->pauseAccepting();
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
    SCOPE_EXIT { done.wait(); };
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
            LOG(ERROR) << "Failed to dump server snapshot on long shutdown: "
                       << folly::exceptionStr(std::current_exception());
          }
        }

        constexpr auto msgTemplate =
            "Could not drain active requests within allotted deadline. "
            "Deadline value: {} secs. {} because undefined behavior is possible. "
            "Underlying reasons could be either requests that have never "
            "terminated, long running requests, or long queues that could "
            "not be fully processed.";
        if (quickExitOnShutdownTimeout_) {
          LOG(ERROR) << fmt::format(
              msgTemplate,
              getWorkersJoinTimeout().count(),
              "quick_exiting (no coredump)");
          // similar to abort but without generating a coredump
          try_quick_exit(124);
        }
        if (FLAGS_thrift_abort_if_exceeds_shutdown_deadline) {
          LOG(FATAL) << fmt::format(
              msgTemplate, getWorkersJoinTimeout().count(), "Aborting");
        }
      }
    }
  });

  // Clear the decorated processor factory so that it's re-created if the server
  // is restarted.
  decoratedProcessorFactory_.reset();

  internalStatus_.store(ServerStatus::NOT_RUNNING, std::memory_order_release);
}

void ThriftServer::ensureDecoratedProcessorFactoryInitialized() {
  DCHECK(getProcessorFactory().get());
  if (decoratedProcessorFactory_ == nullptr) {
    decoratedProcessorFactory_ = createDecoratedProcessorFactory(
        getProcessorFactory(),
        getStatusInterface(),
        getMonitoringInterface(),
        getControlInterface(),
        isCheckUnimplementedExtraInterfacesAllowed() &&
            THRIFT_FLAG(server_check_unimplemented_extra_interfaces));
  }
}

void ThriftServer::callOnStartServing() {
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
      LOG(FATAL) << "Exception thrown by onStartServing(): "
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
      LOG(FATAL) << "Exception thrown by onStopRequested(): "
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

void ThriftServer::setGlobalServer(ThriftServer* server) {
  globalServer = server;
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
  // Wait for tasks running on AsyncScope to join
  folly::coro::blockingWait(asyncScope_->joinAsync());
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
          [acceptor] { acceptor->resetSSLContextConfigs(); });
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
    setTicketSeeds(std::move(*seeds));
  }
  tlsCredWatcher_.withWLock([this, &ticketPath](auto& credWatcher) {
    if (!credWatcher) {
      credWatcher.emplace(this);
    }
    credWatcher->setTicketPathToWatch(ticketPath);
  });
}

PreprocessResult ThriftServer::preprocess(
    const PreprocessParams& params) const {
  const auto& method = params.method;
  if (preprocess_ && !getMethodsBypassMaxRequestsLimit().contains(method)) {
    return preprocess_(params);
  }
  return {};
}

folly::Optional<server::ServerConfigs::ErrorCodeAndMessage>
ThriftServer::checkOverload(
    const transport::THeader::StringToStringMap* readHeaders,
    const std::string* method) {
  if (UNLIKELY(
          isOverloaded_ &&
          (method == nullptr ||
           !getMethodsBypassMaxRequestsLimit().contains(*method)) &&
          isOverloaded_(readHeaders, method))) {
    return {std::make_pair(
        kAppOverloadedErrorCode,
        "load shedding due to custom isOverloaded() callback")};
  }

  // If active request tracking is disabled or we are using resource pools,
  // skip max requests enforcement here. Resource pools has its own separate
  // concurrency limiting mechanism.
  bool useQueueConcurrency = !resourcePoolSet().empty() &&
      THRIFT_FLAG(enforce_queue_concurrency_resource_pools);
  if (!isActiveRequestsTrackingDisabled() && !useQueueConcurrency) {
    if (auto maxRequests = getMaxRequests(); maxRequests > 0 &&
        (method == nullptr ||
         !getMethodsBypassMaxRequestsLimit().contains(*method)) &&
        static_cast<uint32_t>(getActiveRequests()) >= maxRequests) {
      getCPUConcurrencyController().requestShed();
      return {std::make_pair(
          kOverloadedErrorCode, "load shedding due to max request limit")};
    }
  }

  if (auto maxQps = getMaxQps(); maxQps > 0 &&
      FLAGS_thrift_server_enforces_qps_limit &&
      (method == nullptr ||
       !getMethodsBypassMaxRequestsLimit().contains(*method)) &&
      !qpsTokenBucket_.consume(1.0, maxQps, maxQps)) {
    getCPUConcurrencyController().requestShed();
    return {
        std::make_pair(kOverloadedErrorCode, "load shedding due to qps limit")};
  }

  return {};
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
          [](std::vector<WorkerIOMemory> workerIOMems) -> ServerIOMemory {
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
  return folly::observer::makeObserver(
      [rocketPreferredObserver =
           THRIFT_FLAG_OBSERVE(server_alpn_prefer_rocket)] {
        const auto rocketPreferred = *rocketPreferredObserver.getSnapshot();
        if (rocketPreferred) {
          return std::list<std::string>{
              "rs",
              "thrift",
              "h2",
              // "http" is not a legit specifier but need to include it for
              // legacy.  Thrift's HTTP2RoutingHandler uses this, and clients
              // may be sending it.
              "http",
              // Many clients still send http/1.1 which is handled by the
              // default handler.
              "http/1.1"};
        }
        return std::list<std::string>{
            "thrift",
            "h2",
            // "http" is not a legit specifier but need to include it for
            // legacy.  Thrift's HTTP2RoutingHandler uses this, and clients
            // may be sending it.
            "http",
            // Many clients still send http/1.1 which is handled by the
            // default handler.
            "http/1.1",
            "rs"};
      });
}

folly::observer::Observer<bool> ThriftServer::enableStopTLS() {
  return THRIFT_FLAG_OBSERVE(server_enable_stoptls);
}

folly::observer::Observer<bool> ThriftServer::enableTLSCertRevocation() {
  return THRIFT_FLAG_OBSERVE(enable_mrl_check_for_thrift_server);
}

folly::observer::Observer<bool> ThriftServer::enforceTLSCertRevocation() {
  return THRIFT_FLAG_OBSERVE(enforce_mrl_check_for_thrift_server);
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
        evb->runInEventBaseThread([acceptor, ssl] {
          for (auto& sslContext : acceptor->getConfig().sslContextConfigs) {
            sslContext = *ssl;
          }
          acceptor->resetSSLContextConfigs();
        });
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

} // namespace thrift
} // namespace apache
