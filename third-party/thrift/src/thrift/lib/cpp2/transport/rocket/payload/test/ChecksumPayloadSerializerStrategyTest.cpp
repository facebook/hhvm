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

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/transport/rocket/RequestPayload.h>
#include <thrift/lib/cpp2/transport/rocket/payload/ChecksumPayloadSerializerStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/payload/DefaultPayloadSerializerStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>

using namespace apache::thrift;
using namespace apache::thrift::rocket;

TEST(ChecksumPayloadSerializerStrategyTest, TestPackWithoutChecksum) {
  ChecksumPayloadSerializerStrategy<DefaultPayloadSerializerStrategy> strategy;
  RequestRpcMetadata metadata;
  metadata.protocol() = ProtocolId::COMPACT;
  auto payload = strategy.packWithFds(
      &metadata, folly::IOBuf::copyBuffer("test"), folly::SocketFds(), nullptr);

  auto other = strategy.unpack<RequestPayload>(std::move(payload));
  EXPECT_EQ(other.hasException(), false);
}

TEST(ChecksumPayloadSerializerStrategyTest, TestPackWithChecksumHappyPath) {
  ChecksumPayloadSerializerStrategy<DefaultPayloadSerializerStrategy> strategy;
  RequestRpcMetadata metadata;
  metadata.protocol() = ProtocolId::COMPACT;

  Checksum checksum;
  checksum.algorithm() = ChecksumAlgorithm::XXH3_64;

  metadata.checksum() = checksum;

  auto payload = strategy.packWithFds(
      &metadata, folly::IOBuf::copyBuffer("test"), folly::SocketFds(), nullptr);

  auto other = strategy.unpack<RequestPayload>(std::move(payload));
  EXPECT_EQ(other.hasException(), false);
  EXPECT_TRUE(other->metadata.checksum().has_value());
  EXPECT_EQ(
      metadata.checksum().value().checksum(),
      other->metadata.checksum().value().checksum());
}
