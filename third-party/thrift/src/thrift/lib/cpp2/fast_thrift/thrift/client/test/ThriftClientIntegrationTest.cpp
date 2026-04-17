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

#include <gtest/gtest.h>

#include <folly/futures/Promise.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/THeader.h>
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
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientErrorFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/adapter/ThriftClientTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientMetadataPushHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientRocketInterfaceHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/test/TestAsyncTransport.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::test::TestAllocator;
using apache::thrift::fast_thrift::transport::test::TestAsyncTransport;

// RSocket error codes (from ErrorDecoding.h)
constexpr uint32_t kErrorCodeApplicationError = 0x00000201;
constexpr uint32_t kErrorCodeRejected = 0x00000202;
constexpr uint32_t kErrorCodeCanceled = 0x00000203;
constexpr uint32_t kErrorCodeInvalid = 0x00000204;

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

namespace {

// Shared state that outlives the self-deleting MockRequestCallback.
struct CallbackState {
  bool responseReceived{false};
  bool errorReceived{false};
  folly::exception_wrapper error;
  uint16_t protocolId{0};
  apache::thrift::MessageType messageType{apache::thrift::MessageType::T_CALL};
  std::unique_ptr<folly::IOBuf> responseBuffer;

  std::string errorMessage() const {
    std::string msg;
    error.handle([&](const apache::thrift::TApplicationException& e) {
      msg = e.what();
    });
    return msg;
  }
};

class MockRequestCallback : public apache::thrift::RequestClientCallback {
 public:
  explicit MockRequestCallback(std::shared_ptr<CallbackState> state)
      : state_(std::move(state)) {}

  void onResponse(
      apache::thrift::ClientReceiveState&& state) noexcept override {
    state_->responseReceived = true;
    state_->protocolId = state.protocolId();
    state_->messageType = state.messageType();
    if (state.hasResponseBuffer()) {
      state_->responseBuffer =
          std::move(state).extractSerializedResponse().buffer;
    }
    delete this;
  }

  void onResponseError(folly::exception_wrapper e) noexcept override {
    state_->errorReceived = true;
    state_->error = std::move(e);
    delete this;
  }

 private:
  std::shared_ptr<CallbackState> state_;
};

std::pair<
    apache::thrift::RequestClientCallback::Ptr,
    std::shared_ptr<CallbackState>>
makeCallback() {
  auto state = std::make_shared<CallbackState>();
  auto* callback = new MockRequestCallback(state);
  return {apache::thrift::RequestClientCallback::Ptr(callback), state};
}

/**
 * Serialize ResponseRpcMetadata using Binary protocol.
 */
std::unique_ptr<folly::IOBuf> serializeResponseMetadata(
    const apache::thrift::ResponseRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

} // namespace

class ThriftClientChannelIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override { allocator_.reset(); }

  void TearDown() override {
    channel_.reset();
    pipeline_.reset();
    transportHandler_.reset();
    testTransport_ = nullptr;
  }

  /**
   * Set up the full pipeline with TestAsyncTransport.
   *
   * Creates:
   * - TestAsyncTransport for in-memory testing
   * - ThriftClientChannel owning the transport
   * - Full pipeline with all client handlers
   */
  void setupPipeline() {
    auto transport =
        folly::AsyncTransport::UniquePtr(new TestAsyncTransport(&evb_));
    testTransport_ = static_cast<TestAsyncTransport*>(transport.get());

    transportHandler_ =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            std::move(transport));
    channel_ = ThriftClientChannel::newChannel(&evb_);

