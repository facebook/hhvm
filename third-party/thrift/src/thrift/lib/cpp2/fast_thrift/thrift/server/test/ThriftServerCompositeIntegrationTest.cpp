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
 * ThriftServerCompositeAppAdapter integration tests.
 *
 * Drives the full server-side two-pipeline stack with the composite at
 * the thrift pipeline tail:
 *
 *   Rocket pipeline:
 *     Transport -> FrameLengthParser -> FrameLengthEncoder -> FrameCodec ->
 *     Setup -> StreamState -> RequestResponse -> RocketServerAppAdapter
 *
 *   Thrift pipeline:
 *     ThriftServerTransportAdapter -> ThriftServerCompositeAppAdapter
 *                                       |- child A (ThriftServerAppAdapter)
 *                                       \- child B (ThriftServerAppAdapter)
 *
 * Modeled on ThriftServerIntegrationTest.cpp (which uses ThriftServerChannel
 * + MockAsyncProcessor as the thrift tail). Here the tail is the composite,
 * with real ThriftServerAppAdapter children whose registered method handlers
 * actually write responses through the pipeline.
 *
 * Frames are injected and captured at the byte level via TestAsyncTransport,
 * so every layer between socket and child handler is exercised end-to-end.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/handler/FrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameDefragmentationHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FragmentationHandlerConfig.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameFragmentationHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/common/RocketServerConnection.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerMessageMarshalHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerCompositeAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponsePayloads.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/test/TestAsyncTransport.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift::server::test {

using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::test::TestAllocator;
using apache::thrift::fast_thrift::rocket::server::RocketServerAppAdapter;
using apache::thrift::fast_thrift::transport::test::TestAsyncTransport;

HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(frame_codec_handler);
HANDLER_TAG(frame_defragmentation_handler);
HANDLER_TAG(frame_fragmentation_handler);
HANDLER_TAG(rocket_server_message_marshal_handler);
HANDLER_TAG(server_setup_frame_handler);
HANDLER_TAG(server_request_response_frame_handler);
HANDLER_TAG(server_stream_state_handler);

namespace {

// =============================================================================
// RecordingAppAdapter — ThriftServerAppAdapter subclass whose registered
// method handlers (a) record the inbound request and (b) write a success
// response carrying the request payload as the response body. Lets tests
// verify routing dispatched to the correct child *and* that the response
// path back through the composite + thrift pipeline + rocket pipeline ends
// up on the wire intact.
// =============================================================================

class RecordingAppAdapter : public ThriftServerAppAdapter {
 public:
  using Ptr = std::unique_ptr<RecordingAppAdapter, Destructor>;

  explicit RecordingAppAdapter(std::string id) : id_(std::move(id)) {}

  // Register a method that records the call but doesn't write a response.
  // Use when the test cares about dispatch fan-out across multiple children
  // and doesn't need to assert on the wire — avoids the deferred-write
  // interaction with TestAsyncTransport that blocks subsequent inbound
  // reads when responses are written synchronously inside the dispatch.
  void registerNoResponse(std::string_view name) {
    addMethodHandler(
        name,
        +[](ThriftServerAppAdapter* self,
            uint32_t streamId,
            std::unique_ptr<folly::IOBuf> data,
            apache::thrift::ProtocolId protocol,
            std::unique_ptr<ThriftRequestContext>) noexcept {
          auto* t = static_cast<RecordingAppAdapter*>(self);
          Recorded r;
          r.streamId = streamId;
          r.protocol = protocol;
          if (data) {
            r.payload =
                data->cloneCoalescedAsValue().moveToFbString().toStdString();
          }
          t->received.push_back(std::move(r));
        });
  }

  // Register a method that echoes the inbound payload back as a successful
  // response. Records the call for test inspection.
  void registerEcho(std::string_view name) {
    addMethodHandler(
        name,
        +[](ThriftServerAppAdapter* self,
            uint32_t streamId,
            std::unique_ptr<folly::IOBuf> data,
            apache::thrift::ProtocolId protocol,
            std::unique_ptr<ThriftRequestContext>) noexcept {
          auto* t = static_cast<RecordingAppAdapter*>(self);
          Recorded r;
          r.streamId = streamId;
          r.protocol = protocol;
          if (data) {
            r.payload =
                data->cloneCoalescedAsValue().moveToFbString().toStdString();
          }
          t->received.push_back(std::move(r));

          auto metadata =
              std::make_unique<apache::thrift::ResponseRpcMetadata>();
          // No need to populate metadata fields beyond defaults for the
          // success path — codegen's fillSuccessResponseMetadata isn't
          // strictly required for the wire to be parseable; tests only
          // inspect frame type / streamId / data.
          auto responseData = data ? data->clone() : nullptr;
          t->writeResponse(
              apache::thrift::fast_thrift::thrift::makeResponseMessage(
                  streamId, std::move(responseData), std::move(metadata)));
        });
  }

  struct Recorded {
    uint32_t streamId{0};
    apache::thrift::ProtocolId protocol{};
    std::string payload;
  };

  std::string id_;
  std::vector<Recorded> received;
};

// =============================================================================
// Helpers
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
    const std::string& methodName,
    apache::thrift::ProtocolId protocolId =
        apache::thrift::ProtocolId::BINARY) {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = methodName;
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata.protocol() = protocolId;
  return metadata;
}

apache::thrift::ResponseRpcError deserializeResponseRpcError(
    const folly::IOBuf& buf) {
  apache::thrift::ResponseRpcError error;
  apache::thrift::CompactProtocolReader reader;
  reader.setInput(&buf);
  error.read(&reader);
  return error;
}

} // namespace

class ThriftServerCompositeIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    rocketAllocator_.reset();
    thriftAllocator_.reset();
  }

  void TearDown() override {
    // Drain pending write callbacks without blocking — once the body's own
    // loopOnce has consumed the runInLoop wakeup, the EVB has nothing left
    // to fire and a plain loopOnce would block forever.
    evb_.loopOnce(EVLOOP_NONBLOCK);

    if (transportAdapter_) {
      // Close the transport before tearing down the pipelines, mirroring
      // ThriftServerIntegrationTest's teardown.
      transportHandler_()->close(folly::exception_wrapper{});
      transportHandler_()->resetPipeline();
      transportAdapter_->resetPipeline();
      appAdapter_()->resetPipeline();
      // composite_ also holds a pipelineGuard on thriftPipeline_; release it
      // here so thriftPipeline_.reset() below actually destroys the pipeline
      // while all handlers (head transportAdapter_, tail composite_) are
      // still alive — otherwise callHandlerRemoved fires on a freed handler.
      composite_->resetPipeline();
    }
    thriftPipeline_.reset();
    transportAdapter_.reset(); // tears down rocket connection
    composite_.reset();
    childA_.reset();
    childB_.reset();
    testTransport_ = nullptr;
  }

  // Build the full server stack: rocket pipeline + thrift pipeline + the
  // composite as thrift pipeline tail, with two RecordingAppAdapter children
  // pre-registered with disjoint method sets.
  //   childA: "method.a1", "method.a2"
  //   childB: "method.b1"
  void setupPipeline() {
    auto transport =
        folly::AsyncTransport::UniquePtr(new TestAsyncTransport(&evb_));
    testTransport_ = static_cast<TestAsyncTransport*>(transport.get());

    auto rocketConn =
        std::make_unique<rocket::server::RocketServerConnection>();
    rocketConn->transportHandler =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            std::move(transport));

    // 1. Rocket pipeline (identical to ThriftServerIntegrationTest setup).
    rocketConn->pipeline =
        PipelineBuilder<
            apache::thrift::fast_thrift::transport::TransportHandler,
            RocketServerAppAdapter,
            TestAllocator>()
            .setEventBase(&evb_)
            .setHead(rocketConn->transportHandler.get())
            .setTail(rocketConn->appAdapter.get())
            .setAllocator(&rocketAllocator_)
            .addNextInbound<apache::thrift::fast_thrift::frame::read::handler::
                                FrameLengthParserHandler>(
                frame_length_parser_handler_tag)
            .addNextOutbound<apache::thrift::fast_thrift::frame::write::
                                 handler::FrameLengthEncoderHandler>(
                frame_length_encoder_handler_tag)
            .addNextDuplex<
                apache::thrift::fast_thrift::frame::handler::FrameCodecHandler>(
                frame_codec_handler_tag)
            .addNextInbound<apache::thrift::fast_thrift::frame::read::handler::
                                FrameDefragmentationHandler>(
                frame_defragmentation_handler_tag)
            .addNextOutbound<apache::thrift::fast_thrift::frame::write::
                                 handler::FrameFragmentationHandler>(
                frame_fragmentation_handler_tag,
                apache::thrift::fast_thrift::frame::write::
                    FragmentationHandlerConfig{})
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerMessageMarshalHandler>(
                rocket_server_message_marshal_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerSetupFrameHandler>(
                server_setup_frame_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerStreamStateHandler>(
                server_stream_state_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerRequestResponseHandler>(
                server_request_response_frame_handler_tag)
            .build();

    rocketConn->appAdapter->setPipeline(rocketConn->pipeline.get());

    // 2. Build children + composite. Children are added BEFORE composite is
    // wired into a pipeline (per ThriftServerCompositeAppAdapter.h doc).
    childA_.reset(new RecordingAppAdapter("A"));
    childB_.reset(new RecordingAppAdapter("B"));
    childA_->registerEcho("method.a1");
    childA_->registerEcho("method.a2");
    childB_->registerEcho("method.b1");
    // No-response variants used by multi-child dispatch tests.
    childA_->registerNoResponse("method.a1.silent");
    childB_->registerNoResponse("method.b1.silent");

    composite_.reset(new ThriftServerCompositeAppAdapter());
    composite_->addChild(childA_.get());
    composite_->addChild(childB_.get());

    // 3. Thrift pipeline with composite as tail. ThriftServerTransportAdapter
    //    takes ownership of the rocket connection bundle.
    transportAdapter_ =
        std::make_unique<ThriftServerTransportAdapter>(std::move(rocketConn));

    thriftPipeline_ = PipelineBuilder<
                          ThriftServerTransportAdapter,
                          ThriftServerCompositeAppAdapter,
                          TestAllocator>()
                          .setEventBase(&evb_)
                          .setHead(transportAdapter_.get())
                          .setTail(composite_.get())
                          .setAllocator(&thriftAllocator_)
                          .build();

    transportAdapter_->setPipeline(thriftPipeline_.get());
    // composite's setPipeline fans out to children — children need
    // pipeline_ set so their writeResponse fires through the same pipeline.
    composite_->setPipeline(thriftPipeline_.get());
    // Activate the thrift pipeline so composite's onPipelineActive fans out
    // to children (Ready -> Open). Without this, child onRead rejects with
    // Result::Error because base state-checks state == Open.
    thriftPipeline_->activate();

    transportHandler_()->setPipeline(rocketPipeline_());
    transportHandler_()->onConnect();
  }

  void injectSetupFrame() {
    auto setupFrame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::SetupHeader{
            .majorVersion = 1,
            .minorVersion = 0,
            .keepaliveTime = 30000,
            .maxLifetime = 60000},
        nullptr,
        nullptr);
    injectFrame(std::move(setupFrame));
  }

  void setupPipelineWithSetup() {
    setupPipeline();
    injectSetupFrame();
  }

  void injectFrame(std::unique_ptr<folly::IOBuf> frame) {
    size_t frameLength = frame->computeChainDataLength();
    auto lengthPrefix = folly::IOBuf::create(
        apache::thrift::fast_thrift::frame::kMetadataLengthSize);
    uint8_t* data = lengthPrefix->writableData();
    data[0] = static_cast<uint8_t>((frameLength >> 16) & 0xFF);
    data[1] = static_cast<uint8_t>((frameLength >> 8) & 0xFF);
    data[2] = static_cast<uint8_t>(frameLength & 0xFF);
    lengthPrefix->append(
        apache::thrift::fast_thrift::frame::kMetadataLengthSize);
    lengthPrefix->appendChain(std::move(frame));
    testTransport_->injectReadData(std::move(lengthPrefix));
  }

  std::unique_ptr<folly::IOBuf> createRequestFrame(
      uint32_t streamId,
      const apache::thrift::RequestRpcMetadata& rpcMetadata,
      std::unique_ptr<folly::IOBuf> payloadData) {
    auto metadata = serializeRequestMetadata(rpcMetadata);
    return apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
            .streamId = streamId},
        std::move(metadata),
        std::move(payloadData));
  }

  std::unique_ptr<folly::IOBuf> getWrittenFrame() {
    return testTransport_->getWrittenData();
  }

  apache::thrift::fast_thrift::frame::read::ParsedFrame parseWrittenFrame(
      std::unique_ptr<folly::IOBuf> data) {
    folly::IOBufQueue queue;
    queue.append(std::move(data));
    queue.trimStart(apache::thrift::fast_thrift::frame::kMetadataLengthSize);
    return apache::thrift::fast_thrift::frame::read::parseFrame(queue.move());
  }

  folly::EventBase evb_;
  TestAsyncTransport* testTransport_{nullptr};
  // ThriftServerTransportAdapter owns the rocket connection (transport
  // handler, app adapter, rocket pipeline) post-construction. Accessors
  // below dereference it.
  std::unique_ptr<ThriftServerTransportAdapter> transportAdapter_;
  RecordingAppAdapter::Ptr childA_;
  RecordingAppAdapter::Ptr childB_;
  ThriftServerCompositeAppAdapter::Ptr composite_;
  PipelineImpl::Ptr thriftPipeline_;
  TestAllocator rocketAllocator_;
  TestAllocator thriftAllocator_;

  rocket::server::RocketServerConnection& rocketConn_() {
    return transportAdapter_->rocketConnection();
  }
  apache::thrift::fast_thrift::transport::TransportHandler*
  transportHandler_() {
    return rocketConn_().transportHandler.get();
  }
  RocketServerAppAdapter* appAdapter_() {
    return rocketConn_().appAdapter.get();
  }
  PipelineImpl* rocketPipeline_() { return rocketConn_().pipeline.get(); }
};

