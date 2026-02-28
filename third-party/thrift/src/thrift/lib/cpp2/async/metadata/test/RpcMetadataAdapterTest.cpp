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
#include <thrift/lib/cpp2/async/metadata/CursorBasedRequestRpcMetadataAdapter.h>

#include <thrift/lib/cpp2/async/metadata/RequestRpcMetadataFacade.h>
#include <thrift/lib/cpp2/async/metadata/TCompactRequestRpcMetadataAdapter.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace ::testing;
using namespace apache::thrift;
TEST(RpcMetadataAdapterTest, testEmptyWithTCompact) {
  RequestRpcMetadata metadata;

  CompactProtocolWriter writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  metadata.write(&writer);

  auto adapter = TCompactRequestRpcMetadataAdapter(queue.move());
  EXPECT_FALSE(adapter.protocolId().has_value());
}

TEST(RpcMetadataAdapterTest, testEmptyWithCursor) {
  RequestRpcMetadata metadata;

  CompactProtocolWriter writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  metadata.write(&writer);

  auto adapter = CursorBasedRequestRpcMetadataAdapter(queue.move());
  EXPECT_FALSE(adapter.protocolId().has_value());
}

TEST(RpcMetadataAdapterTest, testMetadataWithTCompact) {
  RequestRpcMetadata metadata;
  metadata.protocol() = ProtocolId::COMPACT;
  metadata.name() = "test";
  metadata.kind() = RpcKind::SINGLE_REQUEST_NO_RESPONSE;

  CompactProtocolWriter writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  metadata.write(&writer);

  auto adapter = TCompactRequestRpcMetadataAdapter(queue.move());
  EXPECT_EQ(adapter.protocolId().value(), ProtocolId::COMPACT);
  EXPECT_EQ(adapter.name().value().get().str(), "test");
  EXPECT_EQ(adapter.kind().value(), RpcKind::SINGLE_REQUEST_NO_RESPONSE);
}

TEST(RpcMetadataAdapterTest, testMetadataWithCursor) {
  RequestRpcMetadata metadata;
  metadata.protocol() = ProtocolId::COMPACT;
  metadata.name() = "test";
  metadata.kind() = RpcKind::SINGLE_REQUEST_NO_RESPONSE;

  BinaryProtocolWriter writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  metadata.write(&writer);

  auto adapter = CursorBasedRequestRpcMetadataAdapter(queue.move());
  EXPECT_EQ(adapter.protocolId().value(), ProtocolId::COMPACT);
  EXPECT_EQ(adapter.name().value().get().str(), "test");
  EXPECT_EQ(adapter.kind().value(), RpcKind::SINGLE_REQUEST_NO_RESPONSE);
}

TEST(RpcMetadataAdapterTest, testMetadataWithAdapter) {
  RequestRpcMetadata metadata;
  metadata.protocol() = ProtocolId::COMPACT;
  metadata.name() = "test";
  metadata.kind() = RpcKind::SINGLE_REQUEST_NO_RESPONSE;

  CompactProtocolWriter writer;
  folly::IOBufQueue queue;
  writer.setOutput(&queue);
  metadata.write(&writer);

  auto facade = RequestRpcMetadataFacade(queue.move());
  EXPECT_EQ(facade.protocolId().value(), ProtocolId::COMPACT);
  EXPECT_EQ(facade.name().value().get().str(), "test");
  EXPECT_EQ(facade.kind().value(), RpcKind::SINGLE_REQUEST_NO_RESPONSE);
}
