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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/PayloadVariants.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftRequestContext.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerChecksumHandler.h>
#include <thrift/lib/cpp2/transport/rocket/ChecksumGenerator.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

namespace {

class MockHandlerContext {
 public:
  Result fireWrite(TypeErasedBox&& msg) noexcept {
    writeMessages_.push_back(std::move(msg));
    return Result::Success;
  }

  Result fireRead(TypeErasedBox&& msg) noexcept {
    readMessages_.push_back(std::move(msg));
    return Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exceptions_.push_back(std::move(e));
  }

  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }
  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }
  std::vector<folly::exception_wrapper>& exceptions() { return exceptions_; }

 private:
  std::vector<TypeErasedBox> writeMessages_;
  std::vector<TypeErasedBox> readMessages_;
  std::vector<folly::exception_wrapper> exceptions_;
};

constexpr uint32_t kStreamId = 7;

std::unique_ptr<apache::thrift::RequestRpcMetadata> requestMetadata(
    apache::thrift::ChecksumAlgorithm algo, int64_t checksum, int64_t salt) {
  auto md = std::make_unique<apache::thrift::RequestRpcMetadata>();
  apache::thrift::Checksum c;
  c.algorithm() = algo;
  c.checksum() = checksum;
  c.salt() = salt;
  md->checksum() = c;
  return md;
}

ThriftServerRequestMessage makeRequest(
    std::unique_ptr<apache::thrift::RequestRpcMetadata> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  return ThriftServerRequestMessage{
      .requestContext = std::make_unique<ThriftRequestContext>(),
      .payload =
          ThriftRequestResponsePayload{
              .data = std::move(data),
              .metadata = std::move(metadata),
          },
      .streamId = kStreamId,
  };
}

// Builds an initial-response message carrying a per-request context with the
// given algorithm (as FastHandlerCallback would forward it) and an empty
// response metadata (the handler stamps the checksum onto it).
ThriftServerResponseMessage makeResponseWithContext(
    apache::thrift::ChecksumAlgorithm algo,
    std::unique_ptr<folly::IOBuf> data) {
  auto requestContext = std::make_unique<ThriftRequestContext>();
  requestContext->setChecksumAlgorithm(algo);
  return ThriftServerResponseMessage{
      .requestContext = std::move(requestContext),
      .payload = ThriftInitialResponsePayload{
          .data = std::move(data),
          .metadata = std::make_unique<apache::thrift::ResponseRpcMetadata>(),
          .streamId = kStreamId,
      }};
}

const ThriftServerRequestMessage& peekRequest(const TypeErasedBox& box) {
  return box.get<ThriftServerRequestMessage>();
}

const ThriftInitialResponsePayload& peekResponsePayload(
    const TypeErasedBox& box) {
  return box.get<ThriftServerResponseMessage>()
      .payload.get<ThriftInitialResponsePayload>();
}

} // namespace

class ThriftServerChecksumHandlerTest : public ::testing::Test {
 protected:
  Result callOnRead(TypeErasedBox msg) {
    return handler_.onRead(ctx_, std::move(msg));
  }

  Result callOnWrite(TypeErasedBox msg) {
    return handler_.onWrite(ctx_, std::move(msg));
  }

  MockHandlerContext ctx_;
  ThriftServerChecksumHandler<MockHandlerContext> handler_;
};

// =============================================================================
// Inbound — request checksum validation
// =============================================================================