    pipeline_ =
        PipelineBuilder<
            ThriftClientChannel,
            apache::thrift::fast_thrift::transport::TransportHandler,
            TestAllocator>()
            .setEventBase(&evb_)
            .setTail(transportHandler_.get())
            .setHead(channel_.get())
            .setAllocator(&allocator_)
            .addNextInbound<apache::thrift::fast_thrift::frame::read::handler::
                                FrameLengthParserHandler>(
                frame_length_parser_handler_tag)
            .addNextOutbound<apache::thrift::fast_thrift::frame::write::
                                 handler::FrameLengthEncoderHandler>(
                frame_length_encoder_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::client::
                               handler::RocketClientFrameCodecHandler>(
                rocket_client_frame_codec_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::client::
                               handler::RocketClientSetupFrameHandler>(
                rocket_client_setup_handler_tag,
                []() {
                  // Simple setup metadata factory
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
            .addNextDuplex<client::handler::ThriftClientRocketInterfaceHandler>(
                thrift_client_rocket_interface_handler_tag)
            .addNextInbound<client::handler::ThriftClientMetadataPushHandler>(
                thrift_client_metadata_push_handler_tag)
            .build();

    channel_->setPipeline(pipeline_.get());
    transportHandler_->setPipeline(*pipeline_);

    // Drive event loop to process the SETUP frame write callback
    evb_.loopOnce();

    // Discard the SETUP frame
    discardSetupFrame();
  }

  /**
   * Read and discard the SETUP frame sent on connect.
   * SETUP frame does not have the length prefix (it bypasses
   * FrameLengthEncoderHandler).
   */
  void discardSetupFrame() {
    auto setupFrame = testTransport_->getWrittenData();
    ASSERT_NE(setupFrame, nullptr) << "Expected SETUP frame on connect";
    // Just discard - don't parse/validate
  }

  /**
   * Get the next frame written to the transport.
   */
  std::unique_ptr<folly::IOBuf> getWrittenFrame() {
    return testTransport_->getWrittenData();
  }

  /**
   * Inject a frame as if it was received from the network.
   * Prepends the 3-byte length prefix required by FrameLengthParserHandler.
   */
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

  /**
   * Strip 3-byte length prefix and parse written frame.
   * Request frames (from app side) have the length prefix added by
   * FrameLengthEncoderHandler.
   */
  apache::thrift::fast_thrift::frame::read::ParsedFrame parseWrittenFrame(
      std::unique_ptr<folly::IOBuf> data) {
    folly::IOBufQueue queue;
    queue.append(std::move(data));
    queue.trimStart(apache::thrift::fast_thrift::frame::kMetadataLengthSize);
    return apache::thrift::fast_thrift::frame::read::parseFrame(queue.move());
  }

  /**
   * Create a PAYLOAD response frame.
   */
  std::unique_ptr<folly::IOBuf> createPayloadResponse(
      uint32_t streamId,
      std::unique_ptr<folly::IOBuf> metadata,
      std::unique_ptr<folly::IOBuf> data) {
    return apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::PayloadHeader{
            .streamId = streamId, .complete = true, .next = true},
        std::move(metadata),
        std::move(data));
  }

  /**
   * Create a simple ResponseRpcMetadata with basic fields.
   */
  apache::thrift::ResponseRpcMetadata createBasicResponseMetadata() {
    apache::thrift::ResponseRpcMetadata metadata;
    metadata.payloadMetadata().ensure().set_responseMetadata(
        apache::thrift::PayloadResponseMetadata{});
    return metadata;
  }

  /**
   * Helper to create serialized request data.
   */
  apache::thrift::SerializedRequest createSerializedRequest(
      const std::string& data) {
    return apache::thrift::SerializedRequest(folly::IOBuf::copyBuffer(data));
  }

  /**
   * Helper to create method metadata.
   */
  apache::thrift::MethodMetadata createMethodMetadata(
      const std::string& methodName) {
    return apache::thrift::MethodMetadata(methodName);
  }

  /**
   * Helper to create THeader.
   */
  std::shared_ptr<apache::thrift::transport::THeader> createHeader() {
    return std::make_shared<apache::thrift::transport::THeader>();
  }

  folly::EventBase evb_;
  TestAsyncTransport* testTransport_{nullptr};
  apache::thrift::fast_thrift::transport::TransportHandler::Ptr
      transportHandler_;
  ThriftClientChannel::UniquePtr channel_;
  PipelineImpl::Ptr pipeline_;
  TestAllocator allocator_;
};

// =============================================================================
// Request Path Tests - Verify outbound flow
// =============================================================================

TEST_F(
    ThriftClientChannelIntegrationTest, RequestFlowsThroughPipelineToSocket) {
  setupPipeline();

  // Send a request through the channel
  auto [cb, state] = makeCallback();
  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test payload"),
      createHeader(),
      std::move(cb),
      nullptr);

  // Drive event loop to flush writes
  evb_.loopOnce();

  // Read the frame from the test transport
  auto requestFrame = getWrittenFrame();
  ASSERT_NE(requestFrame, nullptr) << "Expected request frame to be written";

  // Parse and verify the frame (strip length prefix)
  auto parsed = parseWrittenFrame(std::move(requestFrame));
  ASSERT_TRUE(parsed.isValid()) << "Failed to parse request frame";
  EXPECT_EQ(
      parsed.type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(parsed.streamId(), 1u) << "First request should have stream ID 1";

  // Frame should have metadata (serialized RequestRpcMetadata)
  EXPECT_TRUE(parsed.hasMetadata());
  EXPECT_GT(parsed.metadataSize(), 0u);

  // Frame should have data (our test payload)
  EXPECT_GT(parsed.dataSize(), 0u);
}

TEST_F(
    ThriftClientChannelIntegrationTest, MultipleRequestsGetDistinctStreamIds) {
  setupPipeline();

  // Send first request
  auto [cb1, state1] = makeCallback();
  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("method1"),
      createSerializedRequest("payload1"),
      createHeader(),
      std::move(cb1),
      nullptr);

  evb_.loopOnce();
  auto frame1 = getWrittenFrame();
  ASSERT_NE(frame1, nullptr);
  auto parsed1 = parseWrittenFrame(std::move(frame1));
  ASSERT_TRUE(parsed1.isValid());
  EXPECT_EQ(parsed1.streamId(), 1u);

  // Send second request
  auto [cb2, state2] = makeCallback();
  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("method2"),
      createSerializedRequest("payload2"),
      createHeader(),
      std::move(cb2),
      nullptr);

