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

#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/RequestMetadata.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

TEST(RequestMetadataTest, PopulatesNameKindProtocol) {
  apache::thrift::RpcOptions options;
  auto md = makeRequestMetadata(
      options,
      apache::thrift::ManagedStringView::from_static(
          std::string_view{"myMethod"}),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::COMPACT);

  ASSERT_NE(md, nullptr);
  EXPECT_EQ(md->name()->str(), "myMethod");
  EXPECT_EQ(
      *md->kind(), apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  EXPECT_EQ(*md->protocol(), apache::thrift::ProtocolId::COMPACT);
}

TEST(RequestMetadataTest, SetsTimeoutFromRpcOptions) {
  apache::thrift::RpcOptions options;
  options.setTimeout(std::chrono::milliseconds(500));

  auto md = makeRequestMetadata(
      options,
      apache::thrift::ManagedStringView::from_static(std::string_view{"foo"}),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::BINARY);

  EXPECT_EQ(*md->clientTimeoutMs(), 500);
}

TEST(RequestMetadataTest, SetsQueueTimeoutFromRpcOptions) {
  apache::thrift::RpcOptions options;
  options.setQueueTimeout(std::chrono::milliseconds(100));

  auto md = makeRequestMetadata(
      options,
      apache::thrift::ManagedStringView::from_static(std::string_view{"bar"}),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::COMPACT);

  EXPECT_EQ(*md->queueTimeoutMs(), 100);
}

TEST(RequestMetadataTest, OmitsZeroTimeouts) {
  apache::thrift::RpcOptions options;

  auto md = makeRequestMetadata(
      options,
      apache::thrift::ManagedStringView::from_static(std::string_view{"baz"}),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::COMPACT);

  EXPECT_FALSE(md->clientTimeoutMs().has_value());
  EXPECT_FALSE(md->queueTimeoutMs().has_value());
}

TEST(RequestMetadataTest, OmitsChecksumWhenNoneRequested) {
  apache::thrift::RpcOptions options;

  auto md = makeRequestMetadata(
      options,
      apache::thrift::ManagedStringView::from_static(std::string_view{"x"}),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::COMPACT);

  EXPECT_FALSE(md->checksum().has_value());
  EXPECT_FALSE(md->crc32c().has_value());
}

TEST(RequestMetadataTest, SetsChecksumAlgorithmXXH3_64) {
  apache::thrift::RpcOptions options;
  options.setChecksum(apache::thrift::RpcOptions::Checksum::XXH3_64);

  auto md = makeRequestMetadata(
      options,
      apache::thrift::ManagedStringView::from_static(std::string_view{"x"}),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::COMPACT);

  ASSERT_TRUE(md->checksum().has_value());
  EXPECT_EQ(
      *md->checksum()->algorithm(), apache::thrift::ChecksumAlgorithm::XXH3_64);
  EXPECT_FALSE(md->crc32c().has_value());
}

TEST(RequestMetadataTest, CRC32IsUnsupportedAndOmitsChecksum) {
  apache::thrift::RpcOptions options;
  options.setChecksum(apache::thrift::RpcOptions::Checksum::CRC32);

  auto md = makeRequestMetadata(
      options,
      apache::thrift::ManagedStringView::from_static(std::string_view{"x"}),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::COMPACT);

  EXPECT_FALSE(md->checksum().has_value());
  EXPECT_FALSE(md->crc32c().has_value());
}

TEST(RequestMetadataTest, ServerOnlyCRC32IsUnsupportedAndOmitsChecksum) {
  apache::thrift::RpcOptions options;
  options.setChecksum(apache::thrift::RpcOptions::Checksum::SERVER_ONLY_CRC32);

  auto md = makeRequestMetadata(
      options,
      apache::thrift::ManagedStringView::from_static(std::string_view{"x"}),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::COMPACT);

  EXPECT_FALSE(md->checksum().has_value());
  EXPECT_FALSE(md->crc32c().has_value());
}

TEST(RequestMetadataTest, OptionsLessOverloadOmitsTimeoutsAndPriority) {
  auto md = makeRequestMetadata(
      apache::thrift::ManagedStringView::from_static(std::string_view{"q"}),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::BINARY);

  EXPECT_EQ(md->name()->str(), "q");
  EXPECT_FALSE(md->clientTimeoutMs().has_value());
  EXPECT_FALSE(md->queueTimeoutMs().has_value());
  EXPECT_FALSE(md->priority().has_value());
}

} // namespace apache::thrift::fast_thrift::thrift
