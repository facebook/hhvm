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

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseFrameHandler.h>

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
 * Helper to create a parsed REQUEST_RESPONSE frame for inbound testing.
 */
apache::thrift::fast_thrift::frame::read::ParsedFrame makeRequestResponseFrame(
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> data = nullptr,
    std::unique_ptr<folly::IOBuf> metadata = nullptr) {
  auto frame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
          .streamId = streamId,
          .follows = false,
      },
      std::move(metadata),
      std::move(data));
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));
}

/**
 * Helper to create a parsed CANCEL frame.
 */
apache::thrift::fast_thrift::frame::read::ParsedFrame makeCancelFrame(
    uint32_t streamId) {
  auto frame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::CancelHeader{
          .streamId = streamId});
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));
}

/**
 * Helper to create a parsed ERROR frame.
 */
apache::thrift::fast_thrift::frame::read::ParsedFrame makeErrorFrame(
    uint32_t streamId) {
  auto frame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId, .errorCode = 0x00000201},
      nullptr,
      copyBuffer("error"));
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));
}

/**
 * Helper to create a parsed REQUEST_STREAM frame (non-request-response).
 */
apache::thrift::fast_thrift::frame::read::ParsedFrame makeRequestStreamFrame(
    uint32_t streamId) {
  auto frame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestStreamHeader{
          .streamId = streamId, .initialRequestN = 100},
      nullptr,
      copyBuffer("data"));
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));
}

/**
 * Helper to read string content from a ParsedFrame's data section.
 */
std::string readFrameData(
    const apache::thrift::fast_thrift::frame::read::ParsedFrame& frame) {
  auto cursor = frame.dataCursor();
  std::string data;
  data.resize(frame.dataSize());
  cursor.pull(data.data(), frame.dataSize());
  return data;
}

/**
 * MockContext for testing RocketServerRequestResponseFrameHandler.
 */
class MockContext {
 public:
  Result fireWrite(TypeErasedBox&& msg) noexcept {
    if (returnError_) {
      return Result::Error;
    }
    writeMessages_.push_back(std::move(msg));
    return returnBackpressure_ ? Result::Backpressure : Result::Success;
  }

  Result fireRead(TypeErasedBox&& msg) noexcept {
    if (returnError_) {
      return Result::Error;
    }
    readMessages_.push_back(std::move(msg));
    return returnBackpressure_ ? Result::Backpressure : Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  void deactivate() noexcept { disconnectCalled_ = true; }

  void fireWriteReady() noexcept { writeReadyCalled_ = true; }

  void setReturnBackpressure(bool value) { returnBackpressure_ = value; }

  void setReturnError(bool value) { returnError_ = value; }

  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }

  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }

  bool hasException() const { return static_cast<bool>(exception_); }

  bool disconnectCalled() const { return disconnectCalled_; }

  bool writeReadyCalled() const { return writeReadyCalled_; }

  void reset() {
    writeMessages_.clear();
    readMessages_.clear();
    exception_ = folly::exception_wrapper();
    returnBackpressure_ = false;
    returnError_ = false;
    disconnectCalled_ = false;
    writeReadyCalled_ = false;
  }

 private:
  std::vector<TypeErasedBox> writeMessages_;
  std::vector<TypeErasedBox> readMessages_;
  folly::exception_wrapper exception_;
  bool returnBackpressure_{false};
  bool returnError_{false};
  bool disconnectCalled_{false};
  bool writeReadyCalled_{false};
};

} // namespace

class ServerRequestResponseFrameHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  /**
   * Helper to register a request-response stream by sending
   * an inbound REQUEST_RESPONSE frame.
   */
  void registerStream(uint32_t streamId) {
    auto frame = makeRequestResponseFrame(streamId);
    EXPECT_EQ(
        handler_.onRead(ctx_, erase_and_box(std::move(frame))),
        Result::Success);
    EXPECT_TRUE(handler_.hasPendingRequestResponse(streamId));
    ctx_.reset();
  }

  MockContext ctx_;
  RocketServerRequestResponseFrameHandler handler_;
};

// =============================================================================
// Inbound: Request Tracking
// =============================================================================