// =============================================================================
// Routing matrix — each child receives requests for its own methods only.
// =============================================================================

TEST_F(ThriftServerCompositeIntegrationTest, RoutesToChildA) {
  setupPipelineWithSetup();

  auto rpcMeta = makeRequestMetadata("method.a1");
  injectFrame(
      createRequestFrame(1, rpcMeta, folly::IOBuf::copyBuffer("payload-a1")));

  ASSERT_EQ(childA_->received.size(), 1u);
  EXPECT_EQ(childA_->received[0].streamId, 1u);
  EXPECT_EQ(childA_->received[0].payload, "payload-a1");
  EXPECT_EQ(childB_->received.size(), 0u);
}

TEST_F(ThriftServerCompositeIntegrationTest, RoutesToChildB) {
  setupPipelineWithSetup();

  auto rpcMeta = makeRequestMetadata("method.b1");
  injectFrame(
      createRequestFrame(1, rpcMeta, folly::IOBuf::copyBuffer("payload-b1")));

  ASSERT_EQ(childB_->received.size(), 1u);
  EXPECT_EQ(childB_->received[0].payload, "payload-b1");
  EXPECT_EQ(childA_->received.size(), 0u);
}

// Multi-child dispatch: send requests addressed to different children over
// the same connection and verify each child's handler ran exactly once.
// Uses the no-response method variants so the dispatch fan-out isn't
// blocked by deferred-write backpressure interactions with
// TestAsyncTransport — those are covered separately by the single-RPC
// response-path tests below.
TEST_F(
    ThriftServerCompositeIntegrationTest,
    BothChildrenDispatchedOnSameConnection) {
  setupPipelineWithSetup();

  injectFrame(createRequestFrame(
      1,
      makeRequestMetadata("method.a1.silent"),
      folly::IOBuf::copyBuffer("for-a")));
  injectFrame(createRequestFrame(
      3,
      makeRequestMetadata("method.b1.silent"),
      folly::IOBuf::copyBuffer("for-b")));

  ASSERT_EQ(childA_->received.size(), 1u);
  EXPECT_EQ(childA_->received[0].streamId, 1u);
  EXPECT_EQ(childA_->received[0].payload, "for-a");

  ASSERT_EQ(childB_->received.size(), 1u);
  EXPECT_EQ(childB_->received[0].streamId, 3u);
  EXPECT_EQ(childB_->received[0].payload, "for-b");

  // Explicit cross-pollination check: each child must NOT see the other's
  // request. Implied by sizes + values above, but state it directly so a
  // future regression that double-dispatches surfaces as a clear failure.
  for (const auto& r : childA_->received) {
    EXPECT_NE(r.streamId, 3u) << "child A must not see B's stream";
    EXPECT_NE(r.payload, "for-b") << "child A must not see B's payload";
  }
  for (const auto& r : childB_->received) {
    EXPECT_NE(r.streamId, 1u) << "child B must not see A's stream";
    EXPECT_NE(r.payload, "for-a") << "child B must not see A's payload";
  }
}

