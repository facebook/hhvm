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

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientChannel.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace apache::thrift::fast_thrift::thrift {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
using apache::thrift::fast_thrift::channel_pipeline::test::MockHandler;
using MockTransportHandler =
    apache::thrift::fast_thrift::channel_pipeline::test::MockHeadHandler;
using apache::thrift::fast_thrift::channel_pipeline::test::TestAllocator;

HANDLER_TAG(test_handler);

namespace {

// Shared state that outlives the self-deleting MockRequestCallback.
// Tests assert against this after the callback fires and deletes itself.
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

// Self-deleting mock per Thrift's RequestClientCallback contract:
// after onResponse/onResponseError, the callback must not be accessed again.
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

} // namespace

class ThriftClientChannelTest : public ::testing::Test {
 protected:
  void SetUp() override {
    MockHandler::resetOrderCounter();
    allocator_.reset();
    mockTransport_.setOnWriteCallback(
        [](channel_pipeline::TypeErasedBox&&) { return Result::Success; });
  }

  ThriftClientChannel::UniquePtr createChannel() {
    return ThriftClientChannel::newChannel(&evb_);
  }

  ThriftClientChannel::UniquePtr createChannelWithProtocol(
      uint16_t protocolId) {
    return ThriftClientChannel::newChannel(&evb_, protocolId);
  }

  PipelineImpl::Ptr buildPipelineWithHandler(
      ThriftClientChannel* channel,
      std::function<Result(
          apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&&)> writeHandler) {
    handler_ = std::make_unique<MockHandler>();
    handler_->setOnWrite(std::move(writeHandler));

    return PipelineBuilder<
               MockTransportHandler,
               ThriftClientChannel,
               TestAllocator>()
        .setEventBase(&evb_)
        .setHead(&mockTransport_)
        .setTail(channel)
        .setAllocator(&allocator_)
        .addNextDuplex<MockHandler>(test_handler_tag, std::move(handler_))
        .build();
  }

  apache::thrift::SerializedRequest createSerializedRequest(
      const std::string& data) {
    return apache::thrift::SerializedRequest(folly::IOBuf::copyBuffer(data));
  }

  apache::thrift::MethodMetadata createMethodMetadata(
      const std::string& methodName) {
    return apache::thrift::MethodMetadata(methodName);
  }

  std::shared_ptr<apache::thrift::transport::THeader> createHeader() {
    return std::make_shared<apache::thrift::transport::THeader>();
  }

  ThriftResponseMessage createSuccessResponse(
      uint32_t requestHandle,
      uint16_t /* protocolId */,
      apache::thrift::MessageType /* mtype */,
      const std::string& data) {
    apache::thrift::ResponseRpcMetadata metadata;
    metadata.payloadMetadata().ensure().set_responseMetadata(
        apache::thrift::PayloadResponseMetadata{});

    apache::thrift::BinaryProtocolWriter writer;
    folly::IOBufQueue metadataQueue(folly::IOBufQueue::cacheChainLength());
    writer.setOutput(&metadataQueue);
    metadata.write(&writer);
    auto serializedMetadata = metadataQueue.move();

    auto frame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::PayloadHeader{
            .streamId = 1,
            .follows = false,
            .complete = true,
            .next = true,
        },
        std::move(serializedMetadata),
        folly::IOBuf::copyBuffer(data));

    ThriftResponseMessage response;
    response.frame =
        apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));
    response.requestHandle = requestHandle;
    response.streamType =
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE;
    return response;
  }

  folly::EventBase evb_;
  MockTransportHandler mockTransport_;
  TestAllocator allocator_;
  std::unique_ptr<MockHandler> handler_;
};

// =============================================================================
// Factory Method and Construction
// =============================================================================

TEST_F(ThriftClientChannelTest, NewChannelReturnsValidChannel) {
  auto channel = createChannel();
  EXPECT_NE(channel, nullptr);
}

TEST_F(ThriftClientChannelTest, NewChannelSetsDefaultProtocolId) {
  auto channel = createChannel();
  EXPECT_EQ(
      channel->getProtocolId(), apache::thrift::protocol::T_COMPACT_PROTOCOL);
}

