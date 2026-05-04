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

TEST(ResponseSerializerTest, SerializeResponseRoundTrip) {
  auto buf = serializeResponse<apache::thrift::CompactProtocolWriter>(
      [](apache::thrift::CompactProtocolWriter& w) {
        w.writeStructBegin("Result");
        w.writeFieldBegin("val", apache::thrift::protocol::T_I32, 0);
        w.writeI32(12345);
        w.writeFieldEnd();
        w.writeFieldStop();
        w.writeStructEnd();
      },
      [](apache::thrift::CompactProtocolWriter&) -> uint32_t { return 16; });

  ASSERT_NE(buf, nullptr);
  EXPECT_GT(buf->computeChainDataLength(), 0u);
  // The helper must reserve the standard server data headroom at the front so
  // that downstream framing can prepend headers without copying.
  EXPECT_EQ(buf->headroom(), kServerDataHeadroomBytes);

  apache::thrift::CompactProtocolReader reader;
  reader.setInput(buf.get());
  std::string structName;
  reader.readStructBegin(structName);
  std::string fieldName;
  apache::thrift::protocol::TType fieldType;
  int16_t fieldId;
  reader.readFieldBegin(fieldName, fieldType, fieldId);
  EXPECT_EQ(fieldType, apache::thrift::protocol::T_I32);
  EXPECT_EQ(fieldId, 0);
  int32_t val = 0;
  reader.readI32(val);
  EXPECT_EQ(val, 12345);
}

} // namespace apache::thrift::fast_thrift::thrift
