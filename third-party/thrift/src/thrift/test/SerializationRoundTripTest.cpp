/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <fatal/type/slice.h>
#include <folly/lang/Pretty.h>
#include <folly/portability/GTest.h>
#include <thrift/conformance/cpp2/AnyRegistry.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/reflection/populator.h>
#include <thrift/lib/cpp2/reflection/reflection.h>
#include <thrift/test/testset/Testing.h>
#include <thrift/test/testset/gen-cpp2/testset_fatal_types.h>

namespace apache::thrift::test {

namespace {

using conformance::Any;
using conformance::AnyRegistry;
using conformance::StandardProtocol;

template <typename T>
class SerializationRoundTripTest : public testing::Test {
 private:
  std::mt19937 rng_;

 protected:
  T populated() {
    T result;
    populator::populate(result, {}, rng_);
    return result;
  }

  template <typename SerializerT>
  void testSerializer() {
    SCOPED_TRACE(folly::pretty_name<T>());
    T obj1 = this->populated();
    auto data = SerializerT::template serialize<std::string>(obj1);
    T obj2;
    SerializerT::template deserialize(data, obj2);
    EXPECT_EQ(obj1, obj2);
  }

  template <StandardProtocol P>
  void testAny() {
    SCOPED_TRACE(folly::pretty_name<T>());
    T obj1 = this->populated();
    Any any = AnyRegistry::generated().store<P>(obj1);
    T obj2 = AnyRegistry::generated().load<T>(any);
    EXPECT_EQ(obj1, obj2);
  }
};

TYPED_TEST_CASE_P(SerializationRoundTripTest);

TYPED_TEST_P(SerializationRoundTripTest, Compact) {
  this->template testSerializer<CompactSerializer>();
}

TYPED_TEST_P(SerializationRoundTripTest, Binary) {
  this->template testSerializer<BinarySerializer>();
}

TYPED_TEST_P(SerializationRoundTripTest, SimpleJson) {
  this->template testSerializer<SimpleJSONSerializer>();
}

TYPED_TEST_P(SerializationRoundTripTest, Compact_Any) {
  this->template testAny<StandardProtocol::Compact>();
}

TYPED_TEST_P(SerializationRoundTripTest, Binary_Any) {
  this->template testAny<StandardProtocol::Binary>();
}

TYPED_TEST_P(SerializationRoundTripTest, SimpleJson_Any) {
  this->template testAny<StandardProtocol::SimpleJson>();
}

REGISTER_TYPED_TEST_CASE_P(
    SerializationRoundTripTest,
    Compact,
    Binary,
    SimpleJson,
    Compact_Any,
    Binary_Any,
    SimpleJson_Any);

THRIFT_INST_TESTSET_ALL(SerializationRoundTripTest);

} // namespace
} // namespace apache::thrift::test
