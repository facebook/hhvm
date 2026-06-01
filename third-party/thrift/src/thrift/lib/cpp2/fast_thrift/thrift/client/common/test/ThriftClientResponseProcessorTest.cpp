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

#include <gmock/gmock.h>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/ThriftClientResponseProcessor.h>

namespace apache::thrift::fast_thrift::thrift {

class ThriftClientResponseProcessorTest : public ::testing::Test {
 protected:
  apache::thrift::fast_thrift::frame::read::ParsedFrame createPayloadFrame(
      const apache::thrift::ResponseRpcMetadata& metadata,
      const std::string& data = "") {
    apache::thrift::BinaryProtocolWriter writer;
    folly::IOBufQueue metadataQueue(folly::IOBufQueue::cacheChainLength());
    writer.setOutput(&metadataQueue);
    metadata.write(&writer);
    auto serializedMetadata = metadataQueue.move();

    std::unique_ptr<folly::IOBuf> dataBuffer;
    if (!data.empty()) {
      dataBuffer = folly::IOBuf::copyBuffer(data);
    }

    auto frame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::PayloadHeader{
            .streamId = 1,
            .follows = false,
            .complete = true,
            .next = true,
        },
        std::move(serializedMetadata),
        std::move(dataBuffer));

    return apache::thrift::fast_thrift::frame::read::parseFrame(
        std::move(frame));
  }

  std::unique_ptr<folly::IOBuf> serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode code, const std::string& what) {
    apache::thrift::ResponseRpcError error;
    error.code() = code;
    error.what_utf8() = what;

    apache::thrift::CompactProtocolWriter writer;
    folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
    writer.setOutput(&queue);
    error.write(&writer);
    return queue.move();
  }

  apache::thrift::fast_thrift::frame::read::ParsedFrame createErrorFrame(
      uint32_t errorCode, std::unique_ptr<folly::IOBuf> data = nullptr) {
    auto frame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::ErrorHeader{
            .streamId = 1, .errorCode = errorCode},
        nullptr,
        std::move(data));
    return apache::thrift::fast_thrift::frame::read::parseFrame(
        std::move(frame));
  }
};

// =============================================================================
// Successful Payload Processing
// =============================================================================

TEST_F(ThriftClientResponseProcessorTest, PayloadFrameExtractsData) {
  apache::thrift::ResponseRpcMetadata metadata;
  const std::string expectedData = "response payload";

  auto frame = createPayloadFrame(metadata, expectedData);

  auto result = processRequestResponseFrame(frame);

  ASSERT_TRUE(result.hasValue());
  ASSERT_NE(result.value(), nullptr);

  std::string actualData(
      reinterpret_cast<const char*>(result.value()->data()),
      result.value()->computeChainDataLength());
  EXPECT_EQ(actualData, expectedData);
}

TEST_F(ThriftClientResponseProcessorTest, EmptyPayloadSucceeds) {
  apache::thrift::ResponseRpcMetadata metadata;

  auto frame = createPayloadFrame(metadata, "");

  auto result = processRequestResponseFrame(frame);

  ASSERT_TRUE(result.hasValue());
}

TEST_F(ThriftClientResponseProcessorTest, NullMetadataSucceeds) {
  apache::thrift::ResponseRpcMetadata metadata;

  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue metadataQueue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&metadataQueue);
  metadata.write(&writer);

  auto rawFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::PayloadHeader{
          .streamId = 1,
          .follows = false,
          .complete = true,
          .next = true,
      },
      metadataQueue.move(),
      folly::IOBuf::copyBuffer("data"));

  auto frame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(rawFrame));

  auto result = processRequestResponseFrame(frame);

  ASSERT_TRUE(result.hasValue());
}

TEST_F(
    ThriftClientResponseProcessorTest, DeclaredExceptionPayloadExtractsData) {
  apache::thrift::ResponseRpcMetadata metadata;

  apache::thrift::PayloadExceptionMetadata exceptionMeta;
  apache::thrift::PayloadDeclaredExceptionMetadata declMeta;
  exceptionMeta.declaredException() = std::move(declMeta);

  apache::thrift::PayloadExceptionMetadataBase exBase;
  exBase.name_utf8() = "MyCustomException";
  exBase.what_utf8() = "Custom exception message";
  exBase.metadata() = std::move(exceptionMeta);

  apache::thrift::PayloadMetadata payloadMeta;
  payloadMeta.exceptionMetadata() = std::move(exBase);
  metadata.payloadMetadata() = std::move(payloadMeta);

  auto frame = createPayloadFrame(metadata, "exception payload data");

  auto result = processRequestResponseFrame(frame);

  ASSERT_TRUE(result.hasValue());
}

