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
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

#define private public
#define protected public
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>

namespace apache::thrift::fast_thrift::rocket::client::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;

namespace {

/**
 * Helper to create a RocketRequestMessage with REQUEST_RESPONSE frame type.
 */
RocketRequestMessage makeClientRequest(
    std::unique_ptr<folly::IOBuf> data,
    std::unique_ptr<folly::IOBuf> metadata,
    uint32_t requestHandle = kNoRequestHandle) {
  return RocketRequestMessage{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedRequestResponseFrame{
              .data = std::move(data),
              .metadata = std::move(metadata),
              .header = {.streamId = kInvalidStreamId},
          },
      .requestHandle = requestHandle,
      .streamType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
  };
}

/**
 * Helper to get the streamId from a RocketRequestMessage.
 */
uint32_t getStreamId(const RocketRequestMessage& msg) {
  return msg.frame
      .get<apache::thrift::fast_thrift::frame::ComposedRequestResponseFrame>()
      .header.streamId;
}

/**
 * MockStreamContext for testing RocketClientStreamStateHandler.
 *
 * Captures frames and messages fired via fireRead() and fireWrite(),
 * and exceptions via fireException().
 */
class MockStreamContext {
 public:
  Result fireRead(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    if (returnError_) {
      return Result::Error;
    }
    readMessages_.push_back(std::move(msg));
    return returnBackpressure_ ? Result::Backpressure : Result::Success;
  }

  Result fireWrite(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    if (returnError_) {
      return Result::Error;
    }
    writeMessages_.push_back(std::move(msg));
    return returnBackpressure_ ? Result::Backpressure : Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  void setReturnBackpressure(bool value) { returnBackpressure_ = value; }

  void setReturnError(bool value) { returnError_ = value; }

  std::vector<apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>&
  readMessages() {
    return readMessages_;
  }

  std::vector<apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>&
  writeMessages() {
    return writeMessages_;
  }

  bool hasException() const { return static_cast<bool>(exception_); }

  void reset() {
    readMessages_.clear();
    writeMessages_.clear();
    exception_ = folly::exception_wrapper();
    returnBackpressure_ = false;
    returnError_ = false;
  }

 private:
  std::vector<apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>
      readMessages_;
  std::vector<apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>
      writeMessages_;
  folly::exception_wrapper exception_;
  bool returnBackpressure_{false};
  bool returnError_{false};
};

std::unique_ptr<folly::IOBuf> buildFrame(
    apache::thrift::fast_thrift::frame::FrameType type,
    uint32_t streamId,
    uint16_t flags = 0) {
  const auto& desc = apache::thrift::fast_thrift::frame::getDescriptor(type);
  size_t headerSize = desc.headerSize > 0
      ? desc.headerSize
      : apache::thrift::fast_thrift::frame::kBaseHeaderSize;

  auto buf = folly::IOBuf::create(headerSize);
  auto* data = buf->writableData();
  std::memset(data, 0, headerSize);

  // Write streamId (big-endian)
  data[0] = static_cast<uint8_t>((streamId >> 24) & 0xFF);
  data[1] = static_cast<uint8_t>((streamId >> 16) & 0xFF);
  data[2] = static_cast<uint8_t>((streamId >> 8) & 0xFF);
  data[3] = static_cast<uint8_t>(streamId & 0xFF);

  // Write typeAndFlags (big-endian)
  uint16_t typeAndFlags =
      (static_cast<uint16_t>(type)
       << ::apache::thrift::fast_thrift::frame::detail::kFlagsBits) |
      flags;
  data[4] = static_cast<uint8_t>((typeAndFlags >> 8) & 0xFF);
  data[5] = static_cast<uint8_t>(typeAndFlags & 0xFF);

  buf->append(headerSize);
  return buf;
}

apache::thrift::fast_thrift::frame::read::ParsedFrame parseTestFrame(
    apache::thrift::fast_thrift::frame::FrameType type,
    uint32_t streamId,
    uint16_t flags = 0) {
  return apache::thrift::fast_thrift::frame::read::parseFrame(
      buildFrame(type, streamId, flags));
}

/**
 * Helper to wrap a ParsedFrame into RocketResponseMessage for onRead testing.
 */
RocketResponseMessage makeRocketResponse(
    apache::thrift::fast_thrift::frame::FrameType type,
    uint32_t streamId,
    uint16_t flags = 0) {
  return RocketResponseMessage{
      .frame = parseTestFrame(type, streamId, flags),
  };
}

} // namespace

class ClientStreamStateHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  MockStreamContext ctx_;
  RocketClientStreamStateHandler handler_;
};

// =============================================================================
// Stream ID Generation
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, GeneratesOddStreamIds) {
  EXPECT_EQ(handler_.generateStreamId(), 1);
  EXPECT_EQ(handler_.generateStreamId(), 3);
  EXPECT_EQ(handler_.generateStreamId(), 5);
  EXPECT_EQ(handler_.nextStreamId(), 7);
}

// =============================================================================
// Outbound Write
// =============================================================================

