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
 * ThriftClient Integration Microbenchmarks
 *
 * Measures the full end-to-end pipeline overhead including:
 * - Request path (outbound): ThriftClientChannel -> all handlers -> transport
 * - Response path (inbound): transport -> all handlers -> callback
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>

#include <folly/futures/Promise.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientConnection.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientErrorFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/RequestMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/adapter/ThriftClientTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientMetadataPushHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientRocketInterfaceHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/bench/BenchAsyncTransport.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::channel_pipeline::test;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::thrift;
using namespace apache::thrift::fast_thrift::thrift::client::handler;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read::handler;
using namespace apache::thrift::fast_thrift::frame::write::handler;
using namespace apache::thrift::fast_thrift::frame::read;
using namespace apache::thrift::fast_thrift::frame::write;
using namespace apache::thrift::fast_thrift::transport::bench;

namespace {

// =============================================================================
// Payload Size
// =============================================================================

constexpr size_t kPayloadSize = 4'096;

std::unique_ptr<folly::IOBuf> makePayloadData(size_t size) {
  return folly::IOBuf::copyBuffer(std::string(size, 'x'));
}

// Handler tags for pipeline construction
HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_client_frame_codec_handler);
HANDLER_TAG(rocket_client_setup_handler);
HANDLER_TAG(rocket_client_request_response_frame_handler);
HANDLER_TAG(rocket_client_error_frame_handler);
HANDLER_TAG(rocket_client_stream_state_handler);
HANDLER_TAG(thrift_client_rocket_interface_handler);
HANDLER_TAG(thrift_client_metadata_push_handler);

// =============================================================================
// Benchmark Request Callback - Minimal callback for benchmarks
// =============================================================================

class BenchRequestCallback : public apache::thrift::RequestClientCallback {
 public:
  void onResponse(
      apache::thrift::ClientReceiveState&& /*state*/) noexcept override {
    responseReceived_ = true;
  }

  void onResponseError(folly::exception_wrapper /*e*/) noexcept override {
    errorReceived_ = true;
  }

 private:
  bool responseReceived_{false};
  bool errorReceived_{false};
};

// =============================================================================
// Helper Functions
// =============================================================================

