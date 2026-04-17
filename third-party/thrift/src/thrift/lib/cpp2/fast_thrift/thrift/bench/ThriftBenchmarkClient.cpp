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
 * Thrift E2E Benchmark Client
 *
 * Connects to a BenchmarkService server and runs Ping, Echo, and LargePayload
 * benchmarks. --rpc_type controls the client implementation:
 *   - thrift:          Standard apache::thrift::Client + RocketClientChannel
 *   - fast_thrift:     Generated BenchmarkServiceFastClient + AppAdapter
 *
 * Usage:
 *   buck2 run @//mode/opt-clang-lto \
 *     //thrift/lib/cpp2/fast_thrift/thrift/bench:thrift_benchmark_client \
 *     -- --host=::1 --port=5001 --rpc_type=thrift --runtime_s=10
 */

#include <chrono>
#include <iomanip>
#include <iostream>

#include <gflags/gflags.h>
#include <folly/BenchmarkUtil.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Task.h>
#include <folly/init/Init.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/logging/xlog.h>
#include <folly/stats/Histogram.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_constants.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/BatchingFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientConnection.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientErrorFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/bench/if/gen-cpp2/BenchmarkFastServiceAsyncClient.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/bench/if/gen-cpp2/BenchmarkService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/bench/if/gen-cpp2/BenchmarkServiceAsyncClient.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/adapter/ThriftClientTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

DEFINE_string(host, "::1", "Server host");
DEFINE_int32(port, 0, "Server port");
DEFINE_int32(runtime_s, 10, "Runtime per benchmark (seconds)");
DEFINE_int64(payload_size, 4096, "Payload size for LargePayload benchmark");
DEFINE_string(rpc_type, "thrift", "Client implementation: thrift, fast_thrift");

THRIFT_FLAG_DECLARE_bool(rocket_client_binary_rpc_metadata_encoding);

using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::thrift::bench;

// Pipeline handler tags
HANDLER_TAG(batching_frame_handler);
HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_client_frame_codec_handler);
HANDLER_TAG(rocket_client_setup_handler);
HANDLER_TAG(rocket_client_request_response_frame_handler);
HANDLER_TAG(rocket_client_error_frame_handler);
HANDLER_TAG(rocket_client_stream_state_handler);

namespace {

// =============================================================================
// Connect Callback
// =============================================================================

class ConnectCallback : public folly::AsyncSocket::ConnectCallback {
 public:
  explicit ConnectCallback(
      transport::TransportHandler* transportHandler,
      folly::Baton<>& baton,
      bool& connected)
      : transportHandler_(transportHandler),
        baton_(baton),
        connected_(connected) {}

  void connectSuccess() noexcept override {
    connected_ = true;
    transportHandler_->onConnect();
    baton_.post();
  }

  void connectErr(const folly::AsyncSocketException& ex) noexcept override {
    XLOG(ERR) << "Connection failed: " << ex.what();
    connected_ = false;
    baton_.post();
  }

