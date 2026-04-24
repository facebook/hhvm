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
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>

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
 * MockContext for testing RocketServerSetupFrameHandler.
 *
 * Captures frames and messages fired via fireRead() and fireWrite(),
 * and exceptions via fireException().
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

  void deactivate() noexcept { disconnectCalled_ = true; }

  void close() noexcept { closeCalled_ = true; }

  // NOLINTNEXTLINE(clang-diagnostic-unused-member-function)
  apache::thrift::fast_thrift::channel_pipeline::BytesPtr allocate(
      size_t size) {
    return g_allocator.allocate(size);
  }

  apache::thrift::fast_thrift::channel_pipeline::BytesPtr copyBuffer(
      const void* data, size_t len) noexcept {
    auto buf = g_allocator.allocate(len);
    std::memcpy(buf->writableData(), data, len);
    buf->append(len);
    return buf;
  }

  void setReadResult(Result result) { readResult_ = result; }

  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }

  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }

  bool hasException() const { return static_cast<bool>(exception_); }

  bool disconnectCalled() const { return disconnectCalled_; }

  bool closeCalled() const { return closeCalled_; }

  bool writeReadyCalled() const { return writeReadyCalled_; }

  void reset() {
    readMessages_.clear();
    writeMessages_.clear();
    exception_ = folly::exception_wrapper();
    readResult_ = Result::Success;
    writeResult_ = Result::Success;
    disconnectCalled_ = false;
    closeCalled_ = false;
    writeReadyCalled_ = false;
  }

 private:
  std::vector<TypeErasedBox> readMessages_;
  std::vector<TypeErasedBox> writeMessages_;
  folly::exception_wrapper exception_;
  Result readResult_{Result::Success};
  Result writeResult_{Result::Success};
  bool disconnectCalled_{false};
  bool closeCalled_{false};
  bool writeReadyCalled_{false};
};

// Build a raw frame buffer for non-SETUP frame types (reused from
// ServerStreamStateHandlerTest)
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

apache::thrift::fast_thrift::frame::read::ParsedFrame makeTestFrame(
    apache::thrift::fast_thrift::frame::FrameType type,
    uint32_t streamId,
    uint16_t flags = 0) {
  return apache::thrift::fast_thrift::frame::read::parseFrame(
      buildFrame(type, streamId, flags));
}

apache::thrift::fast_thrift::frame::read::ParsedFrame makeSetupFrame(
    uint16_t majorVersion = 1,
    uint16_t minorVersion = 0,
    uint32_t keepaliveTime = 30000,
    uint32_t maxLifetime = 60000,
    bool lease = false) {
  apache::thrift::fast_thrift::frame::write::SetupHeader header{
      .majorVersion = majorVersion,
      .minorVersion = minorVersion,
      .keepaliveTime = keepaliveTime,
      .maxLifetime = maxLifetime,
      .lease = lease,
  };
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      header, nullptr, nullptr);
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf));
}

} // namespace

class ServerSetupFrameHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  Result callOnRead(
      apache::thrift::fast_thrift::frame::read::ParsedFrame frame) {
    return handler_.onRead(ctx_, erase_and_box(std::move(frame)));
  }

  Result callOnWrite(TypeErasedBox msg) {
    return handler_.onWrite(ctx_, std::move(msg));
  }

  /// Complete a valid setup so tests can exercise post-setup behavior.
  void completeSetup() {
    auto result = callOnRead(makeSetupFrame());
    ASSERT_EQ(result, Result::Success);
    ASSERT_TRUE(handler_.isSetupComplete());
    ctx_.reset();
  }

  MockContext ctx_;
  RocketServerSetupFrameHandler handler_;
};

// =============================================================================
// Setup Success Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, ValidSetupFrameCompletesSetup) {
  auto result = callOnRead(makeSetupFrame(1, 0, 30000, 60000));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(handler_.isSetupComplete());
  EXPECT_FALSE(ctx_.closeCalled());

  const auto& params = handler_.setupParameters();
  EXPECT_EQ(params.majorVersion, 1);
  EXPECT_EQ(params.minorVersion, 0);
  EXPECT_EQ(params.keepaliveTime, 30000);
  EXPECT_EQ(params.maxLifetime, 60000);
  EXPECT_FALSE(params.hasLease);

  // SETUP frame is consumed, not forwarded downstream
  EXPECT_EQ(ctx_.readMessages().size(), 0);
}

TEST_F(ServerSetupFrameHandlerTest, ValidSetupFrameWithLeaseFlag) {
  auto result = callOnRead(makeSetupFrame(1, 0, 30000, 60000, /*lease=*/true));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(handler_.isSetupComplete());
  EXPECT_TRUE(handler_.setupParameters().hasLease);
}

// =============================================================================
// Version Validation Tests
// =============================================================================

TEST_F(
    ServerSetupFrameHandlerTest,
    SetupWithMajorVersionZeroReturnsUnsupportedSetup) {
  auto result = callOnRead(makeSetupFrame(0, 0, 30000, 60000));

  EXPECT_EQ(result, Result::Error);
  EXPECT_FALSE(handler_.isSetupComplete());
  EXPECT_TRUE(ctx_.closeCalled());

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto errorBuf = std::move(
      ctx_.writeMessages()[0]
          .get<apache::thrift::fast_thrift::channel_pipeline::BytesPtr>());
  auto errorFrame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(errorBuf));
  apache::thrift::fast_thrift::frame::read::ErrorView errorView(errorFrame);
  EXPECT_EQ(errorView.errorCode(), setup_error::kUnsupportedSetup);
}

// =============================================================================
// Timer Validation Tests
// =============================================================================

