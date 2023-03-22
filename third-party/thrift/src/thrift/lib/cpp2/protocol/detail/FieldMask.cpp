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

#include <string_view>
#include <type_traits>

#include <folly/Utility.h>
#include <thrift/lib/cpp2/protocol/detail/FieldMask.h>

using apache::thrift::protocol::field_mask_constants;

namespace apache::thrift::protocol::detail {
// Gets the mask of the given field id if it exists in the map, otherwise,
// returns noneMask.
const Mask& getMask(const FieldIdToMask& map, FieldId id) {
  auto fieldId = folly::to_underlying(id);
  return map.find(fieldId) != map.end() ? map.at(fieldId)
                                        : field_mask_constants::noneMask();
}

// Gets the mask of the given map id if it exists in the map, otherwise,
// returns noneMask.
const Mask& getMask(const MapIdToMask& map, MapId id) {
  auto mapId = folly::to_underlying(id);
  return map.find(mapId) != map.end() ? map.at(mapId)
                                      : field_mask_constants::noneMask();
}

// Gets the mask of the given string if it exists in the string map, otherwise,
// returns noneMask.
const Mask& getMask(const MapStringToMask& map, std::string_view key) {
  return map.find(key) != map.end() ? map.at(key)
                                    : field_mask_constants::noneMask();
}

const FieldIdToMask* FOLLY_NULLABLE getFieldMask(const Mask& mask) {
  if (mask.includes_ref()) {
    return &*mask.includes_ref();
  }

  if (mask.excludes_ref()) {
    return &*mask.excludes_ref();
  }

  return nullptr;
}

const MapIdToMask* FOLLY_NULLABLE getMapMask(const Mask& mask) {
  if (mask.includes_map_ref()) {
    return &*mask.includes_map_ref();
  }

  if (mask.excludes_map_ref()) {
    return &*mask.excludes_map_ref();
  }

  return nullptr;
}

const MapStringToMask* FOLLY_NULLABLE getStringMapMask(const Mask& mask) {
  if (mask.includes_string_map_ref()) {
    return &*mask.includes_string_map_ref();
  }

  if (mask.excludes_string_map_ref()) {
    return &*mask.excludes_string_map_ref();
  }

  return nullptr;
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

MaskRef MaskRef::get(MapId id) const {
  if (isAllMask() || isNoneMask()) { // This whole map is included or excluded.
    return *this;
  }
  throwIfNotIntegerMapMask();
  if (mask.includes_map_ref()) {
    return MaskRef{getMask(mask.includes_map_ref().value(), id), is_exclusion};
  }
  return MaskRef{getMask(mask.excludes_map_ref().value(), id), !is_exclusion};
}

MaskRef MaskRef::get(std::string_view key) const {
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

int64_t getIntFromValue(Value v) {
  if (v.is_byte()) {
    return v.byteValue_ref().value();
  }
  if (v.is_i16()) {
    return v.i16Value_ref().value();
  }
  if (v.is_i32()) {
    return v.i32Value_ref().value();
  }
  if (v.is_i64()) {
    return v.i64Value_ref().value();
  }
  folly::throw_exception<std::runtime_error>(
      "mask map only works with an integer key.");
}

// call clear based on the type of the value.
void clear(MaskRef ref, Value& value) {
  if (value.objectValue_ref()) {
    ref.clear(value.objectValue_ref().value());
    return;
  }
  if (value.mapValue_ref()) {
    ref.clear(value.mapValue_ref().value());
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

void MaskRef::clear(protocol::Object& obj) const {
  throwIfNotFieldMask();
  for (auto& [id, value] : obj) {
    MaskRef ref = get(FieldId{id});
    clear_impl(ref, obj, FieldId{id}, value);
  }
}

void MaskRef::clear(std::map<Value, Value>& map) const {
  throwIfNotMapMask();
  for (auto& [key, value] : map) {
    MaskRef ref = get(MapId{getIntFromValue(key)});
    clear_impl(ref, map, key, value);
  }
}

// call copy based on the type of the value.
void copy(MaskRef ref, const Value& src, Value& dst) {
  if (src.objectValue_ref() && dst.objectValue_ref()) {
    ref.copy(src.objectValue_ref().value(), dst.objectValue_ref().value());
    return;
  }
  if (src.mapValue_ref() && dst.mapValue_ref()) {
    ref.copy(src.mapValue_ref().value(), dst.mapValue_ref().value());
    return;
  }
  folly::throw_exception<std::runtime_error>(
      "The mask and object are incompatible.");
}

template <typename T, typename Id>
bool containsId(const T& t, Id id) {
  if constexpr (std::is_same_v<T, Object>) {
    return t.contains(id);
  } else {
    return t.find(id) != t.end();
  }
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
  if (src.at(id).objectValue_ref()) {
    Object newObject;
    ref.copy(src.at(id).objectValue_ref().value(), newObject);
    if (!newObject.empty()) {
      dst[id].emplace_object() = std::move(newObject);
    }
    return;
  }
  if (src.at(id).mapValue_ref()) {
    std::map<Value, Value> newMap;
    ref.copy(src.at(id).mapValue_ref().value(), newMap);
    if (!newMap.empty()) {
      dst[id].emplace_map() = std::move(newMap);
    }
    return;
  }
  folly::throw_exception<std::runtime_error>(
      "The mask and object are incompatible.");
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
    const std::map<Value, Value>& src, std::map<Value, Value>& dst) const {
  throwIfNotMapMask();
  // Get all map keys that are possibly masked.
  for (Value key : getKeysToCopy(src, dst)) {
    MaskRef ref = get(MapId{getIntFromValue(key)});
    copy_impl(ref, src, dst, key);
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

std::set<Value> MaskRef::getKeysToCopy(
    const std::map<Value, Value>& src, std::map<Value, Value>& dst) const {
  // cannot use unordered_set as Value doesn't have hash function.
  // TODO: check if all keys have the same type
  std::set<Value> keys;
  for (auto& [id, _] : src) {
    keys.insert(id);
  }
  for (auto& [id, _] : dst) {
    keys.insert(id);
  }
  return keys;
}

void throwIfContainsMapMask(const Mask& mask) {
  if (mask.includes_map_ref() || mask.excludes_map_ref() ||
      mask.includes_string_map_ref() || mask.excludes_string_map_ref()) {
    folly::throw_exception<std::runtime_error>("map mask is not implemented");
  }
  const FieldIdToMask& map = mask.includes_ref() ? mask.includes_ref().value()
                                                 : mask.excludes_ref().value();
  for (auto& [_, nestedMask] : map) {
    throwIfContainsMapMask(nestedMask);
  }
}

// Returns the MapId for the given key.
MapId findMapIdByValue(const Mask& mask, const Value& newKey) {
  MapId mapId = MapId{reinterpret_cast<int64_t>(&newKey)};
  if (!(mask.includes_map_ref() || mask.excludes_map_ref())) {
    return mapId;
  }
  const auto& mapIdToMask = mask.includes_map_ref() ? *mask.includes_map_ref()
                                                    : *mask.excludes_map_ref();
  auto it = std::find_if(
      mapIdToMask.begin(), mapIdToMask.end(), [&newKey](const auto& kv) {
        return *(reinterpret_cast<Value*>(kv.first)) == newKey;
      });
  return it == mapIdToMask.end() ? mapId : MapId{it->first};
}
} // namespace apache::thrift::protocol::detail
