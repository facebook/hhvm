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

#pragma once

#include <forward_list>
#include <type_traits>
#include <utility>

#include <thrift/lib/cpp2/Adapt.h>
#include <thrift/lib/cpp2/Adapter.h>
#include <thrift/lib/cpp2/op/detail/BasePatch.h>
#include <thrift/lib/cpp2/op/detail/ContainerPatch.h>
#include <thrift/lib/cpp2/op/detail/StructPatch.h>
#include <thrift/lib/cpp2/op/detail/ValuePatch.h>
#include <thrift/lib/thrift/gen-cpp2/patch_op_types.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

// Adapter for all base types.
template <typename T>
using AssignPatchAdapter = InlineAdapter<AssignPatch<T>>;
template <typename T>
using BoolPatchAdapter = InlineAdapter<BoolPatch<T>>;
template <typename T>
using NumberPatchAdapter = InlineAdapter<NumberPatch<T>>;
template <typename T>
using StringPatchAdapter = InlineAdapter<StringPatch<T>>;
template <typename T>
using BinaryPatchAdapter = InlineAdapter<BinaryPatch<T>>;

// Adapters for structred types.
template <typename T>
using FieldPatchAdapter = InlineAdapter<FieldPatch<T>>;
template <typename T>
using StructPatchAdapter = InlineAdapter<StructPatch<T>>;
template <typename T>
using UnionPatchAdapter = InlineAdapter<UnionPatch<T>>;

// Adapters for containers.
template <typename T>
using ListPatchAdapter = InlineAdapter<ListPatch<T>>;
template <typename T>
using SetPatchAdapter = InlineAdapter<SetPatch<T>>;
template <typename T>
using MapPatchAdapter = InlineAdapter<MapPatch<T>>;

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
