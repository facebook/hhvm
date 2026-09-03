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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServer.h>

#include <chrono>
#include <csignal>
#include <stdexcept>
#include <utility>
#include <vector>

#include <fmt/core.h>

#include <folly/Executor.h>
#include <folly/Function.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/io/async/AsyncSignalHandler.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/fast_thrift/connection/common/ConnectionStats.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/common/TLSStats.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerConnectionFactory.h>

namespace apache::thrift::fast_thrift::thrift {

using channel_pipeline::PipelineBuilder;
using channel_pipeline::PipelineImpl;
using channel_pipeline::SimpleBufferAllocator;

FastThriftServer::FastThriftServer(FastThriftServerConfig config)
    : config_(std::move(config)) {}

void FastThriftServer::setInterface(
    std::shared_ptr<ThriftServerAppAdapterFactory> handler) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setInterface must be called before start()/serve()";
  CHECK(handler)
      << "FastThriftServer::setInterface requires a non-null handler";
  CHECK(!handler_)
      << "FastThriftServer::setInterface called more than once; only a single "
         "handler is supported today";
  handler_ = std::move(handler);
}

void FastThriftServer::setMonitoringInterface(
    std::shared_ptr<fast_thrift::MonitoringServerInterface> handler) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setMonitoringInterface must be called before "
         "start()/serve()";
  CHECK(handler)
      << "FastThriftServer::setMonitoringInterface requires a non-null handler";
  CHECK(!auxInterfaces_.monitoringHandler)
      << "FastThriftServer::setMonitoringInterface called more than once";
  auxInterfaces_.monitoringHandler = std::move(handler);
}

void FastThriftServer::setStatusInterface(
    std::shared_ptr<fast_thrift::StatusServerInterface> handler) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setStatusInterface must be called before "
         "start()/serve()";
  CHECK(handler)
      << "FastThriftServer::setStatusInterface requires a non-null handler";
  CHECK(!auxInterfaces_.statusHandler)
      << "FastThriftServer::setStatusInterface called more than once";
  auxInterfaces_.statusHandler = std::move(handler);
}

void FastThriftServer::setDebugInterface(
    std::shared_ptr<fast_thrift::DebugServerInterface> handler) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setDebugInterface must be called before "
         "start()/serve()";
  CHECK(handler)
      << "FastThriftServer::setDebugInterface requires a non-null handler";
  CHECK(!auxInterfaces_.debugHandler)
      << "FastThriftServer::setDebugInterface called more than once";
  auxInterfaces_.debugHandler = std::move(handler);
}

void FastThriftServer::setControlInterface(
    std::shared_ptr<fast_thrift::ControlServerInterface> handler) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setControlInterface must be called before "
         "start()/serve()";
  CHECK(handler)
      << "FastThriftServer::setControlInterface requires a non-null handler";
  CHECK(!auxInterfaces_.controlHandler)
      << "FastThriftServer::setControlInterface called more than once";
  auxInterfaces_.controlHandler = std::move(handler);
}

void FastThriftServer::setSecurityInterface(
    std::shared_ptr<fast_thrift::SecurityServerInterface> handler) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setSecurityInterface must be called before "
         "start()/serve()";
  CHECK(handler)
      << "FastThriftServer::setSecurityInterface requires a non-null handler";
  CHECK(!auxInterfaces_.securityHandler)
      << "FastThriftServer::setSecurityInterface called more than once";
  auxInterfaces_.securityHandler = std::move(handler);
}

void FastThriftServer::setStats(std::shared_ptr<ServerStats> stats) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setStats must be called before start()/serve()";
  CHECK(stats) << "FastThriftServer::setStats requires non-null stats";
  CHECK(!stats_) << "FastThriftServer::setStats called more than once";
  stats_ = std::move(stats);
}