  evb_.loopOnce();
  auto frame2 = getWrittenFrame();
  ASSERT_NE(frame2, nullptr);
  auto parsed2 = parseWrittenFrame(std::move(frame2));
  ASSERT_TRUE(parsed2.isValid());
  EXPECT_EQ(parsed2.streamId(), 3u) << "Second request should have stream ID 3";
}

// =============================================================================
// Response Path Tests - Verify inbound flow
// =============================================================================

TEST_F(
    ThriftClientChannelIntegrationTest,
    ResponseFlowsFromSocketThroughPipeline) {
  setupPipeline();

  // Send a request
  auto [cb, state] = makeCallback();

  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  evb_.loopOnce();

  // Read and parse the request to get stream ID
  auto requestFrame = getWrittenFrame();
  ASSERT_NE(requestFrame, nullptr);
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  ASSERT_TRUE(parsedRequest.isValid());
  uint32_t streamId = parsedRequest.streamId();
  EXPECT_EQ(streamId, 1u);

  // Create and inject a response
  auto responseMetadata = createBasicResponseMetadata();
  auto serializedMetadata = serializeResponseMetadata(responseMetadata);
  auto responseData = folly::IOBuf::copyBuffer("response payload");

  auto responseFrame = createPayloadResponse(
      streamId, std::move(serializedMetadata), std::move(responseData));

  injectFrame(std::move(responseFrame));

  // Drive event loop to process the incoming response
  evb_.loopOnce();
  evb_.loopOnce(); // May need multiple iterations

  // Verify callback received the response
  EXPECT_TRUE(state->responseReceived)
      << "Callback should have received response";
  EXPECT_FALSE(state->errorReceived);
  EXPECT_EQ(state->protocolId, apache::thrift::protocol::T_COMPACT_PROTOCOL);
  EXPECT_EQ(state->messageType, apache::thrift::MessageType::T_REPLY);
}

