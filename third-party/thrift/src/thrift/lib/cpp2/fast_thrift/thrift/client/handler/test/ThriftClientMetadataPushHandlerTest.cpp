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
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientMetadataPushHandler.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift::client::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

namespace {

/**
 * MockHandlerContext for testing ThriftClientMetadataPushHandler.
 *
 * Captures messages fired via fireRead() and exceptions.
 */
class MockHandlerContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    if (returnError_) {
      return Result::Error;
    }
    readMessages_.push_back(std::move(msg));
    return returnBackpressure_ ? Result::Backpressure : Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exceptions_.push_back(std::move(e));
  }

  // NOLINTNEXTLINE(clang-diagnostic-unused-member-function)
  void close() noexcept { closeCalled_ = true; }

  void setReturnBackpressure(bool value) { returnBackpressure_ = value; }

  void setReturnError(bool value) { returnError_ = value; }

  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }

  std::vector<folly::exception_wrapper>& exceptions() { return exceptions_; }

  void reset() {
    readMessages_.clear();
    exceptions_.clear();
    returnBackpressure_ = false;
    returnError_ = false;
    closeCalled_ = false;
  }

 private:
  std::vector<TypeErasedBox> readMessages_;
  std::vector<folly::exception_wrapper> exceptions_;
  bool returnBackpressure_{false};
  bool returnError_{false};
  bool closeCalled_{false};
};

} // namespace

class ThriftClientMetadataPushHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ctx_.reset();
    handler_ = ThriftClientMetadataPushHandler();
  }

  Result callOnRead(TypeErasedBox msg) {
    return handler_.onRead(ctx_, std::move(msg));
  }

  void callOnException(folly::exception_wrapper&& e) {
    handler_.onException(ctx_, std::move(e));
  }

  // Helper to serialize ServerPushMetadata using Compact protocol
  std::unique_ptr<folly::IOBuf> serializeServerPushMetadata(
      const apache::thrift::ServerPushMetadata& meta) {
    apache::thrift::CompactProtocolWriter writer;
    folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
    writer.setOutput(&queue);
    meta.write(&writer);
    return queue.move();
  }

  // Helper to create a METADATA_PUSH frame with ServerPushMetadata
  apache::thrift::fast_thrift::frame::read::ParsedFrame createMetadataPushFrame(
      const apache::thrift::ServerPushMetadata& meta) {
    auto serialized = serializeServerPushMetadata(meta);
    auto frame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::MetadataPushHeader{},
        std::move(serialized));
    return apache::thrift::fast_thrift::frame::read::parseFrame(
        std::move(frame));
  }

  // Helper to create a METADATA_PUSH frame with raw metadata
  apache::thrift::fast_thrift::frame::read::ParsedFrame
  createMetadataPushFrameRaw(std::unique_ptr<folly::IOBuf> metadata) {
    auto frame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::MetadataPushHeader{},
        std::move(metadata));
    return apache::thrift::fast_thrift::frame::read::parseFrame(
        std::move(frame));
  }

  // Helper to create a payload frame (non-METADATA_PUSH)
  apache::thrift::fast_thrift::frame::read::ParsedFrame createPayloadFrame(
      uint32_t streamId = 1) {
    auto frame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::PayloadHeader{
            .streamId = streamId,
            .follows = false,
            .complete = true,
            .next = true,
        },
        nullptr,
        folly::IOBuf::copyBuffer("test payload"));
    return apache::thrift::fast_thrift::frame::read::parseFrame(
        std::move(frame));
  }

  // Helper to create an ERROR frame
  apache::thrift::fast_thrift::frame::read::ParsedFrame createErrorFrame(
      uint32_t streamId = 1) {
    auto frame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::ErrorHeader{
            .streamId = streamId, .errorCode = 0x0201},
        nullptr,
        folly::IOBuf::copyBuffer("error data"));
    return apache::thrift::fast_thrift::frame::read::parseFrame(
        std::move(frame));
  }

  // Helper to create a KEEPALIVE frame
  apache::thrift::fast_thrift::frame::read::ParsedFrame createKeepAliveFrame() {
    auto frame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::KeepAliveHeader{
            .lastReceivedPosition = 0, .respond = false},
        nullptr);
    return apache::thrift::fast_thrift::frame::read::parseFrame(
        std::move(frame));
  }

  // Helper to create a ThriftResponseMessage with ParsedFrame payload
  // (used for stream-level frames)
  ThriftResponseMessage makeParsedFrameResponse(
      apache::thrift::fast_thrift::frame::read::ParsedFrame frame,
      apache::thrift::fast_thrift::frame::FrameType requestFrameType,
      uint32_t requestHandle =
          apache::thrift::fast_thrift::rocket::kNoRequestHandle) {
    ThriftResponseMessage response;
    response.frame = std::move(frame);
    response.requestHandle = requestHandle;
    response.requestFrameType = requestFrameType;
    return response;
  }

  // Helper to create a ThriftResponseMessage with bare ParsedFrame payload
  // (used for connection-level frames like METADATA_PUSH, KEEPALIVE)
  ThriftResponseMessage makeFrameResponse(
      apache::thrift::fast_thrift::frame::read::ParsedFrame frame,
      apache::thrift::fast_thrift::frame::FrameType requestFrameType) {
    ThriftResponseMessage response;
    response.frame = std::move(frame);
    response.requestFrameType = requestFrameType;
    return response;
  }

  MockHandlerContext ctx_;
  ThriftClientMetadataPushHandler handler_;
};

