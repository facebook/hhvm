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
 *     -> FrameCodecHandler
 *     -> FrameDefragmentationHandler / FrameFragmentationHandler
 *     -> RocketServerMessageMarshalHandler
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
/**
 * ThriftServerChannelConnection — channel server's per-accepted-client state.
 * Owns the thrift pipeline; the transport adapter at its head owns the
 * underlying rocket::server::RocketServerConnection (transport handler,
 * rocket app adapter, rocket pipeline). Satisfies connection::Connection
 * (close, drain, setCloseCallback) so it can be returned from the
 * connection factory directly.
 *
 * Move-only. Default-constructible so the factory can fill members
 * incrementally during pipeline wiring before returning.
 */
struct ThriftServerChannelConnection {
  // Tail of the thrift pipeline; outlives the pipeline because the
  // pipeline holds a raw pointer to it.
  std::shared_ptr<ThriftServerChannel> serverChannel;
  // Buffer allocator used by the thrift pipeline.
  channel_pipeline::SimpleBufferAllocator thriftAllocator;
  // Head of the thrift pipeline. Owns the
  // rocket::server::RocketServerConnection (transport handler + rocket app
  // adapter + rocket pipeline) via its rocketConnection() member.
  std::unique_ptr<server::ThriftServerTransportAdapter> transportAdapter;
  // Thrift pipeline. Destroyed first among the owned fields here.
  channel_pipeline::PipelineImpl::Ptr thriftPipeline;
  // Stable indirection for the ConnectionHandler-installed close callback.
  // The connection is moved twice (factory return → AnyConnection wrap),
  // so storing the callback as a plain function<> means any reference to
  // it would dangle. The shared_ptr is created in buildConnection and
  // captured by the serverChannel's own closeCallback, so when the rocket
  // pipeline tears down (EOF → bridge → ThriftServerChannel::onException)
  // the channel can fire through the holder regardless of how many times
  // the outer struct has been moved.
  std::shared_ptr<std::function<void()>> closeCbHolder{
      std::make_shared<std::function<void()>>()};
  bool closed{false};

  ThriftServerChannelConnection() = default;
  ThriftServerChannelConnection(ThriftServerChannelConnection&&) noexcept =
      default;
  ThriftServerChannelConnection& operator=(
      ThriftServerChannelConnection&&) noexcept = default;
  ThriftServerChannelConnection(const ThriftServerChannelConnection&) = delete;
  ThriftServerChannelConnection& operator=(
      const ThriftServerChannelConnection&) = delete;
  ~ThriftServerChannelConnection() = default;

  // Connection concept: forceful synchronous teardown. Closes the thrift
  // pipeline (which propagates handlerRemoved into the transport adapter,
  // which in turn tears down the rocket connection) and fires the close
  // callback installed by ConnectionHandler.
  void close() noexcept {
    if (closed) {
      return;
    }
    closed = true;
    if (thriftPipeline) {
      thriftPipeline->deactivate();
      thriftPipeline->close();
      thriftPipeline.reset();
    }
    transportAdapter.reset();
    if (closeCbHolder && *closeCbHolder) {
      auto cb = std::move(*closeCbHolder);
      cb();
    }
  }

  // Connection concept: graceful drain. Channel server has no peer-
  // disconnect frame analog here; route through close().
  void drain() noexcept { close(); }