void FastThriftServer::setConnectionStats(
    std::shared_ptr<connection::ConnectionStats> stats) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setConnectionStats must be called before "
         "start()/serve()";
  CHECK(stats)
      << "FastThriftServer::setConnectionStats requires non-null stats";
  CHECK(!connectionStats_)
      << "FastThriftServer::setConnectionStats called more than once";
  connectionStats_ = std::move(stats);
}

void FastThriftServer::setTLSStats(
    std::shared_ptr<connection::security::TLSStats> stats) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setTLSStats must be called before start()/serve()";
  CHECK(stats) << "FastThriftServer::setTLSStats requires non-null stats";
  CHECK(!tlsStats_) << "FastThriftServer::setTLSStats called more than once";
  tlsStats_ = std::move(stats);
}

void FastThriftServer::addNativeThriftPipelineHandlers(
    std::vector<server::ThriftPipelineHandlerFactory> factories) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::addNativeThriftPipelineHandlers must be called "
         "before start()/serve()";
  thriftPipelineHandlerFactories_.reserve(
      thriftPipelineHandlerFactories_.size() + factories.size());
  for (auto& factory : factories) {
    thriftPipelineHandlerFactories_.push_back(std::move(factory));
  }
}

void FastThriftServer::addModule(FastServerModule module) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::addModule must be called before start()/serve()";
  if (module.name().empty()) {
    // The empty namespace is reserved for top-level
    // addNativeThriftPipelineHandlers ids; rejecting empty module names keeps
    // module and top-level id streams disjoint.
    throw std::logic_error(
        "FastThriftServer::addModule: module name must be non-empty");
  }
  if (moduleNames_.contains(module.name())) {
    throw std::logic_error(
        fmt::format(
            "FastThriftServer::addModule: duplicate module name: {}",
            module.name()));
  }
  // Refuse rather than degrade: without a per-connection context the
  // connection events carry nothing, and an extension that gates on what it
  // reads there would silently see an empty connection.
  if (module.requiresConnectionContext() && !config_.enableRequestContext) {
    throw std::logic_error(
        fmt::format(
            "FastThriftServer::addModule: module '{}' registers a connection "
            "extension, which requires enableRequestContext",
            module.name()));
  }
  // Same posture for headers: they are reachable only through the per-request
  // context, and only this setting puts them there. An extension that gates on
  // a header it never receives would admit everything.
  if (module.requiresHeaders() && !config_.enableRequestHeaders) {
    throw std::logic_error(
        fmt::format(
            "FastThriftServer::addModule: module '{}' registers an extension "
            "that uses headers, which requires enableRequestHeaders",
            module.name()));
  }
  // Two independent things pausing and resuming the same connection's reads,
  // with nothing arbitrating between them: WriteBufferBackpressureHandler
  // resumes as soon as its own buffer drains, which would lift a pause the
  // extension still wants held. Fail loudly rather than intermittently.
  if (module.controlsReads() && config_.enableWriteBufferBackpressure) {
    throw std::logic_error(
        fmt::format(
            "FastThriftServer::addModule: module '{}' registers an extension "
            "that controls reads, which cannot be combined with "
            "enableWriteBufferBackpressure",
            module.name()));
  }
  auto name = module.name();
  // Splice the module's handlers into the ordered list at the current call
  // position, preserving intra-module order.
  auto factories = std::move(module).handlers();
  thriftPipelineHandlerFactories_.reserve(
      thriftPipelineHandlerFactories_.size() + factories.size());
  for (auto& factory : factories) {
    thriftPipelineHandlerFactories_.push_back(std::move(factory));
  }
  // Claim the name only once the splice succeeded, so a throwing splice does
  // not leave the name permanently reserved against a retry.
  moduleNames_.insert(std::move(name));
}
void FastThriftServer::setOnConnectionAccepted(OnConnectionAcceptedFn cb) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setOnConnectionAccepted must be called before "
         "start()/serve()";
  onConnectionAccepted_ = std::move(cb);
}