std::unique_ptr<folly::IOBuf> serializeResponseMetadata(
    const apache::thrift::ResponseRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

std::unique_ptr<folly::IOBuf> createPayloadResponse(
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  return serialize(
      PayloadHeader{.streamId = streamId, .complete = true, .next = true},
      std::move(metadata),
      std::move(data));
}

// =============================================================================
// Response Metadata Helpers
// =============================================================================

apache::thrift::ResponseRpcMetadata createSuccessResponseMetadata() {
  apache::thrift::ResponseRpcMetadata metadata;
  metadata.payloadMetadata().ensure().set_responseMetadata(
      apache::thrift::PayloadResponseMetadata{});
  return metadata;
}

apache::thrift::ResponseRpcMetadata createDeclaredExceptionMetadata() {
  apache::thrift::ResponseRpcMetadata metadata;

  apache::thrift::PayloadExceptionMetadataBase exMeta;
  exMeta.name_utf8() = "MyServiceException";
  exMeta.what_utf8() = "Expected service exception";

  apache::thrift::PayloadExceptionMetadata payloadExMeta;
  payloadExMeta.set_declaredException(
      apache::thrift::PayloadDeclaredExceptionMetadata{});
  exMeta.metadata() = std::move(payloadExMeta);

  metadata.payloadMetadata().ensure().set_exceptionMetadata(std::move(exMeta));
  return metadata;
}

apache::thrift::ResponseRpcMetadata createUndeclaredExceptionMetadata() {
  apache::thrift::ResponseRpcMetadata metadata;

  apache::thrift::PayloadExceptionMetadataBase exMeta;
  exMeta.name_utf8() = "std::runtime_error";
  exMeta.what_utf8() = "Unexpected runtime error";

  apache::thrift::PayloadExceptionMetadata payloadExMeta;
  payloadExMeta.set_DEPRECATED_proxyException(
      apache::thrift::PayloadProxyExceptionMetadata{});
  exMeta.metadata() = std::move(payloadExMeta);

  metadata.payloadMetadata().ensure().set_exceptionMetadata(std::move(exMeta));
  return metadata;
}

std::unique_ptr<folly::IOBuf> serializeResponseRpcError(
    apache::thrift::ResponseRpcErrorCode code, const std::string& message) {
  apache::thrift::ResponseRpcError error;
  error.code() = code;
  error.what_utf8() = message;

  apache::thrift::CompactProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  error.write(&writer);
  return queue.move();
}

constexpr uint32_t kErrorCodeRejected = 0x00000202;

std::unique_ptr<folly::IOBuf> createErrorFrame(
    uint32_t streamId, uint32_t errorCode, std::unique_ptr<folly::IOBuf> data) {
  return serialize(
      ErrorHeader{.streamId = streamId, .errorCode = errorCode},
      nullptr,
      std::move(data));
}

// =============================================================================
// Request Metadata Helpers
// =============================================================================

apache::thrift::RequestRpcMetadata createMinimalRequestMetadata() {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = "benchMethod";
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;
  return metadata;
}

apache::thrift::RequestRpcMetadata createRequestMetadataWithHeaders() {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = "benchMethod";
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;
  metadata.clientTimeoutMs() = 10000;
  metadata.queueTimeoutMs() = 5000;

  auto& headers = metadata.otherMetadata().ensure();
  headers["request_id"] = "abc123";
  headers["trace_id"] = "trace-456";
  headers["client_name"] = "benchmark_client";
  headers["routing_key"] = "shard-42";
  headers["priority"] = "high";

  return metadata;
}

std::shared_ptr<apache::thrift::transport::THeader> createHeader() {
  return std::make_shared<apache::thrift::transport::THeader>();
}

// Strip 3-byte length prefix from written data and parse the frame
ParsedFrame parseWrittenFrame(std::unique_ptr<folly::IOBuf> data) {
  // Skip the 3-byte length prefix
  folly::IOBufQueue queue;
  queue.append(std::move(data));
  queue.trimStart(kMetadataLengthSize);
  return parseFrame(queue.move());
}

// Prepend 3-byte length prefix to a frame (for injection)
std::unique_ptr<folly::IOBuf> prependLengthPrefix(
    std::unique_ptr<folly::IOBuf> frame) {
  size_t frameLength = frame->computeChainDataLength();
  auto lengthPrefix = folly::IOBuf::create(kMetadataLengthSize);
  uint8_t* data = lengthPrefix->writableData();
  data[0] = static_cast<uint8_t>((frameLength >> 16) & 0xFF);
  data[1] = static_cast<uint8_t>((frameLength >> 8) & 0xFF);
  data[2] = static_cast<uint8_t>(frameLength & 0xFF);
  lengthPrefix->append(kMetadataLengthSize);
  lengthPrefix->appendChain(std::move(frame));
  return lengthPrefix;
}

// =============================================================================
// Rocket connection builder
// =============================================================================
std::unique_ptr<rocket::client::RocketClientConnection> createRocketConnection(
    BenchAsyncTransport* benchTransport, folly::EventBase* evb) {
  auto transport = folly::AsyncTransport::UniquePtr(benchTransport);
  auto connection = std::make_unique<rocket::client::RocketClientConnection>();
  connection->transportHandler =
      apache::thrift::fast_thrift::transport::TransportHandler::create(
          std::move(transport));

  connection->pipeline =
      PipelineBuilder<
          apache::thrift::fast_thrift::rocket::client::RocketClientAppAdapter,
          apache::thrift::fast_thrift::transport::TransportHandler,
          channel_pipeline::SimpleBufferAllocator>()
          .setEventBase(evb)
          .setHead(connection->appAdapter.get())
          .setTail(connection->transportHandler.get())
          .setAllocator(&connection->allocator)
          .addNextInbound<FrameLengthParserHandler>(
              frame_length_parser_handler_tag)
          .addNextOutbound<FrameLengthEncoderHandler>(
              frame_length_encoder_handler_tag)
          .addNextDuplex<apache::thrift::fast_thrift::rocket::client::handler::
                             RocketClientFrameCodecHandler>(
              rocket_client_frame_codec_handler_tag)
          .addNextDuplex<apache::thrift::fast_thrift::rocket::client::handler::
                             RocketClientSetupFrameHandler>(
              rocket_client_setup_handler_tag,
              []() {
                return std::make_pair(
                    folly::IOBuf::copyBuffer("setup"),
                    std::unique_ptr<folly::IOBuf>());
              })
          .addNextDuplex<apache::thrift::fast_thrift::rocket::client::handler::
                             RocketClientRequestResponseFrameHandler>(
              rocket_client_request_response_frame_handler_tag)
          .addNextInbound<apache::thrift::fast_thrift::rocket::client::handler::
                              RocketClientErrorFrameHandler>(
              rocket_client_error_frame_handler_tag)
          .addNextDuplex<apache::thrift::fast_thrift::rocket::client::handler::
                             RocketClientStreamStateHandler>(
              rocket_client_stream_state_handler_tag)
          .build();

  connection->appAdapter->setPipeline(connection->pipeline.get());
  connection->transportHandler->setPipeline(*connection->pipeline);

  return connection;
}

// =============================================================================
// Benchmark Fixture
// =============================================================================

struct BenchmarkFixture {
  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};
  apache::thrift::fast_thrift::transport::TransportHandler::Ptr
      transportHandler;
  ThriftClientChannel::UniquePtr channel;
  PipelineImpl::Ptr pipeline;
  TestAllocator allocator;

  // Track pending requests for response injection
  std::vector<uint32_t> pendingStreamIds;

  void setup() {
    auto transport =
        folly::AsyncTransport::UniquePtr(new BenchAsyncTransport(&evb));
    testTransport = static_cast<BenchAsyncTransport*>(transport.get());

    transportHandler =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            std::move(transport));
    channel = ThriftClientChannel::newChannel(&evb);

    pipeline = PipelineBuilder<
                   ThriftClientChannel,
                   apache::thrift::fast_thrift::transport::TransportHandler,
                   TestAllocator>()
                   .setEventBase(&evb)
                   .setTail(transportHandler.get())
                   .setHead(channel.get())
                   .setAllocator(&allocator)
                   .addNextInbound<FrameLengthParserHandler>(
                       frame_length_parser_handler_tag)
                   .addNextOutbound<FrameLengthEncoderHandler>(
                       frame_length_encoder_handler_tag)
                   .addNextDuplex<apache::thrift::fast_thrift::rocket::client::
                                      handler::RocketClientFrameCodecHandler>(
                       rocket_client_frame_codec_handler_tag)
                   .addNextDuplex<apache::thrift::fast_thrift::rocket::client::
                                      handler::RocketClientSetupFrameHandler>(
                       rocket_client_setup_handler_tag,
                       []() {
                         return std::make_pair(
                             folly::IOBuf::copyBuffer("setup"),
                             std::unique_ptr<folly::IOBuf>());
                       })
                   .addNextDuplex<
                       apache::thrift::fast_thrift::rocket::client::handler::
                           RocketClientRequestResponseFrameHandler>(
                       rocket_client_request_response_frame_handler_tag)
                   .addNextInbound<apache::thrift::fast_thrift::rocket::client::
                                       handler::RocketClientErrorFrameHandler>(
                       rocket_client_error_frame_handler_tag)
                   .addNextDuplex<apache::thrift::fast_thrift::rocket::client::
                                      handler::RocketClientStreamStateHandler>(
                       rocket_client_stream_state_handler_tag)
                   .addNextDuplex<ThriftClientRocketInterfaceHandler>(
                       thrift_client_rocket_interface_handler_tag)
                   .addNextInbound<ThriftClientMetadataPushHandler>(
                       thrift_client_metadata_push_handler_tag)
                   .build();

    channel->setPipeline(pipeline.get());
    transportHandler->setPipeline(*pipeline);

    // Drive event loop and discard SETUP frame
    evb.loopOnce();
    testTransport->getWrittenData();
  }
};

