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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientErrorFrameHandler.h>

namespace apache::thrift::fast_thrift::rocket::client::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

namespace {

/**
 * Mock context for testing RocketClientErrorFrameHandler.
 * Captures frames fired via fireRead() and exceptions via fireException().
 */
class MockErrorContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    readMessages_.push_back(std::move(msg));
    return Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }

  bool hasException() const { return static_cast<bool>(exception_); }

  folly::exception_wrapper& exception() { return exception_; }

  void reset() {
    readMessages_.clear();
    exception_ = folly::exception_wrapper();
  }

 private:
  std::vector<TypeErasedBox> readMessages_;
  folly::exception_wrapper exception_;
};

/**
 * Build a raw frame buffer with specified type, stream ID, flags, and optional
 * error code for ERROR frames.
 */
std::unique_ptr<folly::IOBuf> buildFrame(
    apache::thrift::fast_thrift::frame::FrameType type,
    uint32_t streamId,
    uint16_t flags = 0,
    std::optional<uint32_t> errorCode = std::nullopt,
    std::string_view errorMessage = {}) {
  const auto& desc = apache::thrift::fast_thrift::frame::getDescriptor(type);
  size_t headerSize = desc.headerSize > 0
      ? desc.headerSize
      : apache::thrift::fast_thrift::frame::kBaseHeaderSize;
  size_t totalSize = headerSize + errorMessage.size();

  auto buf = folly::IOBuf::create(totalSize);
  auto* data = buf->writableData();
  std::memset(data, 0, totalSize);

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

  // For ERROR frames, write error code at bytes 6-9 (part of header)
  if (type == apache::thrift::fast_thrift::frame::FrameType::ERROR &&
      errorCode.has_value()) {
    uint32_t code = *errorCode;
    data[6] = static_cast<uint8_t>((code >> 24) & 0xFF);
    data[7] = static_cast<uint8_t>((code >> 16) & 0xFF);
    data[8] = static_cast<uint8_t>((code >> 8) & 0xFF);
    data[9] = static_cast<uint8_t>(code & 0xFF);
  }

  // Append error message payload after header
  if (!errorMessage.empty()) {
    std::memcpy(data + headerSize, errorMessage.data(), errorMessage.size());
  }

  buf->append(totalSize);
  return buf;
}

apache::thrift::fast_thrift::frame::read::ParsedFrame parseTestFrame(
    apache::thrift::fast_thrift::frame::FrameType type,
    uint32_t streamId,
    uint16_t flags = 0,
    std::optional<uint32_t> errorCode = std::nullopt,
    std::string_view errorMessage = {}) {
  return apache::thrift::fast_thrift::frame::read::parseFrame(
      buildFrame(type, streamId, flags, errorCode, errorMessage));
}

/**
 * Helper to wrap a ParsedFrame into RocketResponseMessage for onRead testing.
 */
RocketResponseMessage makeRocketResponse(
    apache::thrift::fast_thrift::frame::FrameType type,
    uint32_t streamId,
    uint16_t flags = 0,
    std::optional<uint32_t> errorCode = std::nullopt,
    std::string_view errorMessage = {}) {
  return RocketResponseMessage{
      .frame = parseTestFrame(type, streamId, flags, errorCode, errorMessage),
  };
}

} // namespace

class RocketClientErrorFrameHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  MockErrorContext ctx_;
  RocketClientErrorFrameHandler handler_;
};

// =============================================================================
// Handler Lifecycle
// =============================================================================

TEST_F(RocketClientErrorFrameHandlerTest, HandlerAddedAndRemovedNoOp) {
  // Just verify lifecycle methods don't crash
  handler_.handlerAdded(ctx_);
  handler_.handlerRemoved(ctx_);
}

TEST_F(RocketClientErrorFrameHandlerTest, OnConnectNoOp) {
  // onConnect should be a no-op
  handler_.onPipelineActive(ctx_);
  EXPECT_FALSE(ctx_.hasException());
  EXPECT_EQ(ctx_.readMessages().size(), 0);
}

// =============================================================================
// Non-ERROR Frame Pass-through
// =============================================================================

TEST_F(RocketClientErrorFrameHandlerTest, PayloadFramePassesThrough) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD, 1)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_FALSE(ctx_.hasException());
}

