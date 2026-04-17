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

#include <thrift/lib/cpp2/fast_thrift/bench/tcp/TcpServer.h>

#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>

namespace apache::thrift::fast_thrift::bench {

/**
 * ServerAppAdapter is a minimal server-side adapter for benchmarking.
 *
 * This adapter owns the pipeline. The transport handler is owned by
 * the ConnectionManager. It is optimized for performance:
 * - No message storage (just echo back)
 * - No synchronization primitives for waiting
 * - Minimal overhead per message
 */
class TcpServer::ServerAppAdapter {
 public:
  using BytesPtr = apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
  using TypeErasedBox =
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
  using Result = apache::thrift::fast_thrift::channel_pipeline::Result;

  ServerAppAdapter() = default;

  void setPipeline(
      apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline) {
    pipeline_ = pipeline;
  }

  /**
   * Called by the pipeline when a message reaches the application layer.
   * Immediately echoes the message back.
   */
  Result onMessage(TypeErasedBox&& msg) noexcept {
    if (!pipeline_) {
      XLOG(ERR) << "ServerAppAdapter::onMessage called with null pipeline";
      return Result::Error;
    }
    if (msg.empty()) {
      XLOG(ERR) << "ServerAppAdapter::onMessage called with empty message";
      return Result::Error;
    }

    auto& bytes = msg.get<BytesPtr>();
    auto response = bytes->clone();
    (void)pipeline_->fireWrite(TypeErasedBox(std::move(response)));
    return Result::Success;
  }

  void onException(folly::exception_wrapper&& /*e*/) noexcept {}

 private:
  apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline_{
      nullptr};
};

TcpServer::TcpServer(
    folly::SocketAddress address,
    uint32_t numIOThreads,
    size_t zeroCopyThreshold)
    : executor_(std::make_shared<folly::IOThreadPoolExecutor>(numIOThreads)),
      zeroCopyThreshold_(zeroCopyThreshold) {
  apache::thrift::fast_thrift::rocket::server::connection::ConnectionFactory
      connectionFactory = [this](folly::AsyncSocket::UniquePtr socket)
      -> apache::thrift::fast_thrift::rocket::server::connection::
          RocketServerConnection {
            auto* evb = socket->getEventBase();
            auto transportHandler = apache::thrift::fast_thrift::transport::
                TransportHandler::create(std::move(socket));

            auto adapter = std::make_unique<ServerAppAdapter>();

            auto pipeline =
                apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder<
                    apache::thrift::fast_thrift::transport::TransportHandler,
                    ServerAppAdapter,
                    apache::thrift::fast_thrift::channel_pipeline::
                        SimpleBufferAllocator>()
                    .setEventBase(evb)
                    .setHead(transportHandler.get())
                    .setTail(adapter.get())
                    .setAllocator(&allocator_)
                    .setHeadToTailOp(
                        apache::thrift::fast_thrift::channel_pipeline::
                            HeadToTailOp::Read)
                    .build();

            adapter->setPipeline(pipeline.get());
            adapters_.insert(std::move(adapter));

            transportHandler->setPipeline(*pipeline);

            // Configure MSG_ZEROCOPY for large payloads
            if (zeroCopyThreshold_ > 0) {
              if (!transportHandler->setZeroCopy(true)) {
                XLOG(WARN) << "MSG_ZEROCOPY not supported on this socket";
              }
              transportHandler->setZeroCopyEnableThreshold(zeroCopyThreshold_);
            }

            return apache::thrift::fast_thrift::rocket::server::connection::
                RocketServerConnection{
                    .transportHandler = std::move(transportHandler),
                    .pipeline = std::move(pipeline),
                    .allocator = {},
                };
          };

  connectionManager_ = TcpConnectionManager::create(
      std::move(address),
      folly::getKeepAliveToken(executor_.get()),
      std::move(connectionFactory));
}

TcpServer::~TcpServer() = default;

void TcpServer::start() {
  connectionManager_->start();
}

void TcpServer::stop() {
  connectionManager_->stop();
  adapters_.clear();
}

folly::SocketAddress TcpServer::getAddress() const {
  return connectionManager_->getAddress();
}

} // namespace apache::thrift::fast_thrift::bench