// =============================================================================
// AppAdapter Benchmark Client — holds adapter as member (composition)
// =============================================================================

class BenchAppAdapterClient {
 public:
  BenchAppAdapterClient()
      : adapter_(ThriftClientAppAdapter::Ptr(new ThriftClientAppAdapter())) {}

  ThriftClientAppAdapter& adapter() { return *adapter_; }

  void sendBenchRequest(
      std::unique_ptr<folly::IOBuf> metadata,
      std::unique_ptr<folly::IOBuf> data) {
    ThriftRequestMessage msg{
        .payload = ThriftRequestPayload{
            .metadata = std::move(metadata),
            .data = std::move(data),
            .rpcKind = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
            .complete = true}};

    adapter_->write([](auto&&) noexcept {}, erase_and_box(std::move(msg)));
  }

 private:
  ThriftClientAppAdapter::Ptr adapter_;
};

// =============================================================================
// AppAdapter Benchmark Fixture (two-pipeline mode)
//
// Rocket pipeline: RocketClientAppAdapter → [rocket handlers] →
// TransportHandler Thrift pipeline: ThriftClientAppAdapter →
// ThriftClientTransportAdapter
// =============================================================================

struct AppAdapterBenchFixture {
  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};

  // Thrift pipeline (app adapter → transport adapter)
  BenchAppAdapterClient client;
  std::unique_ptr<client::ThriftClientTransportAdapter> transportAdapter;
  PipelineImpl::Ptr thriftPipeline;
  TestAllocator thriftAllocator;

  void setup() {
    testTransport = new BenchAsyncTransport(&evb);
    auto rocketConnection = createRocketConnection(testTransport, &evb);

    // Create transport adapter (takes ownership of rocket connection)
    transportAdapter = std::make_unique<client::ThriftClientTransportAdapter>(
        std::move(rocketConnection));

    // Build thrift pipeline
    thriftPipeline = PipelineBuilder<
                         ThriftClientAppAdapter,
                         client::ThriftClientTransportAdapter,
                         TestAllocator>()
                         .setEventBase(&evb)
                         .setHead(&client.adapter())
                         .setTail(transportAdapter.get())
                         .setAllocator(&thriftAllocator)
                         .addNextInbound<ThriftClientMetadataPushHandler>(
                             thrift_client_metadata_push_handler_tag)
                         .build();

    client.adapter().setPipeline(thriftPipeline.get());
    transportAdapter->setPipeline(thriftPipeline.get());

    // Connect transport and discard SETUP frame
    evb.loopOnce();
    testTransport->getWrittenData();
  }
};

// =============================================================================
// RocketClientChannel Benchmark Fixture
// =============================================================================

struct RocketBenchmarkFixture {
  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};
  apache::thrift::RocketClientChannel::Ptr channel;

  void setup() {
    auto transport =
        folly::AsyncTransport::UniquePtr(new BenchAsyncTransport(&evb));
    testTransport = static_cast<BenchAsyncTransport*>(transport.get());
    channel =
        apache::thrift::RocketClientChannel::newChannel(std::move(transport));

    // Drive event loop and discard SETUP frame
    evb.loopOnce();
    testTransport->getWrittenData();
  }
};

// =============================================================================
// RocketClientChannel Frame Helpers
// =============================================================================

std::unique_ptr<folly::IOBuf> createRocketPayloadResponse(
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  apache::thrift::rocket::Flags flags;
  flags.next(true).complete(true);
  apache::thrift::rocket::PayloadFrame frame(
      apache::thrift::rocket::StreamId{streamId},
      apache::thrift::rocket::Payload::makeFromMetadataAndData(
          std::move(metadata), std::move(data)),
      flags);
  return std::move(frame).serialize();
}

std::unique_ptr<folly::IOBuf> createRocketErrorFrame(
    uint32_t streamId,
    apache::thrift::rocket::ErrorCode errorCode,
    std::unique_ptr<folly::IOBuf> data) {
  apache::thrift::rocket::ErrorFrame frame(
      apache::thrift::rocket::StreamId{streamId},
      errorCode,
      apache::thrift::rocket::Payload::makeFromData(std::move(data)));
  return std::move(frame).serialize();
}

// Parse the stream ID from a Rocket frame (after the 3-byte length prefix)
// Stream ID is the first 4 bytes (big-endian) after the length prefix
uint32_t parseRocketRequestStreamId(std::unique_ptr<folly::IOBuf> data) {
  // Skip the 3-byte length prefix, then read 4-byte stream ID (big-endian)
  folly::io::Cursor cursor(data.get());
  cursor.skip(3); // Skip length prefix
  return cursor.readBE<uint32_t>();
}

// =============================================================================
// Request Path Benchmarks - Minimal Metadata
// =============================================================================

