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

#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/StreamPayloadMetadata.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

apache::thrift::StreamPayloadMetadata makePopulatedMetadata() {
  apache::thrift::StreamPayloadMetadata md;
  md.compression() = apache::thrift::CompressionAlgorithm::ZSTD;
  md.otherMetadata().emplace();
  (*md.otherMetadata())["traceId"] = "abc123";
  (*md.otherMetadata())["spanId"] = "xyz789";
  return md;
}

template <typename Reader>
apache::thrift::StreamPayloadMetadata deserialize(folly::IOBuf& buf) {
  apache::thrift::StreamPayloadMetadata out;
  Reader reader;
  reader.setInput(&buf);
  out.read(&reader);
  return out;
}

} // namespace

TEST(SerializeStreamPayloadMetadataTest, RoundTripsBinary) {
  auto md = makePopulatedMetadata();
  auto buf = serializeStreamPayloadMetadata(
      md, rocket::server::MetadataProtocol::BINARY);
  ASSERT_NE(buf, nullptr);

  auto decoded = deserialize<apache::thrift::BinaryProtocolReader>(*buf);
  EXPECT_EQ(*decoded.compression(), apache::thrift::CompressionAlgorithm::ZSTD);
  ASSERT_TRUE(decoded.otherMetadata().has_value());
  EXPECT_EQ((*decoded.otherMetadata())["traceId"], "abc123");
  EXPECT_EQ((*decoded.otherMetadata())["spanId"], "xyz789");
}

TEST(SerializeStreamPayloadMetadataTest, RoundTripsCompact) {
  auto md = makePopulatedMetadata();
  auto buf = serializeStreamPayloadMetadata(
      md, rocket::server::MetadataProtocol::COMPACT);
  ASSERT_NE(buf, nullptr);

  auto decoded = deserialize<apache::thrift::CompactProtocolReader>(*buf);
  EXPECT_EQ(*decoded.compression(), apache::thrift::CompressionAlgorithm::ZSTD);
  ASSERT_TRUE(decoded.otherMetadata().has_value());
  EXPECT_EQ((*decoded.otherMetadata())["traceId"], "abc123");
  EXPECT_EQ((*decoded.otherMetadata())["spanId"], "xyz789");
}

TEST(SerializeStreamPayloadMetadataTest, ReservesFrameHeaderHeadroom) {
  auto md = makePopulatedMetadata();
  auto buf = serializeStreamPayloadMetadata(
      md, rocket::server::MetadataProtocol::BINARY);
  ASSERT_NE(buf, nullptr);
  EXPECT_GE(buf->headroom(), kStreamPayloadMetadataHeadroomBytes);
}

TEST(SerializeStreamPayloadMetadataTest, EmptyMetadataSerializes) {
  apache::thrift::StreamPayloadMetadata md;
  auto buf = serializeStreamPayloadMetadata(
      md, rocket::server::MetadataProtocol::BINARY);
  ASSERT_NE(buf, nullptr);
  // Empty struct still serializes (just a STOP marker for binary protocol);
  // round-trip yields an empty StreamPayloadMetadata.
  auto decoded = deserialize<apache::thrift::BinaryProtocolReader>(*buf);
  EXPECT_FALSE(decoded.compression().has_value());
  EXPECT_FALSE(decoded.otherMetadata().has_value());
}

} // namespace apache::thrift::fast_thrift::thrift
