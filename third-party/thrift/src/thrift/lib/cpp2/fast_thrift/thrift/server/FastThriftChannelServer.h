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

#include <cstdint>
#include <mutex>
#include <optional>
#include <type_traits>
#include <unordered_map>

#include <folly/SocketAddress.h>
#include <folly/Synchronized.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/common/Stats.h>
#include <thrift/lib/cpp2/fast_thrift/connection/ConnectionManager.h>
#include <thrift/lib/cpp2/fast_thrift/connection/SocketOptions.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerCertConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/ThriftTlsConfig.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Configuration for FastThriftChannelServer.
 */
struct FastThriftServerConfig {
  // Address to bind to.
  folly::SocketAddress address;

  // Number of IO threads. Each thread runs its own EventBase and accepts
  // connections via SO_REUSEPORT.
  uint32_t numIOThreads{1};

  // Minimum payload size in bytes for MSG_ZEROCOPY. 0 disables zero-copy.
  size_t zeroCopyThreshold{0};

  // When true, FastThriftServer auto-mounts the ThriftMetadataService
  // alongside the user handler so introspection tools (e.g. Thrift Fiddle)
  // can discover the service schema. Calls handler->getServiceMetadata(...)
  // once at start() and serves the cached response on every
  // getThriftServiceMetadata() RPC.
  //
  // For the response to be non-empty, the underlying thrift_library must be
  // built with `with_schema = True` so the per-service schema bundle is
  // available at runtime. With the flag off (or the schema not bundled),
  // Fiddle will report no functions for the service. Default off.
  bool enableMetadataService{false};

  // When true, FastThriftServer constructs a per-connection ThriftConnContext
  // on accept and wires the ThriftServerRequestContextHandler +
  // ThriftServerConnectionContextHandler into the thrift pipeline, so each
  // request's ThriftRequestContext is populated with the ThriftConnContext.
  // The setOnConnectionAccepted callback receives a pointer to the
  // ThriftConnContext (or nullptr when this flag is off).
  //
  // Off by default — opt in only when embedder code needs per-connection
  // context propagation (e.g., setUserData stashing). Only takes effect on
  // FastThriftServer; ignored by FastThriftChannelServer.
  bool enableRequestContext{false};
};

/**
 * FastThriftServerT - A standalone server that uses the fast_thrift pipeline
 * to serve Thrift RPCs.
 *
 * Template parameter Stats controls metrics collection:
 * - Stats = NoStats (default): No metrics collected, handlers not added
 * - Stats = Custom type satisfying FastThriftStatsConcept: Metrics collected
 *
 * Uses a two-pipeline architecture:
 *
 * Rocket pipeline (owned by RocketServerConnection):
 *   TransportHandler
 *     -> FrameLengthParserHandler
 *     -> BatchingFrameHandler
 *     -> FrameLengthEncoderHandler
 *     -> RocketServerFrameCodecHandler
 *     -> RocketServerSetupFrameHandler
 *     -> RocketServerStreamStateHandler
 *     -> RocketServerRequestResponseHandler
 *     -> RocketMetricsHandler        <-- added only if Stats != NoStats
 *     -> RocketServerAppAdapter
 *
 * Thrift pipeline (owned by ThriftConnectionContext):
 *   ThriftServerTransportAdapter
 *     -> ThriftMetricsHandler         <-- added only if Stats != NoStats
 *     -> ThriftServerChannel
 *
 * The ThriftServerTransportAdapter bridges between the two pipelines,
 * converting between rocket and thrift message types.
 *
 * Supports request-response and oneway RPCs. Streaming and sink RPCs are
 * not yet supported.
 *
 * Usage (no metrics):
 *   auto handler = std::make_shared<MyServiceHandler>();
 *   FastThriftServerConfig config;
 *   config.address.setFromLocalPort(5001);
 *   config.numIOThreads = 8;
 *
 *   FastThriftChannelServer server(config, handler);
 *   server.serve();  // Blocks until stop() is called from another thread.
 *
 * Usage (with metrics):
 *   struct MyStats { ... }; // Must satisfy FastThriftStatsConcept
 *   FastThriftServerT<MyStats> server(config, handler);
 */
