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

#include <csignal>

#include <folly/io/async/AsyncSignalHandler.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/BatchingFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/MetadataAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerCompositeAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

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

rocket::server::connection::ConnectionFactory
FastThriftServer::createConnectionFactory() {
  return [this](folly::AsyncTransport::UniquePtr socket)
             -> rocket::server::connection::RocketServerConnection {
    auto* evb = socket->getEventBase();
    auto transportHandler =
        transport::TransportHandler::create(std::move(socket));

    rocket::server::connection::RocketServerConnection conn;

    // 1. Build the thrift adapter first so the rocket pipeline's SETUP
    //    handler can capture a callback into it. When any auxiliary
    //    interface (monitoring/status/metadata) is wired up, the tail is a
    //    composite fronting the user adapter + aux adapters; otherwise the
    //    tail is the user adapter directly.
    const bool needsComposite = auxInterfaces_.monitoringHandler ||
        auxInterfaces_.statusHandler || auxInterfaces_.debugHandler ||
        metadataResponse_;

    if (needsComposite) {
      CompositeConnection ctx;
      ctx.allocator = std::make_unique<SimpleBufferAllocator>();
      ctx.transportAdapter =
          std::make_unique<server::ThriftServerTransportAdapter>(
              *conn.appAdapter);

      // 2. Build rocket pipeline.
      auto* transportAdapter = ctx.transportAdapter.get();
      auto rocketPipeline = buildRocketPipeline(
          evb,
          transportHandler.get(),
          conn.appAdapter.get(),
          [transportAdapter](
              const rocket::server::handler::SetupParameters& p) noexcept {
            transportAdapter->setMetadataProtocol(p.metadataProtocol);
          });
      conn.appAdapter->setPipeline(rocketPipeline.get());
      transportHandler->setPipeline(rocketPipeline.get());

      if (config_.zeroCopyThreshold > 0) {
        if (!transportHandler->setZeroCopy(true)) {
          XLOG(WARN) << "MSG_ZEROCOPY not supported on this socket";
        }
        transportHandler->setZeroCopyEnableThreshold(config_.zeroCopyThreshold);
      }
      conn.transportHandler = std::move(transportHandler);
      conn.pipeline = std::move(rocketPipeline);

      // Children live in ctx.children (owned); composite borrows raw
      // pointers. Declaration order in CompositeConnection ensures children
      // outlive the composite.
      ctx.children.push_back(handler_->getAppAdapter(handler_));
      if (auxInterfaces_.monitoringHandler) {
        ctx.children.push_back(auxInterfaces_.monitoringHandler->getAppAdapter(
            auxInterfaces_.monitoringHandler));
      }
      if (auxInterfaces_.statusHandler) {
        ctx.children.push_back(auxInterfaces_.statusHandler->getAppAdapter(
            auxInterfaces_.statusHandler));
      }
      if (auxInterfaces_.debugHandler) {
        ctx.children.push_back(auxInterfaces_.debugHandler->getAppAdapter(
            auxInterfaces_.debugHandler));
      }
      if (metadataResponse_) {
        ctx.children.push_back(
            ThriftServerAppAdapter::Ptr{
                new MetadataAppAdapter(metadataResponse_)});
      }
      ctx.adapter = ThriftServerCompositeAppAdapter::Ptr{
          new ThriftServerCompositeAppAdapter()};
      for (auto& child : ctx.children) {
        ctx.adapter->addChild(child.get());
      }

      // 3. Build thrift pipeline templated on the composite so its onRead
      //    is the resolved tail; setPipeline on the typed pointer also
      //    fans out to every child.
      auto thriftPipeline = PipelineBuilder<
                                server::ThriftServerTransportAdapter,
                                ThriftServerCompositeAppAdapter,
                                SimpleBufferAllocator>()
                                .setEventBase(evb)
                                .setHead(ctx.transportAdapter.get())
                                .setTail(ctx.adapter.get())
                                .setAllocator(ctx.allocator.get())
                                .build();
      ctx.transportAdapter->setPipeline(thriftPipeline.get());
      ctx.adapter->setPipeline(thriftPipeline.get());
      thriftPipeline->activate();

      auto* adapterKey = ctx.adapter.get();
      ctx.pipeline = std::move(thriftPipeline);
      registerConnection(adapterKey, std::move(ctx));
    } else {
      SimpleConnection ctx;
      ctx.adapter = handler_->getAppAdapter(handler_);
      ctx.allocator = std::make_unique<SimpleBufferAllocator>();
      ctx.transportAdapter =
          std::make_unique<server::ThriftServerTransportAdapter>(
              *conn.appAdapter);

      // 2. Build rocket pipeline.
      auto* transportAdapter = ctx.transportAdapter.get();
      auto rocketPipeline = buildRocketPipeline(
          evb,
          transportHandler.get(),
          conn.appAdapter.get(),
          [transportAdapter](
              const rocket::server::handler::SetupParameters& p) noexcept {
            transportAdapter->setMetadataProtocol(p.metadataProtocol);
          });
      conn.appAdapter->setPipeline(rocketPipeline.get());
      transportHandler->setPipeline(rocketPipeline.get());

      if (config_.zeroCopyThreshold > 0) {
        if (!transportHandler->setZeroCopy(true)) {
          XLOG(WARN) << "MSG_ZEROCOPY not supported on this socket";
        }
        transportHandler->setZeroCopyEnableThreshold(config_.zeroCopyThreshold);
      }
      conn.transportHandler = std::move(transportHandler);
      conn.pipeline = std::move(rocketPipeline);

      // 3. Build thrift pipeline templated on the base adapter. Works
      //    because generated FastSvAppAdapter subclasses only populate
      //    dispatch_ via addMethodHandler in their ctor and don't override
      //    base methods.
      auto thriftPipeline = PipelineBuilder<
                                server::ThriftServerTransportAdapter,
                                ThriftServerAppAdapter,
                                SimpleBufferAllocator>()
                                .setEventBase(evb)
                                .setHead(ctx.transportAdapter.get())
                                .setTail(ctx.adapter.get())
                                .setAllocator(ctx.allocator.get())
                                .build();
      ctx.transportAdapter->setPipeline(thriftPipeline.get());
      ctx.adapter->setPipeline(thriftPipeline.get());
      thriftPipeline->activate();

      auto* adapterKey = ctx.adapter.get();
      ctx.pipeline = std::move(thriftPipeline);
      registerConnection(adapterKey, std::move(ctx));
    }

    return conn;
  };
}

