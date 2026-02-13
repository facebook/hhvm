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

#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/patch/detail/PatchBadge.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/protocol/detail/Patch.h>
#include <thrift/lib/cpp2/type/Type.h>
#include <thrift/lib/thrift/detail/AnyPatch.h>
#include <thrift/lib/thrift/detail/DynamicPatch.h>
#include <thrift/lib/thrift/gen-cpp2/any_patch_types.h>

namespace apache::thrift::op::detail {

using apache::thrift::protocol::DynamicPatch;
using apache::thrift::protocol::detail::badge;

void throwDuplicatedType(const type::Type& type) {
  folly::throw_exception<std::runtime_error>(fmt::format(
      "duplicated key in AnyPatch: {}", debugStringViaEncode(type)));
}

void throwTypeNotValid(const type::Type& type) {
  folly::throw_exception<std::runtime_error>(
      fmt::format("Invalid type: {}", debugStringViaEncode(type)));
}

void throwAnyNotValid(const type::AnyStruct& any) {
  folly::throw_exception<std::runtime_error>(
      fmt::format("Invalid any: {}", debugStringViaEncode(any)));
}

void throwUnsupportedAnyProtocol(const type::AnyStruct& any) {
  folly::throw_exception<std::runtime_error>(fmt::format(
      "Unsupported serialization protocol for AnyPatch: {}",
      debugStringViaEncode(any.protocol().value())));
}

auto TypeToPatchMapAdapter::fromThrift(StandardType&& vec) -> AdaptedType {
  AdaptedType map;
  map.reserve(vec.size());
  for (auto& typeToPatchStruct : vec) {
    throwIfInvalidOrUnsupportedAny(*typeToPatchStruct.patch());
    DynamicPatch patch{DynamicPatch::fromPatch(*typeToPatchStruct.patch())};
    auto it = map.emplace(typeToPatchStruct.type().value(), std::move(patch));
    if (!it.second) {
      throwDuplicatedType(typeToPatchStruct.type().value());
    }
  }
  return map;
}

auto TypeToPatchMapAdapter::toThrift(const AdaptedType& map) -> StandardType {
  TypeToPatchMapAdapter::StandardType vec;
  vec.reserve(map.size());
  for (const auto& [type, patch] : map) {
    auto& obj = vec.emplace_back();
    obj.type() = type;
    obj.patch() = patch.toPatch(type);
  }
  return vec;
}

bool TypeToPatchMapAdapter::equal(
    const AdaptedType& lhs, const AdaptedType& rhs) {
  if (lhs.size() != rhs.size()) {
    return false;
  }

  for (const auto& [type, patch] : lhs) {
    auto p = folly::get_ptr(rhs, type);
    if (!p) {
      return false;
    }

    // TODO: this can probably be optimized
    if (patch.toObject() != p->toObject()) {
      return false;
    }
  }

  return true;
}

bool TypeToPatchMapAdapter::addDynamicPatchToMap(
    AdaptedType& map, const TypeToPatchInternalDoNotUse& typeToPatchStruct) {
  throwIfInvalidOrUnsupportedAny(*typeToPatchStruct.patch());
  DynamicPatch patch{DynamicPatch::fromPatch(*typeToPatchStruct.patch())};
  return map.emplace(typeToPatchStruct.type().value(), std::move(patch)).second;
}

type::AnyStruct TypeToPatchMapAdapter::toAny(
    const protocol::DynamicPatch& patch, const type::Type& type) {
  return patch.toPatch(type);
}

template <class Patch>
void AnyPatch<Patch>::apply(type::AnyStruct& val) const {
  auto applyTypePatches = [&](const TypeToPatchMapAdapter::AdaptedType* prior,
                              const TypeToPatchMapAdapter::AdaptedType* after) {
    std::optional<protocol::Value> dynVal;

    // To support applying AnyPatch to Thrift Any storing type with
    // 'typeHashPrefixSha2_256', we need to iterate the whole map.
    if (prior) {
      for (const auto& [type, patch] : *prior) {
        if (type::identicalType(type, val.type().value())) {
          dynVal = protocol::detail::parseValueFromAny(val);
          patch.apply(*dynVal);
          break;
        }
      }
    }
    if (after) {
      for (const auto& [type, patch] : *after) {
        if (type::identicalType(type, val.type().value())) {
          if (!dynVal) {
            dynVal = protocol::detail::parseValueFromAny(val);
          }
          patch.apply(*dynVal);
          break;
        }
      }
    }

    if (dynVal.has_value()) {
      val = protocol::detail::toAny(
                dynVal.value(), val.type().value(), val.protocol().value())
                .toThrift();
    }
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
      !type::identicalType(
          data_.ensureAny()->type().value(), val.type().value())) {
    val = data_.ensureAny().value();
    applyTypePatches(nullptr, &data_.patchIfTypeIsAfter().value());
    return;
  }
  applyTypePatches(
      &data_.patchIfTypeIsPrior().value(), &data_.patchIfTypeIsAfter().value());
}

template <class Patch>
void AnyPatch<Patch>::patchIfTypeIsImpl(
    type::Type type, type::AnyStruct any, bool after) {
  DynamicPatch patch{DynamicPatch::fromPatch(any)};
  patchIfTypeIsImpl(std::move(type), std::move(patch), after);
}

template <class Patch>
void AnyPatch<Patch>::patchIfTypeIsImpl(
    type::Type type, protocol::DynamicPatch dynamicPatch, bool after) {
  auto& map = after ? *data_.patchIfTypeIsAfter() : *data_.patchIfTypeIsPrior();
  auto [it, inserted] = map.try_emplace(type, std::move(dynamicPatch));
  if (!inserted) {
    // conditional move didn't happen
    // @lint-ignore CLANGTIDY bugprone-use-after-move
    it->second.merge(std::move(dynamicPatch));
  }
}

template <class Patch>
void AnyPatch<Patch>::patchIfTypeIs(
    const type::Type& type, const protocol::DynamicPatch& patch) {
  tryPatchable(type);
  if (ensures(type)) {
    data_.patchIfTypeIsAfter()[type].merge(patch);
  } else {
    data_.patchIfTypeIsPrior()[type].merge(patch);
  }
}

template <class Patch>
void AnyPatch<Patch>::DynamicPatchExtractionVisitor::assign(
    const type::AnyStruct& any) {
  if (type::identicalType(any.type().value(), type_)) {
    // TODO(dokwon): Optimize creating DynamicPatch::assign from Thrift Any.
    protocol::Object patch;
    patch[static_cast<FieldId>(PatchOp::Assign)] =
        protocol::detail::parseValueFromAny(any);
    patch_ = protocol::DynamicPatch::fromObject(std::move(patch));
  }
}
template <class Patch>
void AnyPatch<Patch>::DynamicPatchExtractionVisitor::clear() {
  patch_.visitPatch([&](auto& patch) { patch.clear(); });
  cleared_ = true;
}
template <class Patch>
void AnyPatch<Patch>::DynamicPatchExtractionVisitor::patchIfTypeIs(
    const type::Type& type, const protocol::DynamicPatch& dpatch) {
  if (!type::identicalType(type, type_)) {
    return;
  }
  patch_.merge(dpatch);
}
template <class Patch>
void AnyPatch<Patch>::DynamicPatchExtractionVisitor::ensureAny(
    const type::AnyStruct& any) {
  if (!type::identicalType(any.type().value(), type_)) {
    patch_ = {};
  } else if (cleared_) {
    // After clear(), ensureAny() replaces the entire value since
    // the type was removed. Model as assign in the extracted patch.
    auto ensuredValue = type::AnyData{any};
    if (type::identicalType(ensuredValue.type(), type_)) {
      protocol::Object patchObj;
      patchObj[static_cast<FieldId>(PatchOp::Assign)] =
          protocol::detail::parseValueFromAny(any);
      patch_ = protocol::DynamicPatch::fromObject(std::move(patchObj));
    }
  }
  cleared_ = false;
}

template <class Patch>
protocol::ExtractedMasksFromPatch AnyPatch<Patch>::extractMaskFromPatch()
    const {
  struct Visitor {
    void assign(const type::AnyStruct&) {
      masks = {protocol::noneMask(), protocol::allMask()};
    }
    void clear() { masks = {protocol::noneMask(), protocol::allMask()}; }
    void patchIfTypeIs(
        const type::Type& type, const protocol::DynamicPatch& patch) {
      // granular read/write operation
      // Insert next extracted mask and insert type to read mask if the next
      // extracted mask does not include the type.
      auto getIncludesTypeRef = [](protocol::Mask& mask) {
        return mask.includes_type();
      };
      auto nextMasks = patch.extractMaskFromPatch();
      protocol::detail::insertNextMask(
          masks, nextMasks, type, type, getIncludesTypeRef);
      protocol::detail::insertTypeToMaskIfNotAllMask(masks.read, type);
    }
    void ensureAny(const type::AnyStruct& anyStruct) {
      ensureAnyType_ = anyStruct.type().value();
      masks.write = protocol::allMask();
    }

    void finalize() {
      if (ensureAnyType_) {
        protocol::detail::insertTypeToMaskIfNotAllMask(
            masks.read, ensureAnyType_->get());
      }
      protocol::detail::ensureRWMaskInvariant(masks);
    }

    protocol::ExtractedMasksFromPatch masks{
        protocol::noneMask(), protocol::noneMask()};

   private:
    std::optional<std::reference_wrapper<const type::Type>> ensureAnyType_;
  };
  Visitor v;
  customVisit(v);
  v.finalize();
  return std::move(v.masks);
}

template class AnyPatch<AnyPatchStruct>;

} // namespace apache::thrift::op::detail