void FastThriftServer::setSSLConfig(security::FizzServerCertConfig cfg) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setSSLConfig must be called before start()/serve()";
  sslConfig_ = std::move(cfg);
}

void FastThriftServer::setThriftConfig(security::ThriftTlsConfig cfg) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setThriftConfig must be called before start()/serve()";
  thriftConfig_ = cfg;
}

void FastThriftServer::setIOThreadPool(
    std::shared_ptr<folly::IOThreadPoolExecutorBase> pool) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setIOThreadPool must be called before "
         "start()/serve()";
  CHECK(pool) << "FastThriftServer::setIOThreadPool requires a non-null pool";
  ioThreadPool_ = std::move(pool);
}

void FastThriftServer::setCPUExecutor(folly::Executor::KeepAlive<> executor) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setCPUExecutor must be called before "
         "start()/serve()";
  CHECK(executor)
      << "FastThriftServer::setCPUExecutor requires a non-null executor";
  cpuExecutor_ = std::move(executor);
}

void FastThriftServer::setEnableReusePortBpfSpread(bool enable) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setEnableReusePortBpfSpread must be called before "
         "start()/serve()";
  enableReusePortBpfSpread_ = enable;
}

void FastThriftServer::setSocketOptions(connection::SocketOptions opts) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftServer::setSocketOptions must be called before "
         "start()/serve()";
  socketOptions_ = opts;
}

void FastThriftServer::reloadTLSConfig(security::FizzServerCertConfig cfg) {
  // Snapshot thriftConfig_ under the lock: setThriftConfig writes it under
  // the same mutex, and reloadTLSConfig is documented as safe from any
  // thread, so an unsynchronized read would be a TSAN data race even though
  // the lifecycle states normally exclude an overlap.
  security::ThriftTlsConfig thriftConfigSnapshot;
  {
    std::lock_guard<std::mutex> lock(lifecycleMutex_);
    CHECK(state_ == State::kRunning)
        << "FastThriftServer::reloadTLSConfig requires a running server "
           "(call setSSLConfig before start() for the initial config)";
    thriftConfigSnapshot = thriftConfig_;
  }

  // Build outside the lock — buildTLSParams may do file IO and may throw
  // on unreadable cert/CA files or invalid verifier config; holding the
  // lifecycle mutex across that would block start()/stop()/setters, and a
  // throw leaves the running server untouched.
  auto newParams = std::make_shared<const security::TLSParams>(
      security::buildTLSParams(cfg, thriftConfigSnapshot));

  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  // Re-check: stop() may have raced in between the two acquisitions. Drop
  // the reload silently rather than touching a torn-down connectionManager_.
  if (state_ != State::kRunning) {
    return;
  }
  sslConfig_ = std::move(cfg);
  connectionManager_->setTLSParams(std::move(newParams));
}

FastThriftServer::~FastThriftServer() {
  stop();

  // Destroy the connection manager first: its IOObserver removal fans
  // unregisterEventBase across every IO thread synchronously, draining
  // per-EVB ConnectionHandlers (and the ConnectionFactory closure that
  // captures `this`) before we return.
  connectionManager_.reset();
}

