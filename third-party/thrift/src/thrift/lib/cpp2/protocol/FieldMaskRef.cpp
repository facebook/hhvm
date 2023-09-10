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

using apache::thrift::protocol::field_mask_constants;

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
void copy(MaskRef ref, const Value& src, Value& dst) {
  if (src.is_object() && dst.is_object()) {
    ref.copy(src.as_object(), dst.as_object());
    return;
  }
  if (src.is_map() && dst.is_map()) {
    ref.copy(src.as_map(), dst.as_map());
    return;
  }
  folly::throw_exception<std::runtime_error>(
      "The mask and object are incompatible.");
}

template <typename T, typename Id>
void copy_impl(MaskRef ref, const T& src, T& dst, Id id) {
  // Id doesn't exist in field mask, skip.
  if (ref.isNoneMask()) {
    return;
  }
  bool srcContainsId = containsId(src, id);
  bool dstContainsId = containsId(dst, id);

  // Id that we want to copy.
  if (ref.isAllMask()) {
    if (srcContainsId) {
      dst[id] = src.at(id);
    } else {
      dst.erase(id);
    }
    return;
  }
  if (!srcContainsId && !dstContainsId) { // skip
    return;
  }
  // Field doesn't exist in src, so just clear dst with the mask.
  if (!srcContainsId) {
    clear(ref, dst.at(id));
    return;
  }
  // Field exists in both src and dst, so call copy recursively.
  if (dstContainsId) {
    copy(ref, src.at(id), dst.at(id));
    return;
  }
  // Field only exists in src. Need to construct object/ map only if there's
  // a field to add.
  if (src.at(id).is_object()) {
    Object newObject;
    ref.copy(src.at(id).as_object(), newObject);
    if (!newObject.empty()) {
      dst[id].ensure_object() = std::move(newObject);
    }
    return;
  }
  if (src.at(id).is_map()) {
    folly::F14FastMap<Value, Value> newMap;
    ref.copy(src.at(id).as_map(), newMap);
    if (!newMap.empty()) {
      dst[id].ensure_map() = std::move(newMap);
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
      (mask.excludes_string_map_ref() && !is_exclusion);
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

void MaskRef::copy(const protocol::Object& src, protocol::Object& dst) const {
  throwIfNotFieldMask();
  // Get all field ids that are possibly masked.
  for (FieldId fieldId : getFieldsToCopy(src, dst)) {
    MaskRef ref = get(fieldId);
    copy_impl(ref, src, dst, fieldId);
  }
}

void MaskRef::copy(
    const folly::F14FastMap<Value, Value>& src,
    folly::F14FastMap<Value, Value>& dst) const {
  throwIfNotMapMask();
  // Get all map keys that are possibly masked.
  auto keys = getKeysToCopy(src, dst);

  if (keys.empty()) {
    return;
  }

  if (detail::getArrayKeyFromValue(*keys.begin()) ==
      detail::ArrayKey::Integer) {
    for (Value key : keys) {
      MaskRef ref = get(detail::getMapIdFromValue(key));
      copy_impl(ref, src, dst, key);
    }
  } else {
    for (Value key : keys) {
      MaskRef ref = get(detail::getStringFromValue(key));
      copy_impl(ref, src, dst, key);
    }
  }
}

std::unordered_set<FieldId> MaskRef::getFieldsToCopy(
    const protocol::Object& src, const protocol::Object& dst) const {
  std::unordered_set<FieldId> fieldIds;
  if (isExclusive()) {
    // With exclusive mask, copies fields in either src or dst.
    fieldIds.reserve(src.size() + dst.size());
    for (auto& [id, _] : src) {
      fieldIds.insert(FieldId{id});
    }
    for (auto& [id, _] : dst) {
      fieldIds.insert(FieldId{id});
    }
    return fieldIds;
  }

  // With inclusive mask, just copies fields in the mask.
  const FieldIdToMask& map =
      is_exclusion ? mask.excludes_ref().value() : mask.includes_ref().value();
  fieldIds.reserve(map.size());
  for (auto& [fieldId, _] : map) {
    if (src.contains(FieldId{fieldId}) || dst.contains(FieldId{fieldId})) {
      fieldIds.insert(FieldId{fieldId});
    }
  }
  return fieldIds;
}

std::set<std::reference_wrapper<const Value>, std::less<Value>>
MaskRef::getKeysToCopy(
    const folly::F14FastMap<Value, Value>& src,
    const folly::F14FastMap<Value, Value>& dst) const {
  // cannot use unordered_set as Value doesn't have hash function.
  // TODO: check if all keys have the same type
  std::set<std::reference_wrapper<const Value>, std::less<Value>> keys;
  for (const auto& [id, _] : src) {
    keys.insert(id);
  }
  for (const auto& [id, _] : dst) {
    keys.insert(id);
  }
  return keys;
}

} // namespace apache::thrift::protocol
