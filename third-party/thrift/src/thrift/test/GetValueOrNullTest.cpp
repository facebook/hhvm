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

#include <memory>
#include <type_traits>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/FieldRefTraits.h>
#include <thrift/lib/cpp2/op/Ensure.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/test/gen-cpp2/get_value_or_null_types.h>

namespace apache::thrift::test {
namespace {

template <typename Obj, typename Ord>
void testFieldRef(Obj obj, Ord ordinal) {
  auto field = op::get<decltype(obj), decltype(ordinal)>(obj);
  field = 2;
  EXPECT_EQ(*op::getValueOrNull(field), 2);
  // test with const T&
  const auto& objRef = obj;
  auto fieldConst = op::get<decltype(obj), decltype(ordinal)>(objRef);
  EXPECT_EQ(*op::getValueOrNull(fieldConst), 2);
}

template <typename Obj, typename Ord>
void testGetValueNotOptional(Obj obj, Ord ordinal) {
  auto field = op::get<decltype(obj), decltype(ordinal)>(obj);
  EXPECT_EQ(*op::getValueOrNull(field), 0);
  testFieldRef(obj, ordinal);
}

template <typename Obj, typename Ord>
void testGetValueOptional(Obj obj, Ord ordinal) {
  auto field = op::get<decltype(obj), decltype(ordinal)>(obj);
  EXPECT_EQ(op::getValueOrNull(field), nullptr);
  testFieldRef(obj, ordinal);
}

template <typename Obj, typename Ord>
void testGetValueSmartPointer(Obj obj, Ord ordinal) {
  auto& field = op::get<decltype(obj), decltype(ordinal)>(obj);
  EXPECT_EQ(op::getValueOrNull(field), nullptr);
  if constexpr (detail::is_unique_ptr_v<
                    std::remove_reference_t<decltype(field)>>) {
    field = std::make_unique<int32_t>(2);
  } else {
    field = std::make_shared<int32_t>(2);
  }
  EXPECT_EQ(*op::getValueOrNull(field), 2);
  // test with const T&
  const auto& fieldConst = field;
  EXPECT_EQ(*op::getValueOrNull(fieldConst), 2);
}

TEST(GetValueOrNullTest, FieldRefNotOptional) {
  FieldRefNotOptionalStruct obj;
  op::for_each_ordinal<FieldRefNotOptionalStruct>([&](auto fieldOrdinalTag) {
    testGetValueNotOptional(obj, fieldOrdinalTag);
  });
}

TEST(GetValueOrNullTest, FieldRefOptional) {
  FieldRefOptionalStruct obj;
  op::for_each_ordinal<FieldRefOptionalStruct>([&](auto fieldOrdinalTag) {
    testGetValueOptional(obj, fieldOrdinalTag);
  });
}

TEST(GetValueOrNullTest, SmartPointer) {
  SmartPointerStruct obj;
  op::for_each_ordinal<SmartPointerStruct>([&](auto fieldOrdinalTag) {
    testGetValueSmartPointer(obj, fieldOrdinalTag);
  });
}

TEST(GetValueOrNullTest, Optional) {
  std::optional<int> opt;
  EXPECT_EQ(op::getValueOrNull(opt), nullptr);
  opt = 1;
  EXPECT_EQ(*op::getValueOrNull(opt), 1);
  // test with const T&
  const auto& optConst = opt;
  opt = 2;
  EXPECT_EQ(*op::getValueOrNull(optConst), 2);
}

TEST(GetValueOrNullTest, Union) {
  {
    UnionStruct obj;
    EXPECT_EQ(op::getValueOrNull(obj.field_ref()), nullptr);
    obj.field_ref().emplace().int_field_ref() = 2;
    EXPECT_EQ(op::getValueOrNull(obj.field_ref())->get_int_field(), 2);
  }
  {
    // test with const T&
    UnionStruct obj;
    const auto& objConst = obj;
    EXPECT_EQ(op::getValueOrNull(objConst.field_ref()), nullptr);
    obj.field_ref().emplace().string_field_ref() = "foo";
    EXPECT_EQ(
        op::getValueOrNull(objConst.field_ref())->get_string_field(), "foo");
  }
}
} // namespace
} // namespace apache::thrift::test
