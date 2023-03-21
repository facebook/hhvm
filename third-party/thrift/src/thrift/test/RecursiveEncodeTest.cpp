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
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/testset/Populator.h>

namespace apache::thrift::test {

namespace {

template <typename T>
class RecursiveEncodeTest : public testing::Test {
 private:
  std::mt19937 rng_;

 protected:
  T populated() { return populated_if_not_adapted<T>(rng_); }

  template <typename SerializerT>
  void testSerializer() {
    T obj = this->populated();
    folly::IOBufQueue buffer;
    typename SerializerT::ProtocolWriter writer;
    writer.setOutput(&buffer);
    op::detail::recursive_encode<T>(writer, obj);
    T obj2 = SerializerT::template deserialize<T>(buffer.move().get());
    EXPECT_EQ(obj, obj2);
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

} // namespace
} // namespace apache::thrift::test
