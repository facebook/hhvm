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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/SetupResponseBuilder.h>

#include <memory>
#include <optional>

#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {
namespace {

// Compact-encode a RequestSetupMetadata to feed makeSetupResponseResult, the
// same way a client's SETUP frame metadata arrives on the wire.
std::unique_ptr<folly::IOBuf> encodeRequestSetupMetadata(
    std::optional<int32_t> minVersion, std::optional<int32_t> maxVersion) {
  apache::thrift::RequestSetupMetadata metadata;
  if (minVersion) {
    metadata.minVersion() = *minVersion;
  }
  if (maxVersion) {
    metadata.maxVersion() = *maxVersion;
  }
  apache::thrift::CompactProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

apache::thrift::SetupResponse decodeSetupResponse(folly::IOBuf* buffer) {
  apache::thrift::CompactProtocolReader reader;
  reader.setInput(buffer);
  apache::thrift::ServerPushMetadata push;
  push.read(&reader);
  EXPECT_EQ(
      push.getType(), apache::thrift::ServerPushMetadata::Type::setupResponse);
  return *push.setupResponse();
}

int32_t negotiatedVersion(
    std::optional<int32_t> minVersion, std::optional<int32_t> maxVersion) {
  auto result = makeSetupResponseResult(
      encodeRequestSetupMetadata(minVersion, maxVersion),
      rocket::server::MetadataProtocol::COMPACT);
  EXPECT_FALSE(result.reject.has_value());
  EXPECT_NE(result.metadataPush, nullptr);
  return decodeSetupResponse(result.metadataPush.get()).version().value_or(-1);
}

} // namespace

// The server negotiates the client's maxVersion when it falls inside the
// server range, and always advertises zstdSupported=false (no codec yet).
TEST(SetupResponseBuilderTest, NegotiatesClientMaxWithinRange) {
  auto result = makeSetupResponseResult(
      encodeRequestSetupMetadata(kMinNegotiableVersion, kMaxNegotiableVersion),
      rocket::server::MetadataProtocol::COMPACT);
  ASSERT_FALSE(result.reject.has_value());
  ASSERT_NE(result.metadataPush, nullptr);
  const auto response = decodeSetupResponse(result.metadataPush.get());
  EXPECT_EQ(response.version().value_or(-1), kMaxNegotiableVersion);
  EXPECT_FALSE(response.zstdSupported().value_or(true));
}

// A client asking for more than the server supports is clamped to the server
// max; a mid-range max is honored exactly.
TEST(SetupResponseBuilderTest, ClampsAndHonorsVersion) {
  EXPECT_EQ(
      negotiatedVersion(kMinNegotiableVersion, 12), kMaxNegotiableVersion);
  EXPECT_EQ(negotiatedVersion(kMinNegotiableVersion, 9), 9);
}

// Missing version fields default to the server floor rather than rejecting.
TEST(SetupResponseBuilderTest, DefaultsToFloorWhenVersionsAbsent) {
  EXPECT_EQ(
      negotiatedVersion(std::nullopt, std::nullopt), kMinNegotiableVersion);
}
TEST(SetupResponseBuilderTest, DefaultsAbsentMaxToClientMin) {
  EXPECT_EQ(
      negotiatedVersion(kMaxNegotiableVersion, std::nullopt),
      kMaxNegotiableVersion);
}

// Absent setup metadata is treated as the floor, not a failure.
TEST(SetupResponseBuilderTest, DefaultsToFloorWhenMetadataNull) {
  auto result = makeSetupResponseResult(
      nullptr, rocket::server::MetadataProtocol::COMPACT);
  ASSERT_FALSE(result.reject.has_value());
  ASSERT_NE(result.metadataPush, nullptr);
  EXPECT_EQ(
      decodeSetupResponse(result.metadataPush.get()).version().value_or(-1),
      kMinNegotiableVersion);
}

// No overlap between client [min,max] and server range must reject with
// INVALID_SETUP, matching the legacy server.
TEST(SetupResponseBuilderTest, RejectsWhenClientMaxBelowServerMin) {
  auto result = makeSetupResponseResult(
      encodeRequestSetupMetadata(1, kMinNegotiableVersion - 1),
      rocket::server::MetadataProtocol::COMPACT);
  EXPECT_EQ(result.metadataPush, nullptr);
  ASSERT_TRUE(result.reject.has_value());
  EXPECT_EQ(
      *result.reject,
      apache::thrift::fast_thrift::frame::ErrorCode::INVALID_SETUP);
}

TEST(SetupResponseBuilderTest, RejectsWhenClientMinAboveServerMax) {
  auto result = makeSetupResponseResult(
      encodeRequestSetupMetadata(kMaxNegotiableVersion + 1, 15),
      rocket::server::MetadataProtocol::COMPACT);
  EXPECT_EQ(result.metadataPush, nullptr);
  ASSERT_TRUE(result.reject.has_value());
  EXPECT_EQ(
      *result.reject,
      apache::thrift::fast_thrift::frame::ErrorCode::INVALID_SETUP);
}

} // namespace apache::thrift::fast_thrift::thrift
