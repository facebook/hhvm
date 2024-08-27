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

#include <vector>
#include <fmt/format.h>
#include <folly/container/F14Map.h>
#include <thrift/lib/cpp2/op/detail/BasePatch.h>
#include <thrift/lib/cpp2/protocol/Patch.h>
#include <thrift/lib/cpp2/type/Type.h>
#include <thrift/lib/thrift/gen-cpp2/any_patch_detail_types.h>
#include <thrift/lib/thrift/gen-cpp2/any_rep_types.h>

namespace apache {
namespace thrift {
namespace op {

class TypeToPatchInternalDoNotUse;

namespace detail {

[[noreturn]] void throwIfDuplicatedType(const type::Type& type);

struct TypeToPatchMapAdapter {
  using StandardType = std::vector<TypeToPatchInternalDoNotUse>;
  using AdaptedType =
      folly::F14FastMap<type::Type, std::vector<type::AnyStruct>>;

  static AdaptedType fromThrift(StandardType&& vec) {
    TypeToPatchMapAdapter::AdaptedType map;
    map.reserve(vec.size());
    for (auto& typeToPatchStruct : vec) {
      if (!map.emplace(
                  typeToPatchStruct.type().value(),
                  std::move(typeToPatchStruct.patches().value()))
               .second) {
        throwIfDuplicatedType(typeToPatchStruct.type().value());
      }
    }
    return map;
  }

  static StandardType toThrift(const AdaptedType& map) {
    TypeToPatchMapAdapter::StandardType vec;
    vec.reserve(map.size());
    for (const auto& [type, patches] : map) {
      auto& obj = vec.emplace_back();
      obj.type() = type;
      obj.patches() = patches;
    }
    return vec;
  }

  template <typename Tag, typename Protocol>
  static uint32_t encode(
      Protocol& prot, const TypeToPatchMapAdapter::AdaptedType& map) {
    uint32_t s = 0;
    s += prot.writeListBegin(protocol::TType::T_STRUCT, map.size());
    for (const auto& [type, patches] : map) {
      s += prot.writeStructBegin(
          op::get_class_name_v<TypeToPatchInternalDoNotUse>.data());
      s += prot.writeFieldBegin("type", protocol::TType::T_STRUCT, 1);
      s += op::encode<type::infer_tag<type::Type>>(prot, type);
      s += prot.writeFieldEnd();
      s += prot.writeFieldBegin("patches", protocol::TType::T_LIST, 2);
      s += op::encode<type::list<type::struct_t<type::AnyStruct>>>(
          prot, patches);
      s += prot.writeFieldEnd();
      s += prot.writeStructEnd();
    }
    s += prot.writeListEnd();
    return s;
  }

  template <typename Tag, typename Protocol>
  static void decode(Protocol& prot, TypeToPatchMapAdapter::AdaptedType& map) {
    protocol::TType t;
    uint32_t s;
    prot.readListBegin(t, s);
    if (t != typeTagToTType<Tag>) {
      while (s--) {
        prot.skip(t);
      }
    } else {
      while (s--) {
        TypeToPatchInternalDoNotUse typeToPatchStruct;
        op::decode<type::struct_t<TypeToPatchInternalDoNotUse>>(
            prot, typeToPatchStruct);
        if (!map.emplace(
                    typeToPatchStruct.type().value(),
                    std::move(typeToPatchStruct.patches().value()))
                 .second) {
          throwIfDuplicatedType(typeToPatchStruct.type().value());
        }
      }
    }
    prot.readListEnd();
  }
};

/// Patch for Thrift Any.
/// * `optional AnyStruct assign`
/// * `terse bool clear`
/// * `terse map<Type, AnyStruct> patchIfTypeIsPrior`
/// * `optional AnyStruct ensureAny`
/// * `terse map<Type, AnyStruct> patchIfTypeIsAfter`
template <typename Patch>
class AnyPatch : public BaseClearPatch<Patch, AnyPatch<Patch>> {
  using Base = BaseClearPatch<Patch, AnyPatch>;

 public:
  using Base::apply;
  using Base::Base;
  using Base::operator=;
  using Base::clear;

  /// @copybrief AssignPatch::customVisit
  ///
  /// Users should provide a visitor with the following methods
  ///
  ///     struct Visitor {
  ///       void assign(cosnt AnyStruct&);
  ///       void clear();
  ///       void patchIfTypeIs(const Type&, const AnyStruct&);
  ///       void ensureAny(cosnt AnyStruct&);
  ///     }
  ///
  template <typename Visitor>
  void customVisit(Visitor&& v) const {
    if (false) {
      // Test whether the required methods exist in Visitor
      v.assign(type::AnyStruct{});
      v.clear();
      v.patchIfTypeIs(type::Type{}, std::vector<type::AnyStruct>{});
      v.ensureAny(type::AnyStruct{});
    }
    if (!Base::template customVisitAssignAndClear(v)) {
      // patchIfTypeIsPrior
      for (const auto& [type, patches] : data_.patchIfTypeIsPrior().value()) {
        v.patchIfTypeIs(type, patches);
      }

      // ensureAny
      if (data_.ensureAny().has_value()) {
        v.ensureAny(data_.ensureAny().value());
      }

      // patchIfTypeIsAfter
      for (const auto& [type, patches] : data_.patchIfTypeIsAfter().value()) {
        v.patchIfTypeIs(type, patches);
      }
    }
  }

