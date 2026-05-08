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

#include <thrift/lib/cpp2/fast_thrift/thrift/common/RequestMetadata.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

apache::thrift::RequestRpcMetadata makePopulatedMetadata() {
  apache::thrift::RequestRpcMetadata md;
  md.protocol() = apache::thrift::ProtocolId::COMPACT;
  md.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  md.name() = "myMethod";
  md.clientTimeoutMs() = 250;
  md.queueTimeoutMs() = 100;
  return md;
}

apache::thrift::RequestRpcMetadata deserialize(folly::IOBuf& buf) {
  apache::thrift::RequestRpcMetadata out;
  apache::thrift::BinaryProtocolReader reader;
  reader.setInput(&buf);
  out.read(&reader);
  return out;
}

} // namespace

TEST(SerializeRequestMetadataTest, RoundTripsPopulatedFields) {
  auto md = makePopulatedMetadata();
  auto buf = serializeRequestMetadata(md);
  ASSERT_NE(buf, nullptr);

  auto decoded = deserialize(*buf);
  EXPECT_EQ(decoded.name()->str(), "myMethod");
  EXPECT_EQ(*decoded.protocol(), apache::thrift::ProtocolId::COMPACT);
  EXPECT_EQ(
      *decoded.kind(), apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  EXPECT_EQ(*decoded.clientTimeoutMs(), 250);
  EXPECT_EQ(*decoded.queueTimeoutMs(), 100);
}

TEST(SerializeRequestMetadataTest, ReservesFrameHeaderHeadroom) {
  auto md = makePopulatedMetadata();
  auto buf = serializeRequestMetadata(md);
  ASSERT_NE(buf, nullptr);
  EXPECT_GE(buf->headroom(), kMetadataHeadroomBytes);
}

} // namespace apache::thrift::fast_thrift::thrift
