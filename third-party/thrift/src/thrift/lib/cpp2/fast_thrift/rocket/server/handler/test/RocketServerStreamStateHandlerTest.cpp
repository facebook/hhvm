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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>

namespace apache::thrift::fast_thrift::rocket::server::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

namespace {

apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator
    g_allocator;

apache::thrift::fast_thrift::channel_pipeline::BytesPtr allocate(size_t size) {
  return g_allocator.allocate(size);
}

apache::thrift::fast_thrift::channel_pipeline::BytesPtr copyBuffer(
    folly::StringPiece s) {
  auto buf = g_allocator.allocate(s.size());
  std::memcpy(buf->writableData(), s.data(), s.size());
  buf->append(s.size());
  return buf;
}

/**
 * MockStreamContext for testing RocketServerStreamStateHandler.
 *
 * Captures frames and messages fired via fireRead() and fireWrite(),
 * and exceptions via fireException().
 */
class MockStreamContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    if (returnError_) {
      return Result::Error;
    }
    readMessages_.push_back(std::move(msg));
    if (returnBackpressure_) {
      return Result::Backpressure;
    }
    return Result::Success;
  }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    if (returnError_) {
      return Result::Error;
    }
    writeMessages_.push_back(std::move(msg));
    if (returnBackpressure_) {
      return Result::Backpressure;
    }
    return Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  void deactivate() noexcept { disconnectCalled_ = true; }

  void setReturnBackpressure(bool value) { returnBackpressure_ = value; }

  void setReturnError(bool value) { returnError_ = value; }

  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }

  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }

  const folly::exception_wrapper& exception() const { return exception_; }

  bool hasException() const { return static_cast<bool>(exception_); }

  bool disconnectCalled() const { return disconnectCalled_; }

  bool writeReadyCalled() const { return writeReadyCalled_; }

  void reset() {
    readMessages_.clear();
    writeMessages_.clear();
    exception_ = folly::exception_wrapper();
    returnBackpressure_ = false;
    returnError_ = false;
    disconnectCalled_ = false;
    closeCalled_ = false;
    writeReadyCalled_ = false;
  }

 private:
  std::vector<TypeErasedBox> readMessages_;
  std::vector<TypeErasedBox> writeMessages_;
  folly::exception_wrapper exception_;
  bool returnBackpressure_{false};
  bool returnError_{false};
  bool disconnectCalled_{false};
  bool closeCalled_{false};
  bool writeReadyCalled_{false};
};

std::unique_ptr<folly::IOBuf> buildFrame(
    apache::thrift::fast_thrift::frame::FrameType type,
    uint32_t streamId,
    uint16_t flags = 0) {
  const auto& desc = apache::thrift::fast_thrift::frame::getDescriptor(type);
  size_t headerSize = desc.headerSize > 0
      ? desc.headerSize
      : apache::thrift::fast_thrift::frame::kBaseHeaderSize;

  auto buf = allocate(headerSize);
  auto* data = buf->writableData();
  std::memset(data, 0, headerSize);

  data[0] = static_cast<uint8_t>((streamId >> 24) & 0xFF);
  data[1] = static_cast<uint8_t>((streamId >> 16) & 0xFF);
  data[2] = static_cast<uint8_t>((streamId >> 8) & 0xFF);
  data[3] = static_cast<uint8_t>(streamId & 0xFF);

  // upper 6 bits = type, lower 10 bits = flags
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

} // namespace

class ServerStreamStateHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  Result callOnRead(
      apache::thrift::fast_thrift::frame::read::ParsedFrame frame) {
    return handler_.onRead(ctx_, erase_and_box(std::move(frame)));
  }

  Result callOnWrite(RocketResponseMessage msg) {
    return handler_.onWrite(ctx_, erase_and_box(std::move(msg)));
  }

  MockStreamContext ctx_;
  RocketServerStreamStateHandler handler_;
};

