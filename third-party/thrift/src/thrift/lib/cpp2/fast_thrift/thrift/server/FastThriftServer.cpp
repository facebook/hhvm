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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/BatchingFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>

namespace apache::thrift::fast_thrift::thrift {

using channel_pipeline::PipelineBuilder;
using channel_pipeline::PipelineImpl;
using channel_pipeline::SimpleBufferAllocator;

// Handler tags for rocket pipeline construction
HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(batching_frame_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_server_frame_codec_handler);
HANDLER_TAG(server_setup_frame_handler);
HANDLER_TAG(server_request_response_frame_handler);
HANDLER_TAG(server_stream_state_handler);

FastThriftServer::FastThriftServer(
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

rocket::server::connection::ConnectionFactory
FastThriftServer::createConnectionFactory() {
  return [this](folly::AsyncSocket::UniquePtr socket)
             -> rocket::server::connection::RocketServerConnection {
    auto* evb = socket->getEventBase();
    auto transportHandler =
        transport::TransportHandler::create(std::move(socket));

    // Build the RocketServerConnection with default appAdapter
    rocket::server::connection::RocketServerConnection conn;

    // 1. Build rocket pipeline: TransportHandler → ... → RocketServerAppAdapter
    auto rocketPipeline =
        buildRocketPipeline(evb, transportHandler.get(), conn.appAdapter.get());
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
    auto thriftPipeline =
        PipelineBuilder<
            server::ThriftServerTransportAdapter,
            ThriftServerChannel,
            SimpleBufferAllocator>()
            .setEventBase(evb)
            .setHead(transportAdapter.get())
            .setTail(serverChannel.get())
            .setAllocator(ctx.thriftAllocator.get())
            .setHeadToTailOp(channel_pipeline::HeadToTailOp::Read)
            .build();

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

PipelineImpl::Ptr FastThriftServer::buildRocketPipeline(
    folly::EventBase* evb,
    transport::TransportHandler* transportHandler,
    rocket::server::RocketServerAppAdapter* appAdapter) {
  return PipelineBuilder<
             transport::TransportHandler,
             rocket::server::RocketServerAppAdapter,
             SimpleBufferAllocator>()
      .setEventBase(evb)
      .setHead(transportHandler)
      .setTail(appAdapter)
      .setAllocator(&rocketAllocator_)
      .setHeadToTailOp(channel_pipeline::HeadToTailOp::Read)
      .addNextDuplex<rocket::server::handler::RocketServerStreamStateHandler>(
          server_stream_state_handler_tag)
      .addNextDuplex<
          rocket::server::handler::RocketServerRequestResponseFrameHandler>(
          server_request_response_frame_handler_tag)
      .addNextDuplex<rocket::server::handler::RocketServerSetupFrameHandler>(
          server_setup_frame_handler_tag)
      .addNextDuplex<rocket::server::handler::RocketServerFrameCodecHandler>(
          rocket_server_frame_codec_handler_tag)
      .addNextOutbound<frame::write::handler::FrameLengthEncoderHandler>(
          frame_length_encoder_handler_tag)
      .addNextOutbound<frame::write::handler::BatchingFrameHandler>(
          batching_frame_handler_tag)
      .addNextInbound<frame::read::handler::FrameLengthParserHandler>(
          frame_length_parser_handler_tag)
      .build();
}

void FastThriftServer::registerConnection(
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

FastThriftServer::~FastThriftServer() {
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

void FastThriftServer::start() {
  if (started_.exchange(true)) {
    return;
  }
  connectionManager_->start();
  XLOG(INFO) << "FastThriftServer listening on " << getAddress();
}

void FastThriftServer::serve() {
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

void FastThriftServer::stop() {
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

folly::SocketAddress FastThriftServer::getAddress() const {
  return connectionManager_->getAddress();
}

} // namespace apache::thrift::fast_thrift::thrift