TEST_F(
    ClientStreamStateHandlerTest, OutboundWriteAssignsStreamIdAndStoresState) {
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("test request"),
      folly::IOBuf::copyBuffer("test metadata"));
  auto result = handler_.onWrite(ctx_, erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& writtenMsg = ctx_.writeMessages()[0].get<RocketRequestMessage>();
  EXPECT_EQ(getStreamId(writtenMsg), 1);
  EXPECT_EQ(
      writtenMsg.streamType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
  EXPECT_TRUE(handler_.hasActiveStream(1));
  EXPECT_EQ(handler_.activeStreamCount(), 1);
}

TEST_F(ClientStreamStateHandlerTest, MultipleWritesGenerateSequentialOddIds) {
  for (int i = 0; i < 3; ++i) {
    auto request = makeClientRequest(
        folly::IOBuf::copyBuffer("request"),
        folly::IOBuf::copyBuffer("metadata"));
    EXPECT_EQ(
        handler_.onWrite(ctx_, erase_and_box(std::move(request))),
        Result::Success);
  }

  ASSERT_EQ(ctx_.writeMessages().size(), 3);
  EXPECT_EQ(
      getStreamId(ctx_.writeMessages()[0].get<RocketRequestMessage>()), 1);
  EXPECT_EQ(
      getStreamId(ctx_.writeMessages()[1].get<RocketRequestMessage>()), 3);
  EXPECT_EQ(
      getStreamId(ctx_.writeMessages()[2].get<RocketRequestMessage>()), 5);
  EXPECT_EQ(handler_.activeStreamCount(), 3);
}

// =============================================================================
// Terminal Frame Handling (ERROR, CANCEL, complete PAYLOAD)
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, TerminalPayloadCleansUpStream) {
  constexpr uint32_t kTestHandle = 42;
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"),
      kTestHandle);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_TRUE(handler_.hasActiveStream(1));

  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
          1,
          ::apache::thrift::fast_thrift::frame::detail::kCompleteBit)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(response.requestHandle, kTestHandle);
  EXPECT_EQ(
      response.streamType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(
      apache::thrift::fast_thrift::frame::read::FrameView(response.frame)
          .type(),
      apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_TRUE(
      apache::thrift::fast_thrift::frame::read::FrameView(response.frame)
          .isComplete());
  EXPECT_FALSE(handler_.hasActiveStream(1));
}

TEST_F(ClientStreamStateHandlerTest, ErrorFrameIsAlwaysTerminal) {
  constexpr uint32_t kTestHandle = 42;
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"),
      kTestHandle);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_TRUE(handler_.hasActiveStream(1));

  // ERROR is terminal even without complete flag
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR, 1, 0)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(response.requestHandle, kTestHandle);
  EXPECT_EQ(
      apache::thrift::fast_thrift::frame::read::FrameView(response.frame)
          .type(),
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_FALSE(handler_.hasActiveStream(1));
}

TEST_F(ClientStreamStateHandlerTest, CancelFrameIsAlwaysTerminal) {
  constexpr uint32_t kTestHandle = 42;
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"),
      kTestHandle);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_TRUE(handler_.hasActiveStream(1));

  // CANCEL is terminal even without complete flag
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::CANCEL, 1, 0)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(response.requestHandle, kTestHandle);
  EXPECT_EQ(
      apache::thrift::fast_thrift::frame::read::FrameView(response.frame)
          .type(),
      apache::thrift::fast_thrift::frame::FrameType::CANCEL);
  EXPECT_FALSE(handler_.hasActiveStream(1));
}

// =============================================================================
// Non-Terminal Frame Handling
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, NonTerminalPayloadKeepsStreamOpen) {
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);

  // PAYLOAD without complete flag is non-terminal (streaming scenario).
  // streamType must be stamped on every stream-scoped frame so downstream
  // pattern handlers can dispatch statelessly.
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD, 1, 0)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(
      response.streamType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
  EXPECT_TRUE(handler_.hasActiveStream(1)); // Stream still open
}

TEST_F(
    ClientStreamStateHandlerTest,
    RequestNFramePassesThroughAndKeepsStreamOpen) {
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);

  // REQUEST_N is a flow control frame, non-terminal
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_N, 1, 0)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_EQ(
      apache::thrift::fast_thrift::frame::read::FrameView(
          ctx_.readMessages()[0].get<RocketResponseMessage>().frame)
          .type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_N);
  EXPECT_TRUE(handler_.hasActiveStream(1)); // Stream still open
}

TEST_F(
    ClientStreamStateHandlerTest, NonTerminalFrameForUnknownStreamIsDropped) {
  // Non-terminal frame for unknown stream should be dropped (same as terminal)
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD, 999, 0)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 0); // Dropped with log
}

// =============================================================================
// Unknown Stream ID Handling
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, TerminalFrameForUnknownStreamIsDropped) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
          999,
          ::apache::thrift::fast_thrift::frame::detail::kCompleteBit)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 0); // Dropped with log
}

// =============================================================================
// Connection-Level Frames (streamId == 0)
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, ConnectionFramesPassThrough) {
  // SETUP frame
  auto result1 = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::SETUP, 0)));
  EXPECT_EQ(result1, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 1);

  // KEEPALIVE frame
  auto result2 = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::KEEPALIVE, 0)));
  EXPECT_EQ(result2, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 2);

  // Connection frames don't create active streams
  EXPECT_EQ(handler_.activeStreamCount(), 0);
}

