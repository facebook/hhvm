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

#include <functional>
#include <map>
#include <string_view>

#include <folly/CppAttributes.h>
#include <folly/container/F14Set.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_constants.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_types.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache::thrift::protocol::detail {
// TODO: replace MapId with ValueId
// Runtime and compile time representations for a map id.
enum class MapId : int64_t {};

// Thrift Map Mask supports integer and string key similar to
// https://docs.hhvm.com/hack/built-in-types/arraykey.
enum class ArrayKey {
  Integer = 0,
  String = 1,
};

// Returns mask == allMask() but faster
inline bool isAllMask(const Mask& mask) {
  return mask.excludes_ref() && mask.excludes_ref()->empty();
}

// Returns mask == noneMask() but faster
inline bool isNoneMask(const Mask& mask) {
  return mask.includes_ref() && mask.includes_ref()->empty();
}

// Checks whether it is ALL map mask.
inline bool isAllMapMask(const Mask& mask) {
  if (!mask.excludes_map_ref() && !mask.excludes_string_map_ref() &&
      !mask.includes_map_ref() && !mask.includes_string_map_ref()) {
    folly::throw_exception<std::runtime_error>("This is not a map mask.");
  }

  return (mask.excludes_map_ref() && mask.excludes_map_ref()->empty()) ||
      (mask.excludes_string_map_ref() &&
       mask.excludes_string_map_ref()->empty());
}

// Checks whether it is NONE map mask.
inline bool isNoneMapMask(const Mask& mask) {
  if (!mask.excludes_map_ref() && !mask.excludes_string_map_ref() &&
      !mask.includes_map_ref() && !mask.includes_string_map_ref()) {
    folly::throw_exception<std::runtime_error>("This is not a map mask.");
  }

  return (mask.includes_map_ref() && mask.includes_map_ref()->empty()) ||
      (mask.includes_string_map_ref() &&
       mask.includes_string_map_ref()->empty());
}

// If mask is a field mask, return it, otherwise return nullptr
[[nodiscard]] const FieldIdToMask* FOLLY_NULLABLE
getFieldMask(const Mask& mask);

// If mask is an integer map mask, return it, otherwise return nullptr
[[nodiscard]] const MapIdToMask* FOLLY_NULLABLE
getIntegerMapMask(const Mask& mask);

// If mask is a string map mask, return it, otherwise return nullptr
[[nodiscard]] const MapStringToMask* FOLLY_NULLABLE
getStringMapMask(const Mask& mask);

// Moves the given object to the field (can be field ref or smart pointer).
template <typename T, typename U>
void moveObject(T& field, U&& object) {
  if constexpr (thrift::detail::is_unique_ptr_v<T>) {
    field = std::make_unique<U>(std::forward<U>(object));
  } else if constexpr (thrift::detail::is_shared_ptr_v<T>) {
    field = std::make_shared<U>(std::forward<U>(object));
  } else {
    field = std::forward<U>(object);
  }
}

// Throws a runtime error if the mask contains a map mask.
void throwIfContainsMapMask(const Mask& mask);

template <typename Struct>
void compare_impl(
    const Struct& original, const Struct& modified, FieldIdToMask& mask) {
  op::for_each_field_id<Struct>([&](auto id) {
    using Id = decltype(id);
    int16_t fieldId = folly::to_underlying(id());
    auto&& original_field = op::get<Id>(original);
    auto&& modified_field = op::get<Id>(modified);
    auto* original_ptr = op::getValueOrNull(original_field);
    auto* modified_ptr = op::getValueOrNull(modified_field);

    if (!original_ptr && !modified_ptr) { // same
      return;
    }
    if (!original_ptr || !modified_ptr) { // different
      mask[fieldId] = field_mask_constants::allMask();
      return;
    }
    if (*original_ptr == *modified_ptr) { // same
      return;
    }
    // check if nested fields need to be added to mask
    using FieldType = op::get_native_type<Struct, Id>;
    if constexpr (is_thrift_struct_v<FieldType>) {
      compare_impl(
          *original_ptr, *modified_ptr, mask[fieldId].includes_ref().emplace());
      return;
    }
    // The values are different and not nested.
    mask[fieldId] = field_mask_constants::allMask();
  });
}

// Returns the ArrayKey of the given Value key. If the Value key contains
// a non-integer or non-string value, it throws.
ArrayKey getArrayKeyFromValue(const Value& v);

// Returns the MapId of the given Value key. If the Value key
// contains a non-integer valie, it throws.
MapId getMapIdFromValue(const Value& v);

// Returns the string of the given Value key. If the Value key contains a
// non-string value, it throws.
std::string getStringFromValue(const Value& v);

// Returns the MapId in map mask of the given Value key.
// If it doesn't exist, it returns the new MapId (pointer to the key).
// Assumes the map mask uses pointers to keys.
MapId findMapIdByValueAddress(const Mask& mask, const Value& key);

using ValueIndex = folly::F14FastSet<
    std::reference_wrapper<const Value>,
    std::hash<Value>,
    std::equal_to<Value>>;

// Indexes the values referred to by the given mask by their value. The address
// of the value is preserved by the index.
ValueIndex buildValueIndex(const Mask& mask);

} // namespace apache::thrift::protocol::detail