template <typename Stats = NoStats>
class FastThriftServerT {
  static_assert(
      FastThriftStatsConcept<Stats> || std::is_same_v<Stats, NoStats>,
      "Stats must satisfy FastThriftStatsConcept or be NoStats");

 public:
  static constexpr bool kStatsEnabled = !std::is_same_v<Stats, NoStats>;

  FastThriftServerT(
      FastThriftServerConfig config,
      std::shared_ptr<apache::thrift::AsyncProcessorFactory> processorFactory);
  ~FastThriftServerT();

  FastThriftServerT(const FastThriftServerT&) = delete;
  FastThriftServerT& operator=(const FastThriftServerT&) = delete;
  FastThriftServerT(FastThriftServerT&&) = delete;
  FastThriftServerT& operator=(FastThriftServerT&&) = delete;

  // Configure TLS. After this is called, every accepted connection is
  // wrapped in a fizz::server::AsyncFizzServer; the connection factory only
  // sees fully-handshaked transports. Must be called before start()/serve().
  void setSSLConfig(security::FizzServerCertConfig cfg);

  // Configure thrift-extension knobs negotiated during the fizz handshake
  // (StopTLS, params negotiation, etc.). Must be called before start()/serve().
  void setThriftConfig(security::ThriftTlsConfig cfg);

  // Listening-socket tuning knobs (listen backlog, TCP Fast Open, max
  // accepts per event). Applied by ConnectionHandler at startAccepting
  // time on every IO thread's listening socket. When unset, defaults from
  // SocketOptions.h apply.
  //
  // Must be called before start()/serve().
  void setSocketOptions(connection::SocketOptions opts);

  /**
   * Start accepting connections without blocking.
   */
  void start();

  /**
   * Start accepting connections and block until stop() is called.
   */
  void serve();

  /**
   * Stop accepting new connections and shut down.
   * In-flight requests on existing connections will complete before
   * channels are destroyed.
   */
  void stop();

  /**
   * Get the bound server address.
   * Useful when binding to port 0 to discover the assigned port.
   */
  folly::SocketAddress getAddress() const;

 private:
  using ServerConnectionManager = connection::ConnectionManager;

  // Lifecycle states. Transitions are linear: kNotStarted → kRunning →
  // kStopped. start() and stop() are idempotent — calling either outside
  // its expected source state is a no-op. All transitions and reads are
  // serialized by lifecycleMutex_.
  enum class State : uint8_t {
    kNotStarted,
    kRunning,
    kStopped,
  };

  /**
   * Per-connection thrift-layer context.
   * Owns the thrift pipeline and transport adapter that bridges between
   * the rocket and thrift pipelines.
   */
  struct ThriftConnectionContext {
    std::shared_ptr<ThriftServerChannel> serverChannel;
    std::unique_ptr<server::ThriftServerTransportAdapter> transportAdapter;
    channel_pipeline::PipelineImpl::Ptr thriftPipeline;
    std::unique_ptr<channel_pipeline::SimpleBufferAllocator> thriftAllocator =
        std::make_unique<channel_pipeline::SimpleBufferAllocator>();
    std::shared_ptr<Stats> stats;
  };

  connection::ConnectionFactory createConnectionFactory();

  channel_pipeline::PipelineImpl::Ptr buildRocketPipeline(
      folly::EventBase* evb,
      transport::TransportHandler* transportHandler,
      rocket::server::RocketServerAppAdapter* appAdapter,
      rocket::server::handler::RocketServerSetupFrameHandler::OnSetupCompleteFn
          onSetupComplete,
      std::shared_ptr<Stats> stats);

  void registerConnection(
      ThriftServerChannel* key, ThriftConnectionContext context);

