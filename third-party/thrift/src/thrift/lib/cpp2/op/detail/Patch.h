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

#include <thrift/lib/cpp2/op/detail/BasePatch.h>
#include <thrift/lib/cpp2/op/detail/ContainerPatch.h>
#include <thrift/lib/cpp2/op/detail/StructPatch.h>
#include <thrift/lib/cpp2/op/detail/ValuePatch.h>

namespace apache {
namespace thrift {
namespace op {
namespace detail {

template <template <typename> class PatchType>
struct PatchAdapter {
  template <typename Patch>
  static decltype(auto) toThrift(Patch&& value) {
    return std::forward<Patch>(value).toThrift();
  }

  template <typename Patch>
  static PatchType<Patch> fromThrift(Patch&& value) {
    return PatchType<Patch>{std::forward<Patch>(value)};
  }
};

// Adapter for all base types.
using AssignPatchAdapter = PatchAdapter<AssignPatch>;
using BoolPatchAdapter = PatchAdapter<BoolPatch>;
using NumberPatchAdapter = PatchAdapter<NumberPatch>;
using StringPatchAdapter = PatchAdapter<StringPatch>;
using BinaryPatchAdapter = PatchAdapter<BinaryPatch>;

// Adapters for structred types.
using FieldPatchAdapter = PatchAdapter<FieldPatch>;
using StructPatchAdapter = PatchAdapter<StructPatch>;
using UnionPatchAdapter = PatchAdapter<UnionPatch>;

// Adapters for containers.
using ListPatchAdapter = PatchAdapter<ListPatch>;
using SetPatchAdapter = PatchAdapter<SetPatch>;
using MapPatchAdapter = PatchAdapter<MapPatch>;

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