  void apply(type::AnyStruct& val) const {
    auto applyTypePatches = [&](const std::vector<type::AnyStruct>* prior,
                                const std::vector<type::AnyStruct>* after) {
      if (!prior && !after) {
        return;
      }

      auto dynVal = protocol::detail::parseValueFromAny(val);
      if (prior) {
        for (const auto& p : *prior) {
          auto dynPatch = protocol::detail::parseValueFromAny(p).as_object();
          protocol::applyPatch(dynPatch, dynVal);
        }
      }
      if (after) {
        for (const auto& p : *after) {
          auto dynPatch = protocol::detail::parseValueFromAny(p).as_object();
          protocol::applyPatch(dynPatch, dynVal);
        }
      }
      val = protocol::detail::toAny(
                dynVal, val.type().value(), val.protocol().value())
                .toThrift();
    };

    if (hasAssign()) {
      val = data_.assign().value();
      return;
    }
    if (data_.clear().value()) {
      apache::thrift::clear(val);
    }

    // If 'ensureAny' type does not match the type of stored value in Thrift
    // Any, we can ignore 'patchIfTypeIsPrior'.
    if (data_.ensureAny().has_value() &&
        data_.ensureAny().value().type() != val.type()) {
      val = data_.ensureAny().value();
      const auto* afterTypePatches = folly::get_ptr(
          data_.patchIfTypeIsAfter().value(), val.type().value());
      applyTypePatches(nullptr, afterTypePatches);
      return;
    }

    const auto* priorTypePatches =
        folly::get_ptr(data_.patchIfTypeIsPrior().value(), val.type().value());
    const auto* afterTypePatches =
        folly::get_ptr(data_.patchIfTypeIsAfter().value(), val.type().value());
    applyTypePatches(priorTypePatches, afterTypePatches);
  }

  void ensureAny(type::AnyStruct ensureAny) {
    if (data_.assign().has_value()) {
      data_.clear() = true;
      data_.ensureAny() = std::move(data_.assign().value());
      data_.assign().reset();
    }

    if (ensures(ensureAny.type().value())) {
      return;
    }

    data_.ensureAny() = std::move(ensureAny);
  }

  template <typename VPatch>
  void patchIfTypeIs(const VPatch& patch) {
    // TODO(dokwon): Refactor PatchTrait to use is_patch_v.
    static_assert(std::is_base_of_v<
                  BasePatch<typename VPatch::underlying_type, VPatch>,
                  VPatch>);
    tryPatchable<VPatch>();
    patchIfTypeIsImpl(
        patch, ensures<type::infer_tag<typename VPatch::value_type>>());
  }

  template <typename VPatch>
  void ensureAndPatch(const VPatch& patch) {
    // TODO(dokwon): Refactor PatchTrait to use is_patch_v.
    static_assert(std::is_base_of_v<
                  BasePatch<typename VPatch::underlying_type, VPatch>,
                  VPatch>);
    using VTag = type::infer_tag<typename VPatch::value_type>;
    ensureAny(type::AnyData::toAny<VTag>({}).toThrift());
    patchIfTypeIsImpl(patch, true);
  }

 private:
  using Base::assignOr;
  using Base::data_;
  using Base::hasAssign;

  bool ensures(const type::Type& type) {
    return data_.ensureAny().has_value() &&
        data_.ensureAny().value().type() == type;
  }

  template <typename Tag>
  bool ensures() {
    return ensures(type::Type::create<Tag>());
  }

  // If assign has value and specified 'VPatch' is the corresponding patch
  // type to the type in 'assign' operation, we ensure the patch is patchable
  // by making it to 'clear' + 'ensureAny' operation.
  template <typename VPatch>
  void tryPatchable() {
    using VType = typename VPatch::value_type;
    using VTag = type::infer_tag<VType>;
    tryPatchable(type::Type::create<VTag>());
  }
  void tryPatchable(const type::Type& type) {
    if (data_.assign().has_value()) {
      if (data_.assign().value().type() != type) {
        return;
      }
      data_.clear() = true;
      ensureAny(std::move(data_.assign().value()));
      data_.assign().reset();
    }
  }

  template <typename VPatch>
  void patchIfTypeIsImpl(const VPatch& patch, bool after) {
    auto type =
        type::Type::create<type::infer_tag<typename VPatch::value_type>>();
    auto anyStruct =
        type::AnyData::toAny<type::infer_tag<VPatch>>(patch).toThrift();
    if (after) {
      data_.patchIfTypeIsAfter().value()[std::move(type)].push_back(
          std::move(anyStruct));
    } else {
      data_.patchIfTypeIsPrior().value()[std::move(type)].push_back(
          std::move(anyStruct));
    }
  }

  // Needed for merge.
  void patchIfTypeIs(
      const type::Type& type, const std::vector<type::AnyStruct>& patches) {
    tryPatchable(type);
    if (ensures(type)) {
      auto& vec = data_.patchIfTypeIsAfter().value()[type];
      vec.insert(vec.end(), patches.begin(), patches.end());
    } else {
      auto& vec = data_.patchIfTypeIsPrior().value()[type];
      vec.insert(vec.end(), patches.begin(), patches.end());
    }
  }
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
