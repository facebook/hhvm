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
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/gen-cpp2/RecursiveEncode_types.h>
#include <thrift/test/testset/Populator.h>

namespace apache::thrift::test {

namespace {
template <typename SerializerT, typename T>
void roundTripTest(T obj) {
  folly::IOBufQueue buffer;
  typename SerializerT::ProtocolWriter writer;
  writer.setOutput(&buffer);
  op::detail::recursive_encode<T>(writer, obj);
  T obj2 = SerializerT::template deserialize<T>(buffer.move().get());
  EXPECT_EQ(obj, obj2);
}

template <typename T>
class RecursiveEncodeTest : public testing::Test {
 private:
  std::mt19937 rng_;

 protected:
  template <typename SerializerT>
  void testSerializer() {
    roundTripTest<SerializerT>(populated_if_not_adapted<T>(rng_));
  }
};

TYPED_TEST_CASE_P(RecursiveEncodeTest);

TYPED_TEST_P(RecursiveEncodeTest, Compact) {
  this->template testSerializer<CompactSerializer>();
}

TYPED_TEST_P(RecursiveEncodeTest, Binary) {
  this->template testSerializer<BinarySerializer>();
}

TYPED_TEST_P(RecursiveEncodeTest, SimpleJson) {
  this->template testSerializer<SimpleJSONSerializer>();
}

REGISTER_TYPED_TEST_CASE_P(RecursiveEncodeTest, Compact, Binary, SimpleJson);

THRIFT_INST_TESTSET_ALL(RecursiveEncodeTest);

TEST(RecursiveEncode, TestCustomType) {
  Baz baz;
  baz.field() = 10;
  auto& bar = baz.bar()->value.value;
  bar.field() = 20;

  Foo foo1, foo2;
  foo1.field() = 30;
  foo2.field() = 40;

  bar.foos()->push_back(foo1);
  bar.foos()->push_back(foo2);

  roundTripTest<CompactSerializer>(baz);
  roundTripTest<BinarySerializer>(baz);
  roundTripTest<SimpleJSONSerializer>(baz);

  EXPECT_EQ(debugStringViaRecursiveEncode(baz), R"( {
  1: field (i32) = 10,
  2: bar (struct) =  {
    1: field (i32) = 20,
    2: foos (list) = list<struct>[2] {
      [0] =  {
        1: field (i32) = 30,
      },
      [1] =  {
        1: field (i32) = 40,
      },
    },
  },
})");
  EXPECT_EQ(
      debugStringViaRecursiveEncode(
          baz, apache::thrift::DebugProtocolWriter::Options::simple()),
      R"( {
  field = 10,
  bar =  {
    field = 20,
    foos = list<struct>[2] {
       {
        field = 30,
      },
       {
        field = 40,
      },
    },
  },
})");
}

} // namespace
} // namespace apache::thrift::test