// =============================================================================
// ERROR Frame Handling
// =============================================================================

TEST_F(ThriftClientResponseProcessorTest, ErrorFrameWithApplicationErrorCode) {
  auto errorMsg = folly::IOBuf::copyBuffer("Application error occurred");
  auto frame = createErrorFrame(0x00000201, std::move(errorMsg));

  auto result = processRequestResponseFrame(frame);

  ASSERT_TRUE(result.hasError());
  EXPECT_TRUE(result.error()
                  .is_compatible_with<apache::thrift::TApplicationException>());

  auto* ex =
      result.error().get_exception<apache::thrift::TApplicationException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(ex->getType(), apache::thrift::TApplicationException::UNKNOWN);
  EXPECT_THAT(ex->what(), testing::HasSubstr("Unexpected error frame type"));
}

TEST_F(ThriftClientResponseProcessorTest, ErrorFrameWithRejectedCode_Overload) {
  auto payload = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::OVERLOAD, "Server overloaded");
  auto frame = createErrorFrame(0x00000202, std::move(payload));

  auto result = processRequestResponseFrame(frame);

  ASSERT_TRUE(result.hasError());
  auto* ex =
      result.error().get_exception<apache::thrift::TApplicationException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(ex->getType(), apache::thrift::TApplicationException::LOADSHEDDING);
  EXPECT_THAT(ex->what(), testing::HasSubstr("Server overloaded"));
}

TEST_F(
    ThriftClientResponseProcessorTest, ErrorFrameWithRejectedCode_TaskExpired) {
  auto payload = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::TASK_EXPIRED, "Request timed out");
  auto frame = createErrorFrame(0x00000202, std::move(payload));

  auto result = processRequestResponseFrame(frame);

  ASSERT_TRUE(result.hasError());
  auto* ex =
      result.error().get_exception<apache::thrift::TApplicationException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(ex->getType(), apache::thrift::TApplicationException::TIMEOUT);
  EXPECT_THAT(ex->what(), testing::HasSubstr("Request timed out"));
}

TEST_F(ThriftClientResponseProcessorTest, ErrorFrameWithCanceledCode) {
  auto payload = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::SHUTDOWN, "Server shutting down");
  auto frame = createErrorFrame(0x00000203, std::move(payload));

  auto result = processRequestResponseFrame(frame);

  ASSERT_TRUE(result.hasError());
  auto* ex =
      result.error().get_exception<apache::thrift::TApplicationException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(ex->getType(), apache::thrift::TApplicationException::LOADSHEDDING);
}

TEST_F(ThriftClientResponseProcessorTest, ErrorFrameWithInvalidCode) {
  auto payload = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::UNKNOWN_METHOD, "Method not found");
  auto frame = createErrorFrame(0x00000204, std::move(payload));

  auto result = processRequestResponseFrame(frame);

  ASSERT_TRUE(result.hasError());
  auto* ex =
      result.error().get_exception<apache::thrift::TApplicationException>();
  ASSERT_NE(ex, nullptr);
  EXPECT_EQ(
      ex->getType(), apache::thrift::TApplicationException::UNKNOWN_METHOD);
  EXPECT_THAT(ex->what(), testing::HasSubstr("Method not found"));
}

TEST_F(ThriftClientResponseProcessorTest, ErrorFrameWithEmptyPayload) {
  auto frame = createErrorFrame(0x00000202, nullptr);

  auto result = processRequestResponseFrame(frame);

  ASSERT_TRUE(result.hasError());
  EXPECT_TRUE(result.error()
                  .is_compatible_with<apache::thrift::TApplicationException>());
}

// =============================================================================
// Unexpected Frame Type Handling
// =============================================================================

TEST_F(ThriftClientResponseProcessorTest, UnexpectedFrameTypeReturnsError) {
  auto rawFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestNHeader{
          .streamId = 1,
          .requestN = 10,
      });

  auto frame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(rawFrame));

  auto result = processRequestResponseFrame(frame);

  ASSERT_TRUE(result.hasError());
  EXPECT_TRUE(result.error()
                  .is_compatible_with<apache::thrift::TApplicationException>());
}

} // namespace apache::thrift::fast_thrift::thrift