  // Connection concept: callback fired once the connection has fully torn
  // down. ConnectionHandler uses it to remove the entry from its bookkeeping.
  void setCloseCallback(std::function<void()> cb) {
    *closeCbHolder = std::move(cb);
  }
};

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
   * ConnectionFactory implementation: satisfies connection::ConnectionFactory
   * (has getConnection(socket)). Owned by ConnectionManager via setConnection
   * Factory; delegates per-accept construction back to the server so it can
   * use private state (processorFactory_, config_, allocator).
   */
  class ConnectionFactoryImpl {
   public:
    explicit ConnectionFactoryImpl(FastThriftServerT* server) noexcept
        : server_(server) {}
    ThriftServerChannelConnection getConnection(
        folly::AsyncTransport::UniquePtr socket) {
      return server_->buildConnection(std::move(socket));
    }

   private:
    FastThriftServerT* server_;
  };

  ThriftServerChannelConnection buildConnection(
      folly::AsyncTransport::UniquePtr socket);

  channel_pipeline::PipelineImpl::Ptr buildRocketPipeline(
      folly::EventBase* evb,
      transport::TransportHandler* transportHandler,
      rocket::server::RocketServerAppAdapter* appAdapter,
      rocket::server::handler::RocketServerSetupFrameHandler::OnSetupCompleteFn
          onSetupComplete,
      std::shared_ptr<Stats> stats);

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
#include <thrift/lib/cpp2/fast_thrift/frame/handler/FrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameDefragmentationHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FragmentationHandlerConfig.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/BatchingFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameFragmentationHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/handler/RocketMetricsHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerMessageMarshalHandler.h>
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
HANDLER_TAG(frame_codec_handler);
HANDLER_TAG(frame_defragmentation_handler);
HANDLER_TAG(frame_fragmentation_handler);
HANDLER_TAG(rocket_server_message_marshal_handler);
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
ThriftServerChannelConnection FastThriftServerT<Stats>::buildConnection(
    folly::AsyncTransport::UniquePtr socket) {
  auto* evb = socket->getEventBase();

  // Build the rocket-layer pieces into a
  // rocket::server::RocketServerConnection. ThriftServerTransportAdapter takes
  // ownership of this whole bundle, so the rocket pipeline + transport handler
  // + app adapter tear down together when the thrift pipeline's handlerRemoved
  // propagates into the adapter.
  auto rocketConn = std::make_unique<rocket::server::RocketServerConnection>();
  rocketConn->transportHandler =
      transport::TransportHandler::create(std::move(socket));

  // Per-connection stats (when enabled).
  std::shared_ptr<Stats> stats;
  if constexpr (kStatsEnabled) {
    stats = std::make_shared<Stats>();
  }

  // Build the thrift channel first so the rocket pipeline's SETUP handler can
  // capture a callback that publishes the negotiated metadata protocol into it.
  auto serverChannel = std::make_shared<ThriftServerChannel>(processorFactory_);
  auto* channelPtr = serverChannel.get();

  // Build the rocket pipeline:
  //   TransportHandler → ... → RocketServerAppAdapter
  rocketConn->pipeline = buildRocketPipeline(
      evb,
      rocketConn->transportHandler.get(),
      rocketConn->appAdapter.get(),
      [channelPtr](const rocket::server::handler::SetupParameters& p) noexcept {
        channelPtr->setMetadataProtocol(p.metadataProtocol);
      },
      stats);
  rocketConn->appAdapter->setPipeline(rocketConn->pipeline.get());
  rocketConn->transportHandler->setPipeline(rocketConn->pipeline.get());

  if (config_.zeroCopyThreshold > 0) {
    if (!rocketConn->transportHandler->setZeroCopy(true)) {
      XLOG(WARN) << "MSG_ZEROCOPY not supported on this socket";
    }
    rocketConn->transportHandler->setZeroCopyEnableThreshold(
        config_.zeroCopyThreshold);
  }

  // ThriftServerTransportAdapter takes ownership of the rocket connection.
  // Teardown is driven by the thrift pipeline's handlerRemoved fan-out.
  auto transportAdapter =
      std::make_unique<server::ThriftServerTransportAdapter>(
          std::move(rocketConn));

  ThriftServerChannelConnection conn;
  conn.serverChannel = serverChannel;

  if constexpr (kStatsEnabled) {
    conn.thriftPipeline =
        channel_pipeline::PipelineBuilder<
            server::ThriftServerTransportAdapter,
            ThriftServerChannel,
            channel_pipeline::SimpleBufferAllocator>()
            .setEventBase(evb)
            .setHead(transportAdapter.get())
            .setTail(serverChannel.get())
            .setAllocator(&conn.thriftAllocator)
            .template addNextDuplex<
                ThriftMetricsHandler<Direction::Server, Stats>>(
                thrift_metrics_handler_tag, stats)
            .build();
  } else {
    conn.thriftPipeline = channel_pipeline::PipelineBuilder<
                              server::ThriftServerTransportAdapter,
                              ThriftServerChannel,
                              channel_pipeline::SimpleBufferAllocator>()
                              .setEventBase(evb)
                              .setHead(transportAdapter.get())
                              .setTail(serverChannel.get())
                              .setAllocator(&conn.thriftAllocator)
                              .build();
  }

  transportAdapter->setPipeline(conn.thriftPipeline.get());
  serverChannel->setPipelineRef(*conn.thriftPipeline);
  serverChannel->setWorker(apache::thrift::Cpp2Worker::createDummy(evb));

  conn.transportAdapter = std::move(transportAdapter);

  // Wire the channel-level close path to fan into the connection's
  // ConnectionHandler-installed close callback. ThriftServerChannel fires
  // its own closeCallback_ from onException when the pipeline takes an
  // EOF / error — that's the signal the rocket connection has gone away.
  // Routing it through the holder lets us self-close (and therefore drain
  // the manager's bookkeeping) without holding a back-pointer to a moved
  // ThriftServerChannelConnection.
  auto holder = conn.closeCbHolder;
  serverChannel->setCloseCallback([holder]() {
    if (holder && *holder) {
      auto cb = std::move(*holder);
      cb();
    }
  });

  // Activate the rocket pipeline so it can begin reading. Mirrors
  // ThriftServerConnectionFactory::getConnection's onConnect call.
  conn.transportAdapter->rocketConnection().transportHandler->onConnect();
  return conn;
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
        .addNextDuplex<frame::handler::FrameCodecHandler>(
            frame_codec_handler_tag)
        .addNextInbound<frame::read::handler::FrameDefragmentationHandler>(
            frame_defragmentation_handler_tag)
        .addNextOutbound<frame::write::handler::FrameFragmentationHandler>(
            frame_fragmentation_handler_tag,
            frame::write::FragmentationHandlerConfig{})
        .addNextDuplex<
            rocket::server::handler::RocketServerMessageMarshalHandler>(
            rocket_server_message_marshal_handler_tag)
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
        .addNextDuplex<frame::handler::FrameCodecHandler>(
            frame_codec_handler_tag)
        .addNextInbound<frame::read::handler::FrameDefragmentationHandler>(
            frame_defragmentation_handler_tag)
        .addNextOutbound<frame::write::handler::FrameFragmentationHandler>(
            frame_fragmentation_handler_tag,
            frame::write::FragmentationHandlerConfig{})
        .addNextDuplex<
            rocket::server::handler::RocketServerMessageMarshalHandler>(
            rocket_server_message_marshal_handler_tag)
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
FastThriftServerT<Stats>::~FastThriftServerT() {
  stop();

  // Destroy the connection manager before joining the executor. This triggers
  // deferred ConnectionHandler destruction on the EventBase threads.
  // We must then join the executor so those deferred callbacks run while
  // `this` is still alive. Without this, member destruction order would
  // destroy those members before the executor joins, causing use-after-free
  // in the pipeline factory lambda that captures `this`.
  connectionManager_.reset();
  executor_->join();
}

template <typename Stats>
void FastThriftServerT<Stats>::start() {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  if (state_ != State::kNotStarted) {
    return;
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
  connectionManager_ = ServerConnectionManager::create(
      config_.address,
      folly::getKeepAliveToken(executor_.get()),
      sslPolicy,
      std::move(tlsParams),
      socketOptions_);
  connectionManager_->setConnectionFactory(ConnectionFactoryImpl{this});
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
  {
    std::lock_guard<std::mutex> lock(lifecycleMutex_);
    if (state_ != State::kRunning) {
      return;
    }
    state_ = State::kStopped;
  }

  // ConnectionManager::stop() handles the whole flow: stop accepting, drain
  // in-flight connections, force-close any stragglers. Mirrors
  // FastThriftServer::stop.
  connectionManager_->stop();

  stopBaton_.post();
}

template <typename Stats>
folly::SocketAddress FastThriftServerT<Stats>::getAddress() const {
  return connectionManager_->getAddress();
}

} // namespace apache::thrift::fast_thrift::thrift
