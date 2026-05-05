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
#include <type_traits>
#include <unordered_map>

#include <folly/SocketAddress.h>
#include <folly/Synchronized.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/common/Stats.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/connection/ConnectionManager.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Configuration for FastThriftServer.
 */
struct FastThriftServerConfig {
  // Address to bind to.
  folly::SocketAddress address;

  // Number of IO threads. Each thread runs its own EventBase and accepts
  // connections via SO_REUSEPORT.
  uint32_t numIOThreads{1};

  // Minimum payload size in bytes for MSG_ZEROCOPY. 0 disables zero-copy.
  size_t zeroCopyThreshold{0};
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
 *   FastThriftServer server(config, handler);
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
  using ServerConnectionManager = rocket::server::connection::ConnectionManager;

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

  rocket::server::connection::ConnectionFactory createConnectionFactory();

  channel_pipeline::PipelineImpl::Ptr buildRocketPipeline(
      folly::EventBase* evb,
      transport::TransportHandler* transportHandler,
      rocket::server::RocketServerAppAdapter* appAdapter,
      std::shared_ptr<Stats> stats);

  void registerConnection(
      ThriftServerChannel* key, ThriftConnectionContext context);

  const FastThriftServerConfig config_;
  std::shared_ptr<apache::thrift::AsyncProcessorFactory> processorFactory_;
  std::shared_ptr<folly::IOThreadPoolExecutor> executor_;
  ServerConnectionManager::Ptr connectionManager_;
  channel_pipeline::SimpleBufferAllocator rocketAllocator_;
  folly::Synchronized<
      std::unordered_map<ThriftServerChannel*, ThriftConnectionContext>>
      thriftConnections_;
  folly::Baton<> stopBaton_;
  std::atomic<bool> started_{false};
  std::atomic<bool> stopped_{false};
};

// Default type alias for convenience - no metrics by default
using FastThriftServer = FastThriftServerT<NoStats>;

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
  connectionManager_ = ServerConnectionManager::create(
      config_.address,
      folly::getKeepAliveToken(executor_.get()),
      createConnectionFactory());
}

template <typename Stats>
rocket::server::connection::ConnectionFactory
FastThriftServerT<Stats>::createConnectionFactory() {
  return [this](folly::AsyncSocket::UniquePtr socket)
             -> rocket::server::connection::RocketServerConnection {
    auto* evb = socket->getEventBase();
    auto transportHandler =
        transport::TransportHandler::create(std::move(socket));

    // Create shared per-connection stats if enabled
    std::shared_ptr<Stats> stats;
    if constexpr (kStatsEnabled) {
      stats = std::make_shared<Stats>();
    }

    // Build the RocketServerConnection with default appAdapter
    rocket::server::connection::RocketServerConnection conn;

    // 1. Build rocket pipeline: TransportHandler → ... → RocketServerAppAdapter
    auto rocketPipeline = buildRocketPipeline(
        evb, transportHandler.get(), conn.appAdapter.get(), stats);
    conn.appAdapter->setPipeline(rocketPipeline.get());
    transportHandler->setPipeline(*rocketPipeline);

    if (config_.zeroCopyThreshold > 0) {
      if (!transportHandler->setZeroCopy(true)) {
        XLOG(WARN) << "MSG_ZEROCOPY not supported on this socket";
      }
      transportHandler->setZeroCopyEnableThreshold(config_.zeroCopyThreshold);
    }

    conn.transportHandler = std::move(transportHandler);
    conn.pipeline = std::move(rocketPipeline);

    // 2. Build thrift pipeline: ThriftServerTransportAdapter →
    // ThriftServerChannel
    auto serverChannel =
        std::make_shared<ThriftServerChannel>(processorFactory_);
    auto transportAdapter =
        std::make_unique<server::ThriftServerTransportAdapter>(
            *conn.appAdapter);

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
            server_setup_frame_handler_tag)
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
            server_setup_frame_handler_tag)
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
  if (started_.exchange(true)) {
    return;
  }
  connectionManager_->start();
  XLOG(INFO) << "FastThriftServer listening on " << getAddress();
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
  if (!started_.load() || stopped_.exchange(true)) {
    return;
  }

  // Stop accepting new connections first
  connectionManager_->stop();

  // Clear all thrift contexts under the lock. Any close callback racing from an
  // EventBase thread will either complete before we acquire the write lock (and
  // already erase its entry) or find the map empty after we clear it.
  thriftConnections_.withWLock([&](auto& conns) { conns.clear(); });

  stopBaton_.post();
}

template <typename Stats>
folly::SocketAddress FastThriftServerT<Stats>::getAddress() const {
  return connectionManager_->getAddress();
}

} // namespace apache::thrift::fast_thrift::thrift
