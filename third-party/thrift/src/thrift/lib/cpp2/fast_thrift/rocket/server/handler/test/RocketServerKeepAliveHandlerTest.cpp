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

#include <cstring>

#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameDescriptor.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerKeepAliveHandler.h>

namespace apache::thrift::fast_thrift::rocket::server::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
namespace frame = apache::thrift::fast_thrift::frame;

namespace {

apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator
    g_allocator;

/**
 * MockContext capturing fireRead/fireWrite/fireException, close(), and the
 * copyBuffer() the handler uses to build the CONNECTION_ERROR payload.
 */
class MockContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    if (readResult_ != Result::Success) {
      return readResult_;
    }
    readMessages_.push_back(std::move(msg));
    return Result::Success;
  }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    if (writeResult_ != Result::Success) {
      return writeResult_;
    }
    writeMessages_.push_back(std::move(msg));
    return Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  void close() noexcept { closeCalled_ = true; }

  apache::thrift::fast_thrift::channel_pipeline::BytesPtr copyBuffer(
      const void* data, size_t len) noexcept {
    auto buf = g_allocator.allocate(len);
    std::memcpy(buf->writableData(), data, len);
    buf->append(len);
    return buf;
  }

  void setReadResult(Result result) { readResult_ = result; }
  void setWriteResult(Result result) { writeResult_ = result; }

  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }
  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }
  bool hasException() const { return static_cast<bool>(exception_); }
  bool closeCalled() const { return closeCalled_; }
  bool writeReadyCalled() const { return writeReadyCalled_; }

  void reset() {
    readMessages_.clear();
    writeMessages_.clear();
    exception_ = folly::exception_wrapper();
    readResult_ = Result::Success;
    writeResult_ = Result::Success;
    closeCalled_ = false;
    writeReadyCalled_ = false;
  }

 private:
  std::vector<TypeErasedBox> readMessages_;
  std::vector<TypeErasedBox> writeMessages_;
  folly::exception_wrapper exception_;
  Result readResult_{Result::Success};
  Result writeResult_{Result::Success};
  bool closeCalled_{false};
  bool writeReadyCalled_{false};
};

// Build a KEEPALIVE frame (streamId always 0 on the wire) with the respond
// flag and an optional data payload, then parse it back.
frame::read::ParsedFrame makeKeepAlive(bool respond, folly::StringPiece data) {
  std::unique_ptr<folly::IOBuf> payload =
      data.empty() ? nullptr : folly::IOBuf::copyBuffer(data);
  auto buf = frame::write::serialize(
      frame::write::KeepAliveHeader{
          .lastReceivedPosition = 0, .respond = respond},
      std::move(payload));
  return frame::read::parseFrame(std::move(buf));
}

// Build a raw header-only frame with an arbitrary stream ID and flags. Used to
// forge a KEEPALIVE with a (protocol-illegal) non-zero stream ID, which the
// KeepAliveHeader writer cannot express.
frame::read::ParsedFrame makeRawFrame(
    frame::FrameType type, uint32_t streamId, uint16_t flags) {
  const auto& desc = frame::getDescriptor(type);
  const size_t headerSize =
      desc.headerSize > 0 ? desc.headerSize : frame::kBaseHeaderSize;

  auto buf = g_allocator.allocate(headerSize);
  auto* p = buf->writableData();
  std::memset(p, 0, headerSize);

  p[0] = static_cast<uint8_t>((streamId >> 24) & 0xFF);
  p[1] = static_cast<uint8_t>((streamId >> 16) & 0xFF);
  p[2] = static_cast<uint8_t>((streamId >> 8) & 0xFF);
  p[3] = static_cast<uint8_t>(streamId & 0xFF);

  const uint16_t typeAndFlags =
      (static_cast<uint16_t>(type) << frame::detail::kFlagsBits) | flags;
  p[4] = static_cast<uint8_t>((typeAndFlags >> 8) & 0xFF);
  p[5] = static_cast<uint8_t>(typeAndFlags & 0xFF);

  buf->append(headerSize);
  return frame::read::parseFrame(std::move(buf));
}

std::string coalesceToString(const std::unique_ptr<folly::IOBuf>& buf) {
  const auto bytes = buf->coalesce();
  return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

} // namespace

class ServerKeepAliveHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  Result callOnRead(frame::read::ParsedFrame frame) {
    RocketRequestMessage msg;
    msg.frame = std::move(frame);
    return handler_.onRead(ctx_, erase_and_box(std::move(msg)));
  }

  MockContext ctx_;
  RocketServerKeepAliveHandler handler_;
};