// =============================================================================
// Inbound Request Tests (ParsedFrame -> RocketRequestMessage)
// =============================================================================

TEST_F(
    ServerStreamStateHandlerTest,
    RequestResponseCreatesActiveStreamAndFiresRequest) {
  auto result = callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 1);

  auto& request = ctx_.readMessages()[0].get<RocketRequestMessage>();
  EXPECT_EQ(request.streamId, 1);
  EXPECT_EQ(
      request.streamType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);

  apache::thrift::fast_thrift::frame::read::FrameView view(request.frame);
  EXPECT_EQ(
      view.type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);

  EXPECT_TRUE(handler_.hasActiveStream(1));
  EXPECT_EQ(handler_.activeStreamCount(), 1);
}

TEST_F(ServerStreamStateHandlerTest, MultipleRequestsCreateSeparateStreams) {
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 3));
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM, 5));

  EXPECT_EQ(ctx_.readMessages().size(), 3);
  EXPECT_EQ(handler_.activeStreamCount(), 3);
  EXPECT_TRUE(handler_.hasActiveStream(1));
  EXPECT_TRUE(handler_.hasActiveStream(3));
  EXPECT_TRUE(handler_.hasActiveStream(5));
}

// =============================================================================
// Duplicate Stream ID Tests
// =============================================================================

TEST_F(ServerStreamStateHandlerTest, DuplicateStreamIdReturnsError) {
  auto result = callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));
  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(handler_.hasActiveStream(1));

  result = callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));
  EXPECT_EQ(result, Result::Error);

  EXPECT_TRUE(handler_.hasActiveStream(1));
  EXPECT_EQ(handler_.activeStreamCount(), 1);
}

// =============================================================================
// Inbound CANCEL Tests
// =============================================================================

TEST_F(ServerStreamStateHandlerTest, CancelRemovesActiveStreamAndFiresToApp) {
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));
  EXPECT_TRUE(handler_.hasActiveStream(1));

  ctx_.reset();

  auto result = callOnRead(
      parseTestFrame(apache::thrift::fast_thrift::frame::FrameType::CANCEL, 1));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 1);

  auto& request = ctx_.readMessages()[0].get<RocketRequestMessage>();
  EXPECT_EQ(request.streamId, 1);
  // CANCEL is forwarded with the originating stream's streamType so the
  // App's per-pattern handler downstream can dispatch statelessly.
  EXPECT_EQ(
      request.streamType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);

  apache::thrift::fast_thrift::frame::read::FrameView view(request.frame);
  EXPECT_EQ(view.type(), apache::thrift::fast_thrift::frame::FrameType::CANCEL);

  EXPECT_FALSE(handler_.hasActiveStream(1));
  EXPECT_EQ(handler_.activeStreamCount(), 0);
}

TEST_F(ServerStreamStateHandlerTest, CancelForUnknownStreamIsDropped) {
  auto result = callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::CANCEL, 999));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 0); // Dropped
}

// =============================================================================
// Inbound Non-Terminal Frame Tests
// =============================================================================

TEST_F(ServerStreamStateHandlerTest, RequestNPassesThroughForActiveStream) {
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM, 1));
  EXPECT_TRUE(handler_.hasActiveStream(1));

  ctx_.reset();

  auto result = callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_N, 1));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  // Non-request stream-scoped frames are wrapped in RocketRequestMessage with
  // streamType stamped from the per-stream map so downstream per-pattern
  // handlers can dispatch statelessly.
  auto& request = ctx_.readMessages()[0].get<RocketRequestMessage>();
  EXPECT_EQ(request.streamId, 1u);
  EXPECT_EQ(
      request.streamType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM);
  EXPECT_TRUE(handler_.hasActiveStream(1));
}

TEST_F(ServerStreamStateHandlerTest, NonRequestFrameForUnknownStreamIsDropped) {
  auto result = callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_N, 999));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 0); // Dropped
}

