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

#include <type_traits>

#include <gtest/gtest.h>
#include <thrift/lib/cpp/Field.h>
#include <thrift/lib/cpp2/FieldRefTraits.h>
#include <thrift/lib/cpp2/gen/module_types_h.h>
#include <thrift/lib/cpp2/op/Create.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/thrift/gen-cpp2/type_types.h>
#include <thrift/test/gen-cpp2/ensure_types.h>
#include <thrift/test/testset/Testset.h>
#include <thrift/test/testset/gen-cpp2/testset_types.h>

namespace apache::thrift::op {
namespace {
// TODO(afuller): Use testset instead.
using test::FieldRefStruct;
using test::SmartPointerStruct;
using namespace apache::thrift::type;
namespace testset = apache::thrift::test::testset;

// Wrapper with default constructor deleted.
template <typename T>
struct Wrapper {
  T value;
  Wrapper() = delete;
  explicit Wrapper(T val) : value(std::move(val)) {}
  bool operator==(const Wrapper& other) const { return value == other.value; }
  bool operator<(const Wrapper& other) const { return value < other.value; }
};

// Wrapper with context with default constructor deleted.
template <typename T, typename Struct, int16_t FieldId>
struct WrapperWithContext {
  T value;
  std::string* meta = nullptr;
  WrapperWithContext() = delete;
  explicit WrapperWithContext(T val, std::string& m)
      : value(std::move(val)), meta(&m) {}
  bool operator==(const WrapperWithContext& other) const {
    return value == other.value;
  }
  bool operator<(const WrapperWithContext& other) const {
    return value < other.value;
  }
};

struct TestTypeAdapter {
  template <typename T>
  static Wrapper<T> fromThrift(T value) {
    return Wrapper<T>{value};
  }

  template <typename T>
  static T toThrift(Wrapper<T> wrapper) {
    return wrapper.value;
  }
};

struct TestFieldAdapter {
  template <typename T, typename Struct, int16_t FieldId>
  static WrapperWithContext<T, Struct, FieldId> fromThriftField(
      T value, apache::thrift::FieldContext<Struct, FieldId>&& ctx) {
    return WrapperWithContext<T, Struct, FieldId>{value, ctx.object.meta};
  }

  template <typename T, typename Struct, int16_t FieldId>
  static T toThrift(WrapperWithContext<T, Struct, FieldId> wrapper) {
    return wrapper.value;
  }

