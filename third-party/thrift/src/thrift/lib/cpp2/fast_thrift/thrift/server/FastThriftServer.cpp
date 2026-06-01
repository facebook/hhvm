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
#include <thrift/lib/cpp2/server/Cpp2Worker.h>

namespace apache::thrift::fast_thrift::thrift {

using channel_pipeline::PipelineBuilder;
using channel_pipeline::PipelineImpl;
using channel_pipeline::SimpleBufferAllocator;

// Handler tags for pipeline construction
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
      createPipelineFactory());
}

rocket::server::connection::PipelineFactory<
    FastThriftServer::ServerTransportHandler>
FastThriftServer::createPipelineFactory() {
  return [this](
             folly::EventBase* evb,
             ServerTransportHandler* transportHandler) -> PipelineImpl::Ptr {
    auto serverChannel =
        std::make_shared<ThriftServerChannel>(processorFactory_);

    auto pipeline = buildPipeline(evb, transportHandler, serverChannel.get());

    serverChannel->setPipelineRef(*pipeline);
    serverChannel->setWorker(apache::thrift::Cpp2Worker::createDummy(evb));

    if (config_.zeroCopyThreshold > 0) {
      if (!transportHandler->setZeroCopy(true)) {
        XLOG(WARN) << "MSG_ZEROCOPY not supported on this socket";
      }
      transportHandler->setZeroCopyEnableThreshold(config_.zeroCopyThreshold);
    }

    registerChannel(std::move(serverChannel));

    return pipeline;
  };
}

PipelineImpl::Ptr FastThriftServer::buildPipeline(
    folly::EventBase* evb,
    ServerTransportHandler* transportHandler,
    ThriftServerChannel* serverChannel) {
  return PipelineBuilder<
             ServerTransportHandler,
             ThriftServerChannel,
             SimpleBufferAllocator>()
      .setEventBase(evb)
      .setHead(transportHandler)
      .setTail(serverChannel)
      .setAllocator(&allocator_)
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

void FastThriftServer::registerChannel(
    std::shared_ptr<ThriftServerChannel> channel) {
  auto* rawPtr = channel.get();
  // Close callback is invoked from ThriftServerChannel::onException when the
  // connection closes (pipeline exception propagation). This removes the
  // channel from tracking, releasing the shared_ptr and destroying it.
  channel->setCloseCallback([this, rawPtr]() {
    serverChannels_.withWLock(
        [rawPtr](auto& channels) { channels.erase(rawPtr); });
  });
  serverChannels_.withWLock(
      [&](auto& channels) { channels.emplace(rawPtr, std::move(channel)); });
}

FastThriftServer::~FastThriftServer() {
  stop();

  // Destroy the connection manager before joining the executor. This triggers
  // deferred ConnectionHandler destruction on the EventBase threads.
  // We must then join the executor so those deferred callbacks run while
  // `this` (and all its members like allocator_, serverChannels_) is still
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

  // Move channels out under the lock, then destroy them outside.
  // Channel destructors invoke close callbacks that lock serverChannels_,
  // so destroying inside withWLock would deadlock.
  std::unordered_map<ThriftServerChannel*, std::shared_ptr<ThriftServerChannel>>
      channels;
  serverChannels_.withWLock([&](auto& ch) { channels = std::move(ch); });
  for (auto& [_, ch] : channels) {
    ch->setCloseCallback(nullptr);
  }
  channels.clear();

  stopBaton_.post();
}

folly::SocketAddress FastThriftServer::getAddress() const {
  return connectionManager_->getAddress();
}

} // namespace apache::thrift::fast_thrift::thrift
