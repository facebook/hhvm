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
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientMetadataPushHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/RocketFrameDecoder.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayload.h>
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

  // Build a ThriftResponseMessage carrying a typed ThriftMetadataPushPayload.
  ThriftResponseMessage makeMetadataPushResponse(
      const apache::thrift::ServerPushMetadata& meta) {
    ThriftResponseMessage response;
    response.payload = ThriftClientInboundPayloadVariant{
        ThriftMetadataPushPayload{
            .metadata = serializeServerPushMetadata(meta)},
        apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
    response.streamType =
        apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH;
    return response;
  }

  // Build a ThriftResponseMessage carrying a ThriftMetadataPushPayload with
  // raw bytes — for malformed-input tests.
  ThriftResponseMessage makeMetadataPushResponseRaw(
      std::unique_ptr<folly::IOBuf> metadata) {
    ThriftResponseMessage response;
    response.payload = ThriftClientInboundPayloadVariant{
        ThriftMetadataPushPayload{.metadata = std::move(metadata)},
        apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
    response.streamType =
        apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH;
    return response;
  }

  // Build a ThriftResponseMessage carrying a non-METADATA_PUSH alternative
  // (PAYLOAD/ERROR) — used for pass-through tests.
  ThriftResponseMessage makeFirstResponseMessage(
      void* requestContext = nullptr) {
    ThriftResponseMessage response;
    response.payload = ThriftClientInboundPayloadVariant{
        ThriftFirstResponsePayload{
            .data = folly::IOBuf::copyBuffer("test payload"),
            .metadata = std::make_unique<apache::thrift::ResponseRpcMetadata>(),
            .streamId = 1,
            .complete = true,
            .next = true},
        apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
    response.requestContext =
        apache::thrift::fast_thrift::rocket::borrow(requestContext);
    response.streamType =
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE;
    return response;
  }

  ThriftResponseMessage makeErrorResponseMessage(
      void* requestContext = nullptr) {
    ThriftResponseMessage response;
    response.payload = ThriftClientInboundPayloadVariant{
        ThriftErrorPayload{
            .data = folly::IOBuf::copyBuffer("error data"),
            .metadata = nullptr,
            .streamId = 1,
            .errorCode = 0x0201},
        apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
    response.requestContext =
        apache::thrift::fast_thrift::rocket::borrow(requestContext);
    response.streamType =
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE;
    return response;
  }

  // Transport-failure path — non-wire-derived ThriftClientResponseError.
  ThriftResponseMessage makeTransportErrorResponseMessage() {
    ThriftResponseMessage response;
    response.payload = ThriftClientResponseError{
        .ew = folly::make_exception_wrapper<std::runtime_error>("boom")};
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
  void* const kUserContext = reinterpret_cast<void*>(0x123);
  auto response = makeFirstResponseMessage(kUserContext);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  // Verify the message was passed through unchanged
  auto& forwarded = ctx_.readMessages()[0].get<ThriftResponseMessage>();
  EXPECT_EQ(forwarded.requestContext.get(), kUserContext);
  EXPECT_EQ(
      forwarded.streamType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
}

TEST_F(ThriftClientMetadataPushHandlerTest, ErrorFramePassesThrough) {
  auto response = makeErrorResponseMessage(reinterpret_cast<void*>(0x456));

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

TEST_F(ThriftClientMetadataPushHandlerTest, TransportErrorPassesThrough) {
  // ThriftClientResponseError (transport failure path) is not a wire-derived
  // payload — handler must forward it untouched so the tail can fail just
  // this callback.
  auto response = makeTransportErrorResponseMessage();

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

  auto response = makeMetadataPushResponse(serverMeta);

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

  auto response = makeMetadataPushResponse(serverMeta);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(handler_.isSetupComplete());
}

TEST_F(
    ThriftClientMetadataPushHandlerTest, SetupResponseDoesNotForwardMessage) {
  apache::thrift::ServerPushMetadata serverMeta;
  serverMeta.set_setupResponse();
  serverMeta.setupResponse()->version() = 8;

  auto response = makeMetadataPushResponse(serverMeta);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  // METADATA_PUSH frames are consumed, not forwarded
  EXPECT_EQ(ctx_.readMessages().size(), 0);
}

TEST_F(ThriftClientMetadataPushHandlerTest, SetupResponseWithMissingVersion) {
  apache::thrift::ServerPushMetadata serverMeta;
  serverMeta.set_setupResponse();
  // version not set - should default to 0

  auto response = makeMetadataPushResponse(serverMeta);

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

  auto response = makeMetadataPushResponse(serverMeta);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  // Stream headers are logged and ignored, not forwarded
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_EQ(ctx_.exceptions().size(), 0);
}

// =============================================================================
// Drain Complete Handling Tests
// =============================================================================

TEST_F(ThriftClientMetadataPushHandlerTest, DrainCompleteGenericIsSilent) {
  // Generic drain (no code) is informational — server tearing down via
  // CONNECTION_CLOSE drives the actual close. No exception expected.
  apache::thrift::ServerPushMetadata serverMeta;
  serverMeta.set_drainCompletePush();

  auto response = makeMetadataPushResponse(serverMeta);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_EQ(ctx_.exceptions().size(), 0);
}

TEST_F(
    ThriftClientMetadataPushHandlerTest, DrainCompleteWithMemoryLimitExceeded) {
  apache::thrift::ServerPushMetadata serverMeta;
  serverMeta.set_drainCompletePush();
  serverMeta.drainCompletePush()->drainCompleteCode() =
      apache::thrift::DrainCompleteCode::EXCEEDED_INGRESS_MEM_LIMIT;

  auto response = makeMetadataPushResponse(serverMeta);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.exceptions().size(), 1);

  auto* appEx = ctx_.exceptions()[0]
                    .get_exception<apache::thrift::TApplicationException>();
  ASSERT_NE(appEx, nullptr);
  EXPECT_EQ(
      appEx->getType(), apache::thrift::TApplicationException::LOADSHEDDING);
  EXPECT_NE(std::string(appEx->what()).find("memory limit"), std::string::npos);
}

// =============================================================================
// Error Cases
// =============================================================================

TEST_F(ThriftClientMetadataPushHandlerTest, MalformedMetadataReturnsError) {
  // Create a METADATA_PUSH frame with invalid/garbage data
  auto garbage = folly::IOBuf::copyBuffer("not valid compact protocol data!");

  auto response = makeMetadataPushResponseRaw(std::move(garbage));

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Error);
  ASSERT_EQ(ctx_.exceptions().size(), 1);
  auto* tex =
      ctx_.exceptions()[0]
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(tex, nullptr);
  EXPECT_EQ(
      tex->getType(),
      apache::thrift::transport::TTransportException::CORRUPTED_DATA);
}

TEST_F(ThriftClientMetadataPushHandlerTest, EmptyMetadataReturnsError) {
  auto response = makeMetadataPushResponseRaw(nullptr);

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Error);
  ASSERT_EQ(ctx_.exceptions().size(), 1);
  auto* tex =
      ctx_.exceptions()[0]
          .get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(tex, nullptr);
  EXPECT_EQ(
      tex->getType(),
      apache::thrift::transport::TTransportException::CORRUPTED_DATA);
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

  auto response = makeFirstResponseMessage(reinterpret_cast<void*>(0x789));

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Backpressure);
}

TEST_F(ThriftClientMetadataPushHandlerTest, ErrorPropagatedOnPassThrough) {
  ctx_.setReturnError(true);

  auto response = makeFirstResponseMessage(reinterpret_cast<void*>(0x789));

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Error);
}

// Rocket-server-style METADATA_PUSH wire-format coverage now lives in
// `thrift/client/util/test/RocketFrameDecoderTest.cpp` — the handler
// operates on already-decoded `ThriftMetadataPushPayload` and is
// decoupled from the wire shape.

} // namespace apache::thrift::fast_thrift::thrift::client::handler
