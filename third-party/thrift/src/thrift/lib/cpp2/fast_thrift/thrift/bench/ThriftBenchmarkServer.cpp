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

/**
 * Thrift E2E Benchmark Server
 *
 * Serves a BenchmarkService with --rpc_type controlling the server
 * implementation:
 *   - thrift:      Standard apache::thrift::ThriftServer
 *   - fast_thrift: fast_thrift ConnectionManager + ThriftServerChannel pipeline
 *
 * Usage:
 *   buck2 run @//mode/opt-clang-lto \
 *     //thrift/lib/cpp2/fast_thrift/thrift/bench:thrift_benchmark_server \
 *     -- --port=5001 --rpc_type=thrift
 */

#include <gflags/gflags.h>
#include <folly/Synchronized.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/init/Init.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/server/ThriftServer.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/BatchingFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/connection/ConnectionManager.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/bench/if/gen-cpp2/BenchmarkService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerChannel.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

DEFINE_int32(port, 0, "Port to listen on (0 = system assigned)");
DEFINE_int32(io_threads, 8, "Number of IO threads");
DEFINE_string(
    rpc_type,
    "thrift",
    "Server implementation: thrift (ThriftServer) or fast_thrift (pipeline)");

using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::thrift::bench;

// Handler tags for the fast_thrift server pipeline
HANDLER_TAG(batching_frame_handler);
HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_server_frame_codec_handler);
HANDLER_TAG(rocket_server_setup_frame_handler);
HANDLER_TAG(rocket_server_request_response_frame_handler);
HANDLER_TAG(rocket_server_stream_state_handler);

namespace {

// =============================================================================
// Service Handler
// =============================================================================

class BenchmarkServiceHandler
    : public apache::thrift::ServiceHandler<BenchmarkService> {
 public:
  void ping() override {}

  void echo(
      std::string& response, std::unique_ptr<std::string> message) override {
    response = std::move(*message);
  }

  void sendResponse(std::string& response, int64_t size) override {
    response = std::string(static_cast<size_t>(size), 'x');
  }
};

// =============================================================================
// Standard Thrift Server
// =============================================================================

std::shared_ptr<apache::thrift::ThriftServer> createThriftServer(
    std::shared_ptr<BenchmarkServiceHandler> handler,
    uint16_t port,
    uint32_t numIOThreads) {
  auto server = std::make_shared<apache::thrift::ThriftServer>();
  server->setInterface(std::move(handler));
  server->setPort(port);
  server->setNumIOWorkerThreads(numIOThreads);
  return server;
}

// =============================================================================
// fast_thrift Pipeline Server
// =============================================================================

class FastThriftBenchmarkServer {
 public:
  FastThriftBenchmarkServer(
      std::shared_ptr<BenchmarkServiceHandler> handler,
      uint16_t port,
      uint32_t numIOThreads)
      : handler_(std::move(handler)),
        executor_(std::make_shared<folly::IOThreadPoolExecutor>(numIOThreads)) {
    rocket::server::connection::ConnectionFactory connectionFactory =
        [this](folly::AsyncSocket::UniquePtr socket)
        -> rocket::server::connection::RocketServerConnection {
      auto* evb = socket->getEventBase();
      auto transportHandler =
          transport::TransportHandler::create(std::move(socket));

      auto serverChannel =
          std::make_shared<thrift::ThriftServerChannel>(handler_);

      auto pipeline =
          channel_pipeline::PipelineBuilder<
              transport::TransportHandler,
              thrift::ThriftServerChannel,
              channel_pipeline::SimpleBufferAllocator>()
              .setEventBase(evb)
              .setHead(transportHandler.get())
              .setTail(serverChannel.get())
              .setAllocator(&allocator_)
              .addNextOutbound<frame::write::handler::BatchingFrameHandler>(
                  batching_frame_handler_tag)
              .addNextInbound<frame::read::handler::FrameLengthParserHandler>(
                  frame_length_parser_handler_tag)
              .addNextOutbound<
                  frame::write::handler::FrameLengthEncoderHandler>(
                  frame_length_encoder_handler_tag)
              .addNextDuplex<
                  rocket::server::handler::RocketServerFrameCodecHandler>(
                  rocket_server_frame_codec_handler_tag)
              .addNextDuplex<
                  rocket::server::handler::RocketServerSetupFrameHandler>(
                  rocket_server_setup_frame_handler_tag)
              .addNextDuplex<rocket::server::handler::
                                 RocketServerRequestResponseFrameHandler>(
                  rocket_server_request_response_frame_handler_tag)
              .addNextDuplex<
                  rocket::server::handler::RocketServerStreamStateHandler>(
                  rocket_server_stream_state_handler_tag)
              .build();

      serverChannel->setPipelineRef(*pipeline);
      serverChannel->setWorker(apache::thrift::Cpp2Worker::createDummy(evb));

      transportHandler->setPipeline(*pipeline);

      serverChannels_.withWLock([&](auto& channels) {
        channels.push_back(std::move(serverChannel));
      });

      return rocket::server::connection::RocketServerConnection{
          .transportHandler = std::move(transportHandler),
          .pipeline = std::move(pipeline),
          .allocator = {},
      };
    };

    folly::SocketAddress address;
    address.setFromLocalPort(port);

    connectionManager_ = rocket::server::connection::ConnectionManager::create(
        std::move(address),
        folly::getKeepAliveToken(executor_.get()),
        std::move(connectionFactory));
  }

  void serve() {
    connectionManager_->start();
    XLOG(INFO) << "fast_thrift server started on " << getAddress();
    executor_->join();
  }

  folly::SocketAddress getAddress() const {
    return connectionManager_->getAddress();
  }

 private:
  std::shared_ptr<BenchmarkServiceHandler> handler_;
  std::shared_ptr<folly::IOThreadPoolExecutor> executor_;
  rocket::server::connection::ConnectionManager::Ptr connectionManager_;
  channel_pipeline::SimpleBufferAllocator allocator_;
  folly::Synchronized<std::vector<std::shared_ptr<thrift::ThriftServerChannel>>>
      serverChannels_;
};

} // namespace

int main(int argc, char* argv[]) {
  folly::Init init(&argc, &argv);

  auto handler = std::make_shared<BenchmarkServiceHandler>();
  auto port = static_cast<uint16_t>(FLAGS_port);

  if (FLAGS_rpc_type == "thrift") {
    auto server = createThriftServer(handler, port, FLAGS_io_threads);
    XLOG(INFO) << "Starting ThriftServer on port " << FLAGS_port;
    server->serve();
  } else if (FLAGS_rpc_type == "fast_thrift") {
    FastThriftBenchmarkServer server(handler, port, FLAGS_io_threads);
    server.serve();
  } else {
    XLOG(FATAL) << "Unknown rpc_type: " << FLAGS_rpc_type
                << " (expected: thrift, fast_thrift)";
  }

  return 0;
}
