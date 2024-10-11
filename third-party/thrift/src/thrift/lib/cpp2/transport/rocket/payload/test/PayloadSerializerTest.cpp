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
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/transport/rocket/payload/LegacyPayloadSerializerStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::rocket {

void testPackAndUnpackWithCompactProtocol(PayloadSerializer& serializer) {
  RequestRpcMetadata metadata;
  metadata.protocol() = ProtocolId::COMPACT;
  auto payload = serializer.packCompact(metadata);
  EXPECT_GT(payload->computeChainDataLength(), 1);

  RequestRpcMetadata other;
  serializer.unpack<RequestRpcMetadata>(other, payload.get(), false);
  EXPECT_EQ(other, metadata);
  EXPECT_EQ(other.protocol(), ProtocolId::COMPACT);
}

TEST(PayloadSerializerTest, TestPackWithLegacyStrategy) {
  PayloadSerializer::reset();
  PayloadSerializer::initialize(LegacyPayloadSerializerStrategy());
  auto& serializer = PayloadSerializer::getInstance();
  testPackAndUnpackWithCompactProtocol(serializer);
}

TEST(PayloadSerializerTest, TestPackWitDefaultyStrategy) {
  PayloadSerializer::reset();
  PayloadSerializer::initialize(DefaultPayloadSerializerStrategy());
  auto& serializer = PayloadSerializer::getInstance();
  testPackAndUnpackWithCompactProtocol(serializer);
}

TEST(PayloadSerializerTest, TestPackWithoutChecksumUsingFacade) {
  PayloadSerializer::reset();
  PayloadSerializer::initialize(
      ChecksumPayloadSerializerStrategy<DefaultPayloadSerializerStrategy>());
  auto& serializer = PayloadSerializer::getInstance();
  RequestRpcMetadata metadata;
  metadata.protocol() = ProtocolId::COMPACT;
  auto payload = serializer.packWithFds(
      &metadata, folly::IOBuf::copyBuffer("test"), folly::SocketFds(), nullptr);

  auto other = serializer.unpack<RequestPayload>(std::move(payload));
  EXPECT_EQ(other.hasException(), false);
}

} // namespace apache::thrift::rocket
