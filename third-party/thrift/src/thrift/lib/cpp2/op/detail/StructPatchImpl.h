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
template <class Patch, class Derived>
template <class FieldId>
auto BaseEnsurePatch<Patch, Derived>::patchImpl() -> patch_type& {
  if constexpr (
      op::get_ordinal_v<typename patch_type::underlying_type, FieldId> !=
      FieldOrdinal{0}) {
    maybeEnsure<FieldId>();
    patchAfter<FieldId>();
    return *data_.patch();
  } else {
    // This code path should never be executed since the caller should already
    // validate whether the field is patchable.
    // However, this is needed for explicit instantiation when we can't validate
    // whether field is patchable in codegen.
    folly::throw_exception_fmt_format<std::logic_error>(
        "Field (id={}) is not patchable", folly::to_underlying(FieldId::value));
  }
}

template <class Patch, class Derived>
void BaseEnsurePatch<Patch, Derived>::apply(T& val) const {
  return customVisit(Applier{val});
}
template <class Patch>
void StructPatch<Patch>::merge(const StructPatch& val) {
  Base::merge(val);
}
template <class Patch>
void StructPatch<Patch>::merge(StructPatch&& val) {
  Base::merge(std::move(val));
}
template <class Patch>
void UnionPatch<Patch>::merge(const UnionPatch& val) {
  Base::merge(val);
}
template <class Patch>
void UnionPatch<Patch>::merge(UnionPatch&& val) {
  Base::merge(std::move(val));
}
} // namespace apache::thrift::op::detail