// =============================================================================
// Connection-Level Frame Tests (streamId == 0)
// =============================================================================

TEST_F(ServerStreamStateHandlerTest, ConnectionLevelFrameIsConsumed) {
  auto result = callOnRead(
      parseTestFrame(apache::thrift::fast_thrift::frame::FrameType::SETUP, 0));

  EXPECT_EQ(result, Result::Success);
  // Connection-level frames are consumed, not forwarded to the app
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_EQ(handler_.activeStreamCount(), 0);
}

// =============================================================================
// Outbound Response Tests (RocketResponseMessage -> RocketResponseMessage)
// =============================================================================

TEST_F(
    ServerStreamStateHandlerTest,
    CompleteResponseProducesStreamMessageAndCleansUp) {
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));
  EXPECT_TRUE(handler_.hasActiveStream(1));

  ctx_.reset();

  RocketResponseMessage response{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
              .data = copyBuffer("response data"),
              .header = {.streamId = 1, .complete = true, .next = true},
          },
  };

  auto result = callOnWrite(std::move(response));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.writeMessages().size(), 1);

  auto& wireMsg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  const auto& payload =
      wireMsg.frame
          .get<apache::thrift::fast_thrift::frame::ComposedPayloadFrame>();
  EXPECT_EQ(payload.streamId(), 1u);
  EXPECT_NE(payload.data, nullptr);

  EXPECT_FALSE(handler_.hasActiveStream(1));
  EXPECT_EQ(handler_.activeStreamCount(), 0);
}

TEST_F(ServerStreamStateHandlerTest, IncompleteResponseKeepsStreamActive) {
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM, 1));
  EXPECT_TRUE(handler_.hasActiveStream(1));

  ctx_.reset();

  RocketResponseMessage response{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
              .data = copyBuffer("partial data"),
              .header = {.streamId = 1, .complete = false, .next = true},
          },
  };

  auto result = callOnWrite(std::move(response));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.writeMessages().size(), 1);

  auto& wireMsg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  const auto& payload =
      wireMsg.frame
          .get<apache::thrift::fast_thrift::frame::ComposedPayloadFrame>();
  EXPECT_EQ(payload.streamId(), 1u);

  EXPECT_TRUE(handler_.hasActiveStream(1));
}

TEST_F(ServerStreamStateHandlerTest, ResponseForUnknownStreamReturnsError) {
  RocketResponseMessage response{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
              .data = copyBuffer("data"),
              .header = {.streamId = 999, .complete = true, .next = true},
          },
  };

  auto result = callOnWrite(std::move(response));

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(ctx_.writeMessages().size(), 0);
}

// =============================================================================
// Handler Lifecycle Tests
// =============================================================================

TEST_F(ServerStreamStateHandlerTest, HandlerRemovedClearsState) {
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 3));

  EXPECT_EQ(handler_.activeStreamCount(), 2);

  handler_.handlerRemoved(ctx_);

  EXPECT_EQ(handler_.activeStreamCount(), 0);
}

TEST_F(ServerStreamStateHandlerTest, OnDisconnectIsNoOp) {
  handler_.onPipelineInactive(ctx_);
  EXPECT_FALSE(ctx_.disconnectCalled());
}

TEST_F(ServerStreamStateHandlerTest, OnWriteReadyIsNoOp) {
  handler_.onWriteReady(ctx_);
  EXPECT_FALSE(ctx_.writeReadyCalled());
}

// =============================================================================
// Exception Handling Tests
// =============================================================================

TEST_F(ServerStreamStateHandlerTest, OnExceptionFailsAllActiveStreams) {
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 3));

  EXPECT_EQ(handler_.activeStreamCount(), 2);

  ctx_.reset();

  auto ex = folly::make_exception_wrapper<std::runtime_error>("test error");
  handler_.onException(ctx_, std::move(ex));

  EXPECT_TRUE(ctx_.hasException());
  EXPECT_EQ(handler_.activeStreamCount(), 0);
  // onException clears streams and propagates exception, no per-stream messages
  EXPECT_EQ(ctx_.readMessages().size(), 0);
}

