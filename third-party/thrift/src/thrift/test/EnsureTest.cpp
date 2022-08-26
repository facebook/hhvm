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
#include <thrift/lib/cpp2/FieldRefTraits.h>
#include <thrift/lib/cpp2/op/Ensure.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/test/gen-cpp2/ensure_types.h>

namespace apache::thrift::test {
namespace {

template <typename Obj, typename Ord>
void testEnsure(Obj obj, Ord ordinal) {
  using Struct = decltype(obj);
  using FieldTag = op::get_field_tag<Struct, decltype(ordinal)>;
  auto field = op::get<Struct, decltype(ordinal)>(obj);
  op::ensure<FieldTag>(field, obj);
  EXPECT_EQ(field, 0);
  field = 2;
  op::ensure<FieldTag>(field, obj); // no-op
  EXPECT_EQ(field, 2);
}

template <typename Obj, typename Ord>
void testEnsurePtr(Obj obj, Ord ordinal) {
  using Struct = decltype(obj);
  using FieldTag = op::get_field_tag<Struct, decltype(ordinal)>;
  auto& field = op::get<Struct, decltype(ordinal)>(obj);
  op::ensure<FieldTag>(field, obj);
  EXPECT_EQ(*field, 0);
  if constexpr (detail::is_unique_ptr_v<
                    std::remove_reference_t<decltype(field)>>) {
    field = std::make_unique<int32_t>(2);
  } else {
    field = std::make_shared<int32_t>(2);
  }
  op::ensure<FieldTag>(field, obj); // no-op
  EXPECT_EQ(*field, 2);
}

TEST(EnsureTest, FieldRef) {
  FieldRefStruct obj;
  op::for_each_ordinal<FieldRefStruct>(
      [&](auto fieldOrdinalTag) { testEnsure(obj, fieldOrdinalTag); });
}

TEST(EnsureTest, SmartPointer) {
  SmartPointerStruct obj;
  op::for_each_ordinal<SmartPointerStruct>(
      [&](auto fieldOrdinalTag) { testEnsurePtr(obj, fieldOrdinalTag); });
}

TEST(EnsureTest, Optional) {
  FieldRefStruct obj;
  using FieldTag = op::get_field_tag<FieldRefStruct, field_ordinal<2>>;
  auto opt = obj.optional_i32_ref().to_optional();
  op::ensure<FieldTag>(opt, obj);
  EXPECT_EQ(*opt, 0);
  opt = 2;
  op::ensure<FieldTag>(opt, obj);
  EXPECT_EQ(*opt, 2);
}

} // namespace
} // namespace apache::thrift::test
