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

#include <thrift/lib/cpp2/type/ThriftType.h>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/Testing.h>

namespace apache::thrift::type {
namespace {
using test::TestAdapter;

// Evil types trying to inject themselves into the system!
struct evil_c : enum_c {};
struct evil_t : i64_t {};

// is_concrete_v static asserts.
static_assert(!is_concrete_v<int>);
static_assert(!is_concrete_v<evil_c>);
static_assert(!is_concrete_v<evil_t>);

static_assert(!is_concrete_v<all_c>);
static_assert(!is_concrete_v<primitive_c>);
static_assert(!is_concrete_v<number_c>);
static_assert(!is_concrete_v<integral_c>);
static_assert(!is_concrete_v<floating_point_c>);
static_assert(!is_concrete_v<enum_c>);
static_assert(!is_concrete_v<string_c>);
static_assert(!is_concrete_v<structured_c>);
static_assert(!is_concrete_v<struct_except_c>);
static_assert(!is_concrete_v<struct_c>);
static_assert(!is_concrete_v<union_c>);
static_assert(!is_concrete_v<exception_c>);
static_assert(!is_concrete_v<container_c>);
static_assert(!is_concrete_v<list_c>);
static_assert(!is_concrete_v<set_c>);
static_assert(!is_concrete_v<map_c>);
static_assert(!is_concrete_v<service_c>);

static_assert(is_concrete_v<void_t>);
static_assert(is_concrete_v<bool_t>);
static_assert(is_concrete_v<byte_t>);
static_assert(is_concrete_v<i16_t>);
static_assert(is_concrete_v<i32_t>);
static_assert(is_concrete_v<i64_t>);
static_assert(is_concrete_v<float_t>);
static_assert(is_concrete_v<double_t>);
static_assert(is_concrete_v<string_t>);
static_assert(is_concrete_v<binary_t>);

static_assert(is_concrete_v<enum_t<int>>);
static_assert(is_concrete_v<struct_t<int>>);
static_assert(is_concrete_v<union_t<int>>);
static_assert(is_concrete_v<exception_t<int>>);
static_assert(is_concrete_v<service_t<int>>);

static_assert(!is_concrete_v<list<int>>);
static_assert(is_concrete_v<list<void_t>>);

static_assert(!is_concrete_v<set<int>>);
static_assert(is_concrete_v<set<void_t>>);

static_assert(!is_concrete_v<map<int, void_t>>);
static_assert(!is_concrete_v<map<int, int>>);
static_assert(!is_concrete_v<map<void_t, int>>);
static_assert(is_concrete_v<map<void_t, void_t>>);

static_assert(!is_concrete_v<adapted<int, int>>);
static_assert(is_concrete_v<adapted<int, void_t>>);
static_assert(is_concrete_v<list<adapted<int, void_t>>>);

static_assert(!is_concrete_v<field<adapted<int, int>, FieldContext<void, 0>>>);
static_assert(
    is_concrete_v<field<adapted<int, void_t>, FieldContext<void, 0>>>);
static_assert(
    is_concrete_v<field<list<adapted<int, void_t>>, FieldContext<void, 0>>>);

static_assert(!is_concrete_v<cpp_type<int, int>>);
static_assert(is_concrete_v<cpp_type<int, void_t>>);
static_assert(is_concrete_v<list<cpp_type<int, void_t>>>);

// is_thrift_type_tag_v static asserts.
static_assert(!is_thrift_type_tag_v<int>);
static_assert(!is_thrift_type_tag_v<evil_c>);
static_assert(!is_thrift_type_tag_v<evil_t>);

static_assert(is_thrift_type_tag_v<all_c>);
static_assert(is_thrift_type_tag_v<primitive_c>);
static_assert(is_thrift_type_tag_v<number_c>);
static_assert(is_thrift_type_tag_v<integral_c>);
static_assert(is_thrift_type_tag_v<floating_point_c>);
static_assert(is_thrift_type_tag_v<enum_c>);
static_assert(is_thrift_type_tag_v<string_c>);
static_assert(is_thrift_type_tag_v<structured_c>);
static_assert(is_thrift_type_tag_v<struct_except_c>);
static_assert(is_thrift_type_tag_v<struct_c>);
static_assert(is_thrift_type_tag_v<union_c>);
static_assert(is_thrift_type_tag_v<exception_c>);
static_assert(is_thrift_type_tag_v<container_c>);
static_assert(is_thrift_type_tag_v<list_c>);
static_assert(is_thrift_type_tag_v<set_c>);
static_assert(is_thrift_type_tag_v<map_c>);
static_assert(is_thrift_type_tag_v<service_c>);

static_assert(is_thrift_type_tag_v<void_t>);
static_assert(is_thrift_type_tag_v<bool_t>);
static_assert(is_thrift_type_tag_v<byte_t>);
static_assert(is_thrift_type_tag_v<i16_t>);
static_assert(is_thrift_type_tag_v<i32_t>);
static_assert(is_thrift_type_tag_v<i64_t>);
static_assert(is_thrift_type_tag_v<float_t>);
static_assert(is_thrift_type_tag_v<double_t>);
static_assert(is_thrift_type_tag_v<string_t>);
static_assert(is_thrift_type_tag_v<binary_t>);

static_assert(is_thrift_type_tag_v<enum_t<int>>);
static_assert(is_thrift_type_tag_v<struct_t<int>>);
static_assert(is_thrift_type_tag_v<union_t<int>>);
static_assert(is_thrift_type_tag_v<exception_t<int>>);
static_assert(is_thrift_type_tag_v<service_t<int>>);

static_assert(!is_thrift_type_tag_v<list<int>>);
static_assert(is_thrift_type_tag_v<list<void_t>>);

static_assert(!is_thrift_type_tag_v<set<int>>);
static_assert(is_thrift_type_tag_v<set<void_t>>);

static_assert(!is_thrift_type_tag_v<map<int, int>>);
static_assert(!is_thrift_type_tag_v<map<int, void_t>>);
static_assert(!is_thrift_type_tag_v<map<void_t, int>>);
static_assert(is_thrift_type_tag_v<map<void_t, void_t>>);

static_assert(!is_thrift_type_tag_v<adapted<int, int>>);
static_assert(is_thrift_type_tag_v<adapted<int, void_t>>);
static_assert(is_thrift_type_tag_v<list<adapted<int, void_t>>>);

static_assert(
    is_thrift_type_tag_v<field<adapted<int, int>, FieldContext<void, 0>>>);
static_assert(
    is_thrift_type_tag_v<field<adapted<int, void_t>, FieldContext<void, 0>>>);
static_assert(is_thrift_type_tag_v<
              field<list<adapted<int, void_t>>, FieldContext<void, 0>>>);

static_assert(!is_thrift_type_tag_v<cpp_type<int, int>>);
static_assert(is_thrift_type_tag_v<cpp_type<int, void_t>>);
static_assert(is_thrift_type_tag_v<list<cpp_type<int, void_t>>>);

// is_abstract_v static asserts.
static_assert(!is_abstract_v<int>);
static_assert(!is_abstract_v<evil_c>);
static_assert(!is_abstract_v<evil_t>);

static_assert(is_abstract_v<all_c>);
static_assert(is_abstract_v<primitive_c>);
static_assert(is_abstract_v<number_c>);
static_assert(is_abstract_v<integral_c>);
static_assert(is_abstract_v<floating_point_c>);
static_assert(is_abstract_v<enum_c>);
static_assert(is_abstract_v<string_c>);
static_assert(is_abstract_v<structured_c>);
static_assert(is_abstract_v<struct_except_c>);
static_assert(is_abstract_v<struct_c>);
static_assert(is_abstract_v<union_c>);
static_assert(is_abstract_v<exception_c>);
static_assert(is_abstract_v<container_c>);
static_assert(is_abstract_v<list_c>);
static_assert(is_abstract_v<set_c>);
static_assert(is_abstract_v<map_c>);
static_assert(is_abstract_v<service_c>);

static_assert(!is_abstract_v<void_t>);
static_assert(!is_abstract_v<bool_t>);
static_assert(!is_abstract_v<byte_t>);
static_assert(!is_abstract_v<i16_t>);
static_assert(!is_abstract_v<i32_t>);
static_assert(!is_abstract_v<i64_t>);
static_assert(!is_abstract_v<float_t>);
static_assert(!is_abstract_v<double_t>);
static_assert(!is_abstract_v<string_t>);
static_assert(!is_abstract_v<binary_t>);

static_assert(!is_abstract_v<enum_t<int>>);
static_assert(!is_abstract_v<struct_t<int>>);
static_assert(!is_abstract_v<union_t<int>>);
static_assert(!is_abstract_v<exception_t<int>>);
static_assert(!is_abstract_v<service_t<int>>);

static_assert(!is_abstract_v<list<int>>);
static_assert(!is_abstract_v<list<void_t>>);

static_assert(!is_abstract_v<set<int>>);
static_assert(!is_abstract_v<set<void_t>>);

static_assert(!is_abstract_v<map<int, void_t>>);
static_assert(!is_abstract_v<map<int, int>>);
static_assert(!is_abstract_v<map<void_t, int>>);
static_assert(!is_abstract_v<map<void_t, void_t>>);

static_assert(!is_abstract_v<adapted<int, int>>);
static_assert(!is_abstract_v<adapted<int, void_t>>);
static_assert(is_concrete_v<list<adapted<int, void_t>>>);

static_assert(!is_abstract_v<cpp_type<int, int>>);
static_assert(!is_abstract_v<cpp_type<int, void_t>>);
static_assert(is_concrete_v<list<cpp_type<int, void_t>>>);

// is_a_v static asserts.
static_assert(!is_a_v<void_t, integral_c>);
static_assert(!is_a_v<enum_c, integral_c>);
static_assert(is_a_v<bool_t, integral_c>);
static_assert(is_a_v<byte_t, integral_c>);
static_assert(is_a_v<i16_t, integral_c>);
static_assert(is_a_v<i32_t, integral_c>);
static_assert(is_a_v<i64_t, integral_c>);

static_assert(!is_a_v<void_t, floating_point_c>);
static_assert(is_a_v<float_t, floating_point_c>);
static_assert(is_a_v<double_t, floating_point_c>);

static_assert(!is_a_v<void_t, struct_except_c>);
static_assert(!is_a_v<union_c, struct_except_c>);
static_assert(is_a_v<struct_c, struct_except_c>);
static_assert(is_a_v<struct_t<int>, struct_except_c>);
static_assert(is_a_v<exception_c, struct_except_c>);
static_assert(is_a_v<exception_t<int>, struct_except_c>);
static_assert(is_a_v<service_t<int>, service_c>);

// Uncomment to produce expected compile time errors.
// static_assert(is_a_v<int, int>);
// static_assert(is_a_v<int, i32_t>);
// static_assert(is_a_v<i32_t, int>);
// static_assert(is_a_v<list<int>, list_c>);
// static_assert(is_a_v<adapted<int, int>, i32_t>);
// static_assert(is_a_v<cpp_type<int, int>, i32_t>);
static_assert(is_a_v<integral_c, integral_c>);
static_assert(is_a_v<i32_t, i32_t>);
static_assert(!is_a_v<integral_c, i32_t>);

static_assert(is_a_v<list<integral_c>, list_c>);
static_assert(is_a_v<list<i64_t>, list_c>);
static_assert(is_a_v<list<i64_t>, list<integral_c>>);
static_assert(is_a_v<list<i64_t>, list<i64_t>>);
static_assert(!is_a_v<list_c, list<integral_c>>);
static_assert(!is_a_v<list_c, list<i64_t>>);
static_assert(!is_a_v<list<integral_c>, list<i64_t>>);

static_assert(is_a_v<set<integral_c>, set_c>);
static_assert(is_a_v<set<i64_t>, set_c>);
static_assert(is_a_v<set<i64_t>, set<integral_c>>);
static_assert(is_a_v<set<i64_t>, set<i64_t>>);
static_assert(!is_a_v<set_c, set<integral_c>>);
static_assert(!is_a_v<set_c, set<i64_t>>);
static_assert(!is_a_v<set<integral_c>, set<i64_t>>);

static_assert(is_a_v<map<i64_t, i64_t>, map_c>);
static_assert(is_a_v<map<i64_t, i64_t>, map<integral_c, integral_c>>);
static_assert(is_a_v<map<i64_t, i64_t>, map<integral_c, i64_t>>);
static_assert(!is_a_v<map<i64_t, i64_t>, map<integral_c, i32_t>>);
static_assert(!is_a_v<map<i64_t, i64_t>, map<enum_c, i64_t>>);
static_assert(is_a_v<map<i64_t, i64_t>, map<i64_t, i64_t>>);
static_assert(!is_a_v<map_c, map<i64_t, i64_t>>);
static_assert(!is_a_v<map<integral_c, integral_c>, map<i64_t, i64_t>>);
static_assert(!is_a_v<map<integral_c, i64_t>, map<i64_t, i64_t>>);
static_assert(!is_a_v<map<integral_c, i32_t>, map<i64_t, i64_t>>);
static_assert(!is_a_v<map<enum_c, i64_t>, map<i64_t, i64_t>>);

static_assert(is_a_v<adapted<int, i32_t>, i32_t>);
static_assert(is_a_v<adapted<int, i32_t>, integral_c>);
static_assert(!is_a_v<i32_t, adapted<int, i32_t>>);
static_assert(!is_a_v<integral_c, adapted<int, i32_t>>);
static_assert(is_a_v<adapted<int, list<i32_t>>, list<integral_c>>);
static_assert(!is_a_v<list<i32_t>, adapted<int, list<integral_c>>>);
static_assert(
    is_a_v<adapted<int, list<i32_t>>, adapted<int, list<integral_c>>>);
static_assert(
    !is_a_v<adapted<int, list<integral_c>>, adapted<int, list<i32_t>>>);
static_assert(
    !is_a_v<adapted<int, list<i32_t>>, adapted<uint32_t, list<integral_c>>>);

static_assert(is_a_v<cpp_type<int, i32_t>, integral_c>);
static_assert(is_a_v<cpp_type<int, i32_t>, i32_t>);
static_assert(!is_a_v<i32_t, cpp_type<int, i32_t>>);
static_assert(!is_a_v<integral_c, cpp_type<int, i32_t>>);
static_assert(is_a_v<cpp_type<int, list<i32_t>>, list<integral_c>>);

static_assert(is_a_v<cpp_type<int, i32_t>, integral_c>);
static_assert(is_a_v<cpp_type<int, i32_t>, i32_t>);
static_assert(!is_a_v<cpp_type<int, i32_t>, i16_t>);
static_assert(is_a_v<cpp_type<int, i32_t>, cpp_type<int, integral_c>>);
static_assert(is_a_v<cpp_type<int, i32_t>, cpp_type<int, i32_t>>);
static_assert(!is_a_v<cpp_type<int, i32_t>, cpp_type<int, i16_t>>);
static_assert(!is_a_v<cpp_type<int, integral_c>, cpp_type<int, i32_t>>);

// Test concrete helpers.
template <ConcreteThriftTypeTag Tag>
constexpr bool isConcrete() {
  return true;
}

template <AbstractThriftTypeTag Tag>
constexpr bool isConcrete() {
  return false;
}

// Uncomment to produce expected compile time error.
// static_assert(isConcrete<int>());
static_assert(isConcrete<void_t>());
static_assert(!isConcrete<enum_c>());

// Containers are concrete if their type parameters are concrete.
static_assert(isConcrete<list<void_t>>());
static_assert(!isConcrete<list<enum_c>>());

static_assert(isConcrete<set<void_t>>());
static_assert(!isConcrete<set<enum_c>>());

static_assert(isConcrete<map<void_t, void_t>>());
static_assert(!isConcrete<map<enum_c, void_t>>());
static_assert(!isConcrete<map<void_t, enum_c>>());
static_assert(!isConcrete<map<enum_c, enum_c>>());

// An adapted type is concrete if it's type parameter is concrete.
static_assert(isConcrete<adapted<TestAdapter, void_t>>());
static_assert(!isConcrete<adapted<TestAdapter, enum_c>>());
static_assert(isConcrete<cpp_type<void, void_t>>());
static_assert(!isConcrete<cpp_type<int, enum_c>>());

// A field tag is concrete if it's type tag is concrete.
static_assert(isConcrete<field<void_t, FieldContext<void, 0>>>());
static_assert(
    isConcrete<field<adapted<TestAdapter, void_t>, FieldContext<void, 0>>>());
} // namespace
} // namespace apache::thrift::type