TEST_F(ServerRequestResponseFrameHandlerTest, Read_RequestResponseIsTracked) {
  auto frame = makeRequestResponseFrame(1, copyBuffer("request data"));

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(frame))), Result::Success);

  EXPECT_TRUE(handler_.hasPendingRequestResponse(1));
  EXPECT_EQ(handler_.pendingRequestResponseCount(), 1);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

TEST_F(
    ServerRequestResponseFrameHandlerTest, Read_NonRequestResponse_NotTracked) {
  auto frame = makeRequestStreamFrame(1);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(frame))), Result::Success);

  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

// =============================================================================
// Inbound: Terminal Frame Handling
// =============================================================================

TEST_F(ServerRequestResponseFrameHandlerTest, Read_CancelRemovesTracking) {
  registerStream(1);

  auto cancelFrame = makeCancelFrame(1);
  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(cancelFrame))),
      Result::Success);

  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

TEST_F(
    ServerRequestResponseFrameHandlerTest,
    Read_CancelForUntrackedPassesThrough) {
  auto cancelFrame = makeCancelFrame(99);
  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(cancelFrame))),
      Result::Success);

  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

TEST_F(ServerRequestResponseFrameHandlerTest, Read_ErrorFrameRemovesTracking) {
  registerStream(1);

  auto errorFrame = makeErrorFrame(1);
  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(errorFrame))),
      Result::Success);

  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

// =============================================================================
// Inbound: Rollback on Failure
// =============================================================================

TEST_F(
    ServerRequestResponseFrameHandlerTest,
    Read_RequestResponseRollbackOnError) {
  ctx_.setReturnError(true);
  auto frame = makeRequestResponseFrame(1);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(frame))), Result::Error);

  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));
}

TEST_F(
    ServerRequestResponseFrameHandlerTest,
    Read_RequestResponseNoRollbackOnBackpressure) {
  ctx_.setReturnBackpressure(true);
  auto frame = makeRequestResponseFrame(1);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(frame))),
      Result::Backpressure);

  // Backpressure means the request was accepted - tracking should remain
  EXPECT_TRUE(handler_.hasPendingRequestResponse(1));
}

// =============================================================================
// Outbound: Response Serialization
// =============================================================================

TEST_F(ServerRequestResponseFrameHandlerTest, Write_DataOnly) {
  registerStream(42);

  const std::string dataStr = "response data";
  RocketResponseMessage response{
      .payload = copyBuffer(dataStr), .metadata = {}, .streamId = 42};

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& written = ctx_.writeMessages()[0].get<std::unique_ptr<folly::IOBuf>>();
  auto frame = apache::thrift::fast_thrift::frame::read::parseFrame(
      written->cloneCoalesced());

  EXPECT_EQ(frame.streamId(), 42);
  EXPECT_EQ(
      frame.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_TRUE(frame.isComplete());
  EXPECT_TRUE(frame.hasNext());
  EXPECT_FALSE(frame.hasMetadata());
  EXPECT_EQ(readFrameData(frame), dataStr);
}

TEST_F(ServerRequestResponseFrameHandlerTest, Write_EmptyPayload) {
  registerStream(1);

  RocketResponseMessage response{.payload = {}, .metadata = {}, .streamId = 1};

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);

  auto& written = ctx_.writeMessages()[0].get<std::unique_ptr<folly::IOBuf>>();
  auto frame = apache::thrift::fast_thrift::frame::read::parseFrame(
      written->cloneCoalesced());

  EXPECT_EQ(
      frame.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_TRUE(frame.isComplete());
  EXPECT_TRUE(frame.hasNext());
}

TEST_F(ServerRequestResponseFrameHandlerTest, Write_LargePayload) {
  registerStream(1);

  constexpr size_t dataSize = 1024 * 1024; // 1MB
  auto largeData = allocate(dataSize);
  std::memset(largeData->writableData(), 'D', dataSize);
  largeData->append(dataSize);

  RocketResponseMessage response{
      .payload = std::move(largeData), .metadata = {}, .streamId = 1};

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);

  auto& written = ctx_.writeMessages()[0].get<std::unique_ptr<folly::IOBuf>>();
  EXPECT_GT(written->computeChainDataLength(), dataSize);
}

