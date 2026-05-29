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
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

TEST(RequestMetadataTest, SerializesMethodName) {
  apache::thrift::RpcOptions options;
  auto result = makeSerializedRequestMetadata(
      options,
      apache::thrift::ManagedStringView::from_static(
          std::string_view{"myMethod"}),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::COMPACT);

  ASSERT_NE(result, nullptr);

  // Deserialize and verify
  apache::thrift::RequestRpcMetadata metadata;
  apache::thrift::BinaryProtocolReader reader;
  reader.setInput(result.get());
  metadata.read(&reader);

  EXPECT_EQ(metadata.name()->str(), "myMethod");
  EXPECT_EQ(
      *metadata.kind(),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  EXPECT_EQ(*metadata.protocol(), apache::thrift::ProtocolId::COMPACT);
}

TEST(RequestMetadataTest, SerializesTimeout) {
  apache::thrift::RpcOptions options;
  options.setTimeout(std::chrono::milliseconds(500));

  auto result = makeSerializedRequestMetadata(
      options,
      apache::thrift::ManagedStringView::from_static(std::string_view{"foo"}),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::BINARY);

  ASSERT_NE(result, nullptr);

  apache::thrift::RequestRpcMetadata metadata;
  apache::thrift::BinaryProtocolReader reader;
  reader.setInput(result.get());
  metadata.read(&reader);

  EXPECT_EQ(*metadata.clientTimeoutMs(), 500);
}

TEST(RequestMetadataTest, SerializesQueueTimeout) {
  apache::thrift::RpcOptions options;
  options.setQueueTimeout(std::chrono::milliseconds(100));

  auto result = makeSerializedRequestMetadata(
      options,
      apache::thrift::ManagedStringView::from_static(std::string_view{"bar"}),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::COMPACT);

  ASSERT_NE(result, nullptr);

  apache::thrift::RequestRpcMetadata metadata;
  apache::thrift::BinaryProtocolReader reader;
  reader.setInput(result.get());
  metadata.read(&reader);

  EXPECT_EQ(*metadata.queueTimeoutMs(), 100);
}

TEST(RequestMetadataTest, OmitsZeroTimeout) {
  apache::thrift::RpcOptions options;
  // Default timeout is zero — should not be set

  auto result = makeSerializedRequestMetadata(
      options,
      apache::thrift::ManagedStringView::from_static(std::string_view{"baz"}),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::COMPACT);

  ASSERT_NE(result, nullptr);

  apache::thrift::RequestRpcMetadata metadata;
  apache::thrift::BinaryProtocolReader reader;
  reader.setInput(result.get());
  metadata.read(&reader);

  EXPECT_FALSE(metadata.clientTimeoutMs().has_value());
  EXPECT_FALSE(metadata.queueTimeoutMs().has_value());
}

TEST(RequestMetadataTest, OutputHasHeadroom) {
  apache::thrift::RpcOptions options;
  auto result = makeSerializedRequestMetadata(
      options,
      apache::thrift::ManagedStringView::from_static(std::string_view{"test"}),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      apache::thrift::ProtocolId::COMPACT);

  ASSERT_NE(result, nullptr);
  EXPECT_GE(result->headroom(), kMetadataHeadroomBytes);
}

} // namespace apache::thrift::fast_thrift::thrift
