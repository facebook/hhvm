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

#include <thrift/test/gen-cpp2/Service1.h>
#include <thrift/test/gen-cpp2/SplitsTest_types.h>

#include <gtest/gtest.h>
#include <folly/test/JsonTestUtil.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

using namespace std;
namespace apache::thrift {

template <class T>
T gen() {
  T obj;
  obj.i_ref() = 10;
  obj.s_ref() = "20";
  FOLLY_EXPECT_JSON_EQ(
      SimpleJSONSerializer::serialize<string>(obj), R"({"i": 10, "s": "20"})");
  return obj;
}

template <class Serializer, class Obj>
void roundTripTest(Obj obj1) {
  auto data = Serializer::template serialize<string>(obj1);
  Obj obj2;
  Serializer::deserialize(data, obj2);
  EXPECT_EQ(obj1, obj2);
}

TEST(RoundTrip, test) {
  auto obj1 = gen<struct1>();
  auto obj2 = gen<struct2>();
  struct3 obj3;
  obj3.field1() = obj1;
  obj3.field2() = obj2;

  apply(
      [](auto... obj) {
        (roundTripTest<BinarySerializer>(obj), ...);
        (roundTripTest<CompactSerializer>(obj), ...);
        (roundTripTest<SimpleJSONSerializer>(obj), ...);
      },
      tuple(obj1, obj2, obj3));
}

TEST(Enum, test) {
  EXPECT_STREQ(TEnumTraits<enum1>::findName(enum1::value1), "value1");
  EXPECT_STREQ(TEnumTraits<enum1>::findName(enum1::value2), "value2");

  enum1 e;
  TEnumTraits<enum1>::findValue("value1", &e);
  EXPECT_EQ(e, enum1::value1);
  TEnumTraits<enum1>::findValue("value2", &e);
  EXPECT_EQ(e, enum1::value2);
}

} // namespace apache::thrift
