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

namespace apache {
namespace thrift {
namespace op {
namespace detail {

// Adapter for all base types.
using AssignPatchAdapter = TemplateInlineAdapter<AssignPatch>;
using BoolPatchAdapter = TemplateInlineAdapter<BoolPatch>;
using NumberPatchAdapter = TemplateInlineAdapter<NumberPatch>;
using StringPatchAdapter = TemplateInlineAdapter<StringPatch>;
using BinaryPatchAdapter = TemplateInlineAdapter<BinaryPatch>;

// Adapters for structred types.
using FieldPatchAdapter = TemplateInlineAdapter<FieldPatch>;
using StructPatchAdapter = TemplateInlineAdapter<StructPatch>;
using UnionPatchAdapter = TemplateInlineAdapter<UnionPatch>;

// Adapters for containers.
using ListPatchAdapter = TemplateInlineAdapter<ListPatch>;
using SetPatchAdapter = TemplateInlineAdapter<SetPatch>;
using MapPatchAdapter = TemplateInlineAdapter<MapPatch>;

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
