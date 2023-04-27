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

#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/gen-cpp2/SerializationInFieldIdOrder_types.h>

namespace apache::thrift::test {

// Split serialized data into field token
std::vector<int> get_field_ids(std::string serializedData) {
  std::vector<int> ids;
  std::string name;
  int16_t id;
  TType ftype;
  auto buf = IOBuf::copyBuffer(serializedData);

  CompactProtocolReader reader;
  reader.readStructBegin(name);
  reader.setInput(buf.get());

  while (true) {
    reader.readFieldBegin(name, ftype, id);

    if (ftype == TType::T_STOP) {
      return ids;
    }

    ids.push_back(id);
    reader.skip(ftype);
  }
}

TEST(Foo, RoundTrip) {
  Foo foo;
  Foo2 foo2;
  foo.field1() = 10;
  foo.field2() = 20;
  foo.field3() = 30;
  foo2.field1() = 10;
  foo2.field2() = 20;
  foo2.field3() = 30;
  auto s = CompactSerializer::serialize<std::string>(foo);
  auto s2 = CompactSerializer::serialize<std::string>(foo2);
  EXPECT_EQ(s.size() + 1, s2.size()); // Save 1 byte
  EXPECT_EQ(get_field_ids(s), (std::vector<int>{1, 2, 3}));
  EXPECT_EQ(get_field_ids(s2), (std::vector<int>{3, 1, 2}));
  EXPECT_EQ(CompactSerializer::deserialize<Foo>(s), foo);
  EXPECT_EQ(CompactSerializer::deserialize<Foo2>(s2), foo2);

  // Test backward/forward compatiblity
  EXPECT_EQ(CompactSerializer::deserialize<Foo>(s2), foo);
  EXPECT_EQ(CompactSerializer::deserialize<Foo2>(s), foo2);
}
} // namespace apache::thrift::test
