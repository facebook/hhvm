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
 * FastThrift Client Integration Microbenchmarks
 *
 * Three-way comparison:
 *   BENCHMARK(Thrift_*)              — Client + RocketClientChannel (baseline)
 *   BENCHMARK_RELATIVE(FastThriftChannel_*) — Client + ThriftClientChannel
 *   BENCHMARK_RELATIVE(FastThriftClient_*)  — FastClient (generated)
 *
 * All use BenchAsyncTransport (mock socket) for isolation from kernel/network.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>

#include <folly/coro/BlockingWait.h>
#include <folly/coro/Task.h>
#include <folly/futures/Promise.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/THeader.h>
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
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/bench/if/gen-cpp2/BenchmarkFastServiceAsyncClient.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/bench/if/gen-cpp2/BenchmarkServiceAsyncClient.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/adapter/ThriftClientTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientMetadataPushHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/bench/BenchAsyncTransport.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::channel_pipeline::test;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::thrift;
using namespace apache::thrift::fast_thrift::thrift::bench;
using namespace apache::thrift::fast_thrift::thrift::client::handler;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read::handler;
using namespace apache::thrift::fast_thrift::frame::write::handler;
using namespace apache::thrift::fast_thrift::frame::read;
using namespace apache::thrift::fast_thrift::frame::write;
using namespace apache::thrift::fast_thrift::transport::bench;

namespace {

constexpr size_t kPayloadSize = 4'096;

using ThriftClientType = apache::thrift::Client<BenchmarkService>;
using FastClientType =
    apache::thrift::FastClient<BenchmarkFastService, ThriftClientAppAdapter>;

HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_client_frame_codec_handler);
HANDLER_TAG(rocket_client_setup_handler);
HANDLER_TAG(rocket_client_request_response_handler);
HANDLER_TAG(rocket_client_error_frame_handler);
HANDLER_TAG(rocket_client_stream_state_handler);
HANDLER_TAG(thrift_client_metadata_push_handler);

// === Helpers ===

std::unique_ptr<folly::IOBuf> makePayloadData(size_t size) {
  return folly::IOBuf::copyBuffer(std::string(size, 'x'));
}

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

ParsedFrame parseWrittenFrame(std::unique_ptr<folly::IOBuf> data) {
  folly::IOBufQueue queue;
  queue.append(std::move(data));
  queue.trimStart(kMetadataLengthSize);
  return parseFrame(queue.move());
}

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

std::unique_ptr<folly::IOBuf> serializeVoidResult() {
  apache::thrift::CompactProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  writer.writeStructBegin("");
  writer.writeFieldStop();
  writer.writeStructEnd();
  return queue.move();
}

std::unique_ptr<folly::IOBuf> serializeStringResult(const std::string& value) {
  apache::thrift::CompactProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  writer.writeStructBegin("");
  writer.writeFieldBegin(
      "success", apache::thrift::protocol::TType::T_STRING, 0);
  writer.writeString(value);
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  return queue.move();
}

std::unique_ptr<rocket::client::RocketClientConnection> createRocketConnection(
    BenchAsyncTransport* benchTransport, folly::EventBase* evb) {
  auto transport = folly::AsyncTransport::UniquePtr(benchTransport);
  auto connection = std::make_unique<rocket::client::RocketClientConnection>();
  connection->transportHandler =
      apache::thrift::fast_thrift::transport::TransportHandler::create(
          std::move(transport));
  connection->pipeline =
      PipelineBuilder<
          apache::thrift::fast_thrift::transport::TransportHandler,
          apache::thrift::fast_thrift::rocket::client::RocketClientAppAdapter,
          channel_pipeline::SimpleBufferAllocator>()
          .setEventBase(evb)
          .setHead(connection->transportHandler.get())
          .setTail(connection->appAdapter.get())
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
          .addNextInbound<apache::thrift::fast_thrift::rocket::client::handler::
                              RocketClientErrorFrameHandler>(
              rocket_client_error_frame_handler_tag)
          .addNextDuplex<apache::thrift::fast_thrift::rocket::client::handler::
                             RocketClientStreamStateHandler>(
              rocket_client_stream_state_handler_tag)
          .addNextInbound<apache::thrift::fast_thrift::rocket::client::handler::
                              RocketClientRequestResponseHandler>(
              rocket_client_request_response_handler_tag)
          .build();
  connection->appAdapter->setPipeline(connection->pipeline.get());
  connection->transportHandler->setPipeline(*connection->pipeline);
  return connection;
}

