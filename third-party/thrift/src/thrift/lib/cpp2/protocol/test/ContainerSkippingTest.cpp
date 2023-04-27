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

#include <folly/io/Cursor.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/Module_types.h>

namespace apache {
namespace thrift {
namespace test {

OneOfEach makeTestData1() {
  OneOfEach ooe;
  *ooe.myBool_ref() = false;
  *ooe.myMap_ref() = std::map<std::string, int64_t>(
      {{"key1", 14}, {"key2", 0}, {"key3", 1000}});
  *ooe.myList_ref() = std::vector<std::string>{"good1", "good2", "good3"};
  return ooe;
}

OneOfEach makeTestData2() {
  OneOfEach ooe;
  *ooe.myMap_ref() = std::map<std::string, int64_t>(
      {{"key1", 14}, {"key2", 0}, {"key3", 1000}});
  *ooe.myList_ref() = std::vector<std::string>{"good1", "good2", "good3"};
  *ooe.mySet_ref() = std::set<std::string>{"elem1", "elem2", "elem3"};
  SubStruct sub;
  *sub.mySubI64_ref() = 123456789;
  *sub.mySubString_ref() = "substring";
  *ooe.myStruct_ref() = sub;
  return ooe;
}

template <class ProtocolWriter, class ProtocolReader>
void testSkipMap() {
  OneOfEach ooe = makeTestData1();

  ProtocolWriter protocolWriter;
  folly::IOBufQueue q;
  protocolWriter.setOutput(&q);
  ooe.write(&protocolWriter);

  ProtocolReader protocolReader;
  protocolReader.setInput(q.front());
  // Deserialize into a struct with different map types
  OneOfEach2 ooe2;
  EXPECT_NO_THROW(ooe2.read(&protocolReader));
  EXPECT_EQ(0, ooe2.myMap_ref()->size());
  EXPECT_EQ(*ooe.myList_ref(), *ooe2.myList_ref());
}

template <class ProtocolWriter, class ProtocolReader>
void testSkipListAndSet() {
  OneOfEach ooe = makeTestData2();

  ProtocolWriter protocolWriter;
  folly::IOBufQueue q;
  protocolWriter.setOutput(&q);
  ooe.write(&protocolWriter);

  ProtocolReader protocolReader;
  protocolReader.setInput(q.front());
  // Deserialize into a struct with different list/set types
  OneOfEach3 ooe3;
  EXPECT_NO_THROW(ooe3.read(&protocolReader));
  EXPECT_EQ(*ooe.myMap_ref(), *ooe3.myMap_ref());
  EXPECT_EQ(0, ooe3.myList_ref()->size());
  EXPECT_EQ(0, ooe3.mySet_ref()->size());
  EXPECT_EQ(*ooe.myStruct_ref(), *ooe3.myStruct_ref());
}

TEST(ContainerSkippingTest, BinaryProtocolSkipMap) {
  testSkipMap<BinaryProtocolWriter, BinaryProtocolReader>();
}

TEST(ContainerSkippingTest, CompactProtocolSkipMap) {
  testSkipMap<CompactProtocolWriter, CompactProtocolReader>();
}

TEST(ContainerSkippingTest, BinaryProtocolSkipListAndSet) {
  testSkipListAndSet<BinaryProtocolWriter, BinaryProtocolReader>();
}

TEST(ContainerSkippingTest, CompactProtocolSkipListAndSet) {
  testSkipListAndSet<CompactProtocolWriter, CompactProtocolReader>();
}

TEST(ContainerSkippingTest, SimpleJSONProtocolSkipMap) {
  OneOfEach ooe = makeTestData1();

  SimpleJSONProtocolWriter protocolWriter;
  folly::IOBufQueue q;
  protocolWriter.setOutput(&q);
  ooe.write(&protocolWriter);

  SimpleJSONProtocolReader protocolReader;
  protocolReader.setInput(q.front());
  // Deserialize into a struct with different map types
  OneOfEach2 ooe2;
  EXPECT_THROW(ooe2.read(&protocolReader), TProtocolException);
}

TEST(ContainerSkippingTest, SimpleJSONProtocolSkipListAndSet) {
  OneOfEach ooe = makeTestData2();

  SimpleJSONProtocolWriter protocolWriter;
  folly::IOBufQueue q;
  protocolWriter.setOutput(&q);
  ooe.write(&protocolWriter);

  SimpleJSONProtocolReader protocolReader;
  protocolReader.setInput(q.front());
  // Deserialize into a struct with different list/set types
  OneOfEach3 ooe3;
  EXPECT_THROW(ooe3.read(&protocolReader), TProtocolException);
}
} // namespace test
} // namespace thrift
} // namespace apache
