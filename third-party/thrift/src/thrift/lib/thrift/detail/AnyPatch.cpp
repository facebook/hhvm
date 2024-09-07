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
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/type/Type.h>
#include <thrift/lib/thrift/detail/AnyPatch.h>
#include <thrift/lib/thrift/gen-cpp2/any_patch_types.h>

namespace apache::thrift::op::detail {

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
  TypeToPatchMapAdapter::AdaptedType map;
  map.reserve(vec.size());
  for (auto& typeToPatchStruct : vec) {
    auto it = map.emplace(
        typeToPatchStruct.type().value(),
        std::move(typeToPatchStruct.patches().value()));
    if (!it.second) {
      throwDuplicatedType(typeToPatchStruct.type().value());
    }
    for (const auto& any : it.first->second) {
      throwIfInvalidOrUnsupportedAny(any);
    }
  }
  return map;
}

auto TypeToPatchMapAdapter::toThrift(const AdaptedType& map) -> StandardType {
  TypeToPatchMapAdapter::StandardType vec;
  vec.reserve(map.size());
  for (const auto& [type, patches] : map) {
    auto& obj = vec.emplace_back();
    obj.type() = type;
    obj.patches() = patches;
  }
  return vec;
}

template <class Patch>
void AnyPatch<Patch>::apply(type::AnyStruct& val) const {
  auto applyTypePatches = [&](const TypeToPatchMapAdapter::AdaptedType* prior,
                              const TypeToPatchMapAdapter::AdaptedType* after) {
    std::optional<protocol::Value> dynVal;

    // To support applying AnyPatch to Thrift Any storing type with
    // 'typeHashPrefixSha2_256', we need to iterate the whole map.
    if (prior) {
      for (const auto& [type, patches] : *prior) {
        if (type::identicalType(type, val.type().value())) {
          dynVal = protocol::detail::parseValueFromAny(val);
          for (const auto& p : patches) {
            auto dynPatch = protocol::detail::parseValueFromAny(p).as_object();
            protocol::applyPatch(dynPatch, dynVal.value());
          }
          break;
        }
      }
    }
    if (after) {
      for (const auto& [type, patches] : *after) {
        if (type::identicalType(type, val.type().value())) {
          if (!dynVal) {
            dynVal = protocol::detail::parseValueFromAny(val);
          }
          for (const auto& p : patches) {
            auto dynPatch = protocol::detail::parseValueFromAny(p).as_object();
            protocol::applyPatch(dynPatch, dynVal.value());
          }
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
    type::Type type, type::AnyStruct patch, bool after) {
  if (after) {
    data_.patchIfTypeIsAfter().value()[std::move(type)].push_back(
        std::move(patch));
  } else {
    data_.patchIfTypeIsPrior().value()[std::move(type)].push_back(
        std::move(patch));
  }
}

template <class Patch>
void AnyPatch<Patch>::patchIfTypeIs(
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

template class AnyPatch<AnyPatchStruct>;

} // namespace apache::thrift::op::detail