  template <typename T, typename Struct, int16_t FieldId>
  static void construct(
      WrapperWithContext<T, Struct, FieldId>& field,
      FieldContext<Struct, FieldId>&& ctx) {
    field.meta = &ctx.object.meta;
  }
};

template <typename Tag>
using adapted_tag = adapted<TestTypeAdapter, Tag>;
template <typename Tag>
using field_adapted_tag = adapted<TestFieldAdapter, Tag>;

struct TestThriftType {
  std::string meta;
};

template <typename Tag>
void testCreateWithTag() {
  using tag = Tag;
  using ctx = FieldContext<TestThriftType, 0>;
  using field_tag = type::field<tag, ctx>;
  using type_adapted_field_tag = type::field<adapted_tag<tag>, ctx>;
  using field_adapted_field_tag = type::field<field_adapted_tag<tag>, ctx>;
  using double_type_adapted_field_tag =
      type::field<adapted_tag<adapted_tag<tag>>, ctx>;
  using field_and_type_adapted_field_tag =
      type::field<field_adapted_tag<adapted_tag<tag>>, ctx>;

  TestThriftType object;

  auto type_created = create<tag>();
  auto adapted_created = create<adapted_tag<tag>>();
  auto field_created = create<field_tag>();
  auto type_adapted_field_created = create<type_adapted_field_tag>(object);
  auto field_adapted_field_created = create<field_adapted_field_tag>(object);
  auto double_type_adapted_field_created =
      create<double_type_adapted_field_tag>(object);
  auto field_and_type_adapted_field_created =
      create<field_and_type_adapted_field_tag>(object);

  static_assert(std::is_same_v<decltype(type_created), native_type<tag>>);
  static_assert(
      std::is_same_v<decltype(adapted_created), Wrapper<native_type<tag>>>);
  static_assert(std::is_same_v<decltype(field_created), native_type<tag>>);
  static_assert(std::is_same_v<
                decltype(type_adapted_field_created),
                Wrapper<native_type<tag>>>);
  static_assert(std::is_same_v<
                decltype(field_adapted_field_created),
                WrapperWithContext<native_type<tag>, TestThriftType, 0>>);
  static_assert(std::is_same_v<
                decltype(double_type_adapted_field_created),
                Wrapper<Wrapper<native_type<tag>>>>);
  static_assert(
      std::is_same_v<
          decltype(field_and_type_adapted_field_created),
          WrapperWithContext<Wrapper<native_type<tag>>, TestThriftType, 0>>);

  // Check if the context is correctly populated.
  EXPECT_EQ(&object.meta, field_adapted_field_created.meta);
  EXPECT_EQ(&object.meta, field_and_type_adapted_field_created.meta);
}

template <typename Tag>
void testCreateStructured() {
  testCreateWithTag<struct_t<testset::struct_with<Tag>>>();
  testCreateWithTag<
      struct_t<testset::struct_with<Tag, testset::FieldModifier::Optional>>>();
  testCreateWithTag<
      struct_t<testset::struct_with<Tag, testset::FieldModifier::Terse>>>();
  testCreateWithTag<
      struct_t<testset::struct_with<Tag, testset::FieldModifier::Required>>>();
  testCreateWithTag<exception_t<testset::exception_with<Tag>>>();
  testCreateWithTag<exception_t<
      testset::exception_with<Tag, testset::FieldModifier::Optional>>>();
  testCreateWithTag<exception_t<
      testset::exception_with<Tag, testset::FieldModifier::Terse>>>();
  testCreateWithTag<exception_t<
      testset::exception_with<Tag, testset::FieldModifier::Required>>>();
  testCreateWithTag<union_t<testset::union_with<Tag>>>();
}

template <typename Tag>
void testCreate() {
  testCreateWithTag<Tag>();
  testCreateStructured<Tag>();
}

TEST(CreateTest, Integral) {
  testCreate<bool_t>();
  testCreate<byte_t>();
  testCreate<i16_t>();
  testCreate<i32_t>();
  testCreate<i64_t>();
  // testset does not include structured with Enum.
  testCreateWithTag<enum_t<BaseTypeEnum>>();
}

TEST(CreateTest, FloatingPoint) {
  testCreate<float_t>();
  testCreate<double_t>();
}

TEST(CreateTest, String) {
  testCreate<string_t>();
  testCreate<binary_t>();
}

TEST(CreateTest, Container) {
  testCreate<list<string_t>>();
  testCreate<set<string_t>>();
  testCreate<map<string_t, string_t>>();
}

template <typename Obj, typename Ord>
void testEnsure(Obj obj, Ord ord) {
  auto field = op::get<>(ord, obj);
  EXPECT_EQ(op::ensure<>(ord, obj), 0);
  EXPECT_EQ(field, 0);
  field = 2;
  EXPECT_EQ(op::ensure<>(ord, obj), 2);
  EXPECT_EQ(field, 2);
}

template <typename Obj, typename Ord>
void testEnsurePtr(Obj obj, Ord ord) {
  auto& field = op::get<>(ord, obj);
  EXPECT_EQ(op::ensure<>(ord, obj), 0);
  EXPECT_EQ(*field, 0);
  if constexpr (thrift::detail::is_unique_ptr_v<
                    std::remove_reference_t<decltype(field)>>) {
    field = std::make_unique<int32_t>(2);
  } else {
    field = std::make_shared<int32_t>(2);
  }
  EXPECT_EQ(op::ensure<>(ord, obj), 2);
  EXPECT_EQ(*field, 2);
}

TEST(EnsureTest, FieldRef) {
  FieldRefStruct obj;
  op::for_each_ordinal<FieldRefStruct>([&](auto ord) { testEnsure(obj, ord); });
}

TEST(EnsureTest, SmartPointer) {
  SmartPointerStruct obj;
  op::for_each_ordinal<SmartPointerStruct>(
      [&](auto ord) { testEnsurePtr(obj, ord); });
}

TEST(EnsureTest, IsAbsentAndEnsureValue) {
  using detail::ensureValue;
  using detail::isAbsent;
  {
    auto obj = op::create<struct_t<testset::struct_with<i32_t>>>();
    EXPECT_FALSE(isAbsent(obj.field_1()));
    ensureValue(obj.field_1()) = 1;
    EXPECT_FALSE(isAbsent(obj.field_1()));
  }
  {
    auto obj = op::create<struct_t<
        testset::struct_with<i32_t, testset::FieldModifier::Optional>>>();
    EXPECT_TRUE(isAbsent(obj.field_1()));
    ensureValue(obj.field_1()) = 1;
    EXPECT_FALSE(isAbsent(obj.field_1()));
  }
  {
    auto obj = op::create<struct_t<
        testset::struct_with<i32_t, testset::FieldModifier::Required>>>();
    EXPECT_FALSE(isAbsent(obj.field_1()));
    ensureValue(obj.field_1()) = 1;
    EXPECT_FALSE(isAbsent(obj.field_1()));
  }
  {
    auto obj = op::create<
        struct_t<testset::struct_with<i32_t, testset::FieldModifier::Terse>>>();
    EXPECT_FALSE(isAbsent(obj.field_1()));
    ensureValue(obj.field_1()) = 1;
    EXPECT_FALSE(isAbsent(obj.field_1()));
  }
}

} // namespace
} // namespace apache::thrift::op