  const FastThriftServerConfig config_;
  std::shared_ptr<apache::thrift::AsyncProcessorFactory> processorFactory_;
  std::optional<security::FizzServerCertConfig> sslConfig_;
  security::ThriftTlsConfig thriftConfig_{};
  std::shared_ptr<folly::IOThreadPoolExecutor> executor_;
  // Listening-socket tuning. Defaults from SocketOptions.h apply unless the
  // embedder calls setSocketOptions before start().
  connection::SocketOptions socketOptions_{};
  ServerConnectionManager::Ptr connectionManager_;
  channel_pipeline::SimpleBufferAllocator rocketAllocator_;
  folly::Synchronized<
      std::unordered_map<ThriftServerChannel*, ThriftConnectionContext>>
      thriftConnections_;
  folly::Baton<> stopBaton_;
  // Guards state_ and serializes lifecycle transitions so that stop()
  // observes the connectionManager_ assignment from start() with a proper
  // happens-before. Without this, TSAN reports a race when stop() runs on a
  // different thread from serve().
  std::mutex lifecycleMutex_;
  State state_{State::kNotStarted};
};

// Default type alias for convenience - no metrics by default
using FastThriftChannelServer = FastThriftServerT<NoStats>;

} // namespace apache::thrift::fast_thrift::thrift

// ============================================================================
// Template Implementation
// ============================================================================

#include <csignal>

#include <folly/io/async/AsyncSignalHandler.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/BatchingFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/handler/RocketMetricsHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/handler/ThriftMetricsHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>

namespace apache::thrift::fast_thrift::thrift {

// Handler tags for rocket pipeline construction
HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(batching_frame_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_server_frame_codec_handler);
HANDLER_TAG(server_setup_frame_handler);
HANDLER_TAG(server_request_response_frame_handler);
HANDLER_TAG(server_stream_state_handler);

template <typename Stats>
FastThriftServerT<Stats>::FastThriftServerT(
    FastThriftServerConfig config,
    std::shared_ptr<apache::thrift::AsyncProcessorFactory> processorFactory)
    : config_(std::move(config)),
      processorFactory_(std::move(processorFactory)),
      // config_ is initialized first per member declaration order in the header
      executor_(
          std::make_shared<folly::IOThreadPoolExecutor>(config_.numIOThreads)) {
}

template <typename Stats>
void FastThriftServerT<Stats>::setSSLConfig(
    security::FizzServerCertConfig cfg) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftChannelServer::setSSLConfig must be called before start()";
  sslConfig_ = std::move(cfg);
}

template <typename Stats>
void FastThriftServerT<Stats>::setThriftConfig(security::ThriftTlsConfig cfg) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftChannelServer::setThriftConfig must be called before start()";
  thriftConfig_ = cfg;
}

template <typename Stats>
void FastThriftServerT<Stats>::setSocketOptions(
    connection::SocketOptions opts) {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(state_ == State::kNotStarted)
      << "FastThriftChannelServer::setSocketOptions must be called before start()";
  socketOptions_ = opts;
}