TEST_F(ThriftClientChannelTest, NewChannelWithCustomProtocolId) {
  auto channel =
      createChannelWithProtocol(apache::thrift::protocol::T_BINARY_PROTOCOL);
  EXPECT_EQ(
      channel->getProtocolId(), apache::thrift::protocol::T_BINARY_PROTOCOL);
}

TEST_F(ThriftClientChannelTest, GetEventBaseReturnsCorrectEventBase) {
  auto channel = createChannel();
  EXPECT_EQ(channel->getEventBase(), &evb_);
}

// =============================================================================
// Request Sending - No Pipeline
// =============================================================================

TEST_F(ThriftClientChannelTest, SendRequestWithoutPipelineReturnsError) {
  auto channel = createChannel();

  auto [cb, state] = makeCallback();

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test data"),
      createHeader(),
      std::move(cb),
      nullptr);

  EXPECT_TRUE(state->errorReceived);
  EXPECT_EQ(state->errorMessage(), "Pipeline not set");
}

// =============================================================================
// Response Handling - onMessage
// =============================================================================

TEST_F(ThriftClientChannelTest, OnMessageInvokesCallbackWithCorrectState) {
  auto channel = createChannel();
  uint32_t capturedHandle{};

  auto pipeline = buildPipelineWithHandler(
      channel.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& msg) {
        auto& request = msg.get<ThriftRequestMessage>();
        capturedHandle = request.requestHandle;
        return Result::Success;
      });
  channel->setPipeline(pipeline.get());

  auto [cb, state] = makeCallback();

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  auto response = createSuccessResponse(
      capturedHandle,
      apache::thrift::protocol::T_BINARY_PROTOCOL,
      apache::thrift::MessageType::T_REPLY,
      "response payload");

  auto result = channel->onRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(state->responseReceived);
  EXPECT_FALSE(state->errorReceived);
  EXPECT_EQ(state->protocolId, apache::thrift::protocol::T_COMPACT_PROTOCOL);
  EXPECT_EQ(state->messageType, apache::thrift::MessageType::T_REPLY);
}

TEST_F(ThriftClientChannelTest, OnMessageWithErrorInvokesErrorCallback) {
  auto channel = createChannel();
  uint32_t capturedHandle{};

  auto pipeline = buildPipelineWithHandler(
      channel.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& msg) {
        auto& request = msg.get<ThriftRequestMessage>();
        capturedHandle = request.requestHandle;
        return Result::Success;
      });
  channel->setPipeline(pipeline.get());

  auto [cb, state] = makeCallback();

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  // Send an ERROR frame response
  auto errorFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = 1, .errorCode = 0x00000201},
      nullptr,
      folly::IOBuf::copyBuffer("test error"));
  ThriftResponseMessage errorResponse;
  errorResponse.frame = apache::thrift::fast_thrift::frame::read::parseFrame(
      std::move(errorFrame));
  errorResponse.requestHandle = capturedHandle;
  errorResponse.streamType =
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE;

  std::ignore = channel->onRead(erase_and_box(std::move(errorResponse)));

  EXPECT_FALSE(state->responseReceived);
  EXPECT_TRUE(state->errorReceived);
  EXPECT_TRUE(
      state->error.is_compatible_with<apache::thrift::TApplicationException>());
}

TEST_F(ThriftClientChannelTest, OnReadWithUnknownRequestIdIsDropped) {
  auto channel = createChannel();

  auto pipeline = buildPipelineWithHandler(
      channel.get(),
      [](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
         TypeErasedBox&&) { return Result::Success; });
  channel->setPipeline(pipeline.get());

  auto response = createSuccessResponse(
      99999,
      apache::thrift::protocol::T_COMPACT_PROTOCOL,
      apache::thrift::MessageType::T_REPLY,
      "orphan response");

  auto result = channel->onRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
}