// TODO: multi-RPC response-path-per-connection. Single-RPC response tests
// pass; multi-RPC fails because the synchronous response-write inside the
// first dispatch interacts with TestAsyncTransport's deferred writeSuccess
// in a way that blocks subsequent inbound reads. Not a composite-routing
// concern — same wire setup as the existing ThriftServerIntegrationTest
// would hit it too if it tested back-to-back replies on one connection.

// =============================================================================
// Protocol matrix — composite routes by name, doesn't care which protocol
// the request metadata declares; transport adapter deserializes that. The
// child sees `protocol` on its dispatch thunk; assert it's preserved.
// =============================================================================

TEST_F(ThriftServerCompositeIntegrationTest, BinaryProtocolPreservedToChild) {
  setupPipelineWithSetup();

  injectFrame(createRequestFrame(
      1,
      makeRequestMetadata("method.a1", apache::thrift::ProtocolId::BINARY),
      folly::IOBuf::copyBuffer("p")));

  ASSERT_EQ(childA_->received.size(), 1u);
  EXPECT_EQ(childA_->received[0].protocol, apache::thrift::ProtocolId::BINARY);
}

TEST_F(ThriftServerCompositeIntegrationTest, CompactProtocolPreservedToChild) {
  setupPipelineWithSetup();

  injectFrame(createRequestFrame(
      1,
      makeRequestMetadata("method.a1", apache::thrift::ProtocolId::COMPACT),
      folly::IOBuf::copyBuffer("p")));

  ASSERT_EQ(childA_->received.size(), 1u);
  EXPECT_EQ(childA_->received[0].protocol, apache::thrift::ProtocolId::COMPACT);
}