// ===================== Callbacks =====================
class NoopRequestCallback : public apache::thrift::RequestCallback {
 public:
  void requestSent() override {}
  void requestError(apache::thrift::ClientReceiveState&&) override {}
  void replyReceived(apache::thrift::ClientReceiveState&&) override {}
};

// === Fixture 1: Thrift (Client + RocketClientChannel) — baseline ===

struct ThriftBenchFixture {
  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};
  std::unique_ptr<ThriftClientType> thriftClient;

  void setup() {
    testTransport = new BenchAsyncTransport(&evb);
    auto transport = folly::AsyncTransport::UniquePtr(testTransport);
    auto channel =
        apache::thrift::RocketClientChannel::newChannel(std::move(transport));
    thriftClient = std::make_unique<ThriftClientType>(std::move(channel));

    // Discard SETUP frame
    thriftClient->ping(std::make_unique<NoopRequestCallback>());
    evb.loopOnce();
    testTransport->getWrittenData();
  }
};

// === Fixture 2: FastThriftChannel (Client + ThriftClientChannel) ===

struct FastThriftChannelBenchFixture {
  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};
  SimpleBufferAllocator thriftAllocator;
  std::unique_ptr<client::ThriftClientTransportAdapter> transportAdapter;
  std::unique_ptr<ThriftClientType> thriftClient;
  PipelineImpl::Ptr thriftPipeline;

  void setup() {
    testTransport = new BenchAsyncTransport(&evb);
    auto rocketConnection = createRocketConnection(testTransport, &evb);
    transportAdapter = std::make_unique<client::ThriftClientTransportAdapter>(
        std::move(rocketConnection));
    auto channel = ThriftClientChannel::newChannel(&evb);
    auto* channelPtr = channel.get();
    thriftPipeline = PipelineBuilder<
                         client::ThriftClientTransportAdapter,
                         ThriftClientChannel,
                         SimpleBufferAllocator>()
                         .setEventBase(&evb)
                         .setHead(transportAdapter.get())
                         .setTail(channelPtr)
                         .setAllocator(&thriftAllocator)
                         .addNextInbound<ThriftClientMetadataPushHandler>(
                             thrift_client_metadata_push_handler_tag)
                         .build();
    channelPtr->setPipeline(thriftPipeline.get());
    transportAdapter->setPipeline(thriftPipeline.get());
    thriftClient = std::make_unique<ThriftClientType>(std::move(channel));

    // Discard SETUP frame
    thriftClient->ping(std::make_unique<NoopRequestCallback>());
    evb.loopOnce();
    testTransport->getWrittenData();
  }
};

struct FastClientBenchFixture {
  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};
  SimpleBufferAllocator thriftAllocator;
  std::unique_ptr<client::ThriftClientTransportAdapter> transportAdapter;
  PipelineImpl::Ptr thriftPipeline;
  std::unique_ptr<FastClientType> fastClient;

  void setup() {
    testTransport = new BenchAsyncTransport(&evb);
    auto rocketConnection = createRocketConnection(testTransport, &evb);

    transportAdapter = std::make_unique<client::ThriftClientTransportAdapter>(
        std::move(rocketConnection));
    ThriftClientAppAdapter::Ptr adapter(new ThriftClientAppAdapter(
        static_cast<uint16_t>(apache::thrift::protocol::T_COMPACT_PROTOCOL)));

    thriftPipeline = PipelineBuilder<
                         client::ThriftClientTransportAdapter,
                         ThriftClientAppAdapter,
                         SimpleBufferAllocator>()
                         .setEventBase(&evb)
                         .setHead(transportAdapter.get())
                         .setTail(adapter.get())
                         .setAllocator(&thriftAllocator)
                         .addNextInbound<ThriftClientMetadataPushHandler>(
                             thrift_client_metadata_push_handler_tag)
                         .build();
    adapter->setPipeline(thriftPipeline.get());
    transportAdapter->setPipeline(thriftPipeline.get());
    fastClient = std::make_unique<FastClientType>(std::move(adapter));

    // Discard SETUP frame
    fastClient->ping(std::make_unique<NoopRequestCallback>());
    evb.loopOnce();
    testTransport->getWrittenData();
  }
};