void FastThriftServer::start() {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(handler_)
      << "FastThriftServer::start called before setInterface — no handler "
         "registered";
  if (state_ != State::kNotStarted) {
    return;
  }

  // Build the merged metadata response once if enabled, before accepting
  // any connections. Java does the same (eager build at startup); the
  // cached response is then served from every per-connection
  // MetadataAppAdapter and from DefaultDebug.dumpThriftServiceMetadata.
  //
  // Merge order matters. genServiceMetadataResponse() is additive for the
  // metadata maps + `services` list but *overwrites* the deprecated
  // `context` field. Call extra interfaces first, user handler last, so
  // the response's `context` reflects the user service (matching legacy
  // multiplex behaviour — extras are siblings, not the primary).
  //
  // Without the merge, a debug-client `sendRequest <monitor-method>` fails
  // with "Function not found" because the metadata lookup misses Monitor /
  // Status / Debug methods even though they are dispatchable on the wire.
  if (config_.enableMetadataService) {
    auto resp = std::make_shared<
        apache::thrift::metadata::ThriftServiceMetadataResponse>();
    if (auxInterfaces_.monitoringHandler) {
      auxInterfaces_.monitoringHandler->getServiceMetadata(*resp);
    }
    if (auxInterfaces_.statusHandler) {
      auxInterfaces_.statusHandler->getServiceMetadata(*resp);
    }
    if (auxInterfaces_.debugHandler) {
      auxInterfaces_.debugHandler->getServiceMetadata(*resp);
    }
    if (auxInterfaces_.controlHandler) {
      auxInterfaces_.controlHandler->getServiceMetadata(*resp);
    }
    if (auxInterfaces_.securityHandler) {
      auxInterfaces_.securityHandler->getServiceMetadata(*resp);
    }
    handler_->getServiceMetadata(*resp);
    metadataResponse_ = std::move(resp);
  }

  std::shared_ptr<const security::TLSParams> tlsParams;
  security::SSLPolicy sslPolicy = security::SSLPolicy::DISABLED;
  if (sslConfig_) {
    sslPolicy = sslConfig_->sslPolicy;
    if (sslPolicy != security::SSLPolicy::DISABLED) {
      tlsParams = std::make_shared<const security::TLSParams>(
          security::buildTLSParams(*sslConfig_, thriftConfig_));
    }
  }

  // Materialize the default IO pool only when the embedder didn't supply
  // one via setIOThreadPool.
  //
  // Fixed-size pool: minThreads == maxThreads so threads cannot idle out.
  // The single-arg IOThreadPoolExecutor(N) constructor sets minThreads=0
  // when FLAGS_dynamic_iothreadpoolexecutor is true (the default in many
  // production configs), causing threads to time out and join when their
  // EventBase has no work. Each fast_thrift IO thread only ever does
  // accept() on its own SO_REUSEPORT listening socket, so an EVB that
  // the kernel happens not to route accepts to looks idle and dies —
  // collapsing the configured numIOThreads to a small fraction (~8 of
  // 188 observed in production) and permanently bottlenecking the
  // server. The two-arg constructor with min == max disables this
  // dynamic-shrink behavior and pins the pool size for the process
  // lifetime.
  if (!ioThreadPool_) {
    ioThreadPool_ = std::make_shared<folly::IOThreadPoolExecutor>(
        /*maxThreads=*/config_.numIOThreads,
        /*minThreads=*/config_.numIOThreads);
  }

  // Materialize the CPU pool only when asked for one and the embedder didn't
  // supply it. Leaving it null keeps handlers inline on the IO threads.
  if (!cpuExecutor_ && config_.numCPUThreads > 0) {
    ownedCPUThreadPool_ =
        std::make_shared<folly::CPUThreadPoolExecutor>(config_.numCPUThreads);
    cpuExecutor_ = folly::getKeepAliveToken(ownedCPUThreadPool_.get());
  }
  // Materialize the counters config_.enableStats asks for, leaving alone any
  // layer the embedder already supplied via the setStats family. Done here
  // rather than in the constructor so those setters, which reject a second
  // instance, still have every pre-start call to themselves.
  if (config_.enableStats) {
    if (!stats_) {
      stats_ = std::make_shared<ServerStats>();
    }
    if (!connectionStats_) {
      connectionStats_ = std::make_shared<connection::ConnectionStats>();
    }
    if (!tlsStats_) {
      tlsStats_ = std::make_shared<connection::security::TLSStats>();
    }
  }

  connectionManager_ = connection::ConnectionManager::create(
      config_.address,
      folly::getKeepAliveToken(ioThreadPool_.get()),
      sslPolicy,
      std::move(tlsParams),
      socketOptions_);
  connectionManager_->setEnableReusePortBpfSpread(enableReusePortBpfSpread_);
  connectionManager_->setConnectionStats(connectionStats_.get());
  connectionManager_->setTLSStats(tlsStats_.get());

  // Wire the per-connection factory. The factory carries all per-EVB-handler
  // config (user handler, aux interfaces, metadata, zero-copy threshold,
  // request-context wiring). The embedder onConnectionAccepted hook (if any)
  // runs from the connection-layer ConnectionAcceptCallbackHandler, reaching
  // the per-connection ThriftConnContext via ThriftServerConnection.
  server::ThriftServerConnectionFactoryConfig factoryConfig{
      .handler = handler_,
      .cpuExecutor = cpuExecutor_,
      .monitoringHandler = auxInterfaces_.monitoringHandler,
      .statusHandler = auxInterfaces_.statusHandler,
      .debugHandler = auxInterfaces_.debugHandler,
      .controlHandler = auxInterfaces_.controlHandler,
      .securityHandler = auxInterfaces_.securityHandler,
      .metadataResponse = metadataResponse_,
      .zeroCopyThreshold = config_.zeroCopyThreshold,
      .enableRequestContext = config_.enableRequestContext,
      .enableRequestHeaders = config_.enableRequestHeaders,
      .enableChecksum = config_.enableChecksum,
      .enableWriteBufferBackpressure = config_.enableWriteBufferBackpressure,
      .enableBackpressure = config_.enableBackpressure,
      .batchingConfig = config_.batchingConfig,
      .drainTimeout = config_.drainTimeout,
      .reapTimeout = config_.reapTimeout,
      .thriftPipelineHandlerFactories = thriftPipelineHandlerFactories_,
      .stats = stats_,
  };
  std::function<void(server::ThriftServerConnection&)> onAccept;
  if (onConnectionAccepted_) {
    onAccept = [this](server::ThriftServerConnection& conn) {
      onConnectionAccepted_(conn.connContext.get());
    };
  }
  connectionManager_->setConnectionFactory(
      server::ThriftServerConnectionFactory{std::move(factoryConfig)},
      std::move(onAccept));

  connectionManager_->start();
  state_ = State::kRunning;
  XLOG(INFO) << "FastThriftServer listening on "
             << connectionManager_->getAddress();
}