// =============================================================================
// Respond-flag echo
// =============================================================================

TEST_F(ServerKeepAliveHandlerTest, RespondFlagEchoesKeepAliveWithFlagCleared) {
  auto result = callOnRead(makeKeepAlive(/*respond=*/true, "ping-payload"));

  EXPECT_EQ(result, Result::Success);
  // Consumed: never forwarded to downstream stream handlers.
  EXPECT_EQ(ctx_.readMessages().size(), 0);

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& echo = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(echo.frame.frameType, frame::FrameType::KEEPALIVE);
  EXPECT_EQ(echo.frame.streamId, 0u);
  EXPECT_FALSE(echo.frame.respond);
  EXPECT_EQ(echo.frame.lastReceivedPosition, 0u);
  ASSERT_NE(echo.frame.data, nullptr);
  EXPECT_EQ(coalesceToString(echo.frame.data), "ping-payload");
}

TEST_F(ServerKeepAliveHandlerTest, RespondFlagWithEmptyDataStillEchoes) {
  auto result = callOnRead(makeKeepAlive(/*respond=*/true, ""));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& echo = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(echo.frame.frameType, frame::FrameType::KEEPALIVE);
  EXPECT_FALSE(echo.frame.respond);
}

// =============================================================================
// No respond flag → consumed silently
// =============================================================================

TEST_F(ServerKeepAliveHandlerTest, NoRespondFlagIsConsumedWithoutEcho) {
  auto result = callOnRead(makeKeepAlive(/*respond=*/false, "pong-payload"));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_EQ(ctx_.writeMessages().size(), 0);
  EXPECT_FALSE(ctx_.closeCalled());
}

// =============================================================================
// Non-zero stream ID → connection error
// =============================================================================

TEST_F(
    ServerKeepAliveHandlerTest, NonZeroStreamIdSendsConnectionErrorAndCloses) {
  auto result = callOnRead(
      makeRawFrame(frame::FrameType::KEEPALIVE, /*streamId=*/5, /*flags=*/0));

  EXPECT_EQ(result, Result::Error);
  EXPECT_TRUE(ctx_.closeCalled());
  EXPECT_EQ(ctx_.readMessages().size(), 0);

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& err = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(err.frame.frameType, frame::FrameType::ERROR);
  EXPECT_EQ(err.frame.streamId, 0u);
  EXPECT_EQ(
      err.frame.errorCode,
      static_cast<uint32_t>(frame::ErrorCode::CONNECTION_ERROR));
}

// =============================================================================
// Non-KEEPALIVE frames pass through untouched
// =============================================================================

TEST_F(ServerKeepAliveHandlerTest, NonKeepAliveFramePassesThrough) {
  auto result = callOnRead(
      makeRawFrame(frame::FrameType::REQUEST_RESPONSE, /*streamId=*/1, 0));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_EQ(ctx_.writeMessages().size(), 0);

  auto& request = ctx_.readMessages()[0].get<RocketRequestMessage>();
  EXPECT_EQ(request.frame.type(), frame::FrameType::REQUEST_RESPONSE);
}

TEST_F(ServerKeepAliveHandlerTest, PassThroughPropagatesBackpressure) {
  ctx_.setReadResult(Result::Backpressure);

  auto result = callOnRead(
      makeRawFrame(frame::FrameType::REQUEST_RESPONSE, /*streamId=*/1, 0));

  EXPECT_EQ(result, Result::Backpressure);
}

// =============================================================================
// Outbound passthrough & lifecycle
// =============================================================================

TEST_F(ServerKeepAliveHandlerTest, OnWritePassesThrough) {
  RocketResponseMessage response{
      .frame =
          frame::ComposedFrame{
              .frameType = frame::FrameType::PAYLOAD,
              .streamId = 1,
              .complete = true,
              .next = true,
          },
  };

  auto result = handler_.onWrite(ctx_, erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.writeMessages().size(), 1);
}

TEST_F(ServerKeepAliveHandlerTest, OnExceptionPassesThrough) {
  auto ex = folly::make_exception_wrapper<std::runtime_error>("boom");
  handler_.onException(ctx_, std::move(ex));

  EXPECT_TRUE(ctx_.hasException());
}

TEST_F(ServerKeepAliveHandlerTest, OnWriteReadyIsNoOp) {
  handler_.onWriteReady(ctx_);
  EXPECT_FALSE(ctx_.writeReadyCalled());
}

} // namespace apache::thrift::fast_thrift::rocket::server::handler