template <typename Stats>
connection::ConnectionFactory
FastThriftServerT<Stats>::createConnectionFactory() {
  return [this](folly::AsyncTransport::UniquePtr socket)
             -> connection::RocketServerConnection {
    auto* evb = socket->getEventBase();
    auto transportHandler =
        transport::TransportHandler::create(std::move(socket));

    // Create shared per-connection stats if enabled
    std::shared_ptr<Stats> stats;
    if constexpr (kStatsEnabled) {
      stats = std::make_shared<Stats>();
    }

    // Build the RocketServerConnection with default appAdapter
    connection::RocketServerConnection conn;

    // 1. Build the thrift channel first so the rocket pipeline's SETUP
    //    handler can capture a callback that publishes the negotiated
    //    metadata protocol into it.
    auto serverChannel =
        std::make_shared<ThriftServerChannel>(processorFactory_);

    // 2. Build rocket pipeline: TransportHandler → ... → RocketServerAppAdapter
    auto* channelPtr = serverChannel.get();
    auto rocketPipeline = buildRocketPipeline(
        evb,
        transportHandler.get(),
        conn.appAdapter.get(),
        [channelPtr](
            const rocket::server::handler::SetupParameters& p) noexcept {
          channelPtr->setMetadataProtocol(p.metadataProtocol);
        },
        stats);
    conn.appAdapter->setPipeline(rocketPipeline.get());
    transportHandler->setPipeline(rocketPipeline.get());

    if (config_.zeroCopyThreshold > 0) {
      if (!transportHandler->setZeroCopy(true)) {
        XLOG(WARN) << "MSG_ZEROCOPY not supported on this socket";
      }
      transportHandler->setZeroCopyEnableThreshold(config_.zeroCopyThreshold);
    }

    // 3. Capture heap-stable raw pointers for the bridge's thrift→rocket
    //    action callbacks. The owning unique_ptrs in RocketServerConnection
    //    are address-unstable (the struct is moved into
    //    ConnectionHandler::connections_), but the pointees they manage
    //    live on the heap. The bridge's onClose subscription nulls these
    //    callbacks out once the rocket side completes its handlerRemoved
    //    fan-out, so they never invoke through dangling pointers.
    auto* rawTransport = transportHandler.get();

    conn.transportHandler = std::move(transportHandler);
    conn.pipeline = std::move(rocketPipeline);

    auto transportAdapter =
        std::make_unique<server::ThriftServerTransportAdapter>(
            *conn.appAdapter,
            /*onRocketDisconnect=*/
            [rawTransport]() noexcept {
              // Triggers the close callback (set by ConnectionHandler),
              // which runs ConnectionHandler::removeConnection →
              // RocketServerConnection::close() → disconnect()+destroy().
              // Idempotent in TransportHandler::close.
              rawTransport->close({});
            },
            /*onRocketDestroy=*/
            [rawTransport]() noexcept {
              // Same teardown trigger as disconnect; the difference is
              // intent (full teardown vs. graceful disconnect). Idempotent.
              rawTransport->close({});
            });

    ThriftConnectionContext ctx;
    ctx.stats = stats;

    channel_pipeline::PipelineImpl::Ptr thriftPipeline;
    if constexpr (kStatsEnabled) {
      thriftPipeline = channel_pipeline::PipelineBuilder<
                           server::ThriftServerTransportAdapter,
                           ThriftServerChannel,
                           channel_pipeline::SimpleBufferAllocator>()
                           .setEventBase(evb)
                           .setHead(transportAdapter.get())
                           .setTail(serverChannel.get())
                           .setAllocator(ctx.thriftAllocator.get())
                           .template addNextDuplex<
                               ThriftMetricsHandler<Direction::Server, Stats>>(
                               thrift_metrics_handler_tag, ctx.stats)
                           .build();
    } else {
      thriftPipeline = channel_pipeline::PipelineBuilder<
                           server::ThriftServerTransportAdapter,
                           ThriftServerChannel,
                           channel_pipeline::SimpleBufferAllocator>()
                           .setEventBase(evb)
                           .setHead(transportAdapter.get())
                           .setTail(serverChannel.get())
                           .setAllocator(ctx.thriftAllocator.get())
                           .build();
    }

    transportAdapter->setPipeline(thriftPipeline.get());
    serverChannel->setPipelineRef(*thriftPipeline);
    serverChannel->setWorker(apache::thrift::Cpp2Worker::createDummy(evb));

    ctx.serverChannel = serverChannel;
    ctx.transportAdapter = std::move(transportAdapter);
    ctx.thriftPipeline = std::move(thriftPipeline);

    registerConnection(serverChannel.get(), std::move(ctx));

    return conn;
  };
}

template <typename Stats>
channel_pipeline::PipelineImpl::Ptr
FastThriftServerT<Stats>::buildRocketPipeline(
    folly::EventBase* evb,
    transport::TransportHandler* transportHandler,
    rocket::server::RocketServerAppAdapter* appAdapter,
    rocket::server::handler::RocketServerSetupFrameHandler::OnSetupCompleteFn
        onSetupComplete,
    std::shared_ptr<Stats> stats) {
  if constexpr (kStatsEnabled) {
    return channel_pipeline::PipelineBuilder<
               transport::TransportHandler,
               rocket::server::RocketServerAppAdapter,
               channel_pipeline::SimpleBufferAllocator>()
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
        .addNextDuplex<rocket::server::handler::RocketServerStreamStateHandler>(
            server_stream_state_handler_tag)
        .addNextDuplex<
            rocket::server::handler::RocketServerRequestResponseHandler>(
            server_request_response_frame_handler_tag)
        .template addNextDuplex<RocketMetricsHandler<Direction::Server, Stats>>(
            rocket_metrics_handler_tag, std::move(stats))
        .build();
  } else {
    return channel_pipeline::PipelineBuilder<
               transport::TransportHandler,
               rocket::server::RocketServerAppAdapter,
               channel_pipeline::SimpleBufferAllocator>()
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
        .addNextDuplex<rocket::server::handler::RocketServerStreamStateHandler>(
            server_stream_state_handler_tag)
        .addNextDuplex<
            rocket::server::handler::RocketServerRequestResponseHandler>(
            server_request_response_frame_handler_tag)
        .build();
  }
}

