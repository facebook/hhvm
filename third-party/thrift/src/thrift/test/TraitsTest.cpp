/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <thrift/test/gen-cpp2/traits_types.h>

namespace apache::thrift {
static_assert(is_thrift_class_v<struct0>);
static_assert(is_thrift_class_v<struct1>);
static_assert(is_thrift_class_v<struct2>);
static_assert(is_thrift_class_v<union0>);
static_assert(is_thrift_class_v<union1>);
static_assert(is_thrift_class_v<union2>);
static_assert(is_thrift_class_v<exception0>);
static_assert(is_thrift_class_v<exception1>);
static_assert(is_thrift_class_v<exception2>);
static_assert(!is_thrift_class_v<enum0>);
static_assert(!is_thrift_class_v<enum1>);
static_assert(!is_thrift_class_v<enum2>);

static_assert(is_thrift_struct_v<struct0>);
static_assert(is_thrift_struct_v<struct1>);
static_assert(is_thrift_struct_v<struct2>);
static_assert(!is_thrift_struct_v<union0>);
static_assert(!is_thrift_struct_v<union1>);
static_assert(!is_thrift_struct_v<union2>);
static_assert(!is_thrift_struct_v<exception0>);
static_assert(!is_thrift_struct_v<exception1>);
static_assert(!is_thrift_struct_v<exception2>);
static_assert(!is_thrift_struct_v<enum0>);
static_assert(!is_thrift_struct_v<enum1>);
static_assert(!is_thrift_struct_v<enum2>);

static_assert(!is_thrift_union_v<struct0>);
static_assert(!is_thrift_union_v<struct1>);
static_assert(!is_thrift_union_v<struct2>);
static_assert(is_thrift_union_v<union0>);
static_assert(is_thrift_union_v<union1>);
static_assert(is_thrift_union_v<union2>);
static_assert(!is_thrift_union_v<exception0>);
static_assert(!is_thrift_union_v<exception1>);
static_assert(!is_thrift_union_v<exception2>);
static_assert(!is_thrift_union_v<enum0>);
static_assert(!is_thrift_union_v<enum1>);
static_assert(!is_thrift_union_v<enum2>);

static_assert(!is_thrift_exception_v<struct0>);
static_assert(!is_thrift_exception_v<struct1>);
static_assert(!is_thrift_exception_v<struct2>);
static_assert(!is_thrift_exception_v<union0>);
static_assert(!is_thrift_exception_v<union1>);
static_assert(!is_thrift_exception_v<union2>);
static_assert(is_thrift_exception_v<exception0>);
static_assert(is_thrift_exception_v<exception1>);
static_assert(is_thrift_exception_v<exception2>);
static_assert(!is_thrift_exception_v<enum0>);
static_assert(!is_thrift_exception_v<enum1>);
static_assert(!is_thrift_exception_v<enum2>);
} // namespace apache::thrift