TEST_F(
    ThriftClientChannelIntegrationTest,
    ResponseRoutedToCorrectCallbackWithMultiplePendingRequests) {
  setupPipeline();

  // Send two requests
  auto [cb1, state1] = makeCallback();
  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("method1"),
      createSerializedRequest("req1"),
      createHeader(),
      std::move(cb1),
      nullptr);

  evb_.loopOnce();
  auto frame1 = getWrittenFrame();
  auto parsed1 = parseWrittenFrame(std::move(frame1));
  uint32_t streamId1 = parsed1.streamId();

  auto [cb2, state2] = makeCallback();
  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("method2"),
      createSerializedRequest("req2"),
      createHeader(),
      std::move(cb2),
      nullptr);

  evb_.loopOnce();
  auto frame2 = getWrittenFrame();
  auto parsed2 = parseWrittenFrame(std::move(frame2));
  uint32_t streamId2 = parsed2.streamId();

  // Respond to second request first (out of order)
  auto metadata2 = createBasicResponseMetadata();
  auto response2 = createPayloadResponse(
      streamId2,
      serializeResponseMetadata(metadata2),
      folly::IOBuf::copyBuffer("response2"));
  injectFrame(std::move(response2));
  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_FALSE(state1->responseReceived);
  EXPECT_TRUE(state2->responseReceived);

  // Now respond to first request
  auto metadata1 = createBasicResponseMetadata();
  auto response1 = createPayloadResponse(
      streamId1,
      serializeResponseMetadata(metadata1),
      folly::IOBuf::copyBuffer("response1"));
  injectFrame(std::move(response1));
  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_TRUE(state1->responseReceived);
  EXPECT_TRUE(state2->responseReceived);
}

// =============================================================================
// Error Response Tests
// =============================================================================