TEST_F(ThriftServerChecksumHandlerTest, InboundNoChecksumPassesThrough) {
  auto request = makeRequest(
      std::make_unique<apache::thrift::RequestRpcMetadata>(),
      folly::IOBuf::copyBuffer("hello"));

  auto result = callOnRead(erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
  EXPECT_EQ(
      peekRequest(ctx_.readMessages()[0])
          .requestContext->getChecksumAlgorithm(),
      apache::thrift::ChecksumAlgorithm::NONE);
}

TEST_F(ThriftServerChecksumHandlerTest, InboundValidXXH3RecordsAlgorithm) {
  auto data = folly::IOBuf::copyBuffer("request payload bytes");
  constexpr int64_t kSalt = 0x1234abcd;
  auto expected = apache::thrift::rocket::ChecksumGenerator<
                      apache::thrift::rocket::XXH3_64>{}
                      .calculateChecksumFromIOBuf(*data, kSalt);

  auto request = makeRequest(
      requestMetadata(
          apache::thrift::ChecksumAlgorithm::XXH3_64, expected.checksum, kSalt),
      data->clone());

  auto result = callOnRead(erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
  // Algorithm captured so the response echoes it.
  EXPECT_EQ(
      peekRequest(ctx_.readMessages()[0])
          .requestContext->getChecksumAlgorithm(),
      apache::thrift::ChecksumAlgorithm::XXH3_64);
}

TEST_F(ThriftServerChecksumHandlerTest, InboundMismatchDropsAndEmitsError) {
  auto request = makeRequest(
      requestMetadata(
          apache::thrift::ChecksumAlgorithm::XXH3_64,
          /*checksum=*/0xdeadbeef,
          /*salt=*/0),
      folly::IOBuf::copyBuffer("request payload"));

  auto result = callOnRead(erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Success);
  // Request dropped (not forwarded to tail); an error response was emitted
  // toward head.
  EXPECT_TRUE(ctx_.readMessages().empty());
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages()[0]
                  .get<ThriftServerResponseMessage>()
                  .payload.is<ThriftErrorPayload>());
}

TEST_F(ThriftServerChecksumHandlerTest, InboundCRC32IsUnsupportedEmitsError) {
  auto request = makeRequest(
      requestMetadata(
          apache::thrift::ChecksumAlgorithm::CRC32, /*checksum=*/0, /*salt=*/0),
      folly::IOBuf::copyBuffer("request payload"));

  auto result = callOnRead(erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(ctx_.readMessages().empty());
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages()[0]
                  .get<ThriftServerResponseMessage>()
                  .payload.is<ThriftErrorPayload>());
}

// =============================================================================
// Outbound — response checksum fill
// =============================================================================

TEST_F(ThriftServerChecksumHandlerTest, OutboundFillsStampedXXH3Checksum) {
  auto data = folly::IOBuf::copyBuffer("response payload bytes");
  auto response = makeResponseWithContext(
      apache::thrift::ChecksumAlgorithm::XXH3_64, data->clone());

  auto result = callOnWrite(erase_and_box(std::move(response)));

  ASSERT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  const auto& checksum =
      *peekResponsePayload(ctx_.writeMessages()[0]).metadata->checksum();
  EXPECT_EQ(*checksum.algorithm(), apache::thrift::ChecksumAlgorithm::XXH3_64);
  // The filled value+salt validate against the response data.
  EXPECT_TRUE(
      apache::thrift::rocket::ChecksumGenerator<
          apache::thrift::rocket::XXH3_64>{}
          .validateChecksumFromIOBuf(
              *checksum.checksum(), *checksum.salt(), *data));
}

TEST_F(ThriftServerChecksumHandlerTest, OutboundNoChecksumLeavesMetadataAlone) {
  auto response = makeResponseWithContext(
      apache::thrift::ChecksumAlgorithm::NONE,
      folly::IOBuf::copyBuffer("response"));

  auto result = callOnWrite(erase_and_box(std::move(response)));

  ASSERT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  EXPECT_FALSE(peekResponsePayload(ctx_.writeMessages()[0])
                   .metadata->checksum()
                   .has_value());
}

TEST_F(ThriftServerChecksumHandlerTest, OutboundErrorPayloadBypassed) {
  ThriftServerResponseMessage response{
      .payload = ThriftErrorPayload{
          .data = folly::IOBuf::copyBuffer("err"),
          .metadata = nullptr,
          .streamId = kStreamId,
          .errorCode = 0}};

  auto result = callOnWrite(erase_and_box(std::move(response)));

  ASSERT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages()[0]
                  .get<ThriftServerResponseMessage>()
                  .payload.is<ThriftErrorPayload>());
}

TEST_F(ThriftServerChecksumHandlerTest, ForwardsExceptions) {
  handler_.onException(
      ctx_, folly::make_exception_wrapper<std::runtime_error>("boom"));

  ASSERT_EQ(ctx_.exceptions().size(), 1);
  EXPECT_NE(
      ctx_.exceptions()[0].what().toStdString().find("boom"),
      std::string::npos);
}

} // namespace apache::thrift::fast_thrift::thrift
