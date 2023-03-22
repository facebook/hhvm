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

#include <string_view>

#include <folly/CppAttributes.h>
#include <folly/Traits.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Copy.h>
#include <thrift/lib/cpp2/op/Create.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_constants.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_types.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache::thrift::protocol::detail {
// TODO: replace MapId with ValueId
// Runtime and compile time representations for a map id.
enum class MapId : int64_t {};

// Returns mask == allMask() but faster
inline bool isAllMask(const Mask& mask) {
  return mask.excludes_ref() && mask.excludes_ref()->empty();
}

// Returns mask == noneMask() but faster
inline bool isNoneMask(const Mask& mask) {
  return mask.includes_ref() && mask.includes_ref()->empty();
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

// MaskRef struct represents the Field Mask and whether the mask is coming from
// excludes mask. MaskRef is used for inputs and outputs for Field Mask
// methods to determine the status of the mask, because if the mask is coming
// from excludes mask, the mask actually represents the complement set.
class MaskRef {
 public:
  const Mask& mask;
  bool is_exclusion = false; // Whether the mask comes from excludes mask

  // Get nested MaskRef with the given field id. If the id does not exist in the
  // map, it returns noneMask or allMask depending on whether the field should
  // be included.
  // Throws a runtime exception if the mask is not a field mask.
  MaskRef get(FieldId id) const;

  // Get nested MaskRef with the given map id. If the id does not exist in the
  // map, it returns noneMask or allMask depending on whether the field
  // should be included.
  // Throws a runtime exception if the mask is not a map mask.
  MaskRef get(MapId id) const;

  // Get nested MaskRef with the given string key. If the string key does not
  // exist in the map, it returns noneMask or allMask depending on whether the
  // field should be included. Throws a runtime exception if the mask is not a
  // map mask.
  MaskRef get(std::string_view key) const;

  // Returns whether the ref includes all fields.
  bool isAllMask() const;

  // Returns whether the ref includes no fields.
  bool isNoneMask() const;

  // Returns whether the ref is logically exclusive in context.
  bool isExclusive() const;

  // Returns true if the mask is a field mask.
  bool isFieldMask() const;

  // Returns true if the mask is a map mask.
  bool isMapMask() const;

  // Returns true if the mask is an integer map mask.
  bool isIntegerMapMask() const;

  // Returns true if the mask is a string map mask.
  bool isStringMapMask() const;

  // Removes masked fields in schemaless Thrift Object (Protocol Object).
  // Throws a runtime exception if the mask and object are incompatible.
  void clear(protocol::Object& t) const;

  // Removes masked fields in a map value in schemaless Thrift Object.
  // Throws a runtime exception if the mask and object are incompatible.
  void clear(std::map<Value, Value>& map) const;

  // Copies masked fields from one object to another (schemaless).
  // If the masked field doesn't exist in src, the field in dst will be removed.
  // Throws a runtime exception if the mask and objects are incompatible.
  void copy(const protocol::Object& src, protocol::Object& dst) const;
  void copy(
      const std::map<Value, Value>& src, std::map<Value, Value>& dst) const;

 private:
  // Gets all fields/ keys that need to be copied from src to dst.
  std::unordered_set<FieldId> getFieldsToCopy(
      const protocol::Object& src, const protocol::Object& dst) const;

  std::set<Value> getKeysToCopy(
      const std::map<Value, Value>& src,
      const std::map<Value, Value>& dst) const;

  void throwIfNotFieldMask() const;
  void throwIfNotMapMask() const;
  void throwIfNotIntegerMapMask() const;
  void throwIfNotStringMapMask() const;
};

// Validates the mask with the given Struct. Ensures that mask doesn't contain
// fields not in the Struct.
template <typename Struct>
bool validate_mask(MaskRef ref) {
  // Get the field ids in the thrift struct type.
  std::unordered_set<FieldId> ids;
  ids.reserve(op::size_v<Struct>);
  op::for_each_ordinal<Struct>(
      [&](auto ord) { ids.insert(op::get_field_id<Struct, decltype(ord)>()); });
  const FieldIdToMask& map = ref.mask.includes_ref()
      ? ref.mask.includes_ref().value()
      : ref.mask.excludes_ref().value();
  for (auto& [id, _] : map) {
    // Mask contains a field not in the struct.
    if (ids.find(FieldId{id}) == ids.end()) {
      return false;
    }
  }
  return true;
}

// Validates the fields in the Struct with the MaskRef.
template <typename Struct>
bool validate_fields(MaskRef ref) {
  if (!validate_mask<Struct>(ref)) {
    return false;
  }
  // Validates each field in the struct.
  bool isValid = true;
  op::for_each_ordinal<Struct>([&](auto ord) {
    if (!isValid) { // short circuit
      return;
    }
    using Ord = decltype(ord);
    MaskRef next = ref.get(op::get_field_id<Struct, Ord>());
    if (next.isAllMask() || next.isNoneMask()) {
      return;
    }
    // Check if the field is a thrift struct type. It uses native_type
    // as we don't support adapted struct fields in field mask.
    using FieldType = op::get_native_type<Ord, Struct>;
    if constexpr (is_thrift_struct_v<FieldType>) {
      // Need to validate the struct type.
      isValid &= validate_fields<FieldType>(next);
      return;
    }
    isValid = false;
  });
  return isValid;
}

template <typename Tag>
bool is_compatible_with(const Mask&);

template <typename Tag>
bool is_compatible_with_impl(Tag, const Mask&) {
  return false;
}

template <typename T>
bool is_compatible_with_impl(type::struct_t<T>, const Mask& mask) {
  return validate_fields<T>({mask});
}

template <typename Key, typename Value>
bool is_compatible_with_impl(type::map<Key, Value>, const Mask& mask) {
  // Map mask is compatible only if all nested masks are compatible with
  // `Value`.
  if (const auto* m = getIntegerMapMask(mask)) {
    return std::all_of(m->begin(), m->end(), [](const auto& pair) {
      return is_compatible_with<Value>(pair.second);
    });
  } else if (const auto* m = getStringMapMask(mask)) {
    return std::all_of(m->begin(), m->end(), [](const auto& pair) {
      return is_compatible_with<Value>(pair.second);
    });
  }
  return true;
}

template <typename Tag>
bool is_compatible_with(const Mask& mask) {
  if (isAllMask(mask) || isNoneMask(mask)) {
    return true;
  }
  return is_compatible_with_impl(Tag{}, mask);
}

// Throws an error if a thrift struct type is not compatible with the mask.
template <typename Tag>
void errorIfNotCompatible(const Mask& mask) {
  if (!::apache::thrift::protocol::detail::is_compatible_with<Tag>(mask)) {
    folly::throw_exception<std::runtime_error>(
        "The mask and struct are incompatible.");
  }
}

// Ensures the masked fields in the given thrift struct.
template <typename Struct>
void ensure_fields(MaskRef ref, Struct& t) {
  if (!validate_mask<Struct>(ref)) {
    folly::throw_exception<std::runtime_error>(
        "The mask and struct are incompatible.");
  }
  if constexpr (!std::is_const_v<std::remove_reference_t<Struct>>) {
    op::for_each_ordinal<Struct>([&](auto ord) {
      using Ord = decltype(ord);
      MaskRef next = ref.get(op::get_field_id<Struct, Ord>());
      if (next.isNoneMask()) {
        return;
      }
      using FieldTag = op::get_field_tag<Struct, Ord>;
      auto&& field_ref = op::get<Ord>(t);
      op::ensure<FieldTag>(field_ref, t);
      // Need to ensure the struct object.
      using FieldType = op::get_native_type<Ord, Struct>;
      if constexpr (is_thrift_struct_v<FieldType>) {
        auto& value = *op::getValueOrNull(field_ref);
        ensure_fields(next, value);
        return;
      }
      if (!next.isAllMask()) {
        folly::throw_exception<std::runtime_error>(
            "The mask and struct are incompatible.");
      }
    });
  } else {
    folly::throw_exception<std::runtime_error>("Cannot ensure a const object");
  }
}

// Clears the masked fields in the given thrift struct.
template <typename Struct>
void clear_fields(MaskRef ref, Struct& t) {
  if (!validate_mask<Struct>(ref)) {
    folly::throw_exception<std::runtime_error>(
        "The mask and struct are incompatible.");
  }
  if constexpr (!std::is_const_v<std::remove_reference_t<Struct>>) {
    op::for_each_ordinal<Struct>([&](auto ord) {
      using Ord = decltype(ord);
      MaskRef next = ref.get(op::get_field_id<Struct, Ord>());
      if (next.isNoneMask()) {
        return;
      }
      using FieldTag = op::get_field_tag<Struct, Ord>;
      auto&& field_ref = op::get<Ord>(t);
      if (next.isAllMask()) {
        op::clear_field<FieldTag>(field_ref, t);
        return;
      }
      using FieldType = op::get_native_type<Ord, Struct>;
      auto* field_value = op::getValueOrNull(field_ref);
      if (!field_value) {
        errorIfNotCompatible<op::get_type_tag<Struct, Ord>>(next.mask);
        return;
      }
      // Need to clear the struct object.
      if constexpr (is_thrift_struct_v<FieldType>) {
        clear_fields(next, *field_value);
        return;
      }
      folly::throw_exception<std::runtime_error>(
          "The mask and struct are incompatible.");
    });
  } else {
    folly::throw_exception<std::runtime_error>("Cannot clear a const object");
  }
}

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

// Copies the masked fields from src thrift struct to dst.
// Returns true if it copied a field from src to dst.
template <typename SrcStruct, typename DstStruct>
bool copy_fields(MaskRef ref, SrcStruct& src, DstStruct& dst) {
  static_assert(std::is_same_v<
                folly::remove_cvref_t<SrcStruct>,
                folly::remove_cvref_t<DstStruct>>);
  if (!validate_mask<DstStruct>(ref)) {
    folly::throw_exception<std::runtime_error>(
        "The mask and struct are incompatible.");
  }
  if constexpr (!std::is_const_v<std::remove_reference_t<DstStruct>>) {
    bool copied = false;
    op::for_each_ordinal<DstStruct>([&](auto ord) {
      using Ord = decltype(ord);
      MaskRef next = ref.get(op::get_field_id<DstStruct, Ord>());
      // Id doesn't exist in field mask, skip.
      if (next.isNoneMask()) {
        return;
      }
      using FieldTag = op::get_field_tag<DstStruct, Ord>;
      using FieldType = op::get_native_type<Ord, DstStruct>;
      auto&& src_ref = op::get<Ord>(src);
      auto&& dst_ref = op::get<Ord>(dst);
      bool srcHasValue = bool(op::getValueOrNull(src_ref));
      bool dstHasValue = bool(op::getValueOrNull(dst_ref));
      if (!srcHasValue && !dstHasValue) { // skip
        errorIfNotCompatible<op::get_type_tag<DstStruct, Ord>>(next.mask);
        return;
      }
      // Id that we want to copy.
      if (next.isAllMask()) {
        if (srcHasValue) {
          op::copy(src_ref, dst_ref);
          copied = true;
        } else {
          op::clear_field<FieldTag>(dst_ref, dst);
        }
        return;
      }
      if constexpr (is_thrift_struct_v<FieldType>) {
        // Field doesn't exist in src, so just clear dst with the mask.
        if (!srcHasValue) {
          clear_fields(next, *op::getValueOrNull(dst_ref));
          return;
        }
        // Field exists in both src and dst, so call copy recursively.
        if (dstHasValue) {
          copied |= copy_fields(
              next, *op::getValueOrNull(src_ref), *op::getValueOrNull(dst_ref));
          return;
        }
        // Field only exists in src. Need to construct object only if there's
        // a field to add.
        FieldType newObject;
        bool constructObject =
            copy_fields(next, *op::getValueOrNull(src_ref), newObject);
        if (constructObject) {
          moveObject(dst_ref, std::move(newObject));
          copied = true;
        }
        return;
      }
      folly::throw_exception<std::runtime_error>(
          "The mask and struct are incompatible.");
    });
    return copied;
  } else {
    folly::throw_exception<std::runtime_error>("Cannot copy to a const field");
  }
}

// This converts id list to a field mask with a single field.
template <typename Tag>
Mask path(const Mask& other) {
  // This is the base case as there is no more id.
  errorIfNotCompatible<Tag>(other);
  return other;
}

template <typename Tag, typename Id, typename... Ids>
Mask path(const Mask& other) {
  using Struct = type::native_type<Tag>;
  static_assert(is_thrift_struct_v<Struct>);
  Mask mask;
  using fieldId = op::get_field_id<Struct, Id>;
  static_assert(fieldId::value != FieldId{});
  mask.includes_ref().emplace()[static_cast<int16_t>(fieldId::value)] =
      path<op::get_type_tag<Struct, Id>, Ids...>(other);
  return mask;
}

// This converts field name list from the given index to a field mask with a
// single field.
template <typename Tag>
Mask path(
    const std::vector<folly::StringPiece>& fieldNames,
    size_t index,
    const Mask& other) {
  if (index == fieldNames.size()) {
    errorIfNotCompatible<Tag>(other);
    return other;
  }
  // static_assert doesn't work as it compiles this code for every field.
  using Struct = type::native_type<Tag>;
  if constexpr (is_thrift_struct_v<Struct>) {
    Mask mask;
    op::for_each_field_id<Struct>([&](auto id) {
      using Id = decltype(id);
      if (mask.includes_ref()) { // already set
        return;
      }
      if (op::get_name_v<Struct, Id> == fieldNames[index]) {
        mask.includes_ref().emplace()[folly::to_underlying(id())] =
            path<op::get_type_tag<Struct, Id>>(fieldNames, index + 1, other);
      }
    });
    if (!mask.includes_ref()) { // field not found
      folly::throw_exception<std::runtime_error>("field doesn't exist");
    }
    return mask;
  }
  folly::throw_exception<std::runtime_error>(
      "Path contains a non thrift struct field.");
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
    using FieldType = op::get_native_type<Id, Struct>;
    if constexpr (is_thrift_struct_v<FieldType>) {
      compare_impl(
          *original_ptr, *modified_ptr, mask[fieldId].includes_ref().emplace());
      return;
    }
    // The values are different and not nested.
    mask[fieldId] = field_mask_constants::allMask();
  });
}

// Returns the MapId in map mask of the given Value key.
// If it doesn't exist, it returns the new MapId (pointer to the key).
// Assumes the map mask uses pointers to keys.
MapId findMapIdByValue(const Mask& mask, const Value& key);
} // namespace apache::thrift::protocol::detail