 private:
  transport::TransportHandler* transportHandler_;
  folly::Baton<>& baton_;
  bool& connected_;
};

// =============================================================================
// Setup factory for fast_thrift client pipeline
// =============================================================================

auto makeSetupFactory() {
  return []() {
    apache::thrift::RequestSetupMetadata meta;
    meta.minVersion() = 8;
    meta.maxVersion() = 10;
    meta.clientMetadata().ensure().agent() = "fast_thrift_bench_client";

    apache::thrift::BinaryProtocolWriter writer;
    folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
    writer.setOutput(&queue);
    meta.write(&writer);

    folly::IOBufQueue result(folly::IOBufQueue::cacheChainLength());
    const uint32_t protocolKey =
        apache::thrift::RpcMetadata_constants::kRocketProtocolKey();
    folly::io::QueueAppender appender(&result, sizeof(protocolKey));
    appender.writeBE<uint32_t>(protocolKey);
    result.append(queue.move());

    return std::make_pair(result.move(), std::unique_ptr<folly::IOBuf>());
  };
}

// =============================================================================
// Benchmark Runner
// =============================================================================

struct BenchmarkResult {
  std::string name;
  uint64_t iterations{0};
  double totalTimeMs{0};
  folly::Histogram<double> latencyHistogram{1.0, 0.0, 100000.0};
};

void printResult(const BenchmarkResult& result) {
  double qps = result.iterations / (result.totalTimeMs / 1000.0);
  std::cout << "  === " << result.name << " ===" << std::endl;
  std::cout << "    QPS:        " << std::fixed << std::setprecision(0) << qps
            << std::endl;
  std::cout << "    Requests:   " << result.iterations << std::endl;
  std::cout << "    Latency (us):" << std::endl;
  std::cout << "      P50:  " << std::fixed << std::setprecision(2)
            << result.latencyHistogram.getPercentileEstimate(0.50) << std::endl;
  std::cout << "      P90:  " << std::fixed << std::setprecision(2)
            << result.latencyHistogram.getPercentileEstimate(0.90) << std::endl;
  std::cout << "      P99:  " << std::fixed << std::setprecision(2)
            << result.latencyHistogram.getPercentileEstimate(0.99) << std::endl;
  std::cout << std::endl;
}

// =============================================================================
// Thrift Client (standard RocketClientChannel)
// =============================================================================

struct ThriftClientState {
  std::unique_ptr<folly::ScopedEventBaseThread> thread;
  std::unique_ptr<apache::thrift::Client<BenchmarkService>> client;
};

ThriftClientState createThriftClient(const folly::SocketAddress& addr) {
  auto thread = std::make_unique<folly::ScopedEventBaseThread>();
  auto* evb = thread->getEventBase();

  std::unique_ptr<apache::thrift::Client<BenchmarkService>> client;
  evb->runInEventBaseThreadAndWait([&] {
    auto socket = folly::AsyncSocket::newSocket(evb, addr);
    auto channel =
        apache::thrift::RocketClientChannel::newChannel(std::move(socket));
    client = std::make_unique<apache::thrift::Client<BenchmarkService>>(
        std::move(channel));
  });

  return {std::move(thread), std::move(client)};
}

// =============================================================================
// fast_thrift Client (Generated FastClient + AppAdapter)
// =============================================================================

using FastClient = apache::thrift::
    FastClient<BenchmarkFastService, thrift::ThriftClientAppAdapter>;

struct FastThriftClientState {
  std::unique_ptr<folly::ScopedEventBaseThread> thread;
  std::unique_ptr<thrift::client::ThriftClientTransportAdapter>
      transportAdapter;
  channel_pipeline::PipelineImpl::Ptr thriftPipeline;
  channel_pipeline::SimpleBufferAllocator thriftAllocator;
  std::unique_ptr<ConnectCallback> connectCallback;
  std::unique_ptr<FastClient> client;
};

FastThriftClientState createFastThriftClient(const folly::SocketAddress& addr) {
  FastThriftClientState state;
  state.thread = std::make_unique<folly::ScopedEventBaseThread>();
  auto* evb = state.thread->getEventBase();

  folly::Baton<> connectBaton;
  bool connected = false;
  thrift::ThriftClientAppAdapter::Ptr adapter(
      new thrift::ThriftClientAppAdapter(
          static_cast<uint16_t>(apache::thrift::protocol::T_COMPACT_PROTOCOL)));

  evb->runInEventBaseThreadAndWait([&] {
    auto socket = folly::AsyncSocket::newSocket(evb);
    auto* socketPtr = socket.get();

    // Build rocket pipeline inside a transport connection
    auto connection =
        std::make_unique<rocket::client::RocketClientConnection>();

    connection->transportHandler =
        transport::TransportHandler::create(std::move(socket));

    // Save raw pointer for ConnectCallback (before ownership transfer)
    auto* transportHandlerPtr = connection->transportHandler.get();

    state.connectCallback = std::make_unique<ConnectCallback>(
        transportHandlerPtr, connectBaton, connected);
    socketPtr->connect(state.connectCallback.get(), addr, 30000);

    connection->pipeline =
        channel_pipeline::PipelineBuilder<
            transport::TransportHandler,
            rocket::client::RocketClientAppAdapter,
            channel_pipeline::SimpleBufferAllocator>()
            .setEventBase(evb)
            .setHead(connection->transportHandler.get())
            .setTail(connection->appAdapter.get())
            .setAllocator(&connection->allocator)
            .addNextOutbound<frame::write::handler::BatchingFrameHandler>(
                batching_frame_handler_tag)
            .addNextInbound<frame::read::handler::FrameLengthParserHandler>(
                frame_length_parser_handler_tag)
            .addNextOutbound<frame::write::handler::FrameLengthEncoderHandler>(
                frame_length_encoder_handler_tag)
            .addNextDuplex<
                rocket::client::handler::RocketClientFrameCodecHandler>(
                rocket_client_frame_codec_handler_tag)
            .addNextDuplex<
                rocket::client::handler::RocketClientSetupFrameHandler>(
                rocket_client_setup_handler_tag, makeSetupFactory())
            .addNextDuplex<rocket::client::handler::
                               RocketClientRequestResponseFrameHandler>(
                rocket_client_request_response_frame_handler_tag)
            .addNextInbound<
                rocket::client::handler::RocketClientErrorFrameHandler>(
                rocket_client_error_frame_handler_tag)
            .addNextDuplex<
                rocket::client::handler::RocketClientStreamStateHandler>(
                rocket_client_stream_state_handler_tag)
            .build();

    connection->appAdapter->setPipeline(connection->pipeline.get());
    connection->transportHandler->setPipeline(*connection->pipeline);

    // Create transport adapter (takes ownership of connection)
    state.transportAdapter =
        std::make_unique<thrift::client::ThriftClientTransportAdapter>(
            std::move(connection));

    // Build thrift pipeline
    state.thriftPipeline = channel_pipeline::PipelineBuilder<
                               thrift::client::ThriftClientTransportAdapter,
                               thrift::ThriftClientAppAdapter,
                               channel_pipeline::SimpleBufferAllocator>()
                               .setEventBase(evb)
                               .setHead(state.transportAdapter.get())
                               .setTail(adapter.get())
                               .setAllocator(&state.thriftAllocator)
                               .build();

    adapter->setPipeline(state.thriftPipeline.get());
    state.transportAdapter->setPipeline(state.thriftPipeline.get());
  });

  connectBaton.wait();
  if (!connected) {
    LOG(FATAL) << "Failed to connect to server";
  }

  state.client = std::make_unique<FastClient>(std::move(adapter));
  return state;
}

// =============================================================================
// Benchmark loop — generic over any client with co_ping/co_echo/co_sendResponse
// =============================================================================

template <typename ClientT>
BenchmarkResult runBenchmark(
    const std::string& name,
    folly::EventBase* evb,
    ClientT& client,
    folly::coro::Task<void>(benchFn)(ClientT&),
    int runtimeSeconds) {
  BenchmarkResult result;
  result.name = name;

  auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(runtimeSeconds);

  auto totalStart = std::chrono::high_resolution_clock::now();

  folly::coro::blockingWait(
      folly::coro::co_withExecutor(evb, [&]() -> folly::coro::Task<void> {
        while (std::chrono::steady_clock::now() < deadline) {
          auto start = std::chrono::high_resolution_clock::now();
          co_await benchFn(client);
          auto end = std::chrono::high_resolution_clock::now();

          auto latency_us =
              std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                  .count();
          result.latencyHistogram.addValue(static_cast<double>(latency_us));
          result.iterations++;
        }
      }()));

  auto totalEnd = std::chrono::high_resolution_clock::now();
  result.totalTimeMs = std::chrono::duration_cast<std::chrono::microseconds>(
                           totalEnd - totalStart)
                           .count() /
      1000.0;

  return result;
}

// =============================================================================
// Benchmark functions for standard Client<BenchmarkService>
// =============================================================================

folly::coro::Task<void> benchPing(
    apache::thrift::Client<BenchmarkService>& client) {
  co_await client.co_ping();
}

folly::coro::Task<void> benchEcho(
    apache::thrift::Client<BenchmarkService>& client) {
  auto result = co_await client.co_echo("hello world");
  folly::doNotOptimizeAway(result);
}

folly::coro::Task<void> benchLargePayload(
    apache::thrift::Client<BenchmarkService>& client) {
  auto result = co_await client.co_sendResponse(FLAGS_payload_size);
  folly::doNotOptimizeAway(result);
}

// =============================================================================
// Benchmark functions for FastClient
// =============================================================================

folly::coro::Task<void> benchPingFast(FastClient& client) {
  co_await client.co_ping();
}

folly::coro::Task<void> benchEchoFast(FastClient& client) {
  auto result = co_await client.co_echo("hello world");
  folly::doNotOptimizeAway(result);
}

folly::coro::Task<void> benchLargePayloadFast(FastClient& client) {
  auto result = co_await client.co_sendResponse(FLAGS_payload_size);
  folly::doNotOptimizeAway(result);
}

// =============================================================================
// Run all benchmarks for a given client type
// =============================================================================

template <typename ClientT, typename BenchFn>
void runAllBenchmarks(
    const std::string& rpcType,
    folly::EventBase* evb,
    ClientT& client,
    BenchFn pingFn,
    BenchFn echoFn,
    BenchFn largePayloadFn,
    int runtimeSeconds) {
  std::cout << "\n========================================" << std::endl;
  std::cout << "  rpc_type: " << rpcType << std::endl;
  std::cout << "  runtime: " << runtimeSeconds << "s per benchmark"
            << std::endl;
  std::cout << "  payload_size: " << FLAGS_payload_size << " bytes"
            << std::endl;
  std::cout << "========================================\n" << std::endl;

  printResult(runBenchmark("Ping", evb, client, pingFn, runtimeSeconds));
  printResult(runBenchmark("Echo", evb, client, echoFn, runtimeSeconds));
  printResult(runBenchmark(
      "LargePayload", evb, client, largePayloadFn, runtimeSeconds));
}

} // namespace

