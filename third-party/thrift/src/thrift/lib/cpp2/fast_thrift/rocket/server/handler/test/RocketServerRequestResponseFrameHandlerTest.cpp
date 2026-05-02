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
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
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
 * Helper to read string content from an IOBuf chain.
 */
std::string readBufString(const folly::IOBuf* buf) {
  if (buf == nullptr) {
    return {};
  }
  std::string out;
  for (auto range : *buf) {
    out.append(reinterpret_cast<const char*>(range.data()), range.size());
  }
  return out;
}

/**
 * Helpers to construct outbound RocketResponseMessage with the typed payload
 * variant.
 */
RocketResponseMessage makePayloadResponse(
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> data,
    std::unique_ptr<folly::IOBuf> metadata = nullptr) {
  return RocketResponseMessage{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
              .data = std::move(data),
              .metadata = std::move(metadata),
              .header = {.streamId = streamId},
          },
  };
}

RocketResponseMessage makeErrorResponse(
    uint32_t streamId, uint32_t errorCode, std::unique_ptr<folly::IOBuf> data) {
  return RocketResponseMessage{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedErrorFrame{
              .data = std::move(data),
              .header = {.streamId = streamId, .errorCode = errorCode},
          },
  };
}

/**
 * Extract the held ComposedPayloadFrame from a forwarded RocketResponseMessage.
 */
const apache::thrift::fast_thrift::frame::ComposedPayloadFrame&
getForwardedPayload(const RocketResponseMessage& msg) {
  return msg.frame
      .get<apache::thrift::fast_thrift::frame::ComposedPayloadFrame>();
}

/**
 * Extract the held ComposedErrorFrame from a forwarded RocketResponseMessage.
 */
const apache::thrift::fast_thrift::frame::ComposedErrorFrame& getForwardedError(
    const RocketResponseMessage& msg) {
  return msg.frame
      .get<apache::thrift::fast_thrift::frame::ComposedErrorFrame>();
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
// Outbound: Typed payload pass-through (handler no longer serializes)
// =============================================================================
//
// The codec downstream of this handler is what serializes the typed payload
// into wire bytes. These tests verify the handler:
//   - Stamps complete=true / next=true on the held ComposedPayloadFrame header
//     for tracked RR streams.
//   - Forwards ComposedErrorFrame unchanged (already terminal).
//   - Forwards untracked-stream messages unchanged.

TEST_F(ServerRequestResponseFrameHandlerTest, Write_DataOnly) {
  registerStream(42);

  const std::string dataStr = "response data";
  auto response = makePayloadResponse(42, copyBuffer(dataStr));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& msg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  const auto& payload = getForwardedPayload(msg);
  EXPECT_EQ(payload.streamId(), 42u);
  EXPECT_TRUE(payload.header.complete);
  EXPECT_TRUE(payload.header.next);
  EXPECT_EQ(payload.metadata, nullptr);
  EXPECT_EQ(readBufString(payload.data.get()), dataStr);
}

TEST_F(ServerRequestResponseFrameHandlerTest, Write_EmptyPayload) {
  registerStream(1);

  auto response = makePayloadResponse(1, nullptr);

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);

  auto& msg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  const auto& payload = getForwardedPayload(msg);
  EXPECT_EQ(payload.streamId(), 1u);
  EXPECT_TRUE(payload.header.complete);
  EXPECT_TRUE(payload.header.next);
  EXPECT_EQ(payload.data, nullptr);
  EXPECT_EQ(payload.metadata, nullptr);
}

TEST_F(ServerRequestResponseFrameHandlerTest, Write_LargePayload) {
  registerStream(1);

  constexpr size_t dataSize = 1024 * 1024; // 1MB
  auto largeData = allocate(dataSize);
  std::memset(largeData->writableData(), 'D', dataSize);
  largeData->append(dataSize);

  auto response = makePayloadResponse(1, std::move(largeData));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);

  auto& msg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  const auto& payload = getForwardedPayload(msg);
  EXPECT_EQ(payload.data->computeChainDataLength(), dataSize);
}

TEST_F(ServerRequestResponseFrameHandlerTest, Write_DataAndMetadata) {
  registerStream(42);

  const std::string dataStr = "response data";
  const std::string metadataStr = "response metadata";
  auto response =
      makePayloadResponse(42, copyBuffer(dataStr), copyBuffer(metadataStr));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& msg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  const auto& payload = getForwardedPayload(msg);
  EXPECT_EQ(payload.streamId(), 42u);
  EXPECT_TRUE(payload.header.complete);
  EXPECT_TRUE(payload.header.next);
  EXPECT_EQ(readBufString(payload.metadata.get()), metadataStr);
  EXPECT_EQ(readBufString(payload.data.get()), dataStr);
}

TEST_F(ServerRequestResponseFrameHandlerTest, Write_MetadataOnly) {
  registerStream(1);

  const std::string metadataStr = "response metadata";
  auto response = makePayloadResponse(1, nullptr, copyBuffer(metadataStr));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& msg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  const auto& payload = getForwardedPayload(msg);
  EXPECT_EQ(payload.streamId(), 1u);
  EXPECT_TRUE(payload.header.complete);
  EXPECT_TRUE(payload.header.next);
  EXPECT_EQ(readBufString(payload.metadata.get()), metadataStr);
  EXPECT_EQ(payload.data, nullptr);
}

TEST_F(ServerRequestResponseFrameHandlerTest, Write_ErrorFrame) {
  registerStream(1);

  const std::string errorData = "something went wrong";
  auto response = makeErrorResponse(1, 0x00000201, copyBuffer(errorData));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& msg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  const auto& err = getForwardedError(msg);
  EXPECT_EQ(err.header.streamId, 1u);
  EXPECT_EQ(err.header.errorCode, 0x00000201u);
  EXPECT_EQ(readBufString(err.data.get()), errorData);
  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));
}

TEST_F(
    ServerRequestResponseFrameHandlerTest,
    Write_ErrorFrameReAddsTrackingOnFailure) {
  registerStream(1);

  ctx_.setReturnError(true);
  auto response = makeErrorResponse(1, 0x00000201, copyBuffer("error"));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Error);
  EXPECT_TRUE(handler_.hasPendingRequestResponse(1));
}

TEST_F(
    ServerRequestResponseFrameHandlerTest, Write_NonRequestResponse_Forwards) {
  // Stream 99 is NOT tracked. Forward the message unchanged.
  auto response = makePayloadResponse(99, copyBuffer("data"));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& forwarded = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  const auto& payload = getForwardedPayload(forwarded);
  EXPECT_EQ(payload.streamId(), 99u);
  // No stamping for untracked stream — header.complete defaults to false.
  EXPECT_FALSE(payload.header.complete);
}

TEST_F(ServerRequestResponseFrameHandlerTest, Write_RemovesTrackingOnSuccess) {
  registerStream(1);

  auto response = makePayloadResponse(1, copyBuffer("data"));

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
  auto response = makePayloadResponse(1, copyBuffer("data"));

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
  auto response = makePayloadResponse(1, copyBuffer("data"));

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
  auto response1 = makePayloadResponse(1, copyBuffer("data"));
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

  handler_.onPipelineInactive(ctx_);

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
  handler_.onPipelineInactive(ctx_);
  EXPECT_FALSE(ctx_.disconnectCalled());
}

TEST_F(ServerRequestResponseFrameHandlerTest, Lifecycle_WriteReadyIsNoOp) {
  handler_.onWriteReady(ctx_);
  EXPECT_FALSE(ctx_.writeReadyCalled());
}

} // namespace apache::thrift::fast_thrift::rocket::server::handler
