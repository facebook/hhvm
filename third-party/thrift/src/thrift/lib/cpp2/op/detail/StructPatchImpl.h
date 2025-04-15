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

#include <thrift/lib/cpp2/op/detail/StructPatch.h>

namespace apache::thrift::op::detail {

template <FieldOrdinal Ord, class Patch>
void* typeErasedPatchImpl(Patch& patch) {
  using FieldPatchType = typename Patch::patch_type::underlying_type;
  if constexpr (folly::to_underlying(Ord) <= size_v<FieldPatchType>) {
    // Sanity check to ensure `patch<Ordinal>()` returns the correct type.
    // Otherwise it's a bug in the patch library.
    using Id = op::get_field_id<FieldPatchType, type::ordinal_tag<Ord>>;
    static_assert(std::is_same_v<
                  decltype(patch.template patchImpl<Id>()),
                  decltype(patch.template patch<Id>())>);
    return &patch.template patchImpl<Id>();
  } else {
    // This code path should never be executed since the caller should already
    // validate whether the field is patchable.
    // However, this is needed for explicit instantiation when we can't validate
    // patchable field size in codegen.
    folly::throw_exception<std::logic_error>(fmt::format(
        "Ordinal ({}) exceeds patchable field size ({})",
        folly::to_underlying(Ord),
        size_v<FieldPatchType>));
  }
}

} // namespace apache::thrift::op::detail
