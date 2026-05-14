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

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientChecksumHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayload.h>
#include <thrift/lib/cpp2/transport/rocket/ChecksumGenerator.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift::client::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

namespace {

class MockHandlerContext {
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
    exceptions_.push_back(std::move(e));
  }

  void setReturnBackpressure(bool value) { returnBackpressure_ = value; }
  void setReturnError(bool value) { returnError_ = value; }

  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }
  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }
  std::vector<folly::exception_wrapper>& exceptions() { return exceptions_; }

 private:
  std::vector<TypeErasedBox> writeMessages_;
  std::vector<TypeErasedBox> readMessages_;
  std::vector<folly::exception_wrapper> exceptions_;
  bool returnBackpressure_{false};
  bool returnError_{false};
};

ThriftRequestMessage makeRequest(
    std::unique_ptr<apache::thrift::RequestRpcMetadata> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  return ThriftRequestMessage{
      .payload =
          ThriftRequestResponsePayload{
              .data = std::move(data),
              .metadata = std::move(metadata),
          },
      .requestContext = {},
  };
}

std::unique_ptr<apache::thrift::RequestRpcMetadata> emptyMetadata() {
  return std::make_unique<apache::thrift::RequestRpcMetadata>();
}

std::unique_ptr<apache::thrift::RequestRpcMetadata> metadataWithAlgorithm(
    apache::thrift::ChecksumAlgorithm algo) {
  auto md = std::make_unique<apache::thrift::RequestRpcMetadata>();
  apache::thrift::Checksum c;
  c.algorithm() = algo;
  md->checksum() = c;
  return md;
}

const ThriftRequestResponsePayload& peekPayload(const TypeErasedBox& box) {
  return box.get<ThriftRequestMessage>()
      .payload.get<ThriftRequestResponsePayload>();
}