int main(int argc, char* argv[]) {
  folly::Init init(&argc, &argv);

  THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

  if (FLAGS_port == 0) {
    std::cerr << "Error: --port is required" << std::endl;
    return 1;
  }

  folly::SocketAddress serverAddress(FLAGS_host, FLAGS_port);
  std::cout << "Server: " << serverAddress.describe() << std::endl;
  std::cout << "RPC Type: " << FLAGS_rpc_type << std::endl;

  if (FLAGS_rpc_type == "thrift") {
    auto state = createThriftClient(serverAddress);
    auto* evb = state.thread->getEventBase();
    runAllBenchmarks(
        "thrift",
        evb,
        *state.client,
        benchPing,
        benchEcho,
        benchLargePayload,
        FLAGS_runtime_s);
    evb->runInEventBaseThreadAndWait([&] { state.client.reset(); });

  } else if (FLAGS_rpc_type == "fast_thrift") {
    auto state = createFastThriftClient(serverAddress);
    auto* evb = state.thread->getEventBase();
    runAllBenchmarks(
        "fast_thrift",
        evb,
        *state.client,
        benchPingFast,
        benchEchoFast,
        benchLargePayloadFast,
        FLAGS_runtime_s);
    evb->runInEventBaseThreadAndWait([&] {
      state.client.reset();
      state.thriftPipeline.reset();
      state.transportAdapter.reset();
    });

  } else {
    std::cerr << "Unknown rpc_type: " << FLAGS_rpc_type
              << " (expected: thrift, fast_thrift)" << std::endl;
    return 1;
  }

  return 0;
}
