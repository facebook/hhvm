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

#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/ErrorDecoding.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {
namespace {

// Error codes matching apache::thrift::rocket::ErrorCode
constexpr uint32_t kApplicationError = 0x00000201;
constexpr uint32_t kRejected = 0x00000202;
constexpr uint32_t kCanceled = 0x00000203;
constexpr uint32_t kInvalid = 0x00000204;

/**
 * Helper to create an ERROR frame for testing.
 */
std::unique_ptr<folly::IOBuf> makeErrorFrame(
    uint32_t streamId, uint32_t errorCode, std::unique_ptr<folly::IOBuf> data) {
  return apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId, .errorCode = errorCode},
      nullptr,
      std::move(data));
}

/**
 * Helper to serialize ResponseRpcError using Compact protocol.
 */
std::unique_ptr<folly::IOBuf> serializeResponseRpcError(
    const apache::thrift::ResponseRpcError& error) {
  apache::thrift::CompactProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  error.write(&writer);
  return queue.move();
}

// =============================================================================
// APPLICATION_ERROR Tests (generic error handling)
// =============================================================================

TEST(ErrorDecodingTest, ApplicationErrorReturnsGenericException) {
  auto frame = makeErrorFrame(
      1, kApplicationError, folly::IOBuf::copyBuffer("Something went wrong"));
  auto parsed =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));

  auto error = decodeErrorFrame(parsed);

  EXPECT_TRUE(
      error.is_compatible_with<apache::thrift::TApplicationException>());
  error.handle([](const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(ex.getType(), apache::thrift::TApplicationException::UNKNOWN);
    EXPECT_THAT(
        std::string(ex.what()),
        testing::HasSubstr("Unexpected error frame type: 513"));
  });
}

// =============================================================================
// REJECTED Error Code Tests (with ResponseRpcError payload)
// =============================================================================

TEST(ErrorDecodingTest, RejectedWithOverloadReturnsLoadshedding) {
  apache::thrift::ResponseRpcError responseError;
  responseError.code() = apache::thrift::ResponseRpcErrorCode::OVERLOAD;
  responseError.what_utf8() = "Server overloaded";

  auto frame =
      makeErrorFrame(1, kRejected, serializeResponseRpcError(responseError));
  auto parsed =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));

  auto error = decodeErrorFrame(parsed);

  EXPECT_TRUE(
      error.is_compatible_with<apache::thrift::TApplicationException>());
  error.handle([](const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(
        ex.getType(), apache::thrift::TApplicationException::LOADSHEDDING);
    EXPECT_EQ(std::string(ex.what()), "Server overloaded");
  });
}

TEST(ErrorDecodingTest, RejectedWithTaskExpiredReturnsTimeout) {
  apache::thrift::ResponseRpcError responseError;
  responseError.code() = apache::thrift::ResponseRpcErrorCode::TASK_EXPIRED;
  responseError.what_utf8() = "Task expired";

  auto frame =
      makeErrorFrame(1, kRejected, serializeResponseRpcError(responseError));
  auto parsed =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));

  auto error = decodeErrorFrame(parsed);

  EXPECT_TRUE(
      error.is_compatible_with<apache::thrift::TApplicationException>());
  error.handle([](const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(ex.getType(), apache::thrift::TApplicationException::TIMEOUT);
    EXPECT_EQ(std::string(ex.what()), "Task expired");
  });
}

TEST(ErrorDecodingTest, RejectedWithQueueOverloadedReturnsLoadshedding) {
  apache::thrift::ResponseRpcError responseError;
  responseError.code() = apache::thrift::ResponseRpcErrorCode::QUEUE_OVERLOADED;
  responseError.what_utf8() = "Queue overloaded";

  auto frame =
      makeErrorFrame(1, kRejected, serializeResponseRpcError(responseError));
  auto parsed =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));

  auto error = decodeErrorFrame(parsed);

  EXPECT_TRUE(
      error.is_compatible_with<apache::thrift::TApplicationException>());
  error.handle([](const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(
        ex.getType(), apache::thrift::TApplicationException::LOADSHEDDING);
    EXPECT_EQ(std::string(ex.what()), "Queue overloaded");
  });
}

