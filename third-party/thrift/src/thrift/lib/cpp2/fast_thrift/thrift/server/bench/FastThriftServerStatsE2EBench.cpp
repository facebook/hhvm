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
 * What attaching server stats costs a request, end to end.
 *
 * The handler microbenchmarks price a single incrementValue() against a stub
 * context. That answers "what does the handler cost" but not "what does
 * turning stats on cost", which is the question a service owner actually
 * asks: the handlers sit in two different pipelines, and their cost has to be
 * read against a whole request rather than against a bare fireRead.
 *
 * So each pair below runs the same request through the same server twice,
 * differing only in whether the two metrics handlers are in the pipelines —
 * which is exactly what FastThriftServer::setStats decides. The NoStats side
 * is not "stats disabled", it is stats absent: no handler, and therefore no
 * branch to skip one.
 *
 *   BENCHMARK(NoStats_*)            — pipelines built without the handlers
 *   BENCHMARK_RELATIVE(WithStats_*) — RocketMetricsHandler in the rocket
 *                                     pipeline, ThriftMetricsHandler in the
 *                                     thrift pipeline, at the positions
 *                                     ThriftServerConnectionFactory uses
 *
 * Both fixtures drive the production dispatch path (FastThriftServerAppAdapter
 * + FastServiceHandler) over BenchAsyncTransport, so the delta is the metrics
 * handlers and nothing else.
 *
 * The fixture holds a bare ServerStatsShard rather than a ServerStats. A
 * connection reaches its shard once, at accept; per request it only ever
 * touches the shard itself, so routing through ServerStats would add setup
 * this benchmark is not trying to measure.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/common/Stats.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/handler/FrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameDefragmentationHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FragmentationHandlerConfig.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameFragmentationHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/RocketStreamContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/handler/RocketMetricsHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/common/RocketServerConnection.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerMessageMarshalHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/handler/ThriftMetricsHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServer.tcc>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/bench/BenchAsyncTransport.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read::handler;
using namespace apache::thrift::fast_thrift::frame::write::handler;
using namespace apache::thrift::fast_thrift::frame::write;
using namespace apache::thrift::fast_thrift::transport::bench;
using namespace apache::thrift::fast_thrift::thrift::test::integration;