TEST_F(ServerStreamStateHandlerTest, OnExceptionWithNoActiveStreams) {
  EXPECT_EQ(handler_.activeStreamCount(), 0);

  auto ex = folly::make_exception_wrapper<std::runtime_error>("test error");
  handler_.onException(ctx_, std::move(ex));

  EXPECT_TRUE(ctx_.hasException());
  EXPECT_EQ(ctx_.readMessages().size(), 0);
}

// =============================================================================
// Backpressure Tests
// =============================================================================

TEST_F(ServerStreamStateHandlerTest, BackpressureFromDownstreamOnRead) {
  ctx_.setReturnBackpressure(true);

  auto result = callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));

  EXPECT_EQ(result, Result::Backpressure);
  // Backpressure means the request was accepted - stream should remain active
  EXPECT_TRUE(handler_.hasActiveStream(1));
}

TEST_F(ServerStreamStateHandlerTest, ErrorFromDownstreamOnReadRollsBack) {
  ctx_.setReturnError(true);

  auto result = callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));

  EXPECT_EQ(result, Result::Error);
  // Stream should be rolled back since app didn't receive the request
  EXPECT_FALSE(handler_.hasActiveStream(1));
}

TEST_F(ServerStreamStateHandlerTest, CancelWithDownstreamErrorRemovesStream) {
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));
  EXPECT_TRUE(handler_.hasActiveStream(1));

  ctx_.reset();
  ctx_.setReturnError(true);

  // CANCEL fails to propagate to app
  auto result = callOnRead(
      parseTestFrame(apache::thrift::fast_thrift::frame::FrameType::CANCEL, 1));

  EXPECT_EQ(result, Result::Error);
  // Stream is removed even on failure. There is no retry mechanism for
  // terminal events, and re-adding would create orphaned state.
  EXPECT_FALSE(handler_.hasActiveStream(1));
}

TEST_F(ServerStreamStateHandlerTest, ErrorFromDownstreamOnWrite) {
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));

  ctx_.reset();
  ctx_.setReturnError(true);

  RocketResponseMessage response{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
              .data = copyBuffer("data"),
              .header = {.streamId = 1, .complete = true, .next = true},
          },
  };

  auto result = callOnWrite(std::move(response));

  EXPECT_EQ(result, Result::Error);
  // Stream is dropped on write failure — no retry
  EXPECT_FALSE(handler_.hasActiveStream(1));
}

TEST_F(ServerStreamStateHandlerTest, BackpressureFromDownstreamOnWrite) {
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));

  ctx_.reset();
  ctx_.setReturnBackpressure(true);

  RocketResponseMessage response{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
              .data = copyBuffer("data"),
              .header = {.streamId = 1, .complete = true, .next = true},
          },
  };

  auto result = callOnWrite(std::move(response));

  EXPECT_EQ(result, Result::Backpressure);
  // Backpressure means the write was accepted - stream should remain removed
  EXPECT_FALSE(handler_.hasActiveStream(1));
}

TEST_F(
    ServerStreamStateHandlerTest,
    IncompleteWriteFailureDoesNotAffectStreamState) {
  (void)callOnRead(parseTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM, 1));

  ctx_.reset();
  ctx_.setReturnError(true);

  RocketResponseMessage response{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
              .data = copyBuffer("data"),
              .header = {.streamId = 1, .complete = false, .next = true},
          },
  };

  auto result = callOnWrite(std::move(response));

  EXPECT_EQ(result, Result::Error);
  // Stream was not removed (complete=false), so it should still be active
  EXPECT_TRUE(handler_.hasActiveStream(1));
}

} // namespace apache::thrift::fast_thrift::rocket::server::handler