// =============================================================================
// Handler Lifecycle
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, HandlerRemovedClearsAllState) {
  auto request1 = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request1))),
      Result::Success);
  auto request2 = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request2))),
      Result::Success);
  EXPECT_EQ(handler_.activeStreamCount(), 2);
  EXPECT_GT(handler_.nextStreamId(), 1);

  handler_.handlerRemoved(ctx_);

  EXPECT_EQ(handler_.activeStreamCount(), 0);
  EXPECT_EQ(handler_.nextStreamId(), 1);
}

TEST_F(ClientStreamStateHandlerTest, HandlerRemovedFailsActiveStreams) {
  constexpr uint32_t kHandle1 = 1;
  constexpr uint32_t kHandle2 = 2;
  auto request1 = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"),
      kHandle1);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request1))),
      Result::Success);
  auto request2 = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"),
      kHandle2);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request2))),
      Result::Success);
  EXPECT_EQ(handler_.activeStreamCount(), 2);

  ctx_.writeMessages().clear();
  handler_.handlerRemoved(ctx_);

  // All active streams should be cleared when handler is removed
  // Error handling is now done via fireException at the pipeline level
  EXPECT_EQ(handler_.activeStreamCount(), 0);
}

// =============================================================================
// Exception Handling
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, OnExceptionClearsActiveStreams) {
  auto request1 = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request1))),
      Result::Success);
  auto request2 = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request2))),
      Result::Success);
  EXPECT_EQ(handler_.activeStreamCount(), 2);

  auto ex = folly::make_exception_wrapper<std::runtime_error>("test error");
  handler_.onException(ctx_, std::move(ex));

  EXPECT_TRUE(ctx_.hasException());
  EXPECT_EQ(handler_.activeStreamCount(), 0);
}

// =============================================================================
// Backpressure and Error Propagation
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, DownstreamResultPropagatedOnRead) {
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);

  ctx_.setReturnBackpressure(true);
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
          1,
          ::apache::thrift::fast_thrift::frame::detail::kCompleteBit)));

  EXPECT_EQ(result, Result::Backpressure);
}

TEST_F(ClientStreamStateHandlerTest, DownstreamResultPropagatedOnWrite) {
  ctx_.setReturnBackpressure(true);
  auto request1 = makeClientRequest(
      folly::IOBuf::copyBuffer("test"), folly::IOBuf::copyBuffer("metadata"));
  auto result1 = handler_.onWrite(ctx_, erase_and_box(std::move(request1)));
  EXPECT_EQ(result1, Result::Backpressure);

  ctx_.setReturnBackpressure(false);
  ctx_.setReturnError(true);
  auto request2 = makeClientRequest(
      folly::IOBuf::copyBuffer("test"), folly::IOBuf::copyBuffer("metadata"));
  auto result2 = handler_.onWrite(ctx_, erase_and_box(std::move(request2)));
  EXPECT_EQ(result2, Result::Error);
}

TEST_F(ClientStreamStateHandlerTest, DownstreamErrorPropagatedOnRead) {
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);

  ctx_.setReturnError(true);
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
          1,
          ::apache::thrift::fast_thrift::frame::detail::kCompleteBit)));

  EXPECT_EQ(result, Result::Error);
}

// =============================================================================
// Request Context Propagation
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, RequestHandleReturnedInResponse) {
  constexpr uint32_t kTestHandle = 42;
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"),
      kTestHandle);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_TRUE(handler_.hasActiveStream(1));

  // Send terminal frame and verify handle is returned
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
          1,
          ::apache::thrift::fast_thrift::frame::detail::kCompleteBit)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(response.requestHandle, kTestHandle);
  EXPECT_FALSE(handler_.hasActiveStream(1));
}

// =============================================================================
// Stream ID Collision Detection (Death Test)
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, DuplicateStreamIdCrashes) {
  // Send a request to create stream ID 1
  auto request1 = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request1))),
      Result::Success);
  EXPECT_TRUE(handler_.hasActiveStream(1));

  // Manually reset the stream ID counter to create a duplicate
  handler_.nextStreamId_ = 1;

  auto request2 = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));

#ifndef NDEBUG
  // DCHECK fires only in debug builds
  EXPECT_DEATH(
      (void)handler_.onWrite(ctx_, erase_and_box(std::move(request2))),
      "Stream ID 1 already exists in active streams");
#else
  // In opt builds, DCHECK is a no-op so the write succeeds
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request2))),
      Result::Success);
#endif
}

// =============================================================================
// Connection Frame Edge Cases
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, ErrorFrameOnStreamZeroPassesThrough) {
  // Connection-level ERROR frame (stream ID 0) should pass through
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR, 0, 0)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_EQ(
      apache::thrift::fast_thrift::frame::read::FrameView(
          ctx_.readMessages()[0].get<RocketResponseMessage>().frame)
          .type(),
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
}

} // namespace apache::thrift::fast_thrift::rocket::client::handler