// =============================================================================
// Handler Lifecycle Tests
// =============================================================================

TEST_F(ThriftClientMetadataPushHandlerTest, HandlerAddedAndRemovedNoOp) {
  // Just verify lifecycle methods don't crash
  handler_.handlerAdded(ctx_);
  handler_.handlerRemoved(ctx_);

  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_EQ(ctx_.exceptions().size(), 0);
}

// =============================================================================
// Non-METADATA_PUSH Pass-through Tests
// =============================================================================

TEST_F(ThriftClientMetadataPushHandlerTest, PayloadFramePassesThrough) {
  auto response = makeParsedFrameResponse(
      createPayloadFrame(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
      123);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  // Verify the message was passed through unchanged
  auto& forwarded = ctx_.readMessages()[0].get<ThriftResponseMessage>();
  EXPECT_EQ(forwarded.requestHandle, 123);
  EXPECT_EQ(
      forwarded.requestFrameType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
}

TEST_F(ThriftClientMetadataPushHandlerTest, ErrorFramePassesThrough) {
  auto response = makeParsedFrameResponse(
      createErrorFrame(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
      456);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

TEST_F(ThriftClientMetadataPushHandlerTest, KeepAliveFramePassesThrough) {
  auto response = makeParsedFrameResponse(
      createKeepAliveFrame(),
      apache::thrift::fast_thrift::frame::FrameType::KEEPALIVE);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

// =============================================================================
// Setup Response Handling Tests
// =============================================================================

TEST_F(ThriftClientMetadataPushHandlerTest, SetupResponseSetsServerVersion) {
  apache::thrift::ServerPushMetadata serverMeta;
  serverMeta.set_setupResponse();
  serverMeta.setupResponse()->version() = 8;
  serverMeta.setupResponse()->zstdSupported() = true;

  auto response = makeFrameResponse(
      createMetadataPushFrame(serverMeta),
      apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(handler_.serverVersion(), 8);
  EXPECT_TRUE(handler_.serverSupportsZstd());
}

TEST_F(ThriftClientMetadataPushHandlerTest, SetupResponseMarksSetupComplete) {
  EXPECT_FALSE(handler_.isSetupComplete());

  apache::thrift::ServerPushMetadata serverMeta;
  serverMeta.set_setupResponse();
  serverMeta.setupResponse()->version() = 7;

  auto response = makeFrameResponse(
      createMetadataPushFrame(serverMeta),
      apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(handler_.isSetupComplete());
}

TEST_F(
    ThriftClientMetadataPushHandlerTest, SetupResponseDoesNotForwardMessage) {
  apache::thrift::ServerPushMetadata serverMeta;
  serverMeta.set_setupResponse();
  serverMeta.setupResponse()->version() = 8;

  auto response = makeFrameResponse(
      createMetadataPushFrame(serverMeta),
      apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  // METADATA_PUSH frames are consumed, not forwarded
  EXPECT_EQ(ctx_.readMessages().size(), 0);
}

TEST_F(ThriftClientMetadataPushHandlerTest, SetupResponseWithMissingVersion) {
  apache::thrift::ServerPushMetadata serverMeta;
  serverMeta.set_setupResponse();
  // version not set - should default to 0

  auto response = makeFrameResponse(
      createMetadataPushFrame(serverMeta),
      apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(handler_.serverVersion(), 0);
  EXPECT_TRUE(handler_.isSetupComplete());
}

// =============================================================================
// Stream Headers Push Handling Tests
// =============================================================================

TEST_F(ThriftClientMetadataPushHandlerTest, StreamHeadersPushLogsAndIgnores) {
  apache::thrift::ServerPushMetadata serverMeta;
  serverMeta.set_streamHeadersPush();
  serverMeta.streamHeadersPush()->streamId() = 42;

  auto response = makeFrameResponse(
      createMetadataPushFrame(serverMeta),
      apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  // Stream headers are logged and ignored, not forwarded
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_EQ(ctx_.exceptions().size(), 0);
}

// =============================================================================
// Drain Complete Handling Tests
// =============================================================================

TEST_F(ThriftClientMetadataPushHandlerTest, DrainCompleteFiresException) {
  apache::thrift::ServerPushMetadata serverMeta;
  serverMeta.set_drainCompletePush();

  auto response = makeFrameResponse(
      createMetadataPushFrame(serverMeta),
      apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  ASSERT_EQ(ctx_.exceptions().size(), 1);
  EXPECT_TRUE(ctx_.exceptions()[0]
                  .is_compatible_with<apache::thrift::TApplicationException>());
}

TEST_F(
    ThriftClientMetadataPushHandlerTest, DrainCompleteWithMemoryLimitExceeded) {
  apache::thrift::ServerPushMetadata serverMeta;
  serverMeta.set_drainCompletePush();
  serverMeta.drainCompletePush()->drainCompleteCode() =
      apache::thrift::DrainCompleteCode::EXCEEDED_INGRESS_MEM_LIMIT;

  auto response = makeFrameResponse(
      createMetadataPushFrame(serverMeta),
      apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.exceptions().size(), 1);

  auto* appEx = ctx_.exceptions()[0]
                    .get_exception<apache::thrift::TApplicationException>();
  ASSERT_NE(appEx, nullptr);
  EXPECT_NE(std::string(appEx->what()).find("memory limit"), std::string::npos);
}

// =============================================================================
// Error Cases
// =============================================================================

TEST_F(ThriftClientMetadataPushHandlerTest, MalformedMetadataReturnsError) {
  // Create a METADATA_PUSH frame with invalid/garbage data
  auto garbage = folly::IOBuf::copyBuffer("not valid compact protocol data!");

  auto response = makeFrameResponse(
      createMetadataPushFrameRaw(std::move(garbage)),
      apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Error);
  ASSERT_EQ(ctx_.exceptions().size(), 1);
  EXPECT_TRUE(ctx_.exceptions()[0]
                  .is_compatible_with<apache::thrift::TApplicationException>());
}

TEST_F(ThriftClientMetadataPushHandlerTest, EmptyMetadataReturnsError) {
  // Create a METADATA_PUSH frame with empty metadata
  auto response = makeFrameResponse(
      createMetadataPushFrameRaw(nullptr),
      apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH);

  auto result = callOnRead(erase_and_box(std::move(response)));

  // Empty metadata will fail to deserialize
  EXPECT_EQ(result, Result::Error);
  ASSERT_EQ(ctx_.exceptions().size(), 1);
}

// =============================================================================
// Exception Forwarding Tests
// =============================================================================

TEST_F(ThriftClientMetadataPushHandlerTest, OnExceptionForwardsToContext) {
  auto exception =
      folly::make_exception_wrapper<std::runtime_error>("Test exception");

  callOnException(std::move(exception));

  ASSERT_EQ(ctx_.exceptions().size(), 1);
  EXPECT_TRUE(ctx_.exceptions()[0].is_compatible_with<std::runtime_error>());
}

// =============================================================================
// Backpressure and Error Propagation Tests
// =============================================================================

TEST_F(
    ThriftClientMetadataPushHandlerTest, BackpressurePropagatedOnPassThrough) {
  ctx_.setReturnBackpressure(true);

  auto response = makeParsedFrameResponse(
      createPayloadFrame(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
      789);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Backpressure);
}

TEST_F(ThriftClientMetadataPushHandlerTest, ErrorPropagatedOnPassThrough) {
  ctx_.setReturnError(true);

  auto response = makeParsedFrameResponse(
      createPayloadFrame(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
      789);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Error);
}

} // namespace apache::thrift::fast_thrift::thrift::client::handler
