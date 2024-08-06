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

#include <map>
#include <set>
#include <type_traits>
#include <unordered_set>

#include <folly/MapUtil.h>
#include <folly/Utility.h>
#include <thrift/lib/cpp2/protocol/FieldMaskRef.h>
#include <thrift/lib/cpp2/protocol/detail/FieldMask.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_constants.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_types.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

using namespace apache::thrift::protocol::field_mask_constants;

namespace apache::thrift::protocol {

template <typename T, typename Id>
bool containsId(const T& t, Id id) {
  if constexpr (std::is_same_v<T, Object>) {
    return t.contains(id);
  } else {
    return t.find(id) != t.end();
  }
}

// call clear based on the type of the value.
void clear(MaskRef ref, Value& value) {
  if (value.is_object()) {
    if (ref.isTypeMask()) {
      folly::throw_exception<std::runtime_error>(
          "TODO: Support typeMask clear");
    }
    ref.clear(value.as_object());
    return;
  }
  if (value.is_map()) {
    ref.clear(value.as_map());
    return;
  }
  folly::throw_exception<std::runtime_error>(
      "The mask and object are incompatible.");
}

template <typename T, typename Id>
void clear_impl(MaskRef ref, T& obj, Id id, Value& value) {
  // Id doesn't exist in mask, skip.
  if (ref.isNoneMask()) {
    return;
  }
  // Id that we want to clear.
  if (ref.isAllMask()) {
    obj.erase(id);
    return;
  }
  clear(ref, value);
}

// call copy based on the type of the value.
protocol::Value filter(MaskRef ref, const Value& src) {
  protocol::Value ret;
  if (src.is_object()) {
    ret.emplace_object(ref.filter(src.as_object()));
  } else if (src.is_map()) {
    ret.emplace_map(ref.filter(src.as_map()));
  } else {
    folly::throw_exception<std::runtime_error>(
        "The mask and object are incompatible.");
  }
  return ret;
}

template <typename T, typename Id>
void filter_impl(MaskRef ref, const T& src, T& ret, Id id) {
  // Id doesn't exist in field mask, skip.
  if (ref.isNoneMask()) {
    return;
  }
  // If field is not in src, skip.
  if (!containsId(src, id)) {
    return;
  }
  if (ref.isAllMask()) {
    ret[id] = src.at(id);
    return;
  }
  if (src.at(id).is_object()) {
    auto recurse = ref.filter(src.at(id).as_object());
    if (!recurse.empty()) {
      ret[id].emplace_object(std::move(recurse));
    }
    return;
  }
  if (src.at(id).is_map()) {
    auto recurse = ref.filter(src.at(id).as_map());
    if (!recurse.empty()) {
      ret[id].emplace_map(std::move(recurse));
    }
    return;
  }
  folly::throw_exception<std::runtime_error>(
      "The mask and object are incompatible.");
}

// Gets the mask of the given field id if it exists in the map, otherwise,
// returns noneMask.
const Mask& getMask(const FieldIdToMask& map, FieldId id) {
  return folly::get_ref_default(
      map, folly::to_underlying(id), field_mask_constants::noneMask());
}

// Gets the mask of the given map id if it exists in the map, otherwise,
// returns noneMask.
const Mask& getMask(const MapIdToMask& map, detail::MapId id) {
  return folly::get_ref_default(
      map, folly::to_underlying(id), field_mask_constants::noneMask());
}

// Gets the mask of the given string if it exists in the string map, otherwise,
// returns noneMask.
const Mask& getMask(const MapStringToMask& map, const std::string& key) {
  return folly::get_ref_default(map, key, field_mask_constants::noneMask());
}

void MaskRef::throwIfNotFieldMask() const {
  if (!isFieldMask()) {
    folly::throw_exception<std::runtime_error>("not a field mask");
  }
}

void MaskRef::throwIfNotMapMask() const {
  if (!isMapMask()) {
    folly::throw_exception<std::runtime_error>("not a map mask");
  }
}

void MaskRef::throwIfNotIntegerMapMask() const {
  if (!isIntegerMapMask()) {
    folly::throw_exception<std::runtime_error>("not an integer map mask");
  }
}

void MaskRef::throwIfNotStringMapMask() const {
  if (!isStringMapMask()) {
    folly::throw_exception<std::runtime_error>("not a string map mask");
  }
}

MaskRef MaskRef::get(FieldId id) const {
  throwIfNotFieldMask();
  if (mask.includes_ref()) {
    return MaskRef{getMask(mask.includes_ref().value(), id), is_exclusion};
  }
  return MaskRef{getMask(mask.excludes_ref().value(), id), !is_exclusion};
}

MaskRef MaskRef::get(detail::MapId id) const {
  if (isAllMask() || isNoneMask()) { // This whole map is included or excluded.
    return *this;
  }
  throwIfNotIntegerMapMask();
  if (mask.includes_map_ref()) {
    return MaskRef{getMask(mask.includes_map_ref().value(), id), is_exclusion};
  }
  return MaskRef{getMask(mask.excludes_map_ref().value(), id), !is_exclusion};
}

MaskRef MaskRef::get(const std::string& key) const {
  if (isAllMask() || isNoneMask()) { // This whole map is included or excluded.
    return *this;
  }
  throwIfNotStringMapMask();
  if (mask.includes_string_map_ref()) {
    return MaskRef{
        getMask(mask.includes_string_map_ref().value(), key), is_exclusion};
  }
  return MaskRef{
      getMask(mask.excludes_string_map_ref().value(), key), !is_exclusion};
}

bool MaskRef::isAllMask() const {
  return (is_exclusion &&
          ::apache::thrift::protocol::detail::isNoneMask(mask)) ||
      (!is_exclusion && ::apache::thrift::protocol::detail::isAllMask(mask));
}

bool MaskRef::isNoneMask() const {
  return (is_exclusion &&
          ::apache::thrift::protocol::detail::isAllMask(mask)) ||
      (!is_exclusion && ::apache::thrift::protocol::detail::isNoneMask(mask));
}

bool MaskRef::isAllMapMask() const {
  return (is_exclusion &&
          ::apache::thrift::protocol::detail::isNoneMapMask(mask)) ||
      (!is_exclusion && ::apache::thrift::protocol::detail::isAllMapMask(mask));
}

bool MaskRef::isNoneMapMask() const {
  return (is_exclusion &&
          ::apache::thrift::protocol::detail::isAllMapMask(mask)) ||
      (!is_exclusion &&
       ::apache::thrift::protocol::detail::isNoneMapMask(mask));
}

bool MaskRef::isExclusive() const {
  return (mask.includes_ref() && is_exclusion) ||
      (mask.excludes_ref() && !is_exclusion) ||
      (mask.includes_map_ref() && is_exclusion) ||
      (mask.excludes_map_ref() && !is_exclusion) ||
      (mask.includes_string_map_ref() && is_exclusion) ||
      (mask.excludes_string_map_ref() && !is_exclusion) ||
      (mask.includes_type_ref() && is_exclusion) ||
      (mask.excludes_type_ref() && !is_exclusion);
}

bool MaskRef::isFieldMask() const {
  return mask.includes_ref() || mask.excludes_ref();
}

bool MaskRef::isMapMask() const {
  return isIntegerMapMask() || isStringMapMask();
}

bool MaskRef::isIntegerMapMask() const {
  return mask.includes_map_ref() || mask.excludes_map_ref();
}

bool MaskRef::isStringMapMask() const {
  return mask.includes_string_map_ref() || mask.excludes_string_map_ref();
}

bool MaskRef::isTypeMask() const {
  return mask.includes_type_ref() || mask.excludes_type_ref();
}

void MaskRef::clear(protocol::Object& obj) const {
  throwIfNotFieldMask();
  for (auto& [id, value] : obj) {
    MaskRef ref = get(FieldId{id});
    clear_impl(ref, obj, FieldId{id}, value);
  }
}

void MaskRef::clear(folly::F14FastMap<Value, Value>& map) const {
  throwIfNotMapMask();
  for (auto& [key, value] : map) {
    MaskRef ref =
        (detail::getArrayKeyFromValue(key) == detail::ArrayKey::Integer)
        ? get(detail::getMapIdFromValue(key))
        : get(detail::getStringFromValue(key));
    clear_impl(ref, map, key, value);
  }
}

protocol::Object MaskRef::filter(const protocol::Object& src) const {
  throwIfNotFieldMask();
  protocol::Object ret;
  for (auto& [fieldId, _] : src) {
    MaskRef ref = get(FieldId{fieldId});
    filter_impl(ref, src, ret, FieldId{fieldId});
  }
  return ret;
}

folly::F14FastMap<Value, Value> MaskRef::filter(
    const folly::F14FastMap<Value, Value>& src) const {
  throwIfNotMapMask();
  folly::F14FastMap<Value, Value> ret;

  std::set<std::reference_wrapper<const Value>, std::less<Value>> keys;
  for (const auto& [id, _] : src) {
    keys.insert(id);
  }
  if (keys.empty()) {
    return ret;
  }

  if (detail::getArrayKeyFromValue(*keys.begin()) ==
      detail::ArrayKey::Integer) {
    for (const Value& key : keys) {
      MaskRef ref = get(detail::getMapIdFromValue(key));
      filter_impl(ref, src, ret, key);
    }
  } else {
    for (Value key : keys) {
      MaskRef ref = get(detail::getStringFromValue(key));
      filter_impl(ref, src, ret, key);
    }
  }
  return ret;
}

} // namespace apache::thrift::protocol
