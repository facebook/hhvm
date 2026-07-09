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
#include <vector>

#include <folly/Executor.h>
#include <folly/Function.h>
#include <folly/io/async/AsyncSignalHandler.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/logging/xlog.h>

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
  connectionManager_ = connection::ConnectionManager::create(
      config_.address,
      folly::getKeepAliveToken(ioThreadPool_.get()),
      sslPolicy,
      std::move(tlsParams),
      socketOptions_);
  connectionManager_->setEnableReusePortBpfSpread(enableReusePortBpfSpread_);

  // Wire the per-connection factory. The factory carries all per-EVB-handler
  // config (user handler, aux interfaces, metadata, zero-copy threshold,
  // request-context wiring). The embedder onConnectionAccepted hook (if any)
  // runs from the connection-layer ConnectionAcceptCallbackHandler, reaching
  // the per-connection ThriftConnContext via ThriftServerConnection.
  server::ThriftServerConnectionFactoryConfig factoryConfig{
      .handler = handler_,
      .monitoringHandler = auxInterfaces_.monitoringHandler,
      .statusHandler = auxInterfaces_.statusHandler,
      .debugHandler = auxInterfaces_.debugHandler,
      .metadataResponse = metadataResponse_,
      .zeroCopyThreshold = config_.zeroCopyThreshold,
      .enableRequestContext = config_.enableRequestContext,
      .enableRequestHeaders = config_.enableRequestHeaders,
      .enableWriteBufferBackpressure = config_.enableWriteBufferBackpressure,
      .batchingConfig = config_.batchingConfig,
      .drainTimeout = config_.drainTimeout,
      .reapTimeout = config_.reapTimeout,
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
