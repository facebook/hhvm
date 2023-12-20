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

#include <thrift/lib/cpp2/FieldRefTraits.h>
#include <thrift/test/gen-cpp2/References_types.h>
#include <thrift/test/terse_write/gen-cpp2/terse_write_types.h>
#include <thrift/test/testset/gen-cpp2/testset_types.h>

using namespace ::apache::thrift::detail;

namespace apache::thrift::test::testset {
namespace {

using Unqualified = decltype(struct_list_i32{}.field_1());
using Optional = decltype(struct_optional_list_i32{}.field_1());
using Required = decltype(struct_required_list_i32{}.field_1());
using Box = decltype(struct_optional_list_i32_box{}.field_1());
using Union = decltype(union_list_i32{}.field_1_ref());
using Unique = std::remove_reference_t<decltype(
    struct_optional_list_i32_cpp_ref{}.field_1_ref())>;
using Shared = std::remove_reference_t<decltype(
    struct_optional_list_i32_shared_cpp_ref{}.field_1_ref())>;
using InternBox = std::remove_reference_t<decltype(
    cpp2::StructuredAnnotation{}.intern_box_field())>;
using TerseInternBox = std::remove_reference_t<decltype(
    terse_write::CppRefTerseStruct{}.intern_boxed_field())>;
using Terse =
    std::remove_reference_t<decltype(terse_write::MyStruct{}.field1())>;

static_assert(is_field_ref_v<Unqualified>);
static_assert(!is_field_ref_v<Optional>);
static_assert(!is_field_ref_v<Required>);
static_assert(!is_field_ref_v<Box>);
static_assert(!is_field_ref_v<Union>);
static_assert(!is_field_ref_v<Unique>);
static_assert(!is_field_ref_v<Shared>);
static_assert(!is_field_ref_v<InternBox>);
static_assert(!is_field_ref_v<TerseInternBox>);
static_assert(!is_field_ref_v<Terse>);

static_assert(!is_optional_field_ref_v<Unqualified>);
static_assert(is_optional_field_ref_v<Optional>);
static_assert(!is_optional_field_ref_v<Required>);
static_assert(!is_optional_field_ref_v<Box>);
static_assert(!is_optional_field_ref_v<Union>);
static_assert(!is_optional_field_ref_v<Unique>);
static_assert(!is_optional_field_ref_v<Shared>);
static_assert(!is_optional_field_ref_v<InternBox>);
static_assert(!is_optional_field_ref_v<TerseInternBox>);
static_assert(!is_optional_field_ref_v<Terse>);

static_assert(!is_required_field_ref_v<Unqualified>);
static_assert(!is_required_field_ref_v<Optional>);
static_assert(is_required_field_ref_v<Required>);
static_assert(!is_required_field_ref_v<Box>);
static_assert(!is_required_field_ref_v<Union>);
static_assert(!is_required_field_ref_v<Unique>);
static_assert(!is_required_field_ref_v<Shared>);
static_assert(!is_required_field_ref_v<InternBox>);
static_assert(!is_required_field_ref_v<TerseInternBox>);
static_assert(!is_required_field_ref_v<Terse>);

static_assert(!is_optional_boxed_field_ref_v<Unqualified>);
static_assert(!is_optional_boxed_field_ref_v<Optional>);
static_assert(!is_optional_boxed_field_ref_v<Required>);
static_assert(is_optional_boxed_field_ref_v<Box>);
static_assert(!is_optional_boxed_field_ref_v<Union>);
static_assert(!is_optional_boxed_field_ref_v<Unique>);
static_assert(!is_optional_boxed_field_ref_v<Shared>);
static_assert(!is_optional_boxed_field_ref_v<InternBox>);
static_assert(!is_optional_boxed_field_ref_v<TerseInternBox>);
static_assert(!is_optional_boxed_field_ref_v<Terse>);

static_assert(!is_union_field_ref_v<Unqualified>);
static_assert(!is_union_field_ref_v<Optional>);
static_assert(!is_union_field_ref_v<Required>);
static_assert(!is_union_field_ref_v<Box>);
static_assert(is_union_field_ref_v<Union>);
static_assert(!is_union_field_ref_v<Unique>);
static_assert(!is_union_field_ref_v<Shared>);
static_assert(!is_union_field_ref_v<InternBox>);
static_assert(!is_union_field_ref_v<TerseInternBox>);
static_assert(!is_union_field_ref_v<Terse>);

static_assert(!is_intern_boxed_field_ref_v<Unqualified>);
static_assert(!is_intern_boxed_field_ref_v<Optional>);
static_assert(!is_intern_boxed_field_ref_v<Required>);
static_assert(!is_intern_boxed_field_ref_v<Box>);
static_assert(!is_intern_boxed_field_ref_v<Union>);
static_assert(!is_intern_boxed_field_ref_v<Unique>);
static_assert(!is_intern_boxed_field_ref_v<Shared>);
static_assert(is_intern_boxed_field_ref_v<InternBox>);
static_assert(!is_intern_boxed_field_ref_v<TerseInternBox>);
static_assert(!is_intern_boxed_field_ref_v<Terse>);

static_assert(!is_terse_intern_boxed_field_ref_v<Unqualified>);
static_assert(!is_terse_intern_boxed_field_ref_v<Optional>);
static_assert(!is_terse_intern_boxed_field_ref_v<Required>);
static_assert(!is_terse_intern_boxed_field_ref_v<Box>);
static_assert(!is_terse_intern_boxed_field_ref_v<Union>);
static_assert(!is_terse_intern_boxed_field_ref_v<Unique>);
static_assert(!is_terse_intern_boxed_field_ref_v<Shared>);
static_assert(!is_terse_intern_boxed_field_ref_v<InternBox>);
static_assert(is_terse_intern_boxed_field_ref_v<TerseInternBox>);
static_assert(!is_terse_intern_boxed_field_ref_v<Terse>);

static_assert(!is_terse_field_ref_v<Unqualified>);
static_assert(!is_terse_field_ref_v<Optional>);
static_assert(!is_terse_field_ref_v<Required>);
static_assert(!is_terse_field_ref_v<Box>);
static_assert(!is_terse_field_ref_v<Union>);
static_assert(!is_terse_field_ref_v<Unique>);
static_assert(!is_terse_field_ref_v<Shared>);
static_assert(!is_terse_field_ref_v<InternBox>);
static_assert(!is_terse_field_ref_v<TerseInternBox>);
static_assert(is_terse_field_ref_v<Terse>);

static_assert(!is_unique_ptr_v<Unqualified>);
static_assert(!is_unique_ptr_v<Optional>);
static_assert(!is_unique_ptr_v<Required>);
static_assert(!is_unique_ptr_v<Box>);
static_assert(!is_unique_ptr_v<Union>);
static_assert(is_unique_ptr_v<Unique>);
static_assert(!is_unique_ptr_v<Shared>);
static_assert(!is_unique_ptr_v<InternBox>);
static_assert(!is_unique_ptr_v<TerseInternBox>);
static_assert(!is_unique_ptr_v<Terse>);

static_assert(!is_shared_ptr_v<Unqualified>);
static_assert(!is_shared_ptr_v<Optional>);
static_assert(!is_shared_ptr_v<Required>);
static_assert(!is_shared_ptr_v<Box>);
static_assert(!is_shared_ptr_v<Union>);
static_assert(!is_shared_ptr_v<Unique>);
static_assert(is_shared_ptr_v<Shared>);
static_assert(!is_shared_ptr_v<InternBox>);
static_assert(!is_shared_ptr_v<TerseInternBox>);
static_assert(!is_shared_ptr_v<Terse>);

static_assert(!is_shared_or_unique_ptr_v<Unqualified>);
static_assert(!is_shared_or_unique_ptr_v<Optional>);
static_assert(!is_shared_or_unique_ptr_v<Required>);
static_assert(!is_shared_or_unique_ptr_v<Box>);
static_assert(!is_shared_or_unique_ptr_v<Union>);
static_assert(is_shared_or_unique_ptr_v<Unique>);
static_assert(is_shared_or_unique_ptr_v<Shared>);
static_assert(!is_shared_or_unique_ptr_v<InternBox>);
static_assert(!is_shared_or_unique_ptr_v<TerseInternBox>);
static_assert(!is_shared_or_unique_ptr_v<Terse>);

// Test for type::is_optional_field_v
// non-optional
static_assert(!type::is_optional_field_v<struct_list_i32, type::field_id<1>>);
static_assert(
    !type::is_optional_field_v<struct_required_list_i32, type::field_id<1>>);
// Terse.
static_assert(
    !type::is_optional_field_v<terse_write::MyStruct, type::field_id<1>>);
// Intern Box.
static_assert(
    !type::is_optional_field_v<cpp2::StructuredAnnotation, type::field_id<4>>);
// Terse Intern Box.
static_assert(!type::is_optional_field_v<
              terse_write::CppRefTerseStruct,
              type::field_id<4>>);

// optional
static_assert(
    type::is_optional_field_v<struct_optional_list_i32, type::field_id<1>>);
static_assert(type::is_optional_field_v<union_list_i32, type::field_id<1>>);
static_assert(
    type::is_optional_field_v<struct_optional_list_i32_box, type::field_id<1>>);

} // namespace
} // namespace apache::thrift::test::testset