TEST_F(ThriftClientChannelTest, MultiplePendingCallbacksRoutedCorrectly) {
  auto channel = createChannel();
  std::vector<uint32_t> capturedHandles;

  auto pipeline = buildPipelineWithHandler(
      channel.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& msg) {
        auto& request = msg.get<ThriftRequestMessage>();
        capturedHandles.push_back(request.requestHandle);
        return Result::Success;
      });
  channel->setPipeline(pipeline.get());

  auto [cb1, state1] = makeCallback();
  auto [cb2, state2] = makeCallback();
  auto [cb3, state3] = makeCallback();

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("method1"),
      createSerializedRequest("req1"),
      createHeader(),
      std::move(cb1),
      nullptr);

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("method2"),
      createSerializedRequest("req2"),
      createHeader(),
      std::move(cb2),
      nullptr);

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("method3"),
      createSerializedRequest("req3"),
      createHeader(),
      std::move(cb3),
      nullptr);

  ASSERT_EQ(capturedHandles.size(), 3);

  // Respond out of order: 2, 3, 1
  auto response2 = createSuccessResponse(
      capturedHandles[1],
      apache::thrift::protocol::T_COMPACT_PROTOCOL,
      apache::thrift::MessageType::T_REPLY,
      "resp2");
  std::ignore = channel->onRead(erase_and_box(std::move(response2)));

  EXPECT_FALSE(state1->responseReceived);
  EXPECT_TRUE(state2->responseReceived);
  EXPECT_FALSE(state3->responseReceived);

  auto response3 = createSuccessResponse(
      capturedHandles[2],
      apache::thrift::protocol::T_COMPACT_PROTOCOL,
      apache::thrift::MessageType::T_REPLY,
      "resp3");
  std::ignore = channel->onRead(erase_and_box(std::move(response3)));

  EXPECT_FALSE(state1->responseReceived);
  EXPECT_TRUE(state2->responseReceived);
  EXPECT_TRUE(state3->responseReceived);

  auto response1 = createSuccessResponse(
      capturedHandles[0],
      apache::thrift::protocol::T_COMPACT_PROTOCOL,
      apache::thrift::MessageType::T_REPLY,
      "resp1");
  std::ignore = channel->onRead(erase_and_box(std::move(response1)));

  EXPECT_TRUE(state1->responseReceived);
  EXPECT_TRUE(state2->responseReceived);
  EXPECT_TRUE(state3->responseReceived);
}

// =============================================================================
// Pipeline Write - Success Path
// =============================================================================

TEST_F(ThriftClientChannelTest, SendRequestWithPipelineCallsHandler) {
  auto channel = createChannel();
  bool handlerCalled = false;

  auto pipeline = buildPipelineWithHandler(
      channel.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& msg) {
        handlerCalled = true;
        auto& request = msg.get<ThriftRequestMessage>();
        EXPECT_EQ(
            request.payload.rpcKind,
            apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
        return Result::Success;
      });
  channel->setPipeline(pipeline.get());

  auto [cb, state] = makeCallback();

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("request payload"),
      createHeader(),
      std::move(cb),
      nullptr);

  EXPECT_TRUE(handlerCalled);
}

// =============================================================================
// Pipeline Write - Error Handling
// =============================================================================

TEST_F(ThriftClientChannelTest, SendRequestWithPipelineErrorInvokesCallback) {
  auto channel = createChannel();

  auto pipeline = buildPipelineWithHandler(
      channel.get(),
      [](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
         TypeErasedBox&&) { return Result::Error; });
  channel->setPipeline(pipeline.get());

  auto [cb, state] = makeCallback();

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  EXPECT_TRUE(state->errorReceived);
  EXPECT_EQ(state->errorMessage(), "Pipeline write failed");
}