TEST(ErrorDecodingTest, RejectedWithUnknownMethodReturnsUnknownMethod) {
  apache::thrift::ResponseRpcError responseError;
  responseError.code() = apache::thrift::ResponseRpcErrorCode::UNKNOWN_METHOD;
  responseError.what_utf8() = "Method not found";

  auto frame =
      makeErrorFrame(1, kRejected, serializeResponseRpcError(responseError));
  auto parsed =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));

  auto error = decodeErrorFrame(parsed);

  EXPECT_TRUE(
      error.is_compatible_with<apache::thrift::TApplicationException>());
  error.handle([](const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(
        ex.getType(), apache::thrift::TApplicationException::UNKNOWN_METHOD);
    EXPECT_EQ(std::string(ex.what()), "Method not found");
  });
}

// =============================================================================
// CANCELED Error Code Tests
// =============================================================================

TEST(ErrorDecodingTest, CanceledWithResponseRpcErrorParsesCorrectly) {
  apache::thrift::ResponseRpcError responseError;
  responseError.code() = apache::thrift::ResponseRpcErrorCode::INTERRUPTION;
  responseError.what_utf8() = "Request cancelled";

  auto frame =
      makeErrorFrame(1, kCanceled, serializeResponseRpcError(responseError));
  auto parsed =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));

  auto error = decodeErrorFrame(parsed);

  EXPECT_TRUE(
      error.is_compatible_with<apache::thrift::TApplicationException>());
  error.handle([](const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(
        ex.getType(), apache::thrift::TApplicationException::INTERRUPTION);
    EXPECT_EQ(std::string(ex.what()), "Request cancelled");
  });
}

// =============================================================================
// INVALID Error Code Tests
// =============================================================================

TEST(ErrorDecodingTest, InvalidWithResponseRpcErrorParsesCorrectly) {
  apache::thrift::ResponseRpcError responseError;
  responseError.code() =
      apache::thrift::ResponseRpcErrorCode::REQUEST_PARSING_FAILURE;
  responseError.what_utf8() = "Invalid request";

  auto frame =
      makeErrorFrame(1, kInvalid, serializeResponseRpcError(responseError));
  auto parsed =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));

  auto error = decodeErrorFrame(parsed);

  EXPECT_TRUE(
      error.is_compatible_with<apache::thrift::TApplicationException>());
  error.handle([](const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(
        ex.getType(),
        apache::thrift::TApplicationException::UNSUPPORTED_CLIENT_TYPE);
    EXPECT_EQ(std::string(ex.what()), "Invalid request");
  });
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST(ErrorDecodingTest, RejectedWithEmptyPayloadReturnsUnknownError) {
  auto frame = makeErrorFrame(1, kRejected, nullptr);
  auto parsed =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));

  auto error = decodeErrorFrame(parsed);

  EXPECT_TRUE(
      error.is_compatible_with<apache::thrift::TApplicationException>());
  error.handle([](const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(ex.getType(), apache::thrift::TApplicationException::UNKNOWN);
  });
}

TEST(ErrorDecodingTest, RejectedWithMalformedPayloadReturnsParsingError) {
  auto frame = makeErrorFrame(
      1, kRejected, folly::IOBuf::copyBuffer("not valid thrift"));
  auto parsed =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));

  auto error = decodeErrorFrame(parsed);

  EXPECT_TRUE(
      error.is_compatible_with<apache::thrift::TApplicationException>());
  error.handle([](const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(ex.getType(), apache::thrift::TApplicationException::UNKNOWN);
    EXPECT_THAT(
        std::string(ex.what()),
        testing::HasSubstr("Error parsing error frame"));
  });
}

TEST(ErrorDecodingTest, RejectedWithUnknownErrorCodeReturnsUnknown) {
  apache::thrift::ResponseRpcError responseError;
  responseError.code() = apache::thrift::ResponseRpcErrorCode::UNKNOWN;
  responseError.what_utf8() = "Unknown error";

  auto frame =
      makeErrorFrame(1, kRejected, serializeResponseRpcError(responseError));
  auto parsed =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));

  auto error = decodeErrorFrame(parsed);

  EXPECT_TRUE(
      error.is_compatible_with<apache::thrift::TApplicationException>());
  error.handle([](const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(ex.getType(), apache::thrift::TApplicationException::UNKNOWN);
    EXPECT_EQ(std::string(ex.what()), "Unknown error");
  });
}

} // namespace
} // namespace apache::thrift::fast_thrift::thrift