TEST_F(ThriftClientChannelIntegrationTest, ErrorFrameWithApplicationErrorCode) {
  setupPipeline();

  auto [cb, state] = makeCallback();

  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  // APPLICATION_ERROR (0x00000201) - generic error, no ResponseRpcError parsing
  auto errorFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId, .errorCode = kErrorCodeApplicationError},
      nullptr,
      folly::IOBuf::copyBuffer("Application error occurred"));

  injectFrame(std::move(errorFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_TRUE(state->errorReceived);
  EXPECT_FALSE(state->responseReceived);
}

/**
 * Serialize ResponseRpcError using Compact protocol.
 * (must match the CompactProtocolReader in ErrorDecoding.h)
 */
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

TEST_F(ThriftClientChannelIntegrationTest, ErrorFrameWithRejectedCode) {
  setupPipeline();

  auto [cb, state] = makeCallback();

  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  // REJECTED (0x00000202) - should parse ResponseRpcError from payload
  auto errorPayload = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::OVERLOAD, "Server overloaded");

  auto errorFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId, .errorCode = kErrorCodeRejected},
      nullptr,
      std::move(errorPayload));

  injectFrame(std::move(errorFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_TRUE(state->errorReceived);
  EXPECT_FALSE(state->responseReceived);
  EXPECT_EQ(state->errorMessage(), "Server overloaded");
}

TEST_F(ThriftClientChannelIntegrationTest, ErrorFrameWithCanceledCode) {
  setupPipeline();

  auto [cb, state] = makeCallback();

  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  // CANCELED (0x00000203) - should parse ResponseRpcError from payload
  auto errorPayload = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::TASK_EXPIRED, "Request canceled");

  auto errorFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId, .errorCode = kErrorCodeCanceled},
      nullptr,
      std::move(errorPayload));

  injectFrame(std::move(errorFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_TRUE(state->errorReceived);
  EXPECT_FALSE(state->responseReceived);
  EXPECT_EQ(state->errorMessage(), "Request canceled");
}

TEST_F(ThriftClientChannelIntegrationTest, ErrorFrameWithInvalidCode) {
  setupPipeline();

  auto [cb, state] = makeCallback();

  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  // INVALID (0x00000204) - should parse ResponseRpcError from payload
  auto errorPayload = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::UNKNOWN_METHOD, "Method not found");

  auto errorFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId, .errorCode = kErrorCodeInvalid},
      nullptr,
      std::move(errorPayload));

  injectFrame(std::move(errorFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_TRUE(state->errorReceived);
  EXPECT_FALSE(state->responseReceived);
  EXPECT_EQ(state->errorMessage(), "Method not found");
}

// =============================================================================
// Exception Metadata Tests
// =============================================================================

TEST_F(ThriftClientChannelIntegrationTest, DeclaredExceptionInPayloadMetadata) {
  setupPipeline();

  auto [cb, state] = makeCallback();

  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  // Create metadata with declared exception
  apache::thrift::ResponseRpcMetadata metadata;
  apache::thrift::PayloadExceptionMetadataBase exMeta;
  exMeta.name_utf8() = "MyServiceException";
  exMeta.what_utf8() = "Something went wrong";
  apache::thrift::PayloadExceptionMetadata payloadExMeta;
  payloadExMeta.set_declaredException(
      apache::thrift::PayloadDeclaredExceptionMetadata{});
  exMeta.metadata() = std::move(payloadExMeta);
  metadata.payloadMetadata().ensure().set_exceptionMetadata(std::move(exMeta));

  auto serializedMetadata = serializeResponseMetadata(metadata);
  auto responseData = folly::IOBuf::copyBuffer("exception payload");

  auto responseFrame = createPayloadResponse(
      streamId, std::move(serializedMetadata), std::move(responseData));

  injectFrame(std::move(responseFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  // Declared exceptions are NOT errors - they're delivered as responses
  EXPECT_TRUE(state->responseReceived);
  EXPECT_FALSE(state->errorReceived);
  EXPECT_EQ(state->messageType, apache::thrift::MessageType::T_REPLY);
}

TEST_F(
    ThriftClientChannelIntegrationTest, UndeclaredExceptionInPayloadMetadata) {
  setupPipeline();

  auto [cb, state] = makeCallback();

  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  // Create metadata with undeclared (DEPRECATED_PROXY) exception
  apache::thrift::ResponseRpcMetadata metadata;
  apache::thrift::PayloadExceptionMetadataBase exMeta;
  exMeta.name_utf8() = "std::runtime_error";
  exMeta.what_utf8() = "Unexpected error";
  apache::thrift::PayloadExceptionMetadata payloadExMeta;
  payloadExMeta.set_DEPRECATED_proxyException(
      apache::thrift::PayloadProxyExceptionMetadata{});
  exMeta.metadata() = std::move(payloadExMeta);
  metadata.payloadMetadata().ensure().set_exceptionMetadata(std::move(exMeta));

  auto serializedMetadata = serializeResponseMetadata(metadata);
  auto responseData = folly::IOBuf::copyBuffer("exception payload");

  auto responseFrame = createPayloadResponse(
      streamId, std::move(serializedMetadata), std::move(responseData));

  injectFrame(std::move(responseFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  // Undeclared exceptions are errors
  EXPECT_TRUE(state->errorReceived);
  EXPECT_FALSE(state->responseReceived);
  EXPECT_EQ(state->errorMessage(), "Unexpected error");
}

// =============================================================================
// Edge Case Tests
// =============================================================================

TEST_F(ThriftClientChannelIntegrationTest, ResponseWithEmptyPayload) {
  setupPipeline();

  auto [cb, state] = makeCallback();

  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  // Create response with metadata but empty data
  auto responseMetadata = createBasicResponseMetadata();
  auto serializedMetadata = serializeResponseMetadata(responseMetadata);

  auto responseFrame = createPayloadResponse(
      streamId,
      std::move(serializedMetadata),
      nullptr); // No data

  injectFrame(std::move(responseFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_TRUE(state->responseReceived);
  EXPECT_FALSE(state->errorReceived);
}

TEST_F(ThriftClientChannelIntegrationTest, ResponseForUnknownStreamId) {
  setupPipeline();

  // Send a request to establish a valid stream
  auto [cb, state] = makeCallback();

  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  evb_.loopOnce();
  getWrittenFrame(); // Discard the request frame

  // Inject a response for an unknown stream ID (stream 99 doesn't exist)
  auto responseMetadata = createBasicResponseMetadata();
  auto serializedMetadata = serializeResponseMetadata(responseMetadata);
  auto responseData = folly::IOBuf::copyBuffer("orphan response");

  auto responseFrame = createPayloadResponse(
      99, std::move(serializedMetadata), std::move(responseData));

  injectFrame(std::move(responseFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  // Original callback should NOT receive this response
  EXPECT_FALSE(state->responseReceived);
  EXPECT_FALSE(state->errorReceived);
}

TEST_F(
    ThriftClientChannelIntegrationTest, THeaderPopulatedFromResponseMetadata) {
  setupPipeline();

  auto [cb, state] = makeCallback();

  channel_->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  // Create response with custom headers in metadata
  apache::thrift::ResponseRpcMetadata metadata;
  metadata.payloadMetadata().ensure().set_responseMetadata(
      apache::thrift::PayloadResponseMetadata{});
  metadata.otherMetadata().ensure()["custom-header"] = "custom-value";

  auto serializedMetadata = serializeResponseMetadata(metadata);
  auto responseData = folly::IOBuf::copyBuffer("response");

  auto responseFrame = createPayloadResponse(
      streamId, std::move(serializedMetadata), std::move(responseData));

  injectFrame(std::move(responseFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_TRUE(state->responseReceived);
  EXPECT_FALSE(state->errorReceived);
  // Note: Full THeader verification would require accessing the header from
  // ClientReceiveState, which the MockRequestCallback currently doesn't expose.
  // This test verifies the response completes successfully with custom headers.
}

} // namespace apache::thrift::fast_thrift::thrift

// =============================================================================
// ThriftClientAppAdapter Integration Tests
// =============================================================================

namespace apache::thrift::fast_thrift::thrift {

namespace {

/**
 * IntegrationTestClient - holds ThriftClientAppAdapter as a member.
 * Demonstrates the composition pattern for generated clients.
 */
class IntegrationTestClient {
 public:
  IntegrationTestClient()
      : adapter_(ThriftClientAppAdapter::Ptr(new ThriftClientAppAdapter())) {}

  ThriftClientAppAdapter& adapter() { return *adapter_; }

  /**
   * Send a benchmark-style request through the adapter.
   * Returns a promise/future pair so the test can wait for the response.
   */
  folly::SemiFuture<ThriftResponseMessage> sendBenchRequest(
      std::unique_ptr<folly::IOBuf> metadata,
      std::unique_ptr<folly::IOBuf> data) {
    auto [promise, future] =
        folly::makePromiseContract<ThriftResponseMessage>();

    ThriftRequestMessage msg{
        .payload = ThriftRequestPayload{
            .metadata = std::move(metadata),
            .data = std::move(data),
            .rpcKind = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
            .complete = true}};

    adapter_->write(
        [promise = std::move(promise)](
            folly::Expected<ThriftResponseMessage, folly::exception_wrapper>&&
                result) mutable noexcept {
          if (result.hasError()) {
            promise.setException(std::move(result.error()));
          } else {
            promise.setValue(std::move(result.value()));
          }
        },
        erase_and_box(std::move(msg)));

    return std::move(future);
  }

 private:
  ThriftClientAppAdapter::Ptr adapter_;
};

} // namespace

class ThriftClientAppAdapterIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override { allocator_.reset(); }

  void TearDown() override {
    pipeline_.reset();
    transportAdapter_.reset();
    testTransport_ = nullptr;
  }

  void setupPipeline() {
    auto transport =
        folly::AsyncTransport::UniquePtr(new TestAsyncTransport(&evb_));
    testTransport_ = static_cast<TestAsyncTransport*>(transport.get());

    // Build rocket pipeline inside a transport connection
    auto connection =
        std::make_unique<rocket::client::RocketClientConnection>();

    connection->transportHandler =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            std::move(transport));

    // Save raw pointer for onConnect() (called after ownership transfer)
    auto* transportHandlerPtr = connection->transportHandler.get();

    connection->pipeline =
        PipelineBuilder<
            apache::thrift::fast_thrift::rocket::client::RocketClientAppAdapter,
            apache::thrift::fast_thrift::transport::TransportHandler,
            apache::thrift::fast_thrift::channel_pipeline::
                SimpleBufferAllocator>()
            .setEventBase(&evb_)
            .setHead(connection->appAdapter.get())
            .setTail(connection->transportHandler.get())
            .setAllocator(&connection->allocator)
            .addNextInbound<apache::thrift::fast_thrift::frame::read::handler::
                                FrameLengthParserHandler>(
                frame_length_parser_handler_tag)
            .addNextOutbound<apache::thrift::fast_thrift::frame::write::
                                 handler::FrameLengthEncoderHandler>(
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
            .build();

    connection->appAdapter->setPipeline(connection->pipeline.get());
    connection->transportHandler->setPipeline(*connection->pipeline);

    // Create transport adapter (takes ownership of connection)
    transportAdapter_ = std::make_unique<client::ThriftClientTransportAdapter>(
        std::move(connection));

    // Build thrift pipeline
    pipeline_ =
        PipelineBuilder<
            ThriftClientAppAdapter,
            client::ThriftClientTransportAdapter,
            TestAllocator>()
            .setEventBase(&evb_)
            .setHead(&client_.adapter())
            .setTail(transportAdapter_.get())
            .setAllocator(&allocator_)
            .addNextInbound<client::handler::ThriftClientMetadataPushHandler>(
                thrift_client_metadata_push_handler_tag)
            .build();

    client_.adapter().setPipeline(pipeline_.get());
    transportAdapter_->setPipeline(pipeline_.get());
    transportHandlerPtr->onConnect();

    evb_.loopOnce();
    discardSetupFrame();
  }

  void discardSetupFrame() {
    auto setupFrame = testTransport_->getWrittenData();
    ASSERT_NE(setupFrame, nullptr) << "Expected SETUP frame on connect";
  }

  std::unique_ptr<folly::IOBuf> getWrittenFrame() {
    return testTransport_->getWrittenData();
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

  apache::thrift::fast_thrift::frame::read::ParsedFrame parseWrittenFrame(
      std::unique_ptr<folly::IOBuf> data) {
    folly::IOBufQueue queue;
    queue.append(std::move(data));
    queue.trimStart(apache::thrift::fast_thrift::frame::kMetadataLengthSize);
    return apache::thrift::fast_thrift::frame::read::parseFrame(queue.move());
  }

  std::unique_ptr<folly::IOBuf> createPayloadResponse(
      uint32_t streamId,
      std::unique_ptr<folly::IOBuf> metadata,
      std::unique_ptr<folly::IOBuf> data) {
    return apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::PayloadHeader{
            .streamId = streamId, .complete = true, .next = true},
        std::move(metadata),
        std::move(data));
  }

  apache::thrift::ResponseRpcMetadata createBasicResponseMetadata() {
    apache::thrift::ResponseRpcMetadata metadata;
    metadata.payloadMetadata().ensure().set_responseMetadata(
        apache::thrift::PayloadResponseMetadata{});
    return metadata;
  }

  folly::EventBase evb_;
  TestAsyncTransport* testTransport_{nullptr};
  std::unique_ptr<client::ThriftClientTransportAdapter> transportAdapter_;
  IntegrationTestClient client_;
  PipelineImpl::Ptr pipeline_;
  TestAllocator allocator_;
};

// =============================================================================
// Request/Response Flow
// =============================================================================

TEST_F(
    ThriftClientAppAdapterIntegrationTest,
    RequestFlowsThroughPipelineToSocket) {
  setupPipeline();

  auto metadata = createBasicResponseMetadata();
  auto serializedMetadata = serializeResponseMetadata(metadata);

  auto future = client_.sendBenchRequest(
      std::move(serializedMetadata), folly::IOBuf::copyBuffer("test payload"));

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  ASSERT_NE(requestFrame, nullptr) << "Expected request frame to be written";

  auto parsed = parseWrittenFrame(std::move(requestFrame));
  ASSERT_TRUE(parsed.isValid()) << "Failed to parse request frame";
  EXPECT_EQ(
      parsed.type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
}

TEST_F(
    ThriftClientAppAdapterIntegrationTest,
    ResponseFlowsFromSocketThroughPipeline) {
  setupPipeline();

  auto metadata = createBasicResponseMetadata();
  auto serializedMetadata = serializeResponseMetadata(metadata);

  auto future = client_.sendBenchRequest(
      serializedMetadata->clone(), folly::IOBuf::copyBuffer("test"));

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  ASSERT_NE(requestFrame, nullptr);
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  auto responseFrame = createPayloadResponse(
      streamId,
      serializeResponseMetadata(metadata),
      folly::IOBuf::copyBuffer("response payload"));

  injectFrame(std::move(responseFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  ASSERT_TRUE(future.isReady());
  auto response = std::move(future).value();
  EXPECT_EQ(
      response.requestFrameType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
}

// =============================================================================
// Error Handling
// =============================================================================

TEST_F(ThriftClientAppAdapterIntegrationTest, ErrorFramePropagatesException) {
  setupPipeline();

  auto metadata = createBasicResponseMetadata();
  auto serializedMetadata = serializeResponseMetadata(metadata);

  auto future = client_.sendBenchRequest(
      std::move(serializedMetadata), folly::IOBuf::copyBuffer("test"));

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  auto errorFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId, .errorCode = kErrorCodeApplicationError},
      nullptr,
      folly::IOBuf::copyBuffer("Application error occurred"));

  injectFrame(std::move(errorFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  // The adapter delivers the response (including error frames) to the handler.
  // Error decoding is the client's responsibility, not the adapter's.
  ASSERT_TRUE(future.isReady());
}

TEST_F(
    ThriftClientAppAdapterIntegrationTest,
    OnExceptionPropagatesExceptionToAllPending) {
  setupPipeline();

  auto metadata = createBasicResponseMetadata();

  auto future1 = client_.sendBenchRequest(
      serializeResponseMetadata(metadata), folly::IOBuf::copyBuffer("req1"));

  auto future2 = client_.sendBenchRequest(
      serializeResponseMetadata(metadata), folly::IOBuf::copyBuffer("req2"));

  evb_.loopOnce();
  getWrittenFrame();
  evb_.loopOnce();
  getWrittenFrame();

  client_.adapter().onException(
      folly::make_exception_wrapper<std::runtime_error>("connection lost"));

  ASSERT_TRUE(future1.isReady());
  ASSERT_TRUE(future2.isReady());
  EXPECT_TRUE(future1.result().hasException());
  EXPECT_TRUE(future2.result().hasException());
}

} // namespace apache::thrift::fast_thrift::thrift
