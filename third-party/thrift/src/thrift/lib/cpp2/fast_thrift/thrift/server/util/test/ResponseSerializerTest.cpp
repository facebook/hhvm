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
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseSerializer.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache::thrift::fast_thrift::thrift {

TEST(ResponseSerializerTest, SerializeResponseWithSizeHint) {
  auto response = serializeResponse<apache::thrift::CompactProtocolWriter>(
      [](apache::thrift::CompactProtocolWriter& w) {
        w.writeStructBegin("Result");
        w.writeFieldBegin("val", apache::thrift::protocol::T_I32, 0);
        w.writeI32(99);
        w.writeFieldEnd();
        w.writeFieldStop();
        w.writeStructEnd();
      },
      [](apache::thrift::CompactProtocolWriter&) -> uint32_t { return 16; },
      42);

  EXPECT_EQ(response.streamId, 42);
  EXPECT_EQ(response.errorCode, 0);
  ASSERT_NE(response.payload.data, nullptr);
  EXPECT_GT(response.payload.data->computeChainDataLength(), 0);
  ASSERT_NE(response.payload.metadata, nullptr);
  EXPECT_TRUE(response.payload.complete);
}

TEST(ResponseSerializerTest, SerializeResponseWithoutSizeHint) {
  auto response = serializeResponse<apache::thrift::CompactProtocolWriter>(
      [](apache::thrift::CompactProtocolWriter& w) {
        w.writeStructBegin("Result");
        w.writeFieldStop();
        w.writeStructEnd();
      },
      7);

  EXPECT_EQ(response.streamId, 7);
  ASSERT_NE(response.payload.data, nullptr);
  ASSERT_NE(response.payload.metadata, nullptr);
  EXPECT_TRUE(response.payload.complete);
}

TEST(ResponseSerializerTest, SerializeResponsePreservesStreamId) {
  auto r1 = serializeResponse<apache::thrift::CompactProtocolWriter>(
      [](apache::thrift::CompactProtocolWriter& w) {
        w.writeStructBegin("R");
        w.writeFieldStop();
        w.writeStructEnd();
      },
      0);
  auto r2 = serializeResponse<apache::thrift::CompactProtocolWriter>(
      [](apache::thrift::CompactProtocolWriter& w) {
        w.writeStructBegin("R");
        w.writeFieldStop();
        w.writeStructEnd();
      },
      999);

  EXPECT_EQ(r1.streamId, 0);
  EXPECT_EQ(r2.streamId, 999);
}

TEST(ResponseSerializerTest, BuildErrorResponseHasNullData) {
  auto response = buildErrorResponse(10, "test error");

  EXPECT_EQ(response.streamId, 10);
  EXPECT_EQ(response.payload.data, nullptr);
  ASSERT_NE(response.payload.metadata, nullptr);
  EXPECT_TRUE(response.payload.complete);
}

TEST(ResponseSerializerTest, BuildErrorResponsePreservesStreamId) {
  auto r1 = buildErrorResponse(0, "a");
  auto r2 = buildErrorResponse(999, "b");

  EXPECT_EQ(r1.streamId, 0);
  EXPECT_EQ(r2.streamId, 999);
}

} // namespace apache::thrift::fast_thrift::thrift