// ===================== Request Path — Ping =====================

BENCHMARK(Thrift_Request_Ping, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftBenchFixture f;
  f.setup();

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.thriftClient->ping(std::make_unique<NoopRequestCallback>());
  }
}

BENCHMARK_RELATIVE(FastThriftChannel_Request_Ping, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftChannelBenchFixture f;
  f.setup();

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.thriftClient->ping(std::make_unique<NoopRequestCallback>());
  }
}

BENCHMARK_RELATIVE(FastThriftClient_Request_Ping, iters) {
  folly::BenchmarkSuspender suspender;
  FastClientBenchFixture f;
  f.setup();

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.fastClient->ping(std::make_unique<NoopRequestCallback>());
  }
}

BENCHMARK_DRAW_LINE();

// ===================== Request Path — Echo =====================

BENCHMARK(Thrift_Request_Echo, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftBenchFixture f;
  f.setup();

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.thriftClient->echo(
        std::make_unique<NoopRequestCallback>(), "hello world");
  }
}

BENCHMARK_RELATIVE(FastThriftChannel_Request_Echo, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftChannelBenchFixture f;
  f.setup();

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.thriftClient->echo(
        std::make_unique<NoopRequestCallback>(), "hello world");
  }
}

BENCHMARK_RELATIVE(FastThriftClient_Request_Echo, iters) {
  folly::BenchmarkSuspender suspender;
  FastClientBenchFixture f;
  f.setup();

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.fastClient->echo(std::make_unique<NoopRequestCallback>(), "hello world");
  }
}

BENCHMARK_DRAW_LINE();

// ===================== Request Path — SendResponse =====================

BENCHMARK(Thrift_Request_SendResponse, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftBenchFixture f;
  f.setup();

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.thriftClient->sendResponse(
        std::make_unique<NoopRequestCallback>(),
        static_cast<int64_t>(kPayloadSize));
  }
}

BENCHMARK_RELATIVE(FastThriftChannel_Request_SendResponse, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftChannelBenchFixture f;
  f.setup();

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.thriftClient->sendResponse(
        std::make_unique<NoopRequestCallback>(),
        static_cast<int64_t>(kPayloadSize));
  }
}

BENCHMARK_RELATIVE(FastThriftClient_Request_SendResponse, iters) {
  folly::BenchmarkSuspender suspender;
  FastClientBenchFixture f;
  f.setup();

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.fastClient->sendResponse(
        std::make_unique<NoopRequestCallback>(),
        static_cast<int64_t>(kPayloadSize));
  }
}

BENCHMARK_DRAW_LINE();

// ===================== Response Path — Success Ping =====================

