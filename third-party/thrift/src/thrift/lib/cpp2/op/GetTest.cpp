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

#include <thrift/lib/cpp2/op/Get.h>

#include <memory>
#include <type_traits>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/FieldRefTraits.h>
#include <thrift/lib/cpp2/op/Create.h>
#include <thrift/lib/thrift/gen-cpp2/schema_types.h>
#include <thrift/test/gen-cpp2/get_value_or_null_types.h>

namespace apache::thrift::op {
namespace {
// TODO(afuller): Use shared test structs instead, e.g. testset
using test::FieldRefNotOptionalStruct;
using test::FieldRefOptionalStruct;
using test::SmartPointerStruct;
using test::UnionStruct;
using type::DecodedUri;

TEST(GetTest, GetField) {
  DecodedUri actual;
  using Tag = type::struct_t<DecodedUri>;
  actual.scheme() = "foo";
  EXPECT_EQ(*(op::get<>(type::ordinal<1>{}, actual)), "foo");
  EXPECT_EQ(*(op::get<ident::scheme>(actual)), "foo");
  EXPECT_EQ(*(op::get<type::field_id<1>, DecodedUri>(actual)), "foo");
  EXPECT_EQ(*(op::get<type::field_id<1>, Tag>(actual)), "foo");
}

// O(N) impl for testing.
template <typename T>
FieldId findIdByName(const std::string& name) {
  return find_by_field_id<T>([&](auto id) {
    return op::get_name_v<T, decltype(id)> == name ? id() : FieldId{};
  });
}

TEST(GetTest, FindByOrdinal) {
  EXPECT_EQ(findIdByName<DecodedUri>("unknown"), FieldId{});
  EXPECT_EQ(findIdByName<DecodedUri>("scheme"), FieldId{1});
}

template <typename Id, typename T>
void testFieldRef(T obj) {
  auto field = op::get<Id>(obj);
  field = 2;
  EXPECT_EQ(*op::getValueOrNull(field), 2);
  // test with const T&
  const auto& objRef = obj;
  auto fieldConst = op::get<Id>(objRef);
  EXPECT_EQ(*op::getValueOrNull(fieldConst), 2);
}

template <typename Id, typename T>
void testGetValueNotOptional(T obj) {
  auto field = op::get<Id>(obj);
  EXPECT_EQ(*op::getValueOrNull(field), 0);
  testFieldRef<Id>(obj);
}

template <typename Id, typename T>
void testGetValueOptional(T obj) {
  auto field = op::get<Id>(obj);
  EXPECT_EQ(op::getValueOrNull(field), nullptr);
  testFieldRef<Id>(obj);
}

template <typename Id, typename T>
void testGetValueSmartPointer(T obj) {
  auto& field = op::get<Id>(obj);
  EXPECT_EQ(op::getValueOrNull(field), nullptr);
  if constexpr (thrift::detail::is_unique_ptr_v<
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
  op::for_each_ordinal<FieldRefNotOptionalStruct>(
      [&](auto id) { testGetValueNotOptional<decltype(id)>(obj); });
}

TEST(GetValueOrNullTest, FieldRefOptional) {
  FieldRefOptionalStruct obj;
  op::for_each_ordinal<FieldRefOptionalStruct>(
      [&](auto id) { testGetValueOptional<decltype(id)>(obj); });
}

TEST(GetValueOrNullTest, SmartPointer) {
  SmartPointerStruct obj;
  op::for_each_ordinal<SmartPointerStruct>(
      [&](auto id) { testGetValueSmartPointer<decltype(id)>(obj); });
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

TEST(FindByOrdinal, Empty) {
  EXPECT_FALSE(find_by_ordinal<test::Empty>([](auto) { return true; }));
}

} // namespace
} // namespace apache::thrift::op