PipelineImpl::Ptr FastThriftServer::buildRocketPipeline(
    folly::EventBase* evb,
    transport::TransportHandler* transportHandler,
    rocket::server::RocketServerAppAdapter* appAdapter,
    rocket::server::handler::RocketServerSetupFrameHandler::OnSetupCompleteFn
        onSetupComplete) {
  return PipelineBuilder<
             transport::TransportHandler,
             rocket::server::RocketServerAppAdapter,
             SimpleBufferAllocator>()
      .setEventBase(evb)
      .setHead(transportHandler)
      .setTail(appAdapter)
      .setAllocator(&rocketAllocator_)
      .addNextInbound<frame::read::handler::FrameLengthParserHandler>(
          frame_length_parser_handler_tag)
      .addNextOutbound<frame::write::handler::BatchingFrameHandler>(
          batching_frame_handler_tag)
      .addNextOutbound<frame::write::handler::FrameLengthEncoderHandler>(
          frame_length_encoder_handler_tag)
      .addNextDuplex<rocket::server::handler::RocketServerFrameCodecHandler>(
          rocket_server_frame_codec_handler_tag)
      .addNextDuplex<rocket::server::handler::RocketServerSetupFrameHandler>(
          server_setup_frame_handler_tag, std::move(onSetupComplete))
      .addNextDuplex<
          rocket::server::handler::RocketServerRequestResponseHandler>(
          server_request_response_frame_handler_tag)
      .addNextDuplex<rocket::server::handler::RocketServerStreamStateHandler>(
          server_stream_state_handler_tag)
      .build();
}

void FastThriftServer::registerConnection(
    ThriftServerAppAdapter* key, SimpleConnection connection) {
  void* rawPtr = key;
  // Close callback fires from ThriftServerAppAdapter::onException when the
  // pipeline propagates an exception (typically connection drop). Removes the
  // entry from the map, releasing the per-connection adapter + pipeline.
  connection.adapter->setCloseCallback([this, rawPtr]() {
    thriftConnections_.withWLock(
        [rawPtr](auto& conns) { conns.erase(rawPtr); });
  });
  thriftConnections_.withWLock(
      [&](auto& conns) { conns.emplace(rawPtr, std::move(connection)); });
}

void FastThriftServer::registerConnection(
    ThriftServerCompositeAppAdapter* key, CompositeConnection connection) {
  void* rawPtr = key;
  // The composite itself doesn't expose setCloseCallback (it derives from
  // DelayedDestruction, not the adapter base). Hook the first child's
  // close callback — on connection drop, the pipeline's onException
  // propagates to the composite, which fans it out to every child via
  // vtable, so the first child's callback reliably fires.
  DCHECK(!connection.children.empty());
  connection.children.front()->setCloseCallback([this, rawPtr]() {
    thriftConnections_.withWLock(
        [rawPtr](auto& conns) { conns.erase(rawPtr); });
  });
  thriftConnections_.withWLock(
      [&](auto& conns) { conns.emplace(rawPtr, std::move(connection)); });
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
  // Without the merge, thriftdbg sendRequest <monitor-method> fails with
  // "Function not found" because the metadata lookup misses Monitor /
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

  security::BuiltFizzServerContext fizzBuilt;
  std::chrono::milliseconds tlsHandshakeTimeout{std::chrono::seconds{5}};
  if (sslConfig_) {
    fizzBuilt = security::buildFizzServerContext(*sslConfig_, thriftConfig_);
    tlsHandshakeTimeout = sslConfig_->handshakeTimeout;
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
  connectionManager_ = ServerConnectionManager::create(
      config_.address,
      folly::getKeepAliveToken(ioThreadPool_.get()),
      createConnectionFactory(),
      std::move(fizzBuilt.fizzContext),
      std::move(fizzBuilt.thriftParams),
      tlsHandshakeTimeout);
  connectionManager_->setEnableReusePortBpfSpread(enableReusePortBpfSpread_);

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
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  if (state_ != State::kRunning) {
    return;
  }
  state_ = State::kStopped;

  connectionManager_->stop();

  // Drain the map under the lock, then destroy entries outside it.
  // ~SimpleConnection runs ~ThriftServerAppAdapter, which fires the close
  // callback synchronously; that callback re-acquires this same write lock,
  // so clearing under the lock would deadlock folly::SharedMutex.
  std::unordered_map<void*, ConnectionVariant> drained;
  thriftConnections_.withWLock([&](auto& conns) { drained.swap(conns); });
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
