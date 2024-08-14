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
#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/op/detail/BasePatch.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/protocol/Patch.h>
#include <thrift/lib/cpp2/type/Type.h>
#include <thrift/lib/thrift/gen-cpp2/any_patch_detail_types.h>
#include <thrift/lib/thrift/gen-cpp2/any_rep_types.h>

namespace apache {
namespace thrift {
namespace op {

class TypeToPatchInternalDoNotUse;

namespace detail {

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
        folly::throw_exception<std::runtime_error>(fmt::format(
            "duplicated key: {}",
            debugStringViaEncode(typeToPatchStruct.type().value())));
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

  // TODO(dokwon): Add customizations to optimize operators in adapted type.
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
    struct Visitor {
      type::AnyStruct& v;
      void assign(const type::AnyStruct& b) { v = b; }
      void clear() { apache::thrift::clear(v); }
      void patchIfTypeIs(
          const type::Type& type, const std::vector<type::AnyStruct>& patches) {
        if (v.type() != type) {
          return;
        }
        auto val = protocol::detail::parseValueFromAny(v);
        for (const auto& p : patches) {
          auto dynPatch = protocol::detail::parseValueFromAny(p).as_object();
          protocol::applyPatch(dynPatch, val);
        }
        v = protocol::detail::toAny(val, v.type().value(), v.protocol().value())
                .toThrift();
      }
      void ensureAny(const type::AnyStruct& any) {
        if (v.type() == any.type()) {
          return;
        }
        v = any;
      }
    };

    return customVisit(Visitor{val});
  }

  void ensureAny(type::AnyStruct ensureAny) {
    ensureImpl(std::move(ensureAny));
  }

  template <typename VPatch>
  void patchIfTypeIs(const VPatch& patch) {
    static_assert(std::is_base_of_v<
                  BasePatch<typename VPatch::underlying_type, VPatch>,
                  VPatch>);
    tryPatchable<VPatch>();
    patchIfTypeIsImpl(
        patch, ensures<type::infer_tag<typename VPatch::value_type>>());
  }

 private:
  using Base::assignOr;
  using Base::data_;

  bool ensures(const type::Type& type) {
    return data_.ensureAny().has_value() &&
        data_.ensureAny().value().type() == type;
  }

  template <typename Tag>
  bool ensures() {
    return ensures(type::Type::create<Tag>());
  }

  // If assign has value and specified 'Vpatch' is the corresponding patch type
  // to the type in 'assign' operation, we ensure the patch is patchable by
  // making it to 'clear' + 'ensureAny' operation.
  template <typename VPatch>
  void tryPatchable() {
    using VType = typename VPatch::value_type;
    using VTag = type::infer_tag<VType>;
    if (data_.assign().has_value()) {
      if (data_.assign().value().type() != type::Type::create<VTag>()) {
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

  bool ensureImpl(type::AnyStruct ensureAny) {
    if (data_.assign().has_value()) {
      data_.clear() = true;
      data_.ensureAny() = std::move(data_.assign().value());
      data_.assign().reset();
    }

    // TODO(dokwon): Handle PatchIfTypeIsAfter

    if (ensures(ensureAny.type().value())) {
      return false;
    }

    // TODO(dokwon): Handle PatchIfTypeIsAfter

    data_.ensureAny() = std::move(ensureAny);
    return true;
  }
};

} // namespace detail
} // namespace op
} // namespace thrift
} // namespace apache