TEST_F(ServerRequestResponseFrameHandlerTest, Write_DataAndMetadata) {
  registerStream(42);

  const std::string dataStr = "response data";
  const std::string metadataStr = "response metadata";
  RocketResponseMessage response{
      .payload = copyBuffer(dataStr),
      .metadata = copyBuffer(metadataStr),
      .streamId = 42};

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& written = ctx_.writeMessages()[0].get<std::unique_ptr<folly::IOBuf>>();
  auto frame = apache::thrift::fast_thrift::frame::read::parseFrame(
      written->cloneCoalesced());

  EXPECT_EQ(frame.streamId(), 42);
  EXPECT_EQ(
      frame.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_TRUE(frame.isComplete());
  EXPECT_TRUE(frame.hasNext());
  EXPECT_TRUE(frame.hasMetadata());
  EXPECT_EQ(frame.metadataSize(), metadataStr.size());
  EXPECT_EQ(readFrameData(frame), dataStr);
}

TEST_F(ServerRequestResponseFrameHandlerTest, Write_MetadataOnly) {
  registerStream(1);

  const std::string metadataStr = "response metadata";
  RocketResponseMessage response{
      .payload = {}, .metadata = copyBuffer(metadataStr), .streamId = 1};

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& written = ctx_.writeMessages()[0].get<std::unique_ptr<folly::IOBuf>>();
  auto frame = apache::thrift::fast_thrift::frame::read::parseFrame(
      written->cloneCoalesced());

  EXPECT_EQ(
      frame.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_TRUE(frame.isComplete());
  EXPECT_TRUE(frame.hasNext());
  EXPECT_TRUE(frame.hasMetadata());
  EXPECT_EQ(frame.metadataSize(), metadataStr.size());
  EXPECT_EQ(frame.dataSize(), 0u);
}

TEST_F(ServerRequestResponseFrameHandlerTest, Write_ErrorFrame) {
  registerStream(1);

  const std::string errorData = "something went wrong";
  RocketResponseMessage response{
      .payload = copyBuffer(errorData),
      .metadata = {},
      .streamId = 1,
      .errorCode = 0x00000201};

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& written = ctx_.writeMessages()[0].get<std::unique_ptr<folly::IOBuf>>();
  auto frame = apache::thrift::fast_thrift::frame::read::parseFrame(
      written->cloneCoalesced());

  EXPECT_EQ(frame.streamId(), 1);
  EXPECT_EQ(frame.type(), apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(
      apache::thrift::fast_thrift::frame::read::ErrorView(frame).errorCode(),
      0x00000201u);
  EXPECT_EQ(readFrameData(frame), errorData);
  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));
}

TEST_F(
    ServerRequestResponseFrameHandlerTest,
    Write_ErrorFrameReAddsTrackingOnFailure) {
  registerStream(1);

  ctx_.setReturnError(true);
  RocketResponseMessage response{
      .payload = copyBuffer("error"),
      .metadata = {},
      .streamId = 1,
      .errorCode = 0x00000201};

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Error);
  EXPECT_TRUE(handler_.hasPendingRequestResponse(1));
}

TEST_F(
    ServerRequestResponseFrameHandlerTest, Write_NonRequestResponse_Forwards) {
  RocketResponseMessage response{
      .payload = copyBuffer("data"), .metadata = {}, .streamId = 99};

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& forwarded = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(forwarded.streamId, 99);
}

TEST_F(ServerRequestResponseFrameHandlerTest, Write_RemovesTrackingOnSuccess) {
  registerStream(1);

  RocketResponseMessage response{
      .payload = folly::IOBuf::copyBuffer("data"),
      .metadata = {},
      .streamId = 1};

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);

  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));
}

// =============================================================================
// Outbound: Write Result Propagation
// =============================================================================

