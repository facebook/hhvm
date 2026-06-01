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

#include <thrift/lib/cpp2/fast_thrift/thrift/FastThriftAdapterBase.h>

#include <gtest/gtest.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {
namespace {

constexpr uint32_t kRejected = 0x00000202;

class TestAdapter : public FastThriftAdapterBase {
 public:
  using FastThriftAdapterBase::handleRequestResponse;
};

std::unique_ptr<folly::IOBuf> serializeResponseRpcError(
    const apache::thrift::ResponseRpcError& error) {
  apache::thrift::CompactProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  error.write(&writer);
  return queue.move();
}

std::unique_ptr<folly::IOBuf> serializeResponseMetadata(
    const apache::thrift::ResponseRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

ThriftResponseMessage makePayloadResponse(
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  auto frameBuf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::PayloadHeader{
          .streamId = 1, .complete = true, .next = true},
      std::move(metadata),
      std::move(data));
  ThriftResponseMessage response;
  response.frame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frameBuf));
  return response;
}

ThriftResponseMessage makeErrorResponse(
    uint32_t errorCode, std::unique_ptr<folly::IOBuf> data) {
  auto frameBuf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = 1, .errorCode = errorCode},
      nullptr,
      std::move(data));
  ThriftResponseMessage response;
  response.frame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frameBuf));
  return response;
}

ThriftResponseMessage makeCancelResponse() {
  auto frameBuf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::CancelHeader{.streamId = 1});
  ThriftResponseMessage response;
  response.frame =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frameBuf));
  return response;
}

// =============================================================================
// Frame type dispatch
// =============================================================================

TEST(FastThriftAdapterBaseTest, PayloadFramePassesThrough) {
  auto response =
      makePayloadResponse(nullptr, folly::IOBuf::copyBuffer("hello"));

  auto result = TestAdapter::handleRequestResponse(std::move(response));

  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(
      result.value().frame.type(),
      apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
}

TEST(FastThriftAdapterBaseTest, ErrorFrameReturnsDecodedException) {
  apache::thrift::ResponseRpcError rpcError;
  rpcError.code() = apache::thrift::ResponseRpcErrorCode::OVERLOAD;
  rpcError.what_utf8() = "Server overloaded";

  auto response =
      makeErrorResponse(kRejected, serializeResponseRpcError(rpcError));

  auto result = TestAdapter::handleRequestResponse(std::move(response));

  ASSERT_TRUE(result.hasError());
  EXPECT_TRUE(result.error()
                  .is_compatible_with<apache::thrift::TApplicationException>());
  result.error().handle([](const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(
        ex.getType(), apache::thrift::TApplicationException::LOADSHEDDING);
    EXPECT_EQ(std::string(ex.what()), "Server overloaded");
  });
}

TEST(FastThriftAdapterBaseTest, UnexpectedFrameTypeReturnsProtocolError) {
  auto response = makeCancelResponse();

  auto result = TestAdapter::handleRequestResponse(std::move(response));

  ASSERT_TRUE(result.hasError());
  EXPECT_TRUE(result.error()
                  .is_compatible_with<apache::thrift::TApplicationException>());
  result.error().handle([](const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(
        ex.getType(), apache::thrift::TApplicationException::PROTOCOL_ERROR);
  });
}

// =============================================================================
// Response metadata handling
// =============================================================================

TEST(FastThriftAdapterBaseTest, PayloadWithNormalMetadataPassesThrough) {
  apache::thrift::ResponseRpcMetadata metadata;
  metadata.payloadMetadata().ensure().set_responseMetadata(
      apache::thrift::PayloadResponseMetadata{});

  auto response = makePayloadResponse(
      serializeResponseMetadata(metadata), folly::IOBuf::copyBuffer("data"));

  auto result = TestAdapter::handleRequestResponse(std::move(response));

  ASSERT_TRUE(result.hasValue());
}

TEST(FastThriftAdapterBaseTest, PayloadWithUndeclaredExceptionReturnsError) {
  apache::thrift::ResponseRpcMetadata metadata;
  apache::thrift::PayloadExceptionMetadataBase exMeta;
  exMeta.what_utf8() = "Undeclared server exception";
  apache::thrift::PayloadExceptionMetadata exMetaInner;
  exMetaInner.set_appUnknownException(
      apache::thrift::PayloadAppUnknownExceptionMetdata{});
  exMeta.metadata() = std::move(exMetaInner);
  metadata.payloadMetadata().ensure().set_exceptionMetadata(std::move(exMeta));

  auto response = makePayloadResponse(
      serializeResponseMetadata(metadata), folly::IOBuf::copyBuffer("data"));

  auto result = TestAdapter::handleRequestResponse(std::move(response));

  ASSERT_TRUE(result.hasError());
  EXPECT_TRUE(result.error()
                  .is_compatible_with<apache::thrift::TApplicationException>());
}

TEST(FastThriftAdapterBaseTest, PayloadWithDeclaredExceptionPassesThrough) {
  apache::thrift::ResponseRpcMetadata metadata;
  apache::thrift::PayloadExceptionMetadataBase exMeta;
  exMeta.what_utf8() = "Declared exception";
  exMeta.name_utf8() = "MyException";
  apache::thrift::PayloadExceptionMetadata exMetaInner;
  exMetaInner.set_declaredException(
      apache::thrift::PayloadDeclaredExceptionMetadata{});
  exMeta.metadata() = std::move(exMetaInner);
  metadata.payloadMetadata().ensure().set_exceptionMetadata(std::move(exMeta));

  auto response = makePayloadResponse(
      serializeResponseMetadata(metadata), folly::IOBuf::copyBuffer("data"));

  auto result = TestAdapter::handleRequestResponse(std::move(response));

  // Declared exceptions pass through — generated code handles them
  ASSERT_TRUE(result.hasValue());
}

TEST(FastThriftAdapterBaseTest, PayloadWithNoMetadataPassesThrough) {
  auto response =
      makePayloadResponse(nullptr, folly::IOBuf::copyBuffer("data"));

  auto result = TestAdapter::handleRequestResponse(std::move(response));

  ASSERT_TRUE(result.hasValue());
}

} // namespace
} // namespace apache::thrift::fast_thrift::thrift
