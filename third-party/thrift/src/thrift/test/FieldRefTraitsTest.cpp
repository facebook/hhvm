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

static_assert(is_field_ref_v<Unqualified>);
static_assert(!is_field_ref_v<Optional>);
static_assert(!is_field_ref_v<Required>);
static_assert(!is_field_ref_v<Box>);
static_assert(!is_field_ref_v<Union>);
static_assert(!is_field_ref_v<Unique>);
static_assert(!is_field_ref_v<Shared>);

static_assert(!is_optional_field_ref_v<Unqualified>);
static_assert(is_optional_field_ref_v<Optional>);
static_assert(!is_optional_field_ref_v<Required>);
static_assert(!is_optional_field_ref_v<Box>);
static_assert(!is_optional_field_ref_v<Union>);
static_assert(!is_optional_field_ref_v<Unique>);
static_assert(!is_optional_field_ref_v<Shared>);

static_assert(!is_required_field_ref_v<Unqualified>);
static_assert(!is_required_field_ref_v<Optional>);
static_assert(is_required_field_ref_v<Required>);
static_assert(!is_required_field_ref_v<Box>);
static_assert(!is_required_field_ref_v<Union>);
static_assert(!is_required_field_ref_v<Unique>);
static_assert(!is_required_field_ref_v<Shared>);

static_assert(!is_optional_boxed_field_ref_v<Unqualified>);
static_assert(!is_optional_boxed_field_ref_v<Optional>);
static_assert(!is_optional_boxed_field_ref_v<Required>);
static_assert(is_optional_boxed_field_ref_v<Box>);
static_assert(!is_optional_boxed_field_ref_v<Union>);
static_assert(!is_optional_boxed_field_ref_v<Unique>);
static_assert(!is_optional_boxed_field_ref_v<Shared>);

static_assert(!is_union_field_ref_v<Unqualified>);
static_assert(!is_union_field_ref_v<Optional>);
static_assert(!is_union_field_ref_v<Required>);
static_assert(!is_union_field_ref_v<Box>);
static_assert(is_union_field_ref_v<Union>);
static_assert(!is_union_field_ref_v<Unique>);
static_assert(!is_union_field_ref_v<Shared>);

static_assert(!is_unique_ptr_v<Unqualified>);
static_assert(!is_unique_ptr_v<Optional>);
static_assert(!is_unique_ptr_v<Required>);
static_assert(!is_unique_ptr_v<Box>);
static_assert(!is_unique_ptr_v<Union>);
static_assert(is_unique_ptr_v<Unique>);
static_assert(!is_unique_ptr_v<Shared>);

static_assert(!is_shared_ptr_v<Unqualified>);
static_assert(!is_shared_ptr_v<Optional>);
static_assert(!is_shared_ptr_v<Required>);
static_assert(!is_shared_ptr_v<Box>);
static_assert(!is_shared_ptr_v<Union>);
static_assert(!is_shared_ptr_v<Unique>);
static_assert(is_shared_ptr_v<Shared>);

static_assert(!is_shared_or_unique_ptr_v<Unqualified>);
static_assert(!is_shared_or_unique_ptr_v<Optional>);
static_assert(!is_shared_or_unique_ptr_v<Required>);
static_assert(!is_shared_or_unique_ptr_v<Box>);
static_assert(!is_shared_or_unique_ptr_v<Union>);
static_assert(is_shared_or_unique_ptr_v<Unique>);
static_assert(is_shared_or_unique_ptr_v<Shared>);

} // namespace
} // namespace apache::thrift::test::testset
