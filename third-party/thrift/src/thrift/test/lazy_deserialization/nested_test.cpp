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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/lazy_deserialization/gen-cpp2/nested_types.h>

namespace apache::thrift::test {

template <typename>
struct NestedStruct : testing::Test {};

using TypeParams = ::testing::Types<Outer, OuterWithMixin>;
TYPED_TEST_SUITE(NestedStruct, TypeParams);

TYPED_TEST(NestedStruct, Test) {
  TypeParam outer, tmp;
  tmp.foo_ref().ensure().field_ref() = 42;
  auto s = CompactSerializer::serialize<std::string>(tmp);

  // `outer` has lazy field `foo` that's not deserialized.
  CompactSerializer::deserialize(s, outer);

  // Reset `foo` in `Outer` to unset `field` in `Inner`
  tmp.foo_ref().emplace();

  // here `foo` in `Outer` will be serialized, but `field` in `Inner` won't
  // since it's optional field that's not set
  s = CompactSerializer::serialize<std::string>(tmp);

  // Deserialize the result. We will first deserialize the lazy field,
  // Then deserialize `foo` on the top of existing value in `outer`.
  //
  // Since `field` in `Inner` is optional field that's unset, we will
  // keep existing value unchanged, which is 42.
  CompactSerializer::deserialize(s, outer);
  EXPECT_EQ(outer.foo_ref()->field_ref(), 42);
}

TYPED_TEST(NestedStruct, Reserialize) {
  TypeParam outer1, outer2;
  outer1.foo_ref().ensure();
  outer2.foo_ref()->field_ref() = 42;

  // Now `outer2` is in a state that the inner field has value `42`
  // Meanwhile, it also has `IOBuf` that contains empty structure.
  CompactSerializer::deserialize(
      CompactSerializer::serialize<std::string>(outer1), outer2);

  // When we re-serialize outer2, we should deserialize `IOBuf`
  // first, then re-serialize the field
  auto outer3 = CompactSerializer::deserialize<TypeParam>(
      CompactSerializer::serialize<std::string>(outer2));

  EXPECT_EQ(outer3.foo_ref()->field_ref(), 42);
}

TEST(NestedStruct, Mixin) {
  OuterWithMixin outer1, outer2;
  outer1.field_ref() = 42;
  CompactSerializer::deserialize(
      CompactSerializer::serialize<std::string>(outer1), outer2);
  EXPECT_EQ(outer2.field_ref(), 42);
}
} // namespace apache::thrift::test