TEST_F(ThriftClientChannelTest, SendRequestWithPipelineBackpressureProceeds) {
  auto channel = createChannel();

  bool handlerCalled = false;

  auto pipeline = buildPipelineWithHandler(
      channel.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& msg) {
        handlerCalled = true;
        auto& request = msg.get<ThriftRequestMessage>();
        EXPECT_EQ(
            request.payload.rpcKind,
            apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
        return Result::Backpressure;
      });
  channel->setPipeline(pipeline.get());

  auto [cb, state] = makeCallback();

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("testMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  EXPECT_TRUE(handlerCalled);
  EXPECT_FALSE(state->errorReceived);
  EXPECT_FALSE(state->responseReceived);
}

// =============================================================================
// Pipeline Write - Message Content Verification
// =============================================================================

TEST_F(ThriftClientChannelTest, SendRequestPassesCorrectMessageContent) {
  auto channel = createChannel();
  apache::thrift::RpcKind capturedRpcKind{};
  std::string capturedMethodName;
  uint32_t capturedHandle{};

  auto pipeline = buildPipelineWithHandler(
      channel.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& msg) {
        auto& request = msg.get<ThriftRequestMessage>();
        capturedRpcKind = request.payload.rpcKind;
        // Deserialize the pre-serialized metadata to verify contents
        apache::thrift::RequestRpcMetadata rpcMetadata;
        apache::thrift::BinaryProtocolReader reader;
        reader.setInput(request.payload.metadata.get());
        rpcMetadata.read(&reader);
        capturedMethodName = std::string(rpcMetadata.name()->view());
        capturedHandle = request.requestHandle;
        return Result::Success;
      });
  channel->setPipeline(pipeline.get());

  auto [cb, state] = makeCallback();

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("myTestMethod"),
      createSerializedRequest("test"),
      createHeader(),
      std::move(cb),
      nullptr);

  EXPECT_EQ(
      capturedRpcKind, apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  EXPECT_EQ(capturedMethodName, "myTestMethod");
  EXPECT_EQ(capturedHandle, 0);
}

// =============================================================================
// onException - Pipeline Exception Handling
// =============================================================================

TEST_F(ThriftClientChannelTest, OnExceptionFailsAllPendingCallbacks) {
  auto channel = createChannel();
  std::vector<uint32_t> capturedHandles;

  auto pipeline = buildPipelineWithHandler(
      channel.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& msg) {
        auto& request = msg.get<ThriftRequestMessage>();
        capturedHandles.push_back(request.requestHandle);
        return Result::Success;
      });
  channel->setPipeline(pipeline.get());

  auto [cb1, state1] = makeCallback();
  auto [cb2, state2] = makeCallback();
  auto [cb3, state3] = makeCallback();

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("method1"),
      createSerializedRequest("req1"),
      createHeader(),
      std::move(cb1),
      nullptr);

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("method2"),
      createSerializedRequest("req2"),
      createHeader(),
      std::move(cb2),
      nullptr);

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("method3"),
      createSerializedRequest("req3"),
      createHeader(),
      std::move(cb3),
      nullptr);

  ASSERT_EQ(capturedHandles.size(), 3);

  // Fire exception - all callbacks should receive the error
  auto error =
      folly::make_exception_wrapper<std::runtime_error>("connection lost");
  channel->onException(std::move(error));

  // All callbacks should have received errors
  EXPECT_TRUE(state1->errorReceived);
  EXPECT_TRUE(state2->errorReceived);
  EXPECT_TRUE(state3->errorReceived);

  // Verify the error type
  EXPECT_TRUE(state1->error.is_compatible_with<std::runtime_error>());
  EXPECT_TRUE(state2->error.is_compatible_with<std::runtime_error>());
  EXPECT_TRUE(state3->error.is_compatible_with<std::runtime_error>());
}

TEST_F(ThriftClientChannelTest, OnExceptionWithNoPendingCallbacksIsNoop) {
  auto channel = createChannel();

  auto pipeline = buildPipelineWithHandler(
      channel.get(),
      [](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
         TypeErasedBox&&) { return Result::Success; });
  channel->setPipeline(pipeline.get());

  // No pending requests - onException should not crash
  auto error =
      folly::make_exception_wrapper<std::runtime_error>("connection lost");
  channel->onException(std::move(error));
}