BENCHMARK(Rocket_Request_MinimalMetadata, iters) {
  folly::BenchmarkSuspender suspender;
  RocketBenchmarkFixture fixture;
  fixture.setup();

  // Preallocate everything
  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto payloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback::Ptr> callbacks;

  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    requests.emplace_back(payloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  // Preallocate minimal metadata
  auto metadataTemplate = createMinimalRequestMetadata();

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    auto metadata = metadataTemplate;
    fixture.channel->sendRequestResponse(
        rpcOptions,
        apache::thrift::MethodMetadata(*metadata.name()),
        std::move(requests[i]),
        header,
        std::move(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();
  }
}

BENCHMARK_RELATIVE(FastThriftWithChannel_Request_MinimalMetadata, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  // Preallocate everything
  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto payloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback::Ptr> callbacks;

  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    requests.emplace_back(payloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  // Preallocate minimal metadata
  auto metadataTemplate = createMinimalRequestMetadata();

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    auto metadata = metadataTemplate;
    fixture.channel->sendRequestResponse(
        rpcOptions,
        apache::thrift::MethodMetadata(*metadata.name()),
        std::move(requests[i]),
        header,
        std::move(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();
  }
}

BENCHMARK_RELATIVE(FastThriftWithAppAdapter_Request_MinimalMetadata, iters) {
  folly::BenchmarkSuspender suspender;
  AppAdapterBenchFixture fixture;
  fixture.setup();

  auto payloadTemplate = makePayloadData(kPayloadSize);
  auto metadataTemplate = makeSerializedRequestMetadata(
      apache::thrift::RpcOptions(),
      apache::thrift::ManagedStringView("benchMethod"),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::BINARY);

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    fixture.client.sendBenchRequest(
        metadataTemplate->clone(), payloadTemplate->clone());
    fixture.evb.loopOnce();
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Request Path Benchmarks - With Headers
// =============================================================================

BENCHMARK(Rocket_Request_WithHeaders, iters) {
  folly::BenchmarkSuspender suspender;
  RocketBenchmarkFixture fixture;
  fixture.setup();

  // Preallocate everything
  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto payloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback::Ptr> callbacks;

  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    requests.emplace_back(payloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  // Preallocate metadata with headers
  auto metadataTemplate = createRequestMetadataWithHeaders();

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    auto metadata = metadataTemplate;
    fixture.channel->sendRequestResponse(
        rpcOptions,
        apache::thrift::MethodMetadata(*metadata.name()),
        std::move(requests[i]),
        header,
        std::move(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();
  }
}

BENCHMARK_RELATIVE(FastThriftWithChannel_Request_WithHeaders, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  // Preallocate everything
  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto payloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback::Ptr> callbacks;

  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    requests.emplace_back(payloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  // Preallocate metadata with headers
  auto metadataTemplate = createRequestMetadataWithHeaders();

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    auto metadata = metadataTemplate;
    fixture.channel->sendRequestResponse(
        rpcOptions,
        apache::thrift::MethodMetadata(*metadata.name()),
        std::move(requests[i]),
        header,
        std::move(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();
  }
}

BENCHMARK_RELATIVE(FastThriftWithAppAdapter_Request_WithHeaders, iters) {
  folly::BenchmarkSuspender suspender;
  AppAdapterBenchFixture fixture;
  fixture.setup();

  auto payloadTemplate = makePayloadData(kPayloadSize);
  auto metadataTempl = createRequestMetadataWithHeaders();
  auto metadataTemplate = makeSerializedRequestMetadata(
      apache::thrift::RpcOptions(),
      apache::thrift::ManagedStringView(*metadataTempl.name()),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::BINARY);

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    fixture.client.sendBenchRequest(
        metadataTemplate->clone(), payloadTemplate->clone());
    fixture.evb.loopOnce();
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Response Path Benchmarks - Success
// =============================================================================

BENCHMARK(Rocket_Response_Success, iters) {
  folly::BenchmarkSuspender suspender;
  RocketBenchmarkFixture fixture;
  fixture.setup();

  // Preallocate everything for requests
  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto requestPayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::MethodMetadata> methodMetadatas;
  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback*> callbacks;

  methodMetadatas.reserve(iters);
  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    methodMetadatas.emplace_back("benchMethod");
    requests.emplace_back(requestPayloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  // Send all requests (not timed) and collect stream IDs
  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.channel->sendRequestResponse(
        rpcOptions,
        std::move(methodMetadatas[i]),
        std::move(requests[i]),
        header,
        BenchRequestCallback::Ptr(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();

    auto written = fixture.testTransport->getWrittenData();
    streamIds.push_back(parseRocketRequestStreamId(std::move(written)));
  }

  // Preallocate success response frames
  auto responseMetadata =
      serializeResponseMetadata(createSuccessResponseMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> responseFrames;
  responseFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    responseFrames.emplace_back(createRocketPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone()));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.testTransport->injectReadData(std::move(responseFrames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftWithChannel_Response_Success, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  // Preallocate everything for requests
  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto requestPayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::MethodMetadata> methodMetadatas;
  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback*> callbacks;

  methodMetadatas.reserve(iters);
  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    methodMetadatas.emplace_back("benchMethod");
    requests.emplace_back(requestPayloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  // Send all requests (not timed) and collect stream IDs
  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.channel->sendRequestResponse(
        rpcOptions,
        std::move(methodMetadatas[i]),
        std::move(requests[i]),
        header,
        BenchRequestCallback::Ptr(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();

    auto written = fixture.testTransport->getWrittenData();
    auto parsed = parseWrittenFrame(std::move(written));
    streamIds.push_back(parsed.streamId());
  }

  // Preallocate success response frames
  auto responseMetadata =
      serializeResponseMetadata(createSuccessResponseMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> responseFrames;
  responseFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    responseFrames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone())));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.testTransport->injectReadData(std::move(responseFrames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftWithAppAdapter_Response_Success, iters) {
  folly::BenchmarkSuspender suspender;
  AppAdapterBenchFixture fixture;
  fixture.setup();

  auto requestPayloadTemplate = makePayloadData(kPayloadSize);
  auto requestMetadataTemplate = makeSerializedRequestMetadata(
      apache::thrift::RpcOptions(),
      apache::thrift::ManagedStringView("benchMethod"),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::BINARY);

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.client.sendBenchRequest(
        requestMetadataTemplate->clone(), requestPayloadTemplate->clone());
    fixture.evb.loopOnce();
    auto written = fixture.testTransport->getWrittenData();
    auto parsed = parseWrittenFrame(std::move(written));
    streamIds.push_back(parsed.streamId());
  }

  auto responseMetadata =
      serializeResponseMetadata(createSuccessResponseMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> responseFrames;
  responseFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    responseFrames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone())));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.testTransport->injectReadData(std::move(responseFrames[i]));
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Response Path Benchmarks - Undeclared Exception
// =============================================================================

BENCHMARK(Rocket_Response_UndeclaredException, iters) {
  folly::BenchmarkSuspender suspender;
  RocketBenchmarkFixture fixture;
  fixture.setup();

  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto requestPayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::MethodMetadata> methodMetadatas;
  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback*> callbacks;

  methodMetadatas.reserve(iters);
  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    methodMetadatas.emplace_back("benchMethod");
    requests.emplace_back(requestPayloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.channel->sendRequestResponse(
        rpcOptions,
        std::move(methodMetadatas[i]),
        std::move(requests[i]),
        header,
        BenchRequestCallback::Ptr(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();

    auto written = fixture.testTransport->getWrittenData();
    streamIds.push_back(parseRocketRequestStreamId(std::move(written)));
  }

  // Preallocate undeclared exception response frames
  auto responseMetadata =
      serializeResponseMetadata(createUndeclaredExceptionMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> responseFrames;
  responseFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    responseFrames.emplace_back(createRocketPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone()));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.testTransport->injectReadData(std::move(responseFrames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftWithChannel_Response_UndeclaredException, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto requestPayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::MethodMetadata> methodMetadatas;
  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback*> callbacks;

  methodMetadatas.reserve(iters);
  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    methodMetadatas.emplace_back("benchMethod");
    requests.emplace_back(requestPayloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.channel->sendRequestResponse(
        rpcOptions,
        std::move(methodMetadatas[i]),
        std::move(requests[i]),
        header,
        BenchRequestCallback::Ptr(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();

    auto written = fixture.testTransport->getWrittenData();
    auto parsed = parseWrittenFrame(std::move(written));
    streamIds.push_back(parsed.streamId());
  }

  // Preallocate undeclared exception response frames
  auto responseMetadata =
      serializeResponseMetadata(createUndeclaredExceptionMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> responseFrames;
  responseFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    responseFrames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone())));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.testTransport->injectReadData(std::move(responseFrames[i]));
  }
}

BENCHMARK_RELATIVE(
    FastThriftWithAppAdapter_Response_UndeclaredException, iters) {
  folly::BenchmarkSuspender suspender;
  AppAdapterBenchFixture fixture;
  fixture.setup();

  auto requestPayloadTemplate = makePayloadData(kPayloadSize);
  auto requestMetadataTemplate = makeSerializedRequestMetadata(
      apache::thrift::RpcOptions(),
      apache::thrift::ManagedStringView("benchMethod"),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::BINARY);

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.client.sendBenchRequest(
        requestMetadataTemplate->clone(), requestPayloadTemplate->clone());
    fixture.evb.loopOnce();
    auto written = fixture.testTransport->getWrittenData();
    auto parsed = parseWrittenFrame(std::move(written));
    streamIds.push_back(parsed.streamId());
  }

  auto responseMetadata =
      serializeResponseMetadata(createUndeclaredExceptionMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> responseFrames;
  responseFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    responseFrames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone())));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.testTransport->injectReadData(std::move(responseFrames[i]));
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Response Path Benchmarks - Declared Exception
// =============================================================================

BENCHMARK(Rocket_Response_DeclaredException, iters) {
  folly::BenchmarkSuspender suspender;
  RocketBenchmarkFixture fixture;
  fixture.setup();

  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto requestPayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::MethodMetadata> methodMetadatas;
  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback*> callbacks;

  methodMetadatas.reserve(iters);
  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    methodMetadatas.emplace_back("benchMethod");
    requests.emplace_back(requestPayloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.channel->sendRequestResponse(
        rpcOptions,
        std::move(methodMetadatas[i]),
        std::move(requests[i]),
        header,
        BenchRequestCallback::Ptr(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();

    auto written = fixture.testTransport->getWrittenData();
    streamIds.push_back(parseRocketRequestStreamId(std::move(written)));
  }

  // Preallocate declared exception response frames
  auto responseMetadata =
      serializeResponseMetadata(createDeclaredExceptionMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> responseFrames;
  responseFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    responseFrames.emplace_back(createRocketPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone()));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.testTransport->injectReadData(std::move(responseFrames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftWithChannel_Response_DeclaredException, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto requestPayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::MethodMetadata> methodMetadatas;
  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback*> callbacks;

  methodMetadatas.reserve(iters);
  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    methodMetadatas.emplace_back("benchMethod");
    requests.emplace_back(requestPayloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.channel->sendRequestResponse(
        rpcOptions,
        std::move(methodMetadatas[i]),
        std::move(requests[i]),
        header,
        BenchRequestCallback::Ptr(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();

    auto written = fixture.testTransport->getWrittenData();
    auto parsed = parseWrittenFrame(std::move(written));
    streamIds.push_back(parsed.streamId());
  }

  // Preallocate declared exception response frames
  auto responseMetadata =
      serializeResponseMetadata(createDeclaredExceptionMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> responseFrames;
  responseFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    responseFrames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone())));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.testTransport->injectReadData(std::move(responseFrames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftWithAppAdapter_Response_DeclaredException, iters) {
  folly::BenchmarkSuspender suspender;
  AppAdapterBenchFixture fixture;
  fixture.setup();

  auto requestPayloadTemplate = makePayloadData(kPayloadSize);
  auto requestMetadataTemplate = makeSerializedRequestMetadata(
      apache::thrift::RpcOptions(),
      apache::thrift::ManagedStringView("benchMethod"),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::BINARY);

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.client.sendBenchRequest(
        requestMetadataTemplate->clone(), requestPayloadTemplate->clone());
    fixture.evb.loopOnce();
    auto written = fixture.testTransport->getWrittenData();
    auto parsed = parseWrittenFrame(std::move(written));
    streamIds.push_back(parsed.streamId());
  }

  auto responseMetadata =
      serializeResponseMetadata(createDeclaredExceptionMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> responseFrames;
  responseFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    responseFrames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone())));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.testTransport->injectReadData(std::move(responseFrames[i]));
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Response Path Benchmarks - Error Frame
// =============================================================================

BENCHMARK(Rocket_Response_ErrorFrame, iters) {
  folly::BenchmarkSuspender suspender;
  RocketBenchmarkFixture fixture;
  fixture.setup();

  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto requestPayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::MethodMetadata> methodMetadatas;
  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback*> callbacks;

  methodMetadatas.reserve(iters);
  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    methodMetadatas.emplace_back("benchMethod");
    requests.emplace_back(requestPayloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.channel->sendRequestResponse(
        rpcOptions,
        std::move(methodMetadatas[i]),
        std::move(requests[i]),
        header,
        BenchRequestCallback::Ptr(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();

    auto written = fixture.testTransport->getWrittenData();
    streamIds.push_back(parseRocketRequestStreamId(std::move(written)));
  }

  // Preallocate ERROR frames (REJECTED with ResponseRpcError)
  auto errorPayloadTemplate = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::OVERLOAD, "Server overloaded");

  std::vector<std::unique_ptr<folly::IOBuf>> errorFrames;
  errorFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    errorFrames.emplace_back(createRocketErrorFrame(
        streamIds[i],
        apache::thrift::rocket::ErrorCode::REJECTED,
        errorPayloadTemplate->clone()));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.testTransport->injectReadData(std::move(errorFrames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftWithChannel_Response_ErrorFrame, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto requestPayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::MethodMetadata> methodMetadatas;
  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback*> callbacks;

  methodMetadatas.reserve(iters);
  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    methodMetadatas.emplace_back("benchMethod");
    requests.emplace_back(requestPayloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.channel->sendRequestResponse(
        rpcOptions,
        std::move(methodMetadatas[i]),
        std::move(requests[i]),
        header,
        BenchRequestCallback::Ptr(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();

    auto written = fixture.testTransport->getWrittenData();
    auto parsed = parseWrittenFrame(std::move(written));
    streamIds.push_back(parsed.streamId());
  }

  // Preallocate ERROR frames (REJECTED with ResponseRpcError)
  auto errorPayloadTemplate = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::OVERLOAD, "Server overloaded");

  std::vector<std::unique_ptr<folly::IOBuf>> errorFrames;
  errorFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    errorFrames.emplace_back(prependLengthPrefix(createErrorFrame(
        streamIds[i], kErrorCodeRejected, errorPayloadTemplate->clone())));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.testTransport->injectReadData(std::move(errorFrames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftWithAppAdapter_Response_ErrorFrame, iters) {
  folly::BenchmarkSuspender suspender;
  AppAdapterBenchFixture fixture;
  fixture.setup();

  auto requestPayloadTemplate = makePayloadData(kPayloadSize);
  auto requestMetadataTemplate = makeSerializedRequestMetadata(
      apache::thrift::RpcOptions(),
      apache::thrift::ManagedStringView("benchMethod"),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::BINARY);

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.client.sendBenchRequest(
        requestMetadataTemplate->clone(), requestPayloadTemplate->clone());
    fixture.evb.loopOnce();
    auto written = fixture.testTransport->getWrittenData();
    auto parsed = parseWrittenFrame(std::move(written));
    streamIds.push_back(parsed.streamId());
  }

  auto errorPayloadTemplate = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::OVERLOAD, "Server overloaded");

  std::vector<std::unique_ptr<folly::IOBuf>> errorFrames;
  errorFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    errorFrames.emplace_back(prependLengthPrefix(createErrorFrame(
        streamIds[i], kErrorCodeRejected, errorPayloadTemplate->clone())));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.testTransport->injectReadData(std::move(errorFrames[i]));
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Advanced Benchmarks - Fragmented Response
// =============================================================================

// Fragmented response - tests FrameLengthParserHandler reassembly
// Response arrives in multiple small chunks (simulating TCP fragmentation)
BENCHMARK(Rocket_Response_Fragmented, iters) {
  folly::BenchmarkSuspender suspender;
  RocketBenchmarkFixture fixture;
  fixture.setup();

  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto requestPayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::MethodMetadata> methodMetadatas;
  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback*> callbacks;

  methodMetadatas.reserve(iters);
  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    methodMetadatas.emplace_back("benchMethod");
    requests.emplace_back(requestPayloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.channel->sendRequestResponse(
        rpcOptions,
        std::move(methodMetadatas[i]),
        std::move(requests[i]),
        header,
        BenchRequestCallback::Ptr(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();

    auto written = fixture.testTransport->getWrittenData();
    streamIds.push_back(parseRocketRequestStreamId(std::move(written)));
  }

  // Preallocate response frames, then fragment them
  auto responseMetadata =
      serializeResponseMetadata(createSuccessResponseMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  // Create fragmented responses (split each frame into 64-byte chunks)
  constexpr size_t kFragmentSize = 64;
  std::vector<std::vector<std::unique_ptr<folly::IOBuf>>> fragmentedFrames;
  fragmentedFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    auto frame = createRocketPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone());

    // Fragment the frame into small chunks
    std::vector<std::unique_ptr<folly::IOBuf>> fragments;
    folly::io::Cursor cursor(frame.get());
    size_t remaining = frame->computeChainDataLength();

    while (remaining > 0) {
      size_t chunkSize = std::min(kFragmentSize, remaining);
      auto chunk = folly::IOBuf::create(chunkSize);
      cursor.pull(chunk->writableTail(), chunkSize);
      chunk->append(chunkSize);
      fragments.push_back(std::move(chunk));
      remaining -= chunkSize;
    }

    fragmentedFrames.push_back(std::move(fragments));
  }

  suspender.dismiss();

  // Inject fragments - tests frame reassembly
  for (size_t i = 0; i < iters; ++i) {
    for (auto& fragment : fragmentedFrames[i]) {
      fixture.testTransport->injectReadData(std::move(fragment));
    }
  }
}

BENCHMARK_RELATIVE(FastThriftWithChannel_Response_Fragmented, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto requestPayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::MethodMetadata> methodMetadatas;
  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback*> callbacks;

  methodMetadatas.reserve(iters);
  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    methodMetadatas.emplace_back("benchMethod");
    requests.emplace_back(requestPayloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.channel->sendRequestResponse(
        rpcOptions,
        std::move(methodMetadatas[i]),
        std::move(requests[i]),
        header,
        BenchRequestCallback::Ptr(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();

    auto written = fixture.testTransport->getWrittenData();
    auto parsed = parseWrittenFrame(std::move(written));
    streamIds.push_back(parsed.streamId());
  }

  // Preallocate response frames, then fragment them
  auto responseMetadata =
      serializeResponseMetadata(createSuccessResponseMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  // Create fragmented responses (split each frame into 64-byte chunks)
  constexpr size_t kFragmentSize = 64;
  std::vector<std::vector<std::unique_ptr<folly::IOBuf>>> fragmentedFrames;
  fragmentedFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    auto frame = prependLengthPrefix(createPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone()));

    // Fragment the frame (with length prefix) into small chunks
    std::vector<std::unique_ptr<folly::IOBuf>> fragments;
    folly::io::Cursor cursor(frame.get());
    size_t remaining = frame->computeChainDataLength();

    while (remaining > 0) {
      size_t chunkSize = std::min(kFragmentSize, remaining);
      auto chunk = folly::IOBuf::create(chunkSize);
      cursor.pull(chunk->writableTail(), chunkSize);
      chunk->append(chunkSize);
      fragments.push_back(std::move(chunk));
      remaining -= chunkSize;
    }

    fragmentedFrames.push_back(std::move(fragments));
  }

  suspender.dismiss();

  // Inject fragments - tests FrameLengthParserHandler reassembly
  for (size_t i = 0; i < iters; ++i) {
    for (auto& fragment : fragmentedFrames[i]) {
      fixture.testTransport->injectReadData(std::move(fragment));
    }
  }
}

BENCHMARK_RELATIVE(FastThriftWithAppAdapter_Response_Fragmented, iters) {
  folly::BenchmarkSuspender suspender;
  AppAdapterBenchFixture fixture;
  fixture.setup();

  auto requestPayloadTemplate = makePayloadData(kPayloadSize);
  auto requestMetadataTemplate = makeSerializedRequestMetadata(
      apache::thrift::RpcOptions(),
      apache::thrift::ManagedStringView("benchMethod"),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::BINARY);

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    fixture.client.sendBenchRequest(
        requestMetadataTemplate->clone(), requestPayloadTemplate->clone());
    fixture.evb.loopOnce();
    auto written = fixture.testTransport->getWrittenData();
    auto parsed = parseWrittenFrame(std::move(written));
    streamIds.push_back(parsed.streamId());
  }

  auto responseMetadata =
      serializeResponseMetadata(createSuccessResponseMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  constexpr size_t kFragmentSize = 64;
  std::vector<std::vector<std::unique_ptr<folly::IOBuf>>> fragmentedFrames;
  fragmentedFrames.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    auto frame = prependLengthPrefix(createPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone()));

    std::vector<std::unique_ptr<folly::IOBuf>> fragments;
    folly::io::Cursor cursor(frame.get());
    size_t remaining = frame->computeChainDataLength();

    while (remaining > 0) {
      size_t chunkSize = std::min(kFragmentSize, remaining);
      auto chunk = folly::IOBuf::create(chunkSize);
      cursor.pull(chunk->writableTail(), chunkSize);
      chunk->append(chunkSize);
      fragments.push_back(std::move(chunk));
      remaining -= chunkSize;
    }

    fragmentedFrames.push_back(std::move(fragments));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    for (auto& fragment : fragmentedFrames[i]) {
      fixture.testTransport->injectReadData(std::move(fragment));
    }
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Advanced Benchmarks - High Concurrency
// =============================================================================

// High concurrency - many pending requests before receiving responses
// Tests F14Map/Set performance with larger working sets
BENCHMARK(Rocket_Response_HighConcurrency, iters) {
  folly::BenchmarkSuspender suspender;
  RocketBenchmarkFixture fixture;
  fixture.setup();

  // Use a fixed number of concurrent pending requests
  constexpr size_t kConcurrentRequests = 1000;
  size_t actualIters =
      std::max(static_cast<size_t>(iters), kConcurrentRequests);

  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto requestPayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::MethodMetadata> methodMetadatas;
  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback*> callbacks;

  methodMetadatas.reserve(actualIters);
  requests.reserve(actualIters);
  callbacks.reserve(actualIters);

  for (size_t i = 0; i < actualIters; ++i) {
    methodMetadatas.emplace_back("benchMethod");
    requests.emplace_back(requestPayloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  // Send ALL requests first to build up pending state
  std::vector<uint32_t> streamIds;
  streamIds.reserve(actualIters);

  for (size_t i = 0; i < actualIters; ++i) {
    fixture.channel->sendRequestResponse(
        rpcOptions,
        std::move(methodMetadatas[i]),
        std::move(requests[i]),
        header,
        BenchRequestCallback::Ptr(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();

    auto written = fixture.testTransport->getWrittenData();
    streamIds.push_back(parseRocketRequestStreamId(std::move(written)));
  }

  // Preallocate response frames
  auto responseMetadata =
      serializeResponseMetadata(createSuccessResponseMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> responseFrames;
  responseFrames.reserve(actualIters);

  for (size_t i = 0; i < actualIters; ++i) {
    responseFrames.emplace_back(createRocketPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone()));
  }

  suspender.dismiss();

  // Now receive all responses - lookups happen with large working set
  for (size_t i = 0; i < actualIters; ++i) {
    fixture.testTransport->injectReadData(std::move(responseFrames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftWithChannel_Response_HighConcurrency, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  // Use a fixed number of concurrent pending requests
  constexpr size_t kConcurrentRequests = 1000;
  size_t actualIters =
      std::max(static_cast<size_t>(iters), kConcurrentRequests);

  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();
  auto requestPayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<apache::thrift::MethodMetadata> methodMetadatas;
  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback*> callbacks;

  methodMetadatas.reserve(actualIters);
  requests.reserve(actualIters);
  callbacks.reserve(actualIters);

  for (size_t i = 0; i < actualIters; ++i) {
    methodMetadatas.emplace_back("benchMethod");
    requests.emplace_back(requestPayloadTemplate->clone());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  // Send ALL requests first to build up pending state
  std::vector<uint32_t> streamIds;
  streamIds.reserve(actualIters);

  for (size_t i = 0; i < actualIters; ++i) {
    fixture.channel->sendRequestResponse(
        rpcOptions,
        std::move(methodMetadatas[i]),
        std::move(requests[i]),
        header,
        BenchRequestCallback::Ptr(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();

    auto written = fixture.testTransport->getWrittenData();
    auto parsed = parseWrittenFrame(std::move(written));
    streamIds.push_back(parsed.streamId());
  }

  // Preallocate response frames
  auto responseMetadata =
      serializeResponseMetadata(createSuccessResponseMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> responseFrames;
  responseFrames.reserve(actualIters);

  for (size_t i = 0; i < actualIters; ++i) {
    responseFrames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone())));
  }

  suspender.dismiss();

  // Now receive all responses - F14 lookups happen with large working set
  for (size_t i = 0; i < actualIters; ++i) {
    fixture.testTransport->injectReadData(std::move(responseFrames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftWithAppAdapter_Response_HighConcurrency, iters) {
  folly::BenchmarkSuspender suspender;
  AppAdapterBenchFixture fixture;
  fixture.setup();

  constexpr size_t kConcurrentRequests = 1000;
  size_t actualIters =
      std::max(static_cast<size_t>(iters), kConcurrentRequests);

  auto requestPayloadTemplate = makePayloadData(kPayloadSize);
  auto requestMetadataTemplate = makeSerializedRequestMetadata(
      apache::thrift::RpcOptions(),
      apache::thrift::ManagedStringView("benchMethod"),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::BINARY);

  std::vector<uint32_t> streamIds;
  streamIds.reserve(actualIters);

  for (size_t i = 0; i < actualIters; ++i) {
    fixture.client.sendBenchRequest(
        requestMetadataTemplate->clone(), requestPayloadTemplate->clone());
    fixture.evb.loopOnce();
    auto written = fixture.testTransport->getWrittenData();
    auto parsed = parseWrittenFrame(std::move(written));
    streamIds.push_back(parsed.streamId());
  }

  auto responseMetadata =
      serializeResponseMetadata(createSuccessResponseMetadata());
  auto responsePayloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> responseFrames;
  responseFrames.reserve(actualIters);

  for (size_t i = 0; i < actualIters; ++i) {
    responseFrames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i],
        responseMetadata->clone(),
        responsePayloadTemplate->clone())));
  }

  suspender.dismiss();

  for (size_t i = 0; i < actualIters; ++i) {
    fixture.testTransport->injectReadData(std::move(responseFrames[i]));
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Advanced Benchmarks - Chained Payload
// =============================================================================

// Chained IOBuf payload - payload is a chain of small IOBufs
// Tests how handlers traverse IOBuf chains
BENCHMARK(Rocket_Request_ChainedPayload, iters) {
  folly::BenchmarkSuspender suspender;
  RocketBenchmarkFixture fixture;
  fixture.setup();

  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();

  // Create chained IOBuf payloads (many small buffers chained together)
  constexpr size_t kChunkSize = 64;
  constexpr size_t kNumChunks = kPayloadSize / kChunkSize;

  auto createChainedPayload = []() {
    std::unique_ptr<folly::IOBuf> chain;
    for (size_t j = 0; j < kNumChunks; ++j) {
      auto chunk = folly::IOBuf::copyBuffer(std::string(kChunkSize, 'x'));
      if (chain) {
        chain->prependChain(std::move(chunk));
      } else {
        chain = std::move(chunk);
      }
    }
    return chain;
  };

  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback::Ptr> callbacks;

  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    requests.emplace_back(createChainedPayload());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.channel->sendRequestResponse(
        rpcOptions,
        apache::thrift::MethodMetadata("benchMethod"),
        std::move(requests[i]),
        header,
        std::move(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();
  }
}

BENCHMARK_RELATIVE(FastThriftWithChannel_Request_ChainedPayload, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  apache::thrift::RpcOptions rpcOptions;
  auto header = createHeader();

  // Create chained IOBuf payloads (many small buffers chained together)
  constexpr size_t kChunkSize = 64;
  constexpr size_t kNumChunks = kPayloadSize / kChunkSize;

  auto createChainedPayload = []() {
    std::unique_ptr<folly::IOBuf> chain;
    for (size_t j = 0; j < kNumChunks; ++j) {
      auto chunk = folly::IOBuf::copyBuffer(std::string(kChunkSize, 'x'));
      if (chain) {
        chain->prependChain(std::move(chunk));
      } else {
        chain = std::move(chunk);
      }
    }
    return chain;
  };

  std::vector<apache::thrift::SerializedRequest> requests;
  std::vector<BenchRequestCallback::Ptr> callbacks;

  requests.reserve(iters);
  callbacks.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    requests.emplace_back(createChainedPayload());
    callbacks.emplace_back(new BenchRequestCallback());
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.channel->sendRequestResponse(
        rpcOptions,
        apache::thrift::MethodMetadata("benchMethod"),
        std::move(requests[i]),
        header,
        std::move(callbacks[i]),
        nullptr);

    fixture.evb.loopOnce();
  }
}

BENCHMARK_RELATIVE(FastThriftWithAppAdapter_Request_ChainedPayload, iters) {
  folly::BenchmarkSuspender suspender;
  AppAdapterBenchFixture fixture;
  fixture.setup();

  auto metadataTemplate = makeSerializedRequestMetadata(
      apache::thrift::RpcOptions(),
      apache::thrift::ManagedStringView("benchMethod"),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::BINARY);

  constexpr size_t kChunkSize = 64;
  constexpr size_t kNumChunks = kPayloadSize / kChunkSize;

  auto createChainedPayload = []() {
    std::unique_ptr<folly::IOBuf> chain;
    for (size_t j = 0; j < kNumChunks; ++j) {
      auto chunk = folly::IOBuf::copyBuffer(std::string(kChunkSize, 'x'));
      if (chain) {
        chain->prependChain(std::move(chunk));
      } else {
        chain = std::move(chunk);
      }
    }
    return chain;
  };

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.client.sendBenchRequest(
        metadataTemplate->clone(), createChainedPayload());
    fixture.evb.loopOnce();
  }
}

} // namespace

// =============================================================================
// Main
// =============================================================================

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