TEST_F(RocketClientErrorFrameHandlerTest, KeepAliveFramePassesThrough) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::KEEPALIVE, 0)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_FALSE(ctx_.hasException());
}

TEST_F(RocketClientErrorFrameHandlerTest, MetadataPushFramePassesThrough) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH, 0)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_FALSE(ctx_.hasException());
}

TEST_F(RocketClientErrorFrameHandlerTest, SetupFramePassesThrough) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::SETUP, 0)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_FALSE(ctx_.hasException());
}

// =============================================================================
// Stream-Level ERROR Frame Pass-through (streamId > 0)
// =============================================================================

TEST_F(RocketClientErrorFrameHandlerTest, StreamLevelErrorPassesThrough) {
  // ERROR frame with streamId > 0 should pass through, not be handled here
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          1, // stream-level
          0,
          static_cast<uint32_t>(apache::thrift::fast_thrift::frame::ErrorCode::
                                    APPLICATION_ERROR))));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_FALSE(ctx_.hasException());
}

TEST_F(RocketClientErrorFrameHandlerTest, StreamLevelRejectedPassesThrough) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          5, // stream-level
          0,
          static_cast<uint32_t>(
              apache::thrift::fast_thrift::frame::ErrorCode::REJECTED))));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_FALSE(ctx_.hasException());
}

// =============================================================================
// Connection-Level ERROR Frame Handling (streamId == 0)
// =============================================================================

TEST_F(RocketClientErrorFrameHandlerTest, ConnectionCloseFiresException) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0, // connection-level
          0,
          static_cast<uint32_t>(apache::thrift::fast_thrift::frame::ErrorCode::
                                    CONNECTION_CLOSE))));

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_TRUE(ctx_.hasException());
  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(), apache::thrift::transport::TTransportException::NOT_OPEN);
}

TEST_F(RocketClientErrorFrameHandlerTest, ConnectionErrorFiresException) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0,
          0,
          static_cast<uint32_t>(apache::thrift::fast_thrift::frame::ErrorCode::
                                    CONNECTION_ERROR))));

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_TRUE(ctx_.hasException());

  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(),
      apache::thrift::transport::TTransportException::END_OF_FILE);
}

TEST_F(RocketClientErrorFrameHandlerTest, InvalidSetupFiresException) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0,
          0,
          static_cast<uint32_t>(
              apache::thrift::fast_thrift::frame::ErrorCode::INVALID_SETUP))));

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_TRUE(ctx_.hasException());

  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(),
      apache::thrift::transport::TTransportException::INVALID_SETUP);
  EXPECT_NE(std::string(ex->what()).find("invalid setup"), std::string::npos);
}

TEST_F(RocketClientErrorFrameHandlerTest, UnsupportedSetupFiresException) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0,
          0,
          static_cast<uint32_t>(apache::thrift::fast_thrift::frame::ErrorCode::
                                    UNSUPPORTED_SETUP))));

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_TRUE(ctx_.hasException());

  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(),
      apache::thrift::transport::TTransportException::NOT_SUPPORTED);
  EXPECT_NE(
      std::string(ex->what()).find("unsupported setup"), std::string::npos);
}

TEST_F(RocketClientErrorFrameHandlerTest, RejectedSetupFiresException) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0,
          0,
          static_cast<uint32_t>(
              apache::thrift::fast_thrift::frame::ErrorCode::REJECTED_SETUP))));

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_TRUE(ctx_.hasException());

  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(), apache::thrift::transport::TTransportException::NOT_OPEN);
  EXPECT_NE(std::string(ex->what()).find("setup rejected"), std::string::npos);
}

TEST_F(RocketClientErrorFrameHandlerTest, RejectedResumeFiresException) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0,
          0,
          static_cast<uint32_t>(apache::thrift::fast_thrift::frame::ErrorCode::
                                    REJECTED_RESUME))));

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_TRUE(ctx_.hasException());

  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(), apache::thrift::transport::TTransportException::NOT_OPEN);
  EXPECT_NE(std::string(ex->what()).find("resume rejected"), std::string::npos);
}