namespace {

constexpr size_t kPayloadSize = 4'096;

HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(frame_codec_handler);
HANDLER_TAG(frame_defragmentation_handler);
HANDLER_TAG(frame_fragmentation_handler);
HANDLER_TAG(rocket_server_message_marshal_handler);
HANDLER_TAG(server_setup_frame_handler);
HANDLER_TAG(server_request_response_frame_handler);
HANDLER_TAG(server_stream_state_handler);

using RocketMetrics = RocketMetricsHandler<Direction::Server, ServerStatsShard>;
using ThriftMetrics = ThriftMetricsHandler<Direction::Server, ServerStatsShard>;

class FastHandler
    : public ::apache::thrift::FastServiceHandler<FastThriftServer> {
 public:
  bool noReply{false};

  void async_tm_ping(
      ::apache::thrift::fast_thrift::thrift::FastHandlerCallbackPtr<void>
          callback) override {
    if (noReply) {
      return;
    }
    callback->done();
  }

  void async_tm_echo(
      ::apache::thrift::fast_thrift::thrift::FastHandlerCallbackPtr<
          std::unique_ptr<EchoResponse>> callback,
      std::unique_ptr<std::string> message) override {
    if (noReply) {
      return;
    }
    auto response = std::make_unique<EchoResponse>();
    response->message() = std::move(*message);
    callback->result(std::move(response));
  }
};

// =============================================================================
// Frame construction
// =============================================================================

std::unique_ptr<folly::IOBuf> serializeRequestMetadata(
    const apache::thrift::RequestRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

apache::thrift::RequestRpcMetadata makeRequestMetadata(
    std::string_view methodName) {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = std::string(methodName);
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;
  return metadata;
}

std::unique_ptr<folly::IOBuf> serializeEmptyPargs() {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  FastThriftServer_ping_pargs pargs;
  pargs.write(&writer);
  return queue.move();
}

std::unique_ptr<folly::IOBuf> serializeEchoPargs(const std::string& payload) {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  FastThriftServer_echo_pargs pargs;
  pargs.template get<0>().value = const_cast<std::string*>(&payload);
  pargs.write(&writer);
  return queue.move();
}

std::unique_ptr<folly::IOBuf> createRequestFrame(
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  return serialize(
      RequestResponseHeader{.streamId = streamId},
      std::move(metadata),
      std::move(data));
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

// =============================================================================
// Pipelines — identical either side of the comparison but for the two
// metrics handlers, which sit where ThriftServerConnectionFactory puts them:
// rocket closest to the tail, thrift closest to the head.
// =============================================================================

std::unique_ptr<rocket::server::RocketServerConnection> buildRocketConnection(
    folly::EventBase* evb,
    BenchAsyncTransport** outTransport,
    ServerStatsShard* shard) {
  auto rocketConn = std::make_unique<rocket::server::RocketServerConnection>();

  auto transport =
      folly::AsyncTransport::UniquePtr(new BenchAsyncTransport(evb));
  *outTransport = static_cast<BenchAsyncTransport*>(transport.get());

  rocketConn->transportHandler = apache::thrift::fast_thrift::rocket::server::
      RocketServerTransportHandler::create(std::move(transport));

  // addState rebinds the builder and leaves the original moved-from, so the
  // chain through it has to be bound here before anything else is added.
  auto builder =
      PipelineBuilder<
          apache::thrift::fast_thrift::rocket::server::
              RocketServerTransportHandler,
          rocket::server::RocketServerAppAdapter,
          SimpleBufferAllocator>()
          .setEventBase(evb)
          .setHead(rocketConn->transportHandler.get())
          .setTail(rocketConn->appAdapter.get())
          .setAllocator(&rocketConn->allocator)
          .addState<
              apache::thrift::fast_thrift::rocket::RocketStreamContexts>();

  builder
      .addNextInbound<FrameLengthParserHandler>(frame_length_parser_handler_tag)
      .addNextOutbound<FrameLengthEncoderHandler>(
          frame_length_encoder_handler_tag)
      .addNextDuplex<frame::handler::FrameCodecHandler>(frame_codec_handler_tag)
      .addNextInbound<frame::read::handler::FrameDefragmentationHandler>(
          frame_defragmentation_handler_tag)
      .addNextOutbound<frame::write::handler::FrameFragmentationHandler>(
          frame_fragmentation_handler_tag,
          frame::write::FragmentationHandlerConfig{})
      .addNextDuplex<
          rocket::server::handler::RocketServerMessageMarshalHandler>(
          rocket_server_message_marshal_handler_tag)
      .addNextDuplex<rocket::server::handler::RocketServerSetupFrameHandler>(
          server_setup_frame_handler_tag)
      .addNextDuplex<rocket::server::handler::RocketServerStreamStateHandler>(
          server_stream_state_handler_tag)
      .addNextDuplex<
          rocket::server::handler::RocketServerRequestResponseHandler>(
          server_request_response_frame_handler_tag);

  if (shard != nullptr) {
    builder.addNextDuplex<RocketMetrics>(rocket_metrics_handler_tag, shard);
  }

  rocketConn->pipeline = builder.build();

  rocketConn->appAdapter->setPipeline(rocketConn->pipeline.get());
  rocketConn->transportHandler->setPipeline(rocketConn->pipeline.get());
  rocketConn->transportHandler->onConnect();

  return rocketConn;
}

// Non-blocking throughout: BenchAsyncTransport::injectReadData delivers
// straight into the read callback rather than scheduling on the EventBase, so
// a blocking loopOnce() would wait on an event that is never coming. The loop
// still has to run, because outbound writes are flushed from a LoopCallback.
void injectSetupFrame(BenchAsyncTransport* transport, folly::EventBase& evb) {
  auto setupFrame = serialize(
      SetupHeader{
          .majorVersion = 1,
          .minorVersion = 0,
          .keepaliveTime = 30000,
          .maxLifetime = 60000},
      nullptr,
      nullptr);
  transport->injectReadData(prependLengthPrefix(std::move(setupFrame)));
  evb.loopOnce(EVLOOP_NONBLOCK);
}

struct BenchFixture {
  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};

  // Stands in for the per-IO-thread shard a real connection is handed.
  ServerStatsShard shard;

  std::unique_ptr<thrift::server::ThriftServerTransportAdapter>
      transportAdapter;
  std::shared_ptr<FastHandler> handler;
  std::unique_ptr<
      FastThriftServerAppAdapter,
      folly::DelayedDestruction::Destructor>
      adapter;
  PipelineImpl::Ptr thriftPipeline;
  SimpleBufferAllocator thriftAllocator;

  void setup(bool withStats, bool noReply) {
    auto* shardPtr = withStats ? &shard : nullptr;
    auto rocketConn = buildRocketConnection(&evb, &testTransport, shardPtr);

    handler = std::make_shared<FastHandler>();
    handler->noReply = noReply;
    adapter.reset(new FastThriftServerAppAdapter(handler));

    transportAdapter =
        std::make_unique<thrift::server::ThriftServerTransportAdapter>(
            std::move(rocketConn));

    PipelineBuilder<
        thrift::server::ThriftServerTransportAdapter,
        FastThriftServerAppAdapter,
        SimpleBufferAllocator>
        builder;
    builder.setEventBase(&evb)
        .setHead(transportAdapter.get())
        .setTail(adapter.get())
        .setAllocator(&thriftAllocator);
    if (shardPtr != nullptr) {
      builder.addNextDuplex<ThriftMetrics>(
          thrift_metrics_handler_tag, shardPtr);
    }
    thriftPipeline = builder.build();

    transportAdapter->setPipeline(thriftPipeline.get());
    adapter->setPipeline(thriftPipeline.get());

    injectSetupFrame(testTransport, evb);
  }

  void injectFrame(std::unique_ptr<folly::IOBuf> frame) {
    testTransport->injectReadData(prependLengthPrefix(std::move(frame)));
  }
};

// =============================================================================
// Bench bodies
// =============================================================================

void runRequestBench(BenchFixture& fixture, uint32_t iters) {
  auto metadataTemplate = serializeRequestMetadata(makeRequestMetadata("ping"));
  auto pargsTemplate = serializeEmptyPargs();

  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    requests.push_back(createRequestFrame(
        i * 2 + 1, metadataTemplate->clone(), pargsTemplate->clone()));
  }

  for (uint32_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce(EVLOOP_NONBLOCK);
  }
}

void runResponseBench(
    BenchFixture& fixture, const std::string& payload, uint32_t iters) {
  auto metadataTemplate = serializeRequestMetadata(makeRequestMetadata("echo"));
  auto pargsTemplate = serializeEchoPargs(payload);

  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    requests.push_back(createRequestFrame(
        i * 2 + 1, metadataTemplate->clone(), pargsTemplate->clone()));
  }

