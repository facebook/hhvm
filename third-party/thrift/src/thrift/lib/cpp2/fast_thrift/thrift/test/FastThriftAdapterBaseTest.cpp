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
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/RocketFrameDecoder.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayload.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/type/Protocol.h>
#include <thrift/lib/cpp2/type/Type.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <thrift/lib/thrift/gen-cpp2/any_rep_types.h>

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

ThriftResponseMessage makePayloadResponse(
    apache::thrift::ResponseRpcMetadata metadata,
    std::unique_ptr<folly::IOBuf> data) {
  ThriftResponseMessage response;
  response.payload = ThriftClientInboundPayloadVariant{
      ThriftFirstResponsePayload{
          .metadata = std::make_unique<apache::thrift::ResponseRpcMetadata>(
              std::move(metadata)),
          .data = std::move(data),
          .streamId = 1,
          .complete = true,
          .next = true},
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
  return response;
}

ThriftResponseMessage makeErrorResponse(
    uint32_t errorCode, std::unique_ptr<folly::IOBuf> data) {
  ThriftResponseMessage response;
  response.payload = ThriftClientInboundPayloadVariant{
      ThriftErrorPayload{
          .data = std::move(data),
          .metadata = nullptr,
          .streamId = 1,
          .errorCode = errorCode},
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
  return response;
}

ThriftResponseMessage makeCancelResponse() {
  ThriftResponseMessage response;
  response.payload = ThriftClientInboundPayloadVariant{
      ThriftCancelPayload{.streamId = 1},
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
  return response;
}

// =============================================================================
// Frame type dispatch
// =============================================================================

TEST(FastThriftAdapterBaseTest, PayloadFrameYieldsResultWithData) {
  auto response = makePayloadResponse({}, folly::IOBuf::copyBuffer("hello"));

  auto result = TestAdapter::handleRequestResponse(
      std::move(response), apache::thrift::protocol::T_COMPACT_PROTOCOL);

  ASSERT_TRUE(result.hasValue());
  ASSERT_NE(result.value(), nullptr);
  EXPECT_EQ(
      result.value()->moveToFbString().toStdString(), std::string{"hello"});
}

TEST(FastThriftAdapterBaseTest, ErrorFrameReturnsDecodedException) {
  apache::thrift::ResponseRpcError rpcError;
  rpcError.code() = apache::thrift::ResponseRpcErrorCode::OVERLOAD;
  rpcError.what_utf8() = "Server overloaded";

  auto response =
      makeErrorResponse(kRejected, serializeResponseRpcError(rpcError));

  auto result = TestAdapter::handleRequestResponse(
      std::move(response), apache::thrift::protocol::T_COMPACT_PROTOCOL);

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

  auto result = TestAdapter::handleRequestResponse(
      std::move(response), apache::thrift::protocol::T_COMPACT_PROTOCOL);

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
      std::move(metadata), folly::IOBuf::copyBuffer("data"));

  auto result = TestAdapter::handleRequestResponse(
      std::move(response), apache::thrift::protocol::T_COMPACT_PROTOCOL);

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
      std::move(metadata), folly::IOBuf::copyBuffer("data"));

  auto result = TestAdapter::handleRequestResponse(
      std::move(response), apache::thrift::protocol::T_COMPACT_PROTOCOL);

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
      std::move(metadata), folly::IOBuf::copyBuffer("data"));

  auto result = TestAdapter::handleRequestResponse(
      std::move(response), apache::thrift::protocol::T_COMPACT_PROTOCOL);

  // Declared exceptions pass through — generated code reads the presult
  // struct from the data IOBuf to surface the typed exception.
  ASSERT_TRUE(result.hasValue());
  ASSERT_NE(result.value(), nullptr);
}

TEST(FastThriftAdapterBaseTest, PayloadWithNoMetadataPassesThrough) {
  auto response = makePayloadResponse({}, folly::IOBuf::copyBuffer("data"));

  auto result = TestAdapter::handleRequestResponse(
      std::move(response), apache::thrift::protocol::T_COMPACT_PROTOCOL);

  ASSERT_TRUE(result.hasValue());
}

TEST(FastThriftAdapterBaseTest, PayloadWithMalformedAnyExceptionReturnsError) {
  // anyException variant: data IOBuf MUST be a SemiAnyStruct serialization.
  // Here we send raw bytes that are not a valid SemiAnyStruct, so
  // extractAnyException's Compact deserialization fails and returns a
  // TApplicationException describing the deserialization failure.
  apache::thrift::ResponseRpcMetadata metadata;
  apache::thrift::PayloadExceptionMetadataBase exMeta;
  exMeta.what_utf8() = "any exception thrown";
  apache::thrift::PayloadExceptionMetadata exMetaInner;
  exMetaInner.set_anyException(apache::thrift::PayloadAnyExceptionMetadata{});
  exMeta.metadata() = std::move(exMetaInner);
  metadata.payloadMetadata().ensure().set_exceptionMetadata(std::move(exMeta));

  auto response = makePayloadResponse(
      std::move(metadata),
      folly::IOBuf::copyBuffer("not-a-valid-semi-any-struct"));

  auto result = TestAdapter::handleRequestResponse(
      std::move(response), apache::thrift::protocol::T_COMPACT_PROTOCOL);

  ASSERT_TRUE(result.hasError());
  EXPECT_TRUE(result.error()
                  .is_compatible_with<apache::thrift::TApplicationException>());
}

TEST(FastThriftAdapterBaseTest, PayloadWithUnregisteredAnyExceptionTypeError) {
  // Build a real SemiAnyStruct pointing at a type URI that's not registered
  // in TypeRegistry. extractAnyException should deserialize the SemiAnyStruct
  // successfully but then fail to load — returning a TApplicationException.
  apache::thrift::type::SemiAnyStruct semiAny;
  semiAny.type() = apache::thrift::type::Type(
      apache::thrift::type::exception_c{},
      "facebook.com/test/UnregisteredException");
  semiAny.protocol() = apache::thrift::type::Protocol::get<
      apache::thrift::type::StandardProtocol::Compact>();
  semiAny.data() =
      folly::IOBuf(*folly::IOBuf::copyBuffer("inner-exception-payload"));

  folly::IOBufQueue payloadQueue;
  apache::thrift::CompactSerializer::serialize(semiAny, &payloadQueue);

  apache::thrift::ResponseRpcMetadata metadata;
  apache::thrift::PayloadExceptionMetadataBase exMeta;
  exMeta.what_utf8() = "any exception thrown";
  apache::thrift::PayloadExceptionMetadata exMetaInner;
  exMetaInner.set_anyException(apache::thrift::PayloadAnyExceptionMetadata{});
  exMeta.metadata() = std::move(exMetaInner);
  metadata.payloadMetadata().ensure().set_exceptionMetadata(std::move(exMeta));

  auto response = makePayloadResponse(std::move(metadata), payloadQueue.move());

  auto result = TestAdapter::handleRequestResponse(
      std::move(response), apache::thrift::protocol::T_COMPACT_PROTOCOL);

  ASSERT_TRUE(result.hasError());
  EXPECT_TRUE(result.error()
                  .is_compatible_with<apache::thrift::TApplicationException>());
}

} // namespace
} // namespace apache::thrift::fast_thrift::thrift