// =============================================================================
// Outcome matrix — response path back to the wire.
// =============================================================================

TEST_F(
    ThriftServerCompositeIntegrationTest,
    SuccessResponseReachesWireWithSameStreamId) {
  setupPipelineWithSetup();

  injectFrame(createRequestFrame(
      7, makeRequestMetadata("method.a1"), folly::IOBuf::copyBuffer("echoed")));
  evb_.loopOnce(EVLOOP_NONBLOCK);

  auto responseFrame = getWrittenFrame();
  ASSERT_NE(responseFrame, nullptr) << "Expected response frame on the wire";

  auto parsed = parseWrittenFrame(std::move(responseFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_EQ(parsed.streamId(), 7u);
  EXPECT_GT(parsed.dataSize(), 0u);
}

TEST_F(
    ThriftServerCompositeIntegrationTest,
    UnknownMethodProducesErrorFrameOnWire) {
  setupPipelineWithSetup();

  injectFrame(createRequestFrame(
      1,
      makeRequestMetadata("method.nobody.owns.this"),
      folly::IOBuf::copyBuffer("p")));
  evb_.loopOnce(EVLOOP_NONBLOCK);

  // No child should have seen the request.
  EXPECT_EQ(childA_->received.size(), 0u);
  EXPECT_EQ(childB_->received.size(), 0u);

  auto responseFrame = getWrittenFrame();
  ASSERT_NE(responseFrame, nullptr)
      << "Expected framework-error frame on the wire";

  auto parsed = parseWrittenFrame(std::move(responseFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(parsed.streamId(), 1u);

  // The framework error body must decode to ResponseRpcError with
  // UNKNOWN_METHOD. Bodies are Compact-serialized regardless of the
  // request's negotiated metadata protocol.
  ASSERT_GT(parsed.dataSize(), 0u);
  auto dataBuf = std::move(parsed).extractData();
  ASSERT_NE(dataBuf, nullptr);
  auto rpcError = deserializeResponseRpcError(*dataBuf);
  EXPECT_EQ(
      *rpcError.code(), apache::thrift::ResponseRpcErrorCode::UNKNOWN_METHOD);
}

} // namespace apache::thrift::fast_thrift::thrift::server::test
