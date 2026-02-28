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

#include <folly/container/F14Map.h>
#include <thrift/lib/cpp2/Adapter.h>
#include <thrift/lib/cpp2/type/Any.h>

namespace apache::thrift::ident {
struct assign;
}

namespace apache::thrift::op::detail {

template <class>
class AnyPatch;

template <class>
class BoolPatch;

template <class>
class NumberPatch;

template <class>
class StringPatch;

template <class>
class BinaryPatch;

template <class>
class AssignPatch;

template <class>
class FieldPatch;

template <class>
class StructPatch;

template <class>
class UnionPatch;

template <class>
class ListPatch;

template <class>
class SetPatch;

template <class>
class MapPatch;

// Latest Thrift Static Patch version that the process is aware of. Any Thrift
// Static Patch with a version higher than this will not be processed by the
// binary. This is to ensure that the binary does not attempt to process a
// Thrift Static Patch that includes operations or features it does not support,
// which could lead to data corruption or other issues
inline constexpr int32_t kThriftStaticPatchVersion = 2;

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

template <class>
constexpr inline bool is_list_patch_v = false;
template <class Patch>
constexpr inline bool is_list_patch_v<ListPatch<Patch>> = true;
template <class>
constexpr inline bool is_set_patch_v = false;
template <class Patch>
constexpr inline bool is_set_patch_v<SetPatch<Patch>> = true;
template <class>
constexpr inline bool is_map_patch_v = false;
template <class Patch>
constexpr inline bool is_map_patch_v<MapPatch<Patch>> = true;
template <class>
constexpr inline bool is_structured_patch_v = false;
template <class Patch>
constexpr inline bool is_structured_patch_v<StructPatch<Patch>> = true;
template <class Patch>
constexpr inline bool is_structured_patch_v<UnionPatch<Patch>> = true;
template <class>
constexpr inline bool is_struct_patch_v = false;
template <class Patch>
constexpr inline bool is_struct_patch_v<StructPatch<Patch>> = true;
template <class>
constexpr inline bool is_union_patch_v = false;
template <class Patch>
constexpr inline bool is_union_patch_v<UnionPatch<Patch>> = true;
template <class>
constexpr inline bool is_any_patch_v = false;
template <class Patch>
constexpr inline bool is_any_patch_v<AnyPatch<Patch>> = true;
template <class>
constexpr inline bool is_assign_patch_v = false;
template <class Patch>
constexpr inline bool is_assign_patch_v<AssignPatch<Patch>> = true;

class MinSafePatchVersionVisitor {
 public:
  // Shared
  template <typename T>
  void assign(const T&) {}
  void clear() {}
  template <typename Patch>
  void recurse(const Patch& patch) {
    if constexpr (
        is_map_patch_v<Patch> || is_structured_patch_v<Patch> ||
        is_any_patch_v<Patch>) {
      // Since all v2 operations are supported with AnyPatch, skip all other
      // patches.
      MinSafePatchVersionVisitor visitor;
      patch.customVisit(visitor);
      version = std::max(version, visitor.version);
    }
  }

  // Container
  template <typename T>
  void add(const T&) {}
  template <typename T>
  void putMulti(const T&) {}
  template <typename T>
  void tryPutMulti(const T&) {}
  template <typename T>
  void remove(const T&) {}
  template <typename T>
  void removeMulti(const T&) {}
  template <typename Key, typename ValuePatch>
  void patchIfSet(const folly::F14NodeMap<Key, ValuePatch>& patches) {
    for (const auto& [k, vp] : patches) {
      recurse(vp);
    }
  }

  // Structured
  template <typename>
  void ensure() {}
  template <typename, typename Field>
  void ensure(const Field&) {}
  template <typename>
  void remove() {}
  template <typename, typename FieldPatch>
  void patchIfSet(const FieldPatch& fieldPatch) {
    recurse(fieldPatch);
  }

  // Thrift Any
  template <typename... T>
  void patchIfTypeIs(T&&...) {
    version = std::max(version, 2);
  }
  void ensureAny(const type::AnyStruct&) { version = std::max(version, 2); }

  int32_t version = 1;
};

template <typename Patch>
int32_t calculateMinSafePatchVersion(const Patch& patch) {
  // is_patch_v
  if constexpr (is_assign_patch_v<Patch>) {
    return 1;
  } else {
    MinSafePatchVersionVisitor visitor;
    patch.customVisit(visitor);
    return visitor.version;
  }
}

template <class Patch>
struct PatchedTypeTag {
  using type = op::get_type_tag<typename Patch::underlying_type, ident::assign>;
};
} // namespace apache::thrift::op::detail