template <typename Stats>
void FastThriftServerT<Stats>::registerConnection(
    ThriftServerChannel* key, ThriftConnectionContext context) {
  auto* rawPtr = key;
  // Close callback is invoked from ThriftServerChannel::onException when the
  // connection closes (pipeline exception propagation). This removes the
  // context from tracking, releasing the shared_ptr and destroying the
  // thrift pipeline + transport adapter.
  context.serverChannel->setCloseCallback([this, rawPtr]() {
    thriftConnections_.withWLock(
        [rawPtr](auto& conns) { conns.erase(rawPtr); });
  });
  thriftConnections_.withWLock(
      [&](auto& conns) { conns.emplace(rawPtr, std::move(context)); });
}

template <typename Stats>
FastThriftServerT<Stats>::~FastThriftServerT() {
  stop();

  // Destroy the connection manager before joining the executor. This triggers
  // deferred ConnectionHandler destruction on the EventBase threads.
  // We must then join the executor so those deferred callbacks run while
  // `this` (and all its members like allocator_, thriftConnections_) is still
  // alive. Without this, member destruction order would destroy those members
  // before the executor joins, causing use-after-free in the pipeline factory
  // lambda that captures `this`.
  connectionManager_.reset();
  executor_->join();
}

template <typename Stats>
void FastThriftServerT<Stats>::start() {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  if (state_ != State::kNotStarted) {
    return;
  }

  std::shared_ptr<const fizz::server::FizzServerContext> fizzContext;
  std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams;
  std::chrono::milliseconds tlsHandshakeTimeout{std::chrono::seconds{5}};
  if (sslConfig_) {
    auto built = security::buildFizzServerContext(*sslConfig_, thriftConfig_);
    fizzContext = std::move(built.fizzContext);
    thriftParams = std::move(built.thriftParams);
    tlsHandshakeTimeout = sslConfig_->handshakeTimeout;
  }
  connectionManager_ = ServerConnectionManager::create(
      config_.address,
      folly::getKeepAliveToken(executor_.get()),
      createConnectionFactory(),
      std::move(fizzContext),
      std::move(thriftParams),
      tlsHandshakeTimeout,
      socketOptions_);

  connectionManager_->start();
  state_ = State::kRunning;
  XLOG(INFO) << "FastThriftChannelServer listening on "
             << connectionManager_->getAddress();
}

template <typename Stats>
void FastThriftServerT<Stats>::serve() {
  start();

  // Set up signal handling on a dedicated EventBase thread so that
  // SIGINT/SIGTERM trigger a clean shutdown via stop().
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

template <typename Stats>
void FastThriftServerT<Stats>::stop() {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  if (state_ != State::kRunning) {
    return;
  }
  state_ = State::kStopped;

  // Stop accepting new connections first
  connectionManager_->stopAccepting();
  connectionManager_->closeConnections();

  // Drain the map under the lock, then destroy entries outside it.
  // ~ThriftConnectionContext runs ~ThriftServerChannel, which fires the close
  // callback synchronously; that callback re-acquires this same write lock,
  // so clearing under the lock would deadlock folly::SharedMutex.
  std::unordered_map<ThriftServerChannel*, ThriftConnectionContext> drained;
  thriftConnections_.withWLock([&](auto& conns) { drained.swap(conns); });

  stopBaton_.post();
}

template <typename Stats>
folly::SocketAddress FastThriftServerT<Stats>::getAddress() const {
  return connectionManager_->getAddress();
}

} // namespace apache::thrift::fast_thrift::thrift