// Build a response message carrying a typed first-response payload. Inputs
// are taken by value so the test can provide either a matching or a
// deliberately-corrupted (checksum, salt) pair.
ThriftResponseMessage makeFirstResponse(
    apache::thrift::ChecksumAlgorithm algo,
    int64_t checksum,
    int64_t salt,
    std::unique_ptr<folly::IOBuf> data) {
  auto metadata = std::make_unique<apache::thrift::ResponseRpcMetadata>();
  apache::thrift::Checksum c;
  c.algorithm() = algo;
  c.checksum() = checksum;
  c.salt() = salt;
  metadata->checksum() = c;

  ThriftResponseMessage response;
  response.payload = ThriftClientInboundPayloadVariant{
      ThriftFirstResponsePayload{
          .data = std::move(data),
          .metadata = std::move(metadata),
          .streamId = 1,
          .complete = true,
          .next = true},
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
  return response;
}

ThriftResponseMessage makeFirstResponseNoChecksum(
    std::unique_ptr<folly::IOBuf> data) {
  ThriftResponseMessage response;
  response.payload = ThriftClientInboundPayloadVariant{
      ThriftFirstResponsePayload{
          .data = std::move(data),
          .metadata = std::make_unique<apache::thrift::ResponseRpcMetadata>(),
          .streamId = 1,
          .complete = true,
          .next = true},
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
  return response;
}

ThriftResponseMessage makeErrorResponse() {
  ThriftResponseMessage response;
  response.payload = ThriftClientInboundPayloadVariant{
      ThriftErrorPayload{
          .data = folly::IOBuf::copyBuffer("err"),
          .metadata = nullptr,
          .streamId = 1,
          .errorCode = 0x0201},
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
  return response;
}

ThriftResponseMessage makeTransportErrorResponse() {
  ThriftResponseMessage response;
  response.payload = ThriftClientResponseError{
      .ew = folly::make_exception_wrapper<std::runtime_error>("boom")};
  return response;
}

} // namespace

class ThriftClientChecksumHandlerTest : public ::testing::Test {
 protected:
  Result callOnWrite(TypeErasedBox msg) {
    return handler_.onWrite(ctx_, std::move(msg));
  }

  Result callOnRead(TypeErasedBox msg) {
    return handler_.onRead(ctx_, std::move(msg));
  }

  MockHandlerContext ctx_;
  ThriftClientChecksumHandler handler_;
};

// =============================================================================
// Pass-through path — no checksum requested
// =============================================================================

TEST_F(ThriftClientChecksumHandlerTest, NoChecksumLeavesMetadataUnchanged) {
  auto request =
      makeRequest(emptyMetadata(), folly::IOBuf::copyBuffer("hello"));

  auto result = callOnWrite(erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  const auto& payload = peekPayload(ctx_.writeMessages()[0]);
  EXPECT_FALSE(payload.metadata->checksum().has_value());
  EXPECT_FALSE(payload.metadata->crc32c().has_value());
}

// =============================================================================
// Checksum struct path — XXH3_64 (only supported algorithm)
// =============================================================================

TEST_F(
    ThriftClientChecksumHandlerTest,
    XXH3_64StampsValueAndSaltMatchingGenerator) {
  auto data = folly::IOBuf::copyBuffer("payload bytes for xxh3");
  auto request = makeRequest(
      metadataWithAlgorithm(apache::thrift::ChecksumAlgorithm::XXH3_64),
      data->clone());

  auto result = callOnWrite(erase_and_box(std::move(request)));

  ASSERT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  const auto& checksum =
      *peekPayload(ctx_.writeMessages()[0]).metadata->checksum();
  EXPECT_EQ(*checksum.algorithm(), apache::thrift::ChecksumAlgorithm::XXH3_64);

  auto expected = apache::thrift::rocket::ChecksumGenerator<
                      apache::thrift::rocket::XXH3_64>{}
                      .calculateChecksumFromIOBuf(*data, *checksum.salt());
  EXPECT_EQ(*checksum.checksum(), expected.checksum);
}

TEST_F(ThriftClientChecksumHandlerTest, AlgorithmNoneIsNoop) {
  auto request = makeRequest(
      metadataWithAlgorithm(apache::thrift::ChecksumAlgorithm::NONE),
      folly::IOBuf::copyBuffer("x"));

  auto result = callOnWrite(erase_and_box(std::move(request)));

  ASSERT_EQ(result, Result::Success);
  const auto& checksum =
      *peekPayload(ctx_.writeMessages()[0]).metadata->checksum();
  EXPECT_EQ(*checksum.checksum(), 0);
  EXPECT_EQ(*checksum.salt(), 0);
}

TEST_F(ThriftClientChecksumHandlerTest, AlgorithmCRC32IsUnsupportedAndNoop) {
  // makeRequestMetadata never produces this state for fast_thrift, but if
  // some other path sets ChecksumAlgorithm::CRC32 directly the handler must
  // leave the placeholder untouched (CRC32 is not supported).
  auto request = makeRequest(
      metadataWithAlgorithm(apache::thrift::ChecksumAlgorithm::CRC32),
      folly::IOBuf::copyBuffer("payload"));

  auto result = callOnWrite(erase_and_box(std::move(request)));

  ASSERT_EQ(result, Result::Success);
  const auto& checksum =
      *peekPayload(ctx_.writeMessages()[0]).metadata->checksum();
  EXPECT_EQ(*checksum.checksum(), 0);
  EXPECT_EQ(*checksum.salt(), 0);
}

// =============================================================================
// Multi-segment IOBuf — verifies the whole chain is checksummed
// =============================================================================

TEST_F(ThriftClientChecksumHandlerTest, ChecksumCoversFullIOBufChain) {
  auto chained = folly::IOBuf::copyBuffer("first-");
  chained->appendToChain(folly::IOBuf::copyBuffer("second-"));
  chained->appendToChain(folly::IOBuf::copyBuffer("third"));
  ASSERT_GT(chained->countChainElements(), 1);

  auto request = makeRequest(
      metadataWithAlgorithm(apache::thrift::ChecksumAlgorithm::XXH3_64),
      chained->clone());

  auto result = callOnWrite(erase_and_box(std::move(request)));

  ASSERT_EQ(result, Result::Success);
  const auto& checksum =
      *peekPayload(ctx_.writeMessages()[0]).metadata->checksum();

  // The value over the chain must differ from the value over only the first
  // segment — proves the handler walked the whole chain.
  auto firstOnly = folly::IOBuf::copyBuffer("first-");
  auto firstOnlyChecksum =
      apache::thrift::rocket::ChecksumGenerator<
          apache::thrift::rocket::XXH3_64>{}
          .calculateChecksumFromIOBuf(*firstOnly, *checksum.salt());
  EXPECT_NE(*checksum.checksum(), firstOnlyChecksum.checksum);
}

// =============================================================================
// Pipeline propagation
// =============================================================================

TEST_F(ThriftClientChecksumHandlerTest, BackpressureFromDownstreamPropagated) {
  ctx_.setReturnBackpressure(true);

  auto request = makeRequest(emptyMetadata(), folly::IOBuf::copyBuffer("x"));

  EXPECT_EQ(
      callOnWrite(erase_and_box(std::move(request))), Result::Backpressure);
}

TEST_F(ThriftClientChecksumHandlerTest, ErrorFromDownstreamPropagated) {
  ctx_.setReturnError(true);

  auto request = makeRequest(emptyMetadata(), folly::IOBuf::copyBuffer("x"));

  EXPECT_EQ(callOnWrite(erase_and_box(std::move(request))), Result::Error);
}

// =============================================================================
// Inbound — checksum validation
// =============================================================================

TEST_F(ThriftClientChecksumHandlerTest, InboundCRC32IsUnsupportedYieldsError) {
  // CRC32 is not supported on inbound either. Even when the value would
  // validate against the data, the handler must fail the response with
  // CHECKSUM_MISMATCH rather than silently accept it.
  auto data = folly::IOBuf::copyBuffer("response payload");
  auto response = makeFirstResponse(
      apache::thrift::ChecksumAlgorithm::CRC32,
      /*checksum=*/0,
      /*salt=*/0,
      std::move(data));

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& fwd = ctx_.readMessages()[0].get<ThriftResponseMessage>();
  ASSERT_TRUE(fwd.payload.is<ThriftClientResponseError>());
  auto* tae = fwd.payload.get<ThriftClientResponseError>()
                  .ew.get_exception<apache::thrift::TApplicationException>();
  ASSERT_NE(tae, nullptr);
  EXPECT_EQ(
      tae->getType(), apache::thrift::TApplicationException::CHECKSUM_MISMATCH);
}

TEST_F(ThriftClientChecksumHandlerTest, InboundMatchingXXH3_64PassesThrough) {
  auto data = folly::IOBuf::copyBuffer("xxh payload bytes");
  constexpr int64_t kSalt = 0xfeedface;
  auto expected = apache::thrift::rocket::ChecksumGenerator<
                      apache::thrift::rocket::XXH3_64>{}
                      .calculateChecksumFromIOBuf(*data, kSalt);

  auto response = makeFirstResponse(
      apache::thrift::ChecksumAlgorithm::XXH3_64,
      expected.checksum,
      kSalt,
      data->clone());

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  auto& fwd = ctx_.readMessages()[0].get<ThriftResponseMessage>();
  EXPECT_TRUE(fwd.payload.is<ThriftClientInboundPayloadVariant>());
}

TEST_F(
    ThriftClientChecksumHandlerTest,
    InboundMismatchedChecksumYieldsThriftClientResponseError) {
  auto data = folly::IOBuf::copyBuffer("response payload");
  // Deliberately wrong checksum value — recompute will not match.
  auto response = makeFirstResponse(
      apache::thrift::ChecksumAlgorithm::XXH3_64,
      /*checksum=*/0xdeadbeef,
      /*salt=*/0,
      std::move(data));

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  // Mismatch → payload swapped to ThriftClientResponseError; channel will fail
  // just this callback.
  auto& fwd = ctx_.readMessages()[0].get<ThriftResponseMessage>();
  ASSERT_TRUE(fwd.payload.is<ThriftClientResponseError>());
  auto* tae = fwd.payload.get<ThriftClientResponseError>()
                  .ew.get_exception<apache::thrift::TApplicationException>();
  ASSERT_NE(tae, nullptr);
  EXPECT_EQ(
      tae->getType(), apache::thrift::TApplicationException::CHECKSUM_MISMATCH);
}

TEST_F(
    ThriftClientChecksumHandlerTest,
    InboundServerOmittedChecksumPassesThrough) {
  // Legacy parity: client may have requested a checksum but the server
  // didn't echo one. Silent pass-through.
  auto response =
      makeFirstResponseNoChecksum(folly::IOBuf::copyBuffer("response"));

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  auto& fwd = ctx_.readMessages()[0].get<ThriftResponseMessage>();
  ASSERT_TRUE(fwd.payload.is<ThriftClientInboundPayloadVariant>());
  EXPECT_TRUE(fwd.payload.get<ThriftClientInboundPayloadVariant>()
                  .is<ThriftFirstResponsePayload>());
}

TEST_F(ThriftClientChecksumHandlerTest, InboundAlgorithmNoneIsPassThrough) {
  // algorithm=NONE on the wire is a no-op — neither validate nor fail.
  auto response = makeFirstResponse(
      apache::thrift::ChecksumAlgorithm::NONE,
      /*checksum=*/0,
      /*salt=*/0,
      folly::IOBuf::copyBuffer("data"));

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  auto& fwd = ctx_.readMessages()[0].get<ThriftResponseMessage>();
  EXPECT_TRUE(fwd.payload.is<ThriftClientInboundPayloadVariant>());
}

TEST_F(
    ThriftClientChecksumHandlerTest, InboundErrorPayloadAlternativeBypassed) {
  // ThriftErrorPayload (wire ERROR frame) and other non-first-response
  // alternatives are not subject to checksum validation — they don't
  // carry a ResponseRpcMetadata. Pass through unchanged.
  auto response = makeErrorResponse();

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  auto& fwd = ctx_.readMessages()[0].get<ThriftResponseMessage>();
  ASSERT_TRUE(fwd.payload.is<ThriftClientInboundPayloadVariant>());
  EXPECT_TRUE(fwd.payload.get<ThriftClientInboundPayloadVariant>()
                  .is<ThriftErrorPayload>());
}

TEST_F(ThriftClientChecksumHandlerTest, InboundTransportErrorVariantBypassed) {
  // ThriftClientResponseError (in-process transport failure) bypasses the
  // wire-derived path entirely — handler must not unpack it.
  auto response = makeTransportErrorResponse();

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  auto& fwd = ctx_.readMessages()[0].get<ThriftResponseMessage>();
  EXPECT_TRUE(fwd.payload.is<ThriftClientResponseError>());
}

} // namespace apache::thrift::fast_thrift::thrift::client::handler