  for (uint32_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce(EVLOOP_NONBLOCK);
    fixture.testTransport->clearWrittenData();
  }
}

} // namespace

// =============================================================================
// Request only — the smallest request there is, so the metrics handlers are
// the largest fraction of it they will ever be.
// =============================================================================

BENCHMARK(NoStats_Request, iters) {
  folly::BenchmarkSuspender suspender;
  BenchFixture fixture;
  fixture.setup(/*withStats=*/false, /*noReply=*/true);
  suspender.dismiss();

  runRequestBench(fixture, iters);
}

BENCHMARK_RELATIVE(WithStats_Request, iters) {
  folly::BenchmarkSuspender suspender;
  BenchFixture fixture;
  fixture.setup(/*withStats=*/true, /*noReply=*/true);
  suspender.dismiss();

  runRequestBench(fixture, iters);
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Request and response — a realistic echo, where the same fixed handler cost
// is measured against a request that does real work.
// =============================================================================

BENCHMARK(NoStats_RequestResponse, iters) {
  folly::BenchmarkSuspender suspender;
  BenchFixture fixture;
  fixture.setup(/*withStats=*/false, /*noReply=*/false);
  std::string payload(kPayloadSize, 'x');
  suspender.dismiss();

  runResponseBench(fixture, payload, iters);
}

BENCHMARK_RELATIVE(WithStats_RequestResponse, iters) {
  folly::BenchmarkSuspender suspender;
  BenchFixture fixture;
  fixture.setup(/*withStats=*/true, /*noReply=*/false);
  std::string payload(kPayloadSize, 'x');
  suspender.dismiss();

  runResponseBench(fixture, payload, iters);
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
