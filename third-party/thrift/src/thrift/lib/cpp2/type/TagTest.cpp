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

#include <thrift/lib/cpp2/type/Tag.h>

#include <list>
#include <map>
#include <unordered_set>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/type/Testing.h>

namespace apache::thrift::type {

all_c testOverload(all_c);
// Intentionally omitted.
// primitive_c testOverload(primitive_c);
integral_c testOverload(integral_c);
floating_point_c testOverload(floating_point_c);
enum_c testOverload(enum_c);
string_c testOverload(string_c);
structured_c testOverload(structured_c);
struct_except_c testOverload(struct_except_c);
struct_c testOverload(struct_c);
// Intentionally omitted.
// union_c testOverload(union_c);
// Intentionally omitted.
// exception_c testOverload(exception_c);
container_c testOverload(container_c);
list_c testOverload(list_c);
// Intentionally omitted.
// set_c testOverload(set_c);
map_c testOverload(map_c);
service_c testOverload(service_c);

void_t testOverload(void_t);

// Intentionally omitted.
// bool_t testOverload(bool_t);
byte_t testOverload(byte_t);
// Intentionally omitted.
// i16_t testOverload(i16_t);
i32_t testOverload(i32_t);
i64_t testOverload(i64_t);
// Intentionally omitted.
// float_t testOverload(float_t);
double_t testOverload(double_t);
string_t testOverload(string_t);
// binary_t testOverload(binary_t);

struct General {};
struct Specialized {};
enum_t<Specialized> testOverload(enum_t<Specialized>);
struct_t<Specialized> testOverload(struct_t<Specialized>);
union_t<Specialized> testOverload(union_t<Specialized>);
exception_t<Specialized> testOverload(exception_t<Specialized>);
service_t<Specialized> testOverload(service_t<Specialized>);

template <typename ListT>
cpp_type<list<Specialized>, ListT> testOverload(
    cpp_type<list<Specialized>, ListT>);
cpp_type<set<Specialized>, std::unordered_set<int>> testOverload(
    cpp_type<set<Specialized>, std::unordered_set<int>>);
map<Specialized, Specialized> testOverload(map<Specialized, Specialized>);

namespace {
using test::same_tag;
using test::TestAdapter;

// Test that type tags can be used to find an overload in ~constant time
// by the compiler.
static_assert(same_tag<all_c, decltype(testOverload(primitive_c{}))>);
static_assert(same_tag<void_t, decltype(testOverload(void_t{}))>);
static_assert(same_tag<integral_c, decltype(testOverload(bool_t{}))>);
static_assert(same_tag<byte_t, decltype(testOverload(byte_t{}))>);
static_assert(same_tag<integral_c, decltype(testOverload(i16_t{}))>);
static_assert(same_tag<i32_t, decltype(testOverload(i32_t{}))>);
static_assert(same_tag<i64_t, decltype(testOverload(i64_t{}))>);
static_assert(same_tag<floating_point_c, decltype(testOverload(float_t{}))>);
static_assert(same_tag<double_t, decltype(testOverload(double_t{}))>);
static_assert(same_tag<string_t, decltype(testOverload(string_t{}))>);
static_assert(same_tag<string_c, decltype(testOverload(binary_t{}))>);

static_assert(same_tag<enum_c, decltype(testOverload(enum_c{}))>);
static_assert(same_tag<enum_c, decltype(testOverload(enum_t<General>{}))>);
static_assert(same_tag<
              enum_t<Specialized>,
              decltype(testOverload(enum_t<Specialized>{}))>);

static_assert(same_tag<struct_c, decltype(testOverload(struct_c{}))>);
static_assert(same_tag<struct_c, decltype(testOverload(struct_t<General>{}))>);
static_assert(same_tag<
              struct_t<Specialized>,
              decltype(testOverload(struct_t<Specialized>{}))>);

static_assert(same_tag<structured_c, decltype(testOverload(union_c{}))>);
static_assert(
    same_tag<structured_c, decltype(testOverload(union_t<General>{}))>);
static_assert(same_tag<
              union_t<Specialized>,
              decltype(testOverload(union_t<Specialized>{}))>);

static_assert(same_tag<struct_except_c, decltype(testOverload(exception_c{}))>);
static_assert(
    same_tag<struct_except_c, decltype(testOverload(exception_t<General>{}))>);
static_assert(same_tag<
              exception_t<Specialized>,
              decltype(testOverload(exception_t<Specialized>{}))>);

static_assert(same_tag<list_c, decltype(testOverload(list_c{}))>);
static_assert(same_tag<list_c, decltype(testOverload(list<void_t>{}))>);
static_assert(same_tag<
              cpp_type<list<Specialized>, std::list<Specialized>>,
              decltype(testOverload(
                  cpp_type<list<Specialized>, std::list<Specialized>>{}))>);
static_assert(same_tag<
              cpp_type<list<Specialized>, std::list<Specialized>>,
              decltype(testOverload(
                  cpp_type<list<Specialized>, std::list<Specialized>>{}))>);

static_assert(same_tag<container_c, decltype(testOverload(set_c{}))>);
static_assert(same_tag<container_c, decltype(testOverload(set<void_t>{}))>);
static_assert(
    same_tag<container_c, decltype(testOverload(set<Specialized>{}))>);
static_assert(same_tag<
              cpp_type<set<Specialized>, std::unordered_set<int>>,
              decltype(testOverload(
                  cpp_type<set<Specialized>, std::unordered_set<int>>{}))>);

static_assert(same_tag<map_c, decltype(testOverload(map_c{}))>);
static_assert(same_tag<map_c, decltype(testOverload(map<void_t, void_t>{}))>);
static_assert(
    same_tag<map_c, decltype(testOverload(map<Specialized, void_t>{}))>);
static_assert(
    same_tag<map_c, decltype(testOverload(map<void_t, Specialized>{}))>);
static_assert(same_tag<
              map<Specialized, Specialized>,
              decltype(testOverload(map<Specialized, Specialized>{}))>);

// Adapted types are convertable to the underlying tag.
static_assert(
    same_tag<void_t, decltype(testOverload(adapted<TestAdapter, void_t>{}))>);
static_assert(same_tag<
              list_c,
              decltype(testOverload(adapted<TestAdapter, list<enum_c>>{}))>);
// CppType types are convertable to the underlying tag.
static_assert(
    same_tag<void_t, decltype(testOverload(cpp_type<TestAdapter, void_t>{}))>);
static_assert(same_tag<
              list_c,
              decltype(testOverload(cpp_type<TestAdapter, list<enum_c>>{}))>);

static_assert(same_tag<service_c, decltype(testOverload(service_c{}))>);
static_assert(
    same_tag<service_c, decltype(testOverload(service_t<General>{}))>);
static_assert(same_tag<
              service_t<Specialized>,
              decltype(testOverload(service_t<Specialized>{}))>);

} // namespace
} // namespace apache::thrift::type
