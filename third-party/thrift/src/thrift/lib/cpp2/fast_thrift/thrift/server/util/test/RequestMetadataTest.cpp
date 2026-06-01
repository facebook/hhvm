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
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/RequestMetadata.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

std::unique_ptr<folly::IOBuf> serializeMetadata(
    const apache::thrift::RequestRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

frame::read::ParsedFrame makePayloadFrame(
    std::unique_ptr<folly::IOBuf> metadata = nullptr,
    std::unique_ptr<folly::IOBuf> data = nullptr) {
  auto raw = frame::write::serialize(
      frame::write::PayloadHeader{
          .streamId = 1, .follows = false, .complete = true, .next = true},
      std::move(metadata),
      std::move(data));
  return frame::read::parseFrame(std::move(raw));
}

} // namespace

TEST(RequestMetadataTest, DeserializesPopulatedMetadata) {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = "myMethod";
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;

  auto frame = makePayloadFrame(serializeMetadata(metadata));

  apache::thrift::RequestRpcMetadata result;
  auto error = deserializeRequestMetadata(frame, result);

  EXPECT_FALSE(error);
  ASSERT_TRUE(result.name().has_value());
  EXPECT_EQ(result.name()->str(), "myMethod");
  ASSERT_TRUE(result.protocol().has_value());
  EXPECT_EQ(*result.protocol(), apache::thrift::ProtocolId::BINARY);
}

TEST(RequestMetadataTest, DeserializesEmptyMetadata) {
  apache::thrift::RequestRpcMetadata metadata;
  auto frame = makePayloadFrame(serializeMetadata(metadata));

  apache::thrift::RequestRpcMetadata result;
  auto error = deserializeRequestMetadata(frame, result);

  EXPECT_FALSE(error);
}

TEST(RequestMetadataTest, FailsOnGarbageMetadata) {
  auto garbage = folly::IOBuf::copyBuffer("not valid binary protocol!");
  auto frame = makePayloadFrame(std::move(garbage));

  apache::thrift::RequestRpcMetadata result;
  auto error = deserializeRequestMetadata(frame, result);

  EXPECT_TRUE(error);
  EXPECT_TRUE(
      error.is_compatible_with<apache::thrift::TApplicationException>());
  auto msg = error.what().toStdString();
  EXPECT_NE(
      msg.find("Failed to deserialize request metadata"), std::string::npos);
}

TEST(RequestMetadataTest, SucceedsWithNoMetadataSection) {
  auto frame = makePayloadFrame(nullptr, folly::IOBuf::copyBuffer("data"));

  apache::thrift::RequestRpcMetadata result;
  auto error = deserializeRequestMetadata(frame, result);

  EXPECT_FALSE(error);
}

} // namespace apache::thrift::fast_thrift::thrift