void FastThriftServer::serve() {
  start();

  folly::ScopedEventBaseThread signalThread("FastThriftSignal");
  auto* evb = signalThread.getEventBase();

  folly::CallbackAsyncSignalHandler signalHandler(evb, [this](int signum) {
    XLOG(INFO) << "Received signal " << signum << ", shutting down...";
    stop();
  });

  evb->runInEventBaseThreadAndWait([&] {
    signalHandler.registerSignalHandler(SIGINT);
    signalHandler.registerSignalHandler(SIGTERM);
  });

  stopBaton_.wait();

  evb->runInEventBaseThreadAndWait([&] {
    signalHandler.unregisterSignalHandler(SIGINT);
    signalHandler.unregisterSignalHandler(SIGTERM);
  });
}

void FastThriftServer::stop() {
  {
    std::lock_guard<std::mutex> lock(lifecycleMutex_);
    if (state_ != State::kRunning) {
      return;
    }
    state_ = State::kStopped;
  }

  // ConnectionManager::stop() handles the whole flow: stop accepting,
  // drain in-flight connections, force-close any stragglers.
  connectionManager_->stop();
  stopBaton_.post();
}

folly::SocketAddress FastThriftServer::getAddress() const {
  CHECK(connectionManager_)
      << "FastThriftServer::getAddress called before start() — use "
         "isRunning() to gate this call when iterating servers via "
         "FastThriftServerRegistry";
  return connectionManager_->getAddress();
}

} // namespace apache::thrift::fast_thrift::thrift
