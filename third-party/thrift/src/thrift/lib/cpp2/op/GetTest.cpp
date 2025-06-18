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

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/FieldRefTraits.h>
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

TEST(VisitUnionTest, Basic) {
  test::Union obj;

  auto visit = [&] {
    int fid;
    op::visit_union_with_tag(
        obj,
        [&]<typename ident>(folly::tag_t<ident>, auto& value) {
          fid = folly::to_underlying(op::get_field_id_v<test::Union, ident>);
          LOG(INFO) << "Active field: "
                    << op::get_name_v<test::Union, ident> << " with value: "
                    << value;
        },
        [&]() {
          fid = 0;
          LOG(INFO) << "No active field";
        });
    return fid;
  };

  EXPECT_EQ(visit(), 0);

  obj.int_field_ref() = 0;
  EXPECT_EQ(visit(), 1);

  obj.string_field_ref() = "foo";
  EXPECT_EQ(visit(), 2);

  apache::thrift::clear(obj);
  EXPECT_EQ(visit(), 0);

  // Ensure examples compile.
  op::visit_union_with_tag(
      obj,
      [&](folly::tag_t<ident::int_field>, int& f) {
        LOG(INFO) << "Int value: " << f;
      },
      [&](folly::tag_t<ident::string_field>, std::string& f) {
        LOG(INFO) << "String value: " << f;
      },
      [&]() { LOG(INFO) << "No active field"; });
  op::visit_union_with_tag(
      obj,
      []<typename ident>(folly::tag_t<ident>, auto& value) {
        LOG(INFO) << op::get_name_v<test::Union, ident> << " --> " << value;
      },
      []() { LOG(INFO) << "Empty union"; });
}

} // namespace
} // namespace apache::thrift::op