TEST_F(ThriftClientChannelTest, OnExceptionAfterSomeResponsesReceived) {
  auto channel = createChannel();
  std::vector<uint32_t> capturedHandles;

  auto pipeline = buildPipelineWithHandler(
      channel.get(),
      [&](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
          TypeErasedBox&& msg) {
        auto& request = msg.get<ThriftRequestMessage>();
        capturedHandles.push_back(request.requestHandle);
        return Result::Success;
      });
  channel->setPipeline(pipeline.get());

  auto [cb1, state1] = makeCallback();
  auto [cb2, state2] = makeCallback();
  auto [cb3, state3] = makeCallback();

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("method1"),
      createSerializedRequest("req1"),
      createHeader(),
      std::move(cb1),
      nullptr);

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("method2"),
      createSerializedRequest("req2"),
      createHeader(),
      std::move(cb2),
      nullptr);

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("method3"),
      createSerializedRequest("req3"),
      createHeader(),
      std::move(cb3),
      nullptr);

  ASSERT_EQ(capturedHandles.size(), 3);

  // Complete callback2 with a successful response
  auto response2 = createSuccessResponse(
      capturedHandles[1],
      apache::thrift::protocol::T_COMPACT_PROTOCOL,
      apache::thrift::MessageType::T_REPLY,
      "resp2");
  std::ignore = channel->onRead(erase_and_box(std::move(response2)));

  EXPECT_TRUE(state2->responseReceived);
  EXPECT_FALSE(state2->errorReceived);

  // Fire exception - only callback1 and callback3 should receive errors
  auto error =
      folly::make_exception_wrapper<std::runtime_error>("connection lost");
  channel->onException(std::move(error));

  // callback1 and callback3 should have errors
  EXPECT_TRUE(state1->errorReceived);
  EXPECT_TRUE(state3->errorReceived);

  // callback2 should still have response, not error
  EXPECT_TRUE(state2->responseReceived);
  EXPECT_FALSE(state2->errorReceived);
}

TEST_F(ThriftClientChannelTest, ClosingThenClosedDrainsPending) {
  // Open → Closing (NOT_OPEN) leaves pending intact;
  // Closing → Closed (any other exception) drains them.
  auto channel = createChannel();

  auto pipeline = buildPipelineWithHandler(
      channel.get(),
      [](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
         TypeErasedBox&&) { return Result::Success; });
  channel->setPipeline(pipeline.get());

  auto [cb, state] = makeCallback();
  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("inflight"),
      createSerializedRequest("req"),
      createHeader(),
      std::move(cb),
      nullptr);

  // Open → Closing must NOT touch pending.
  channel->onException(
      folly::make_exception_wrapper<
          apache::thrift::transport::TTransportException>(
          apache::thrift::transport::TTransportException::NOT_OPEN,
          "Connection closed by server"));
  EXPECT_FALSE(state->errorReceived);
  EXPECT_FALSE(state->responseReceived);

  // Closing → Closed must drain pending with the new exception.
  channel->onException(
      folly::make_exception_wrapper<std::runtime_error>("EOF"));
  EXPECT_TRUE(state->errorReceived);
  EXPECT_TRUE(state->error.is_compatible_with<std::runtime_error>());
}

TEST_F(ThriftClientChannelTest, OnExceptionClearsPendingCallbacks) {
  auto channel = createChannel();

  auto pipeline = buildPipelineWithHandler(
      channel.get(),
      [](apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl&,
         TypeErasedBox&&) { return Result::Success; });
  channel->setPipeline(pipeline.get());

  auto [cb, state] = makeCallback();

  channel->sendRequestResponse(
      apache::thrift::RpcOptions(),
      createMethodMetadata("method1"),
      createSerializedRequest("req1"),
      createHeader(),
      std::move(cb),
      nullptr);

  // Fire exception
  auto error =
      folly::make_exception_wrapper<std::runtime_error>("connection lost");
  channel->onException(std::move(error));

  EXPECT_TRUE(state->errorReceived);

  // Fire another exception - should be a no-op since callbacks were cleared
  auto error2 =
      folly::make_exception_wrapper<std::runtime_error>("second error");
  channel->onException(std::move(error2));

  // callback should still only have the first error
  EXPECT_TRUE(state->error.is_compatible_with<std::runtime_error>());
}

} // namespace apache::thrift::fast_thrift::thrift