BENCHMARK(Thrift_Response_Success_Ping, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->ping(std::make_unique<NoopRequestCallback>());
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeVoidResult();
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftChannel_Response_Success_Ping, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftChannelBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->ping(std::make_unique<NoopRequestCallback>());
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeVoidResult();
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftClient_Response_Success_Ping, iters) {
  folly::BenchmarkSuspender suspender;
  FastClientBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.fastClient->ping(std::make_unique<NoopRequestCallback>());
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeVoidResult();
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_DRAW_LINE();

// ===================== Response Path — Success Echo =====================

BENCHMARK(Thrift_Response_Success_Echo, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->echo(
        std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeStringResult("hello world");
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftChannel_Response_Success_Echo, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftChannelBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->echo(
        std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeStringResult("hello world");
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftClient_Response_Success_Echo, iters) {
  folly::BenchmarkSuspender suspender;
  FastClientBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.fastClient->echo(std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeStringResult("hello world");
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_DRAW_LINE();

// ============= Response Path — Success SendResponse (large) =============

BENCHMARK(Thrift_Response_Success_SendResponse, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->sendResponse(
        std::make_unique<NoopRequestCallback>(),
        static_cast<int64_t>(kPayloadSize));
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeStringResult(std::string(kPayloadSize, 'x'));
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftChannel_Response_Success_SendResponse, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftChannelBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->sendResponse(
        std::make_unique<NoopRequestCallback>(),
        static_cast<int64_t>(kPayloadSize));
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeStringResult(std::string(kPayloadSize, 'x'));
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftClient_Response_Success_SendResponse, iters) {
  folly::BenchmarkSuspender suspender;
  FastClientBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.fastClient->sendResponse(
        std::make_unique<NoopRequestCallback>(),
        static_cast<int64_t>(kPayloadSize));
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeStringResult(std::string(kPayloadSize, 'x'));
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_DRAW_LINE();

// =============== Response Path — Declared Exception ===============

BENCHMARK(Thrift_Response_DeclaredException, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->echo(
        std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createDeclaredExceptionMetadata());
  auto respData = serializeVoidResult();
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftChannel_Response_DeclaredException, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftChannelBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->echo(
        std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createDeclaredExceptionMetadata());
  auto respData = serializeVoidResult();
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftClient_Response_DeclaredException, iters) {
  folly::BenchmarkSuspender suspender;
  FastClientBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.fastClient->echo(std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createDeclaredExceptionMetadata());
  auto respData = serializeVoidResult();
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_DRAW_LINE();

// ============= Response Path — Undeclared Exception ===============

BENCHMARK(Thrift_Response_UndeclaredException, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->echo(
        std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta =
      serializeResponseMetadata(createUndeclaredExceptionMetadata());
  auto respData = makePayloadData(kPayloadSize);
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftChannel_Response_UndeclaredException, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftChannelBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->echo(
        std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta =
      serializeResponseMetadata(createUndeclaredExceptionMetadata());
  auto respData = makePayloadData(kPayloadSize);
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftClient_Response_UndeclaredException, iters) {
  folly::BenchmarkSuspender suspender;
  FastClientBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.fastClient->echo(std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta =
      serializeResponseMetadata(createUndeclaredExceptionMetadata());
  auto respData = makePayloadData(kPayloadSize);
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_DRAW_LINE();

// ================ Response Path — Error Frame ===================

BENCHMARK(Thrift_Response_ErrorFrame, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->echo(
        std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto errPayload = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::OVERLOAD, "Server overloaded");
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createErrorFrame(
        streamIds[i], kErrorCodeRejected, errPayload->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftChannel_Response_ErrorFrame, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftChannelBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->echo(
        std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto errPayload = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::OVERLOAD, "Server overloaded");
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createErrorFrame(
        streamIds[i], kErrorCodeRejected, errPayload->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftClient_Response_ErrorFrame, iters) {
  folly::BenchmarkSuspender suspender;
  FastClientBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.fastClient->echo(std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto errPayload = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::OVERLOAD, "Server overloaded");
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(createErrorFrame(
        streamIds[i], kErrorCodeRejected, errPayload->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_DRAW_LINE();

// =============== Response Path — Fragmented (64-byte chunks) ===============

BENCHMARK(Thrift_Response_Fragmented, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->echo(
        std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeStringResult("hello world");
  constexpr size_t kFragSize = 64;
  std::vector<std::vector<std::unique_ptr<folly::IOBuf>>> allFrags;
  allFrags.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    auto frame = prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone()));
    std::vector<std::unique_ptr<folly::IOBuf>> frags;
    folly::io::Cursor cursor(frame.get());
    size_t rem = frame->computeChainDataLength();
    while (rem > 0) {
      size_t sz = std::min(kFragSize, rem);
      auto chunk = folly::IOBuf::create(sz);
      cursor.pull(chunk->writableTail(), sz);
      chunk->append(sz);
      frags.push_back(std::move(chunk));
      rem -= sz;
    }
    allFrags.push_back(std::move(frags));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    for (auto& frag : allFrags[i]) {
      f.testTransport->injectReadData(std::move(frag));
    }
  }
}

BENCHMARK_RELATIVE(FastThriftChannel_Response_Fragmented, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftChannelBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->echo(
        std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeStringResult("hello world");
  constexpr size_t kFragSize = 64;
  std::vector<std::vector<std::unique_ptr<folly::IOBuf>>> allFrags;
  allFrags.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    auto frame = prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone()));
    std::vector<std::unique_ptr<folly::IOBuf>> frags;
    folly::io::Cursor cursor(frame.get());
    size_t rem = frame->computeChainDataLength();
    while (rem > 0) {
      size_t sz = std::min(kFragSize, rem);
      auto chunk = folly::IOBuf::create(sz);
      cursor.pull(chunk->writableTail(), sz);
      chunk->append(sz);
      frags.push_back(std::move(chunk));
      rem -= sz;
    }
    allFrags.push_back(std::move(frags));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    for (auto& frag : allFrags[i]) {
      f.testTransport->injectReadData(std::move(frag));
    }
  }
}

BENCHMARK_RELATIVE(FastThriftClient_Response_Fragmented, iters) {
  folly::BenchmarkSuspender suspender;
  FastClientBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.fastClient->echo(std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeStringResult("hello world");
  constexpr size_t kFragSize = 64;
  std::vector<std::vector<std::unique_ptr<folly::IOBuf>>> allFrags;
  allFrags.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    auto frame = prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone()));
    std::vector<std::unique_ptr<folly::IOBuf>> frags;
    folly::io::Cursor cursor(frame.get());
    size_t rem = frame->computeChainDataLength();
    while (rem > 0) {
      size_t sz = std::min(kFragSize, rem);
      auto chunk = folly::IOBuf::create(sz);
      cursor.pull(chunk->writableTail(), sz);
      chunk->append(sz);
      frags.push_back(std::move(chunk));
      rem -= sz;
    }
    allFrags.push_back(std::move(frags));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    for (auto& frag : allFrags[i]) {
      f.testTransport->injectReadData(std::move(frag));
    }
  }
}

BENCHMARK_DRAW_LINE();

// =========== Response Path — High Concurrency (1000 in-flight) ===========

BENCHMARK(Thrift_Response_HighConcurrency, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftBenchFixture f;
  f.setup();

  constexpr size_t kConcurrent = 1000;
  size_t actual = std::max(static_cast<size_t>(iters), kConcurrent);
  std::vector<uint32_t> streamIds;
  streamIds.reserve(actual);
  for (size_t i = 0; i < actual; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->echo(
        std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeStringResult("hello world");
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(actual);
  for (size_t i = 0; i < actual; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftChannel_Response_HighConcurrency, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftChannelBenchFixture f;
  f.setup();

  constexpr size_t kConcurrent = 1000;
  size_t actual = std::max(static_cast<size_t>(iters), kConcurrent);
  std::vector<uint32_t> streamIds;
  streamIds.reserve(actual);
  for (size_t i = 0; i < actual; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->echo(
        std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeStringResult("hello world");
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(actual);
  for (size_t i = 0; i < actual; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftClient_Response_HighConcurrency, iters) {
  folly::BenchmarkSuspender suspender;
  FastClientBenchFixture f;
  f.setup();

  constexpr size_t kConcurrent = 1000;
  size_t actual = std::max(static_cast<size_t>(iters), kConcurrent);
  std::vector<uint32_t> streamIds;
  streamIds.reserve(actual);
  for (size_t i = 0; i < actual; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.fastClient->echo(std::make_unique<NoopRequestCallback>(), "hello world");
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  auto respData = serializeStringResult("hello world");
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(actual);
  for (size_t i = 0; i < actual; ++i) {
    frames.emplace_back(prependLengthPrefix(createPayloadResponse(
        streamIds[i], respMeta->clone(), respData->clone())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_DRAW_LINE();

// ============= Response Path — Chained Payload (IOBuf chain) =============

BENCHMARK(Thrift_Response_ChainedPayload, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->sendResponse(
        std::make_unique<NoopRequestCallback>(),
        static_cast<int64_t>(kPayloadSize));
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  constexpr size_t kChunkSize = 64;
  auto mkChained = [&]() {
    auto serialized = serializeStringResult(std::string(kPayloadSize, 'x'));
    std::unique_ptr<folly::IOBuf> chain;
    folly::io::Cursor cursor(serialized.get());
    size_t rem = serialized->computeChainDataLength();
    while (rem > 0) {
      size_t sz = std::min(kChunkSize, rem);
      auto chunk = folly::IOBuf::create(sz);
      cursor.pull(chunk->writableTail(), sz);
      chunk->append(sz);
      if (chain) {
        chain->prependChain(std::move(chunk));
      } else {
        chain = std::move(chunk);
      }
      rem -= sz;
    }
    return chain;
  };
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(
        createPayloadResponse(streamIds[i], respMeta->clone(), mkChained())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftChannel_Response_ChainedPayload, iters) {
  folly::BenchmarkSuspender suspender;
  FastThriftChannelBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.thriftClient->sendResponse(
        std::make_unique<NoopRequestCallback>(),
        static_cast<int64_t>(kPayloadSize));
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  constexpr size_t kChunkSize = 64;
  auto mkChained = [&]() {
    auto serialized = serializeStringResult(std::string(kPayloadSize, 'x'));
    std::unique_ptr<folly::IOBuf> chain;
    folly::io::Cursor cursor(serialized.get());
    size_t rem = serialized->computeChainDataLength();
    while (rem > 0) {
      size_t sz = std::min(kChunkSize, rem);
      auto chunk = folly::IOBuf::create(sz);
      cursor.pull(chunk->writableTail(), sz);
      chunk->append(sz);
      if (chain) {
        chain->prependChain(std::move(chunk));
      } else {
        chain = std::move(chunk);
      }
      rem -= sz;
    }
    return chain;
  };
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(
        createPayloadResponse(streamIds[i], respMeta->clone(), mkChained())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

BENCHMARK_RELATIVE(FastThriftClient_Response_ChainedPayload, iters) {
  folly::BenchmarkSuspender suspender;
  FastClientBenchFixture f;
  f.setup();

  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    std::unique_ptr<folly::IOBuf> written;
    f.fastClient->sendResponse(
        std::make_unique<NoopRequestCallback>(),
        static_cast<int64_t>(kPayloadSize));
    f.evb.loopOnce();
    written = f.testTransport->getWrittenData();
    streamIds.push_back(parseWrittenFrame(std::move(written)).streamId());
  }

  auto respMeta = serializeResponseMetadata(createSuccessResponseMetadata());
  constexpr size_t kChunkSize = 64;
  auto mkChained = [&]() {
    auto serialized = serializeStringResult(std::string(kPayloadSize, 'x'));
    std::unique_ptr<folly::IOBuf> chain;
    folly::io::Cursor cursor(serialized.get());
    size_t rem = serialized->computeChainDataLength();
    while (rem > 0) {
      size_t sz = std::min(kChunkSize, rem);
      auto chunk = folly::IOBuf::create(sz);
      cursor.pull(chunk->writableTail(), sz);
      chunk->append(sz);
      if (chain) {
        chain->prependChain(std::move(chunk));
      } else {
        chain = std::move(chunk);
      }
      rem -= sz;
    }
    return chain;
  };
  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.emplace_back(prependLengthPrefix(
        createPayloadResponse(streamIds[i], respMeta->clone(), mkChained())));
  }

  suspender.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    f.testTransport->injectReadData(std::move(frames[i]));
  }
}

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