TEST_F(RocketClientErrorFrameHandlerTest, UnknownErrorCodeFiresException) {
  // Use an unknown error code value
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0,
          0,
          0x99999999))); // Unknown code

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_TRUE(ctx_.hasException());

  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(),
      apache::thrift::transport::TTransportException::END_OF_FILE);
  EXPECT_NE(std::string(ex->what()).find("Unhandled error"), std::string::npos);
}

TEST_F(
    RocketClientErrorFrameHandlerTest,
    ApplicationErrorOnConnectionFiresException) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0, // connection-level
          0,
          static_cast<uint32_t>(apache::thrift::fast_thrift::frame::ErrorCode::
                                    APPLICATION_ERROR))));

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_TRUE(ctx_.hasException());

  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(),
      apache::thrift::transport::TTransportException::END_OF_FILE);
  EXPECT_NE(std::string(ex->what()).find("Unhandled error"), std::string::npos);
}

TEST_F(RocketClientErrorFrameHandlerTest, RejectedOnConnectionFiresException) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0, // connection-level
          0,
          static_cast<uint32_t>(
              apache::thrift::fast_thrift::frame::ErrorCode::REJECTED))));

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_TRUE(ctx_.hasException());

  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(),
      apache::thrift::transport::TTransportException::END_OF_FILE);
  EXPECT_NE(std::string(ex->what()).find("Unhandled error"), std::string::npos);
}

TEST_F(RocketClientErrorFrameHandlerTest, CanceledOnConnectionFiresException) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0, // connection-level
          0,
          static_cast<uint32_t>(
              apache::thrift::fast_thrift::frame::ErrorCode::CANCELED))));

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_TRUE(ctx_.hasException());

  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(),
      apache::thrift::transport::TTransportException::END_OF_FILE);
  EXPECT_NE(std::string(ex->what()).find("Unhandled error"), std::string::npos);
}

TEST_F(RocketClientErrorFrameHandlerTest, InvalidOnConnectionFiresException) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0, // connection-level
          0,
          static_cast<uint32_t>(
              apache::thrift::fast_thrift::frame::ErrorCode::INVALID))));

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_TRUE(ctx_.hasException());

  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(),
      apache::thrift::transport::TTransportException::END_OF_FILE);
  EXPECT_NE(std::string(ex->what()).find("Unhandled error"), std::string::npos);
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST_F(
    RocketClientErrorFrameHandlerTest, ErrorFrameWithEmptyPayloadUsesReserved) {
  // ERROR frame without error code payload should use RESERVED (0)
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0,
          0,
          std::nullopt))); // No error code

  EXPECT_EQ(result, Result::Error);
  EXPECT_TRUE(ctx_.hasException());

  // RESERVED falls through to default case
  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
}

// =============================================================================
// Exception Forwarding
// =============================================================================

TEST_F(RocketClientErrorFrameHandlerTest, OnExceptionForwardsToContext) {
  auto ex = folly::make_exception_wrapper<std::runtime_error>("test error");
  handler_.onException(ctx_, std::move(ex));

  EXPECT_TRUE(ctx_.hasException());
  EXPECT_TRUE(ctx_.exception().is_compatible_with<std::runtime_error>());
}

// =============================================================================
// ErrorCode toString Tests
// =============================================================================

TEST_F(RocketClientErrorFrameHandlerTest, ToStringReturnsCorrectValues) {
  EXPECT_EQ(
      toString(apache::thrift::fast_thrift::frame::ErrorCode::RESERVED),
      "RESERVED");
  EXPECT_EQ(
      toString(apache::thrift::fast_thrift::frame::ErrorCode::INVALID_SETUP),
      "INVALID_SETUP");
  EXPECT_EQ(
      toString(
          apache::thrift::fast_thrift::frame::ErrorCode::UNSUPPORTED_SETUP),
      "UNSUPPORTED_SETUP");
  EXPECT_EQ(
      toString(apache::thrift::fast_thrift::frame::ErrorCode::REJECTED_SETUP),
      "REJECTED_SETUP");
  EXPECT_EQ(
      toString(apache::thrift::fast_thrift::frame::ErrorCode::REJECTED_RESUME),
      "REJECTED_RESUME");
  EXPECT_EQ(
      toString(apache::thrift::fast_thrift::frame::ErrorCode::CONNECTION_ERROR),
      "CONNECTION_ERROR");
  EXPECT_EQ(
      toString(apache::thrift::fast_thrift::frame::ErrorCode::CONNECTION_CLOSE),
      "CONNECTION_CLOSE");
  EXPECT_EQ(
      toString(
          apache::thrift::fast_thrift::frame::ErrorCode::APPLICATION_ERROR),
      "APPLICATION_ERROR");
  EXPECT_EQ(
      toString(apache::thrift::fast_thrift::frame::ErrorCode::REJECTED),
      "REJECTED");
  EXPECT_EQ(
      toString(apache::thrift::fast_thrift::frame::ErrorCode::CANCELED),
      "CANCELED");
  EXPECT_EQ(
      toString(apache::thrift::fast_thrift::frame::ErrorCode::INVALID),
      "INVALID");
  EXPECT_EQ(
      toString(apache::thrift::fast_thrift::frame::ErrorCode::RESERVED_EXT),
      "RESERVED_EXT");
}

