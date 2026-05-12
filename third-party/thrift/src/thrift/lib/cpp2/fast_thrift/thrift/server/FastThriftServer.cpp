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
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

namespace apache::thrift::fast_thrift::thrift {

using channel_pipeline::PipelineBuilder;
using channel_pipeline::PipelineImpl;
using channel_pipeline::SimpleBufferAllocator;

FastThriftServer::FastThriftServer(FastThriftServerConfig config)
    : config_(std::move(config)),
      executor_(
          std::make_shared<folly::IOThreadPoolExecutor>(config_.numIOThreads)) {
}

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

rocket::server::connection::ConnectionFactory
FastThriftServer::createConnectionFactory() {
  return [this](folly::AsyncTransport::UniquePtr socket)
             -> rocket::server::connection::RocketServerConnection {
    auto* evb = socket->getEventBase();
    auto transportHandler =
        transport::TransportHandler::create(std::move(socket));

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

    // 2. Build thrift pipeline: ThriftServerTransportAdapter → FastSvAppAdapter
    //    The FastSvIf knows the concrete adapter type and constructs it; the
    //    server only sees the type-erased ThriftServerAppAdapter base.
    FastConnection ctx;
    ctx.adapter = handler_->getAppAdapter(handler_);
    ctx.transportAdapter =
        std::make_unique<server::ThriftServerTransportAdapter>(
            *conn.appAdapter);

    // PipelineBuilder is templated on the BASE ThriftServerAppAdapter — works
    // because generated FastSvAppAdapter subclasses don't override base
    // virtuals (they only populate dispatch_ via addMethodHandler in their
    // ctor). If a subclass starts overriding, this needs revisiting.
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

    auto* adapterKey = ctx.adapter.get();
    ctx.pipeline = std::move(thriftPipeline);

    registerConnection(adapterKey, std::move(ctx));

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
      .addNextDuplex<
          rocket::server::handler::RocketServerRequestResponseHandler>(
          server_request_response_frame_handler_tag)
      .addNextDuplex<rocket::server::handler::RocketServerStreamStateHandler>(
          server_stream_state_handler_tag)
      .build();
}

void FastThriftServer::registerConnection(
    ThriftServerAppAdapter* key, FastConnection connection) {
  auto* rawPtr = key;
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

FastThriftServer::~FastThriftServer() {
  stop();

  // Same lifetime ordering as FastThriftChannelServer: destroy the
  // connection manager before joining the executor so deferred
  // ConnectionHandler destruction happens while `this` is still alive.
  connectionManager_.reset();
  executor_->join();
}

void FastThriftServer::start() {
  std::lock_guard<std::mutex> lock(lifecycleMutex_);
  CHECK(handler_)
      << "FastThriftServer::start called before setInterface — no handler "
         "registered";
  if (state_ != State::kNotStarted) {
    return;
  }

  security::BuiltFizzServerContext fizzBuilt;
  std::chrono::milliseconds tlsHandshakeTimeout{std::chrono::seconds{5}};
  if (sslConfig_) {
    fizzBuilt = security::buildFizzServerContext(*sslConfig_, thriftConfig_);
    tlsHandshakeTimeout = sslConfig_->handshakeTimeout;
  }
  connectionManager_ = ServerConnectionManager::create(
      config_.address,
      folly::getKeepAliveToken(executor_.get()),
      createConnectionFactory(),
      std::move(fizzBuilt.fizzContext),
      std::move(fizzBuilt.thriftParams),
      tlsHandshakeTimeout);

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
  // ~FastConnection runs ~ThriftServerAppAdapter, which fires the close
  // callback synchronously; that callback re-acquires this same write lock,
  // so clearing under the lock would deadlock folly::SharedMutex.
  std::unordered_map<ThriftServerAppAdapter*, FastConnection> drained;
  thriftConnections_.withWLock([&](auto& conns) { drained.swap(conns); });
  stopBaton_.post();
}

folly::SocketAddress FastThriftServer::getAddress() const {
  return connectionManager_->getAddress();
}

} // namespace apache::thrift::fast_thrift::thrift