TEST_F(ServerRequestResponseFrameHandlerTest, Write_ErrorReAddsTracking) {
  registerStream(1);

  ctx_.setReturnError(true);
  RocketResponseMessage response{
      .payload = folly::IOBuf::copyBuffer("data"),
      .metadata = {},
      .streamId = 1};

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Error);
  EXPECT_TRUE(handler_.hasPendingRequestResponse(1));
}

TEST_F(
    ServerRequestResponseFrameHandlerTest,
    Write_BackpressureDoesNotReAddTracking) {
  registerStream(1);

  ctx_.setReturnBackpressure(true);
  RocketResponseMessage response{
      .payload = folly::IOBuf::copyBuffer("data"),
      .metadata = {},
      .streamId = 1};

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Backpressure);
  // Backpressure means the write was accepted - tracking should remain removed
  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));
}

TEST_F(
    ServerRequestResponseFrameHandlerTest,
    Write_CancelAfterBackpressureForwardsCorrectly) {
  // Scenario: write gets backpressure (accepted), then CANCEL arrives.
  // Since backpressure means the write was accepted, tracking is already
  // removed. CANCEL should handle the case where stream is not tracked.
  registerStream(1);

  // Step 1: Write gets backpressure — write was accepted, tracking removed
  ctx_.setReturnBackpressure(true);
  RocketResponseMessage response1{
      .payload = folly::IOBuf::copyBuffer("data"),
      .metadata = {},
      .streamId = 1};
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response1))),
      Result::Backpressure);
  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));
  ctx_.reset();

  // Step 2: CANCEL arrives — stream not tracked (already removed), passes
  // through
  auto cancelFrame = makeCancelFrame(1);
  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(cancelFrame))),
      Result::Success);
  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));
}

// =============================================================================
// Stream Tracking
// =============================================================================

TEST_F(ServerRequestResponseFrameHandlerTest, Tracking_MultipleStreams) {
  EXPECT_EQ(handler_.pendingRequestResponseCount(), 0);

  registerStream(1);
  registerStream(3);
  registerStream(5);

  EXPECT_EQ(handler_.pendingRequestResponseCount(), 3);
  EXPECT_TRUE(handler_.hasPendingRequestResponse(1));
  EXPECT_TRUE(handler_.hasPendingRequestResponse(3));
  EXPECT_TRUE(handler_.hasPendingRequestResponse(5));
}

TEST_F(
    ServerRequestResponseFrameHandlerTest,
    Tracking_ClearedOnPipelineDeactivated) {
  registerStream(1);
  registerStream(3);
  EXPECT_EQ(handler_.pendingRequestResponseCount(), 2);

  handler_.onPipelineDeactivated(ctx_);

  EXPECT_EQ(handler_.pendingRequestResponseCount(), 0);
}

TEST_F(ServerRequestResponseFrameHandlerTest, Tracking_ClearedOnException) {
  registerStream(1);
  registerStream(3);
  EXPECT_EQ(handler_.pendingRequestResponseCount(), 2);

  auto exception =
      folly::make_exception_wrapper<std::runtime_error>("connection error");
  handler_.onException(ctx_, std::move(exception));

  EXPECT_EQ(handler_.pendingRequestResponseCount(), 0);
  EXPECT_TRUE(ctx_.hasException());
}

TEST_F(
    ServerRequestResponseFrameHandlerTest, Tracking_ClearedOnHandlerRemoved) {
  registerStream(1);
  EXPECT_EQ(handler_.pendingRequestResponseCount(), 1);

  handler_.handlerRemoved(ctx_);

  EXPECT_EQ(handler_.pendingRequestResponseCount(), 0);
}

// =============================================================================
// Lifecycle Event Forwarding
// =============================================================================

TEST_F(ServerRequestResponseFrameHandlerTest, Lifecycle_DisconnectIsNoOp) {
  handler_.onPipelineDeactivated(ctx_);
  EXPECT_FALSE(ctx_.disconnectCalled());
}

TEST_F(ServerRequestResponseFrameHandlerTest, Lifecycle_WriteReadyIsNoOp) {
  handler_.onWriteReady(ctx_);
  EXPECT_FALSE(ctx_.writeReadyCalled());
}

} // namespace apache::thrift::fast_thrift::rocket::server::handler