TEST_F(
    RocketClientErrorFrameHandlerTest, ToStringReturnsUnknownForInvalidCode) {
  auto unknownCode =
      static_cast<apache::thrift::fast_thrift::frame::ErrorCode>(0xDEADBEEF);
  EXPECT_EQ(toString(unknownCode), "UNKNOWN");
}

// =============================================================================
// Error Message Extraction Tests
// =============================================================================

TEST_F(
    RocketClientErrorFrameHandlerTest, ErrorFrameWithMessageExtractsMessage) {
  std::string_view errorMsg = "Connection failed";
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0, // connection-level
          0,
          static_cast<uint32_t>(
              apache::thrift::fast_thrift::frame::ErrorCode::CONNECTION_ERROR),
          errorMsg)));

  EXPECT_EQ(result, Result::Error);
  EXPECT_TRUE(ctx_.hasException());

  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(),
      apache::thrift::transport::TTransportException::END_OF_FILE);
  // Verify the error message from the frame is included in the exception
  EXPECT_NE(
      std::string(ex->what()).find("Connection failed"), std::string::npos);
}

TEST_F(RocketClientErrorFrameHandlerTest, ErrorFrameWithEmptyMessageWorks) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0,
          0,
          static_cast<uint32_t>(
              apache::thrift::fast_thrift::frame::ErrorCode::CONNECTION_ERROR),
          "" // empty message
          )));

  EXPECT_EQ(result, Result::Error);
  EXPECT_TRUE(ctx_.hasException());

  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  // With empty message, should still have the base error description
  EXPECT_NE(
      std::string(ex->what()).find("Connection error from server"),
      std::string::npos);
  // Should NOT have extra colon from empty message
  EXPECT_EQ(
      std::string(ex->what()).find("Connection error from server:"),
      std::string::npos);
}

TEST_F(
    RocketClientErrorFrameHandlerTest,
    ErrorFrameWithLongMessageExtractsMessage) {
  std::string longMsg(1024, 'x'); // 1KB message
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0,
          0,
          static_cast<uint32_t>(
              apache::thrift::fast_thrift::frame::ErrorCode::REJECTED_SETUP),
          longMsg)));

  EXPECT_EQ(result, Result::Error);
  EXPECT_TRUE(ctx_.hasException());

  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(), apache::thrift::transport::TTransportException::NOT_OPEN);
  // Verify the long message is included
  EXPECT_NE(std::string(ex->what()).find(longMsg), std::string::npos);
}

TEST_F(
    RocketClientErrorFrameHandlerTest, ErrorFrameMessageWithSpecialCharsWorks) {
  std::string_view errorMsg = "Error: connection refused\n\ttimeout=100ms";
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR,
          0,
          0,
          static_cast<uint32_t>(
              apache::thrift::fast_thrift::frame::ErrorCode::CONNECTION_ERROR),
          errorMsg)));

  EXPECT_EQ(result, Result::Error);
  EXPECT_TRUE(ctx_.hasException());

  auto* ex =
      ctx_.exception()
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(ex, nullptr);
  // Verify special characters are preserved in the message
  EXPECT_NE(
      std::string(ex->what()).find("connection refused"), std::string::npos);
  EXPECT_NE(std::string(ex->what()).find("timeout=100ms"), std::string::npos);
}

} // namespace apache::thrift::fast_thrift::rocket::client::handler
