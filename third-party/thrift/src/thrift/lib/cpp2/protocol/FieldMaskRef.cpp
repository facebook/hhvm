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
#include <optional>
#include <set>
#include <string_view>
#include <type_traits>
#include <unordered_set>

#include <folly/MapUtil.h>
#include <folly/Utility.h>
#include <thrift/lib/cpp2/protocol/FieldMaskRef.h>
#include <thrift/lib/cpp2/protocol/detail/FieldMask.h>
#include <thrift/lib/cpp2/protocol/detail/Object.h>
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
  ref.clear(value);
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
const Mask& getMask(const MapStringToMask& map, std::string_view key) {
  return folly::get_ref_default(map, key, field_mask_constants::noneMask());
}

// Gets the mask of the given type if it exists in the type map, otherwise,
// returns noneMask.
const Mask& getMask(const MapTypeToMask& map, const type::Type& type) {
  return folly::get_ref_default(map, type, field_mask_constants::noneMask());
}

// Gets the mask of the given identifier if it exists in the map, otherwise,
// returns nullptr.
const Mask* getMaskOrNull(const FieldIdToMask& map, FieldId id) {
  return folly::get_ptr(map, folly::to_underlying(id));
}
const Mask* getMaskOrNull(const MapIdToMask& map, detail::MapId id) {
  return folly::get_ptr(map, folly::to_underlying(id));
}
const Mask* getMaskOrNull(const MapStringToMask& map, const std::string& key) {
  return folly::get_ptr(map, key);
}
const Mask* getMaskOrNull(const MapTypeToMask& map, const type::Type& type) {
  return folly::get_ptr(map, type);
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

void MaskRef::throwIfNotTypeMask() const {
  if (!isTypeMask()) {
    folly::throw_exception<std::runtime_error>("not a type mask");
  }
}

MaskRef MaskRef::get(FieldId id) const {
  throwIfNotFieldMask();
  if (mask.includes()) {
    return MaskRef{getMask(mask.includes().value(), id), is_exclusion};
  }
  return MaskRef{getMask(mask.excludes().value(), id), !is_exclusion};
}

MaskRef MaskRef::get(detail::MapId id) const {
  if (isAllMask() || isNoneMask()) { // This whole map is included or excluded.
    return *this;
  }
  throwIfNotIntegerMapMask();
  if (mask.includes_map()) {
    return MaskRef{getMask(mask.includes_map().value(), id), is_exclusion};
  }
  return MaskRef{getMask(mask.excludes_map().value(), id), !is_exclusion};
}

MaskRef MaskRef::get(const std::string& key) const {
  if (isAllMask() || isNoneMask()) { // This whole map is included or excluded.
    return *this;
  }
  throwIfNotStringMapMask();
  if (mask.includes_string_map()) {
    return MaskRef{
        getMask(mask.includes_string_map().value(), key), is_exclusion};
  }
  return MaskRef{
      getMask(mask.excludes_string_map().value(), key), !is_exclusion};
}
MaskRef MaskRef::get(std::string_view key) const {
  if (isAllMask() || isNoneMask()) { // This whole map is included or excluded.
    return *this;
  }
  throwIfNotStringMapMask();
  if (mask.includes_string_map()) {
    return MaskRef{
        getMask(mask.includes_string_map().value(), key), is_exclusion};
  }
  return MaskRef{
      getMask(mask.excludes_string_map().value(), key), !is_exclusion};
}

MaskRef MaskRef::get(const type::Type& type) const {
  if (isAllMask() || isNoneMask()) { // All types included or excluded.
    return *this;
  }
  throwIfNotTypeMask();
  if (mask.includes_type()) {
    return MaskRef{getMask(mask.includes_type().value(), type), is_exclusion};
  } else {
    return MaskRef{getMask(mask.excludes_type().value(), type), !is_exclusion};
  }
}

std::optional<MaskRef> MaskRef::tryGet(FieldId id) const {
  throwIfNotFieldMask();
  if (mask.includes()) {
    const auto* m = getMaskOrNull(mask.includes().value(), id);
    return m ? std::make_optional<MaskRef>(*m, is_exclusion) : std::nullopt;
  }
  const auto* m = getMaskOrNull(mask.excludes().value(), id);
  return m ? std::make_optional<MaskRef>(*m, !is_exclusion) : std::nullopt;
}

std::optional<MaskRef> MaskRef::tryGet(detail::MapId id) const {
  if (isAllMask() || isNoneMask()) { // This whole map is included or excluded.
    return *this;
  }
  throwIfNotIntegerMapMask();
  if (mask.includes_map()) {
    const auto* m = getMaskOrNull(mask.includes_map().value(), id);
    return m ? std::make_optional<MaskRef>(*m, is_exclusion) : std::nullopt;
  }
  const auto* m = getMaskOrNull(mask.excludes_map().value(), id);
  return m ? std::make_optional<MaskRef>(*m, !is_exclusion) : std::nullopt;
}

std::optional<MaskRef> MaskRef::tryGet(const std::string& key) const {
  if (isAllMask() || isNoneMask()) { // This whole map is included or excluded.
    return *this;
  }
  throwIfNotStringMapMask();
  if (mask.includes_string_map()) {
    const auto* m = getMaskOrNull(mask.includes_string_map().value(), key);
    return m ? std::make_optional<MaskRef>(*m, is_exclusion) : std::nullopt;
  }
  const auto* m = getMaskOrNull(mask.excludes_string_map().value(), key);
  return m ? std::make_optional<MaskRef>(*m, !is_exclusion) : std::nullopt;
}

std::optional<MaskRef> MaskRef::tryGet(const type::Type& type) const {
  if (isAllMask() || isNoneMask()) { // All types included or excluded.
    return *this;
  }
  throwIfNotTypeMask();
  if (mask.includes_type()) {
    const auto* m = getMaskOrNull(mask.includes_type().value(), type);
    return m ? std::make_optional<MaskRef>(*m, is_exclusion) : std::nullopt;
  }
  const auto* m = getMaskOrNull(mask.excludes_type().value(), type);
  return m ? std::make_optional<MaskRef>(*m, !is_exclusion) : std::nullopt;
}

// Uses type::identicalType to look up the nested MaskRef for the given type
// Use this if type can possibly be a hashed URI
MaskRef MaskRef::getViaIdenticalType_INTERNAL_DO_NOT_USE(
    const type::Type& type) const {
  if (type.isFull()) {
    // Optimize for the common case of a full type.
    return get(type);
  }
  if (isAllMask() || isNoneMask()) { // All types included or excluded.
    return *this;
  }
  throwIfNotTypeMask();
  auto findViaIdenticalType = [&](const auto& map) -> const Mask& {
    for (auto it = map.begin(); it != map.end(); ++it) {
      if (type::identicalType(it->first, type)) {
        return it->second;
      }
    }
    return field_mask_constants::noneMask();
  };

  if (mask.includes_type()) {
    return MaskRef{
        findViaIdenticalType(mask.includes_type().value()), is_exclusion};
  } else {
    return MaskRef{
        findViaIdenticalType(mask.excludes_type().value()), !is_exclusion};
  }
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

bool MaskRef::isAllTypeMask() const {
  return (is_exclusion &&
          ::apache::thrift::protocol::detail::isNoneTypeMask(mask)) ||
      (!is_exclusion &&
       ::apache::thrift::protocol::detail::isAllTypeMask(mask));
}

bool MaskRef::isNoneTypeMask() const {
  return (is_exclusion &&
          ::apache::thrift::protocol::detail::isAllTypeMask(mask)) ||
      (!is_exclusion &&
       ::apache::thrift::protocol::detail::isNoneTypeMask(mask));
}

bool MaskRef::isExclusive() const {
  return (mask.includes() && is_exclusion) ||
      (mask.excludes() && !is_exclusion) ||
      (mask.includes_map() && is_exclusion) ||
      (mask.excludes_map() && !is_exclusion) ||
      (mask.includes_string_map() && is_exclusion) ||
      (mask.excludes_string_map() && !is_exclusion) ||
      (mask.includes_type() && is_exclusion) ||
      (mask.excludes_type() && !is_exclusion);
}

bool MaskRef::isFieldMask() const {
  return mask.includes() || mask.excludes();
}

bool MaskRef::isMapMask() const {
  return isIntegerMapMask() || isStringMapMask();
}

bool MaskRef::isIntegerMapMask() const {
  return mask.includes_map() || mask.excludes_map();
}

bool MaskRef::isStringMapMask() const {
  return mask.includes_string_map() || mask.excludes_string_map();
}

bool MaskRef::isTypeMask() const {
  return mask.includes_type() || mask.excludes_type();
}

void MaskRef::clear(protocol::Value& value) const {
  if (isAllMask()) {
    clearValueInner(value);
    return;
  }
  if (value.is_object()) {
    this->clear(value.as_object());
    return;
  }
  if (value.is_map()) {
    this->clear(value.as_map());
    return;
  }
  folly::throw_exception<std::runtime_error>(
      "The mask and object are incompatible.");
}

void MaskRef::clear(protocol::Object& obj) const {
  if (isTypeMask()) {
    // obj -> thrift.Any!
    type::AnyData any;
    if (!detail::ProtocolValueToThriftValue<type::infer_tag<type::AnyData>>{}(
            obj, any)) {
      folly::throw_exception<std::runtime_error>(
          "Incompatible mask and data: expected Any for type-mask");
    };

    // The type of Any could be a hashed-URI so use getViaIdenticalType
    auto nestedMask = getViaIdenticalType_INTERNAL_DO_NOT_USE(any.type());
    if (!nestedMask.isNoneMask()) {
      // recurse
      auto value = detail::parseValueFromAny(any);
      nestedMask.clear(value);
      any = detail::toAny(value, any.type(), any.protocol());
      obj =
          detail::asValueStruct<type::infer_tag<type::AnyData>>(std::move(any))
              .as_object();
    }
    return;
  }
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

protocol::Value MaskRef::filter(const Value& src) const {
  protocol::Value ret;
  if (isAllMask()) {
    ret = src;
  } else if (src.is_object()) {
    ret.emplace_object(filter(src.as_object()));
  } else if (src.is_map()) {
    ret.emplace_map(filter(src.as_map()));
  } else {
    folly::throw_exception<std::runtime_error>(
        "The mask and object are incompatible.");
  }
  return ret;
}

protocol::Object MaskRef::filter(const protocol::Object& src) const {
  if (isTypeMask()) {
    // obj -> any
    type::AnyData any;
    if (!detail::ProtocolValueToThriftValue<type::infer_tag<type::AnyData>>{}(
            src, any)) {
      folly::throw_exception<std::runtime_error>(
          "Incompatible mask and data: expected Any for type-mask");
    };

    // The type of Any could be a hashed-URI so use getViaIdenticalType
    auto nestedMask = getViaIdenticalType_INTERNAL_DO_NOT_USE(any.type());
    if (nestedMask.isNoneMask()) {
      // Filter failed, return empty object
      return {};
    } else {
      // Recurse
      any = detail::toAny(
          nestedMask.filter(detail::parseValueFromAny(any)),
          any.type(),
          any.protocol());
    }

    return detail::asValueStruct<type::infer_tag<type::AnyData>>(std::move(any))
        .as_object();
  }

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
