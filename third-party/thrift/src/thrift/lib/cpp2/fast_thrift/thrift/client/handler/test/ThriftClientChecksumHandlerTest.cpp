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

  void setReturnBackpressure(bool value) { returnBackpressure_ = value; }
  void setReturnError(bool value) { returnError_ = value; }

  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }

 private:
  std::vector<TypeErasedBox> writeMessages_;
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

} // namespace

class ThriftClientChecksumHandlerTest : public ::testing::Test {
 protected:
  Result callOnWrite(TypeErasedBox msg) {
    return handler_.onWrite(ctx_, std::move(msg));
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

} // namespace apache::thrift::fast_thrift::thrift::client::handler