TEST_F(
    ServerSetupFrameHandlerTest,
    SetupWithZeroKeepaliveTimeReturnsInvalidSetup) {
  auto result = callOnRead(makeSetupFrame(1, 0, 0, 60000));

  EXPECT_EQ(result, Result::Error);
  EXPECT_FALSE(handler_.isSetupComplete());
  EXPECT_TRUE(ctx_.closeCalled());

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto errorBuf = std::move(
      ctx_.writeMessages()[0]
          .get<apache::thrift::fast_thrift::channel_pipeline::BytesPtr>());
  auto errorFrame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(errorBuf));
  apache::thrift::fast_thrift::frame::read::ErrorView errorView(errorFrame);
  EXPECT_EQ(errorView.errorCode(), setup_error::kInvalidSetup);
}

TEST_F(
    ServerSetupFrameHandlerTest, SetupWithZeroMaxLifetimeReturnsInvalidSetup) {
  auto result = callOnRead(makeSetupFrame(1, 0, 30000, 0));

  EXPECT_EQ(result, Result::Error);
  EXPECT_FALSE(handler_.isSetupComplete());
  EXPECT_TRUE(ctx_.closeCalled());

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto errorBuf = std::move(
      ctx_.writeMessages()[0]
          .get<apache::thrift::fast_thrift::channel_pipeline::BytesPtr>());
  auto errorFrame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(errorBuf));
  apache::thrift::fast_thrift::frame::read::ErrorView errorView(errorFrame);
  EXPECT_EQ(errorView.errorCode(), setup_error::kInvalidSetup);
}

// =============================================================================
// Non-SETUP First Frame Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, NonSetupFirstFrameReturnsInvalidSetup) {
  auto result = callOnRead(makeTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));

  EXPECT_EQ(result, Result::Error);
  EXPECT_FALSE(handler_.isSetupComplete());
  EXPECT_TRUE(ctx_.closeCalled());

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto errorBuf = std::move(
      ctx_.writeMessages()[0]
          .get<apache::thrift::fast_thrift::channel_pipeline::BytesPtr>());
  auto errorFrame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(errorBuf));
  EXPECT_EQ(errorFrame.streamId(), 0);
  EXPECT_EQ(
      errorFrame.type(), apache::thrift::fast_thrift::frame::FrameType::ERROR);
  apache::thrift::fast_thrift::frame::read::ErrorView errorView(errorFrame);
  EXPECT_EQ(errorView.errorCode(), setup_error::kInvalidSetup);
}

// =============================================================================
// Duplicate SETUP Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, DuplicateSetupFrameReturnsInvalidSetup) {
  completeSetup();

  auto result = callOnRead(makeSetupFrame(1, 0, 30000, 60000));

  EXPECT_EQ(result, Result::Error);
  // Setup should still be complete from the first SETUP
  EXPECT_TRUE(handler_.isSetupComplete());
  EXPECT_TRUE(ctx_.closeCalled());

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto errorBuf = std::move(
      ctx_.writeMessages()[0]
          .get<apache::thrift::fast_thrift::channel_pipeline::BytesPtr>());
  auto errorFrame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(errorBuf));
  apache::thrift::fast_thrift::frame::read::ErrorView errorView(errorFrame);
  EXPECT_EQ(errorView.errorCode(), setup_error::kInvalidSetup);
}

// =============================================================================
// Post-Setup Passthrough Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, PostSetupRequestFramePassesThrough) {
  completeSetup();

  auto result = callOnRead(makeTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 1);

  auto& frame =
      ctx_.readMessages()[0]
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(
      frame.type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
}

// =============================================================================
// Outbound Passthrough Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, OnWritePassesThrough) {
  auto data = copyBuffer("test data");
  auto frame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::PayloadHeader{
          .streamId = 1, .complete = true, .next = true},
      nullptr,
      std::move(data));
  auto parsed =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));

  auto result = callOnWrite(erase_and_box(std::move(parsed)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.writeMessages().size(), 1);
}

// =============================================================================
// Handler Lifecycle Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, HandlerRemovedResetsState) {
  completeSetup();
  EXPECT_TRUE(handler_.isSetupComplete());

  handler_.handlerRemoved(ctx_);

  EXPECT_FALSE(handler_.isSetupComplete());
  const auto& params = handler_.setupParameters();
  EXPECT_EQ(params.majorVersion, 0);
  EXPECT_EQ(params.minorVersion, 0);
  EXPECT_EQ(params.keepaliveTime, 0);
  EXPECT_EQ(params.maxLifetime, 0);
  EXPECT_FALSE(params.hasLease);
}

TEST_F(ServerSetupFrameHandlerTest, OnDisconnectIsNoOp) {
  handler_.onPipelineInactive(ctx_);
  EXPECT_FALSE(ctx_.disconnectCalled());
}

TEST_F(ServerSetupFrameHandlerTest, OnWriteReadyIsNoOp) {
  handler_.onWriteReady(ctx_);
  EXPECT_FALSE(ctx_.writeReadyCalled());
}

// =============================================================================
// Exception Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, OnExceptionPassesThrough) {
  auto ex = folly::make_exception_wrapper<std::runtime_error>("test error");
  handler_.onException(ctx_, ex);

  EXPECT_TRUE(ctx_.hasException());
}

// =============================================================================
// Backpressure / Rollback Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, PostSetupBackpressurePropagated) {
  completeSetup();

  ctx_.setReadResult(Result::Backpressure);

  auto result = callOnRead(makeTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));

  EXPECT_EQ(result, Result::Backpressure);
  EXPECT_TRUE(handler_.isSetupComplete());
}

} // namespace apache::thrift::fast_thrift::rocket::server::handler
