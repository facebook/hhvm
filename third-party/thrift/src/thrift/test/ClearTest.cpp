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

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/test/gen-cpp2/clear_types.h>

namespace apache::thrift::test {
namespace {

template <typename T>
void checkIsDefault(const T& obj) {
  EXPECT_EQ(*obj.bool_field(), false);
  EXPECT_EQ(*obj.byte_field(), 0);
  EXPECT_EQ(*obj.short_field(), 0);
  EXPECT_EQ(*obj.int_field(), 0);
  EXPECT_EQ(*obj.long_field(), 0);
  EXPECT_EQ(*obj.float_field(), 0.0);
  EXPECT_EQ(*obj.double_field(), 0.0);
  EXPECT_EQ(*obj.string_field(), "");
  EXPECT_EQ(*obj.binary_field(), "");
  EXPECT_EQ(*obj.enum_field(), MyEnum::ME0);
  EXPECT_TRUE(obj.list_field()->empty());
  EXPECT_TRUE(obj.set_field()->empty());
  EXPECT_TRUE(obj.map_field()->empty());
  EXPECT_EQ(*obj.struct_field()->int_field(), 0);
}

TEST(ClearTest, Struct_NoDefaults) {
  StructWithNoDefaultStruct obj;
  checkIsDefault(obj);
  const auto& def = op::getDefault<StructWithNoDefaultStruct>();
  checkIsDefault(def);
  const auto& idef = op::getIntrinsicDefault<StructWithNoDefaultStruct>();
  checkIsDefault(idef);
}

TEST(ClearTest, Struct_Defaults) {
  StructWithDefaultStruct obj;
  const auto& def = op::getDefault<StructWithDefaultStruct>();
  EXPECT_EQ(obj, def);

  apache::thrift::clear(obj);
  EXPECT_NE(obj, def);
  checkIsDefault(obj);

  const auto& idef = op::getIntrinsicDefault<StructWithDefaultStruct>();
  EXPECT_EQ(obj, idef);
  checkIsDefault(idef);
}

TEST(ClearTest, RefFields) {
  StructWithNoDefaultStruct obj;
  obj.ref_field()->int_field() = 42;

  auto obj2 = obj;
  EXPECT_EQ(*obj.ref_field()->int_field(), 42);
  EXPECT_EQ(*obj2.ref_field()->int_field(), 42);

  apache::thrift::clear(obj);
  EXPECT_EQ(*obj.ref_field()->int_field(), 0);
  EXPECT_EQ(*obj2.ref_field()->int_field(), 42);
}

TEST(AdaptTest, ThriftClearTestStruct) {
  static_assert(!folly::is_detected_v<
                adapt_detail::ClearType,
                AdapterWithContext,
                AdaptedWithContext<int64_t, ThriftClearTestStruct, 1>>);
  static_assert(!folly::is_detected_v<
                adapt_detail::IsEmptyType,
                AdapterWithContext,
                AdaptedWithContext<int64_t, ThriftClearTestStruct, 1>>);

  auto obj = ThriftClearTestStruct();

  obj.data()->value = 42;
  obj.meta() = "foo";

  EXPECT_EQ(obj.data()->value, 42);
  EXPECT_EQ(obj.data()->fieldId, 1);
  EXPECT_EQ(*obj.data()->meta, "foo");

  apache::thrift::clear(obj);

  EXPECT_EQ(obj.data()->value, 0);
  EXPECT_EQ(obj.data()->fieldId, 1);
  EXPECT_EQ(*obj.data()->meta, "");
}

TEST(AdaptTest, AdapterClearTestStruct) {
  static_assert(folly::is_detected_v<
                adapt_detail::ClearType,
                AdapterWithContextOptimized,
                AdaptedWithContext<int64_t, AdapterClearTestStruct, 1>>);
  static_assert(folly::is_detected_v<
                adapt_detail::IsEmptyType,
                AdapterWithContextOptimized,
                AdaptedWithContext<int64_t, AdapterClearTestStruct, 1>>);

  auto obj = AdapterClearTestStruct();

  obj.data()->value = 42;
  obj.meta() = "foo";

  EXPECT_EQ(obj.data()->value, 42);
  EXPECT_EQ(obj.data()->fieldId, 1);
  EXPECT_EQ(*obj.data()->meta, "foo");

  apache::thrift::clear(obj);

  EXPECT_EQ(obj.data()->value, 0);
  EXPECT_EQ(obj.data()->fieldId, 1);
  EXPECT_EQ(*obj.data()->meta, "");
}

TEST(AdaptTest, AdapterClearTestStructOpClear) {
  static_assert(folly::is_detected_v<
                adapt_detail::ClearType,
                AdapterWithContextOptimized,
                AdaptedWithContext<int64_t, AdapterClearTestStruct, 1>>);
  using namespace apache::thrift;
  using field_type_tag = op::get_field_tag<AdapterClearTestStruct, ident::data>;

  auto obj = AdapterClearTestStruct();

  obj.data()->value = 42;
  obj.meta() = "foo";

  apache::thrift::op::clear_field<field_type_tag>(obj.data(), obj);

  EXPECT_EQ(obj.data()->value, 0);
  EXPECT_EQ(obj.data()->fieldId, 1);
  EXPECT_EQ(*obj.data()->meta, "foo");

  // Always return false from AdapterWithContextOptimized::isEmpty.
  EXPECT_FALSE(apache::thrift::op::isEmpty<field_type_tag>(obj.data().value()));
}

// TODO: move this to public header
template <typename Struct>
void clear_struct(Struct& s) {
  op::for_each_ordinal<Struct>([&](auto id) { op::clear<>(id, s); });
}

TEST(ClearStructTest, StructWithDefaultStruct) {
  StructWithDefaultStruct obj;
  clear_struct(obj);
  checkIsDefault(obj);
}

TEST(OpClearTest, StructWithDefaultStruct) {
  {
    StructWithDefaultStruct obj;
    clear_struct(obj);
    checkIsDefault(obj);
  }

  {
    auto obj = AdapterClearTestStruct();
    obj.data()->value = 42;
    obj.meta() = "foo";
    clear_struct(obj);
    EXPECT_EQ(obj.data()->value, 0);
    EXPECT_EQ(obj.data()->fieldId, 1);
    EXPECT_EQ(*obj.data()->meta, "");
  }
}

TEST(OpClearTest, OptionalField) {
  OptionalField obj;
  obj.optional_i32() = 10;
  obj.boxed_i32() = 20;
  obj.shared_i32() = std::make_shared<const std::int32_t>(30);
  obj.unique_i32() = std::make_unique<std::int32_t>(40);
  clear_struct(obj);
  EXPECT_FALSE(obj.optional_i32());
  EXPECT_FALSE(obj.boxed_i32());
  EXPECT_FALSE(obj.shared_i32());
  EXPECT_FALSE(obj.unique_i32());
}

TEST(OpClearTest, TerseWriteField) {
  TerseWriteField obj;
  obj.field_1() = 1;
  EXPECT_EQ(obj.field_1(), 1);
  clear_struct(obj);
  EXPECT_EQ(obj.field_1(), 0);
}

} // namespace
} // namespace apache::thrift::test
