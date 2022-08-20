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

#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Copy.h>
#include <thrift/lib/cpp2/op/Ensure.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_constants.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_types.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache::thrift::protocol::detail {
// TODO: replace MapId with ValueId
// Runtime and compile time representations for a map id.
enum class MapId : int64_t {};

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
      const std::map<Value, Value>& src, std::map<Value, Value>& dst) const;

  void throwIfNotFieldMask() const;
  void throwIfNotMapMask() const;
};

// Throws an error if a thrift struct type is not compatible with the mask.
// TODO(aoka): Check compatibility in ensure, clear, and copy methods.
template <typename T>
void errorIfNotCompatible(const Mask& mask) {
  if (!is_compatible_with<T>(mask)) {
    throw std::runtime_error("The mask and struct are incompatible.");
  }
}

// Validates the fields in the Struct with the MaskRef.
template <typename Struct>
bool validate_fields(MaskRef ref) {
  // Get the field ids in the thrift struct type.
  std::unordered_set<FieldId> ids;
  ids.reserve(op::size_v<Struct>);
  op::for_each_ordinal<Struct>([&](auto fieldOrdinalTag) {
    ids.insert(op::get_field_id<Struct, decltype(fieldOrdinalTag)>());
  });
  const FieldIdToMask& map = ref.mask.includes_ref()
      ? ref.mask.includes_ref().value()
      : ref.mask.excludes_ref().value();
  for (auto& [id, _] : map) {
    // Mask contains a field not in the struct.
    if (!ids.contains(FieldId{id})) {
      return false;
    }
  }
  // Validates each field in the struct.
  bool isValid = true;
  op::for_each_ordinal<Struct>([&](auto fieldOrdinalTag) {
    if (!isValid) { // short circuit
      return;
    }
    using OrdinalTag = decltype(fieldOrdinalTag);
    MaskRef next = ref.get(op::get_field_id<Struct, OrdinalTag>());
    if (next.isAllMask() || next.isNoneMask()) {
      return;
    }
    // Check if the field is a thrift struct type. It uses native_type
    // as we don't support adapted struct fields in field mask.
    using FieldType = op::get_native_type<Struct, OrdinalTag>;
    if constexpr (is_thrift_struct_v<FieldType>) {
      // Need to validate the struct type.
      isValid &= detail::validate_fields<FieldType>(next);
      return;
    }
    isValid = false;
  });
  return isValid;
}

// Ensures the masked fields in the given thrift struct.
template <typename T>
void ensure_fields(MaskRef ref, T& t) {
  static_assert(
      !std::is_const_v<std::remove_reference_t<T>>,
      "Cannot clear a const object.");
  op::for_each_ordinal<T>([&](auto fieldOrdinalTag) {
    using OrdinalTag = decltype(fieldOrdinalTag);
    MaskRef next = ref.get(op::get_field_id<T, OrdinalTag>());
    if (next.isNoneMask()) {
      return;
    }
    using FieldTag = op::get_field_tag<T, OrdinalTag>;
    auto&& field_ref = op::get<T, OrdinalTag>(t);
    op::ensure<FieldTag>(field_ref, t);
    // Need to ensure the struct object.
    using FieldType = op::get_native_type<T, OrdinalTag>;
    if constexpr (is_thrift_struct_v<FieldType>) {
      auto& value = *op::get_value_or_null(field_ref);
      ensure_fields(next, value);
    }
  });
}

// Clears the masked fields in the given thrift struct.
template <typename T>
void clear_fields(MaskRef ref, T& t) {
  static_assert(
      !std::is_const_v<std::remove_reference_t<T>>,
      "Cannot clear a const object.");
  op::for_each_ordinal<T>([&](auto fieldOrdinalTag) {
    using OrdinalTag = decltype(fieldOrdinalTag);
    MaskRef next = ref.get(op::get_field_id<T, OrdinalTag>());
    if (next.isNoneMask()) {
      return;
    }
    using FieldTag = op::get_field_tag<T, OrdinalTag>;
    auto&& field_ref = op::get<T, OrdinalTag>(t);
    if (next.isAllMask()) {
      op::clear_field<FieldTag>(field_ref, t);
      return;
    }
    auto* field_value = op::get_value_or_null(field_ref);
    if (!field_value) {
      return;
    }
    // Need to clear the struct object.
    using FieldType = op::get_native_type<T, OrdinalTag>;
    if constexpr (is_thrift_struct_v<FieldType>) {
      clear_fields(next, *field_value);
    }
  });
}

// Moves the given object to the field (can be field ref or smart pointer).
template <typename T, typename U>
void moveObject(T& field, U&& object) {
  if constexpr (thrift::detail::is_shared_or_unique_ptr_v<T>) {
    *field = std::forward<U>(object);
  } else {
    field = std::forward<U>(object);
  }
}

// Copies the masked fields from src thrift struct to dst.
// Returns true if it copied a field from src to dst.
template <typename T>
bool copy_fields(MaskRef ref, const T& src, T& dst) {
  static_assert(
      !std::is_const_v<std::remove_reference_t<T>>,
      "Cannot clear a const object.");
  bool copied = false;
  op::for_each_ordinal<T>([&](auto fieldOrdinalTag) {
    using OrdinalTag = decltype(fieldOrdinalTag);
    MaskRef next = ref.get(op::get_field_id<T, OrdinalTag>());
    // Id doesn't exist in field mask, skip.
    if (next.isNoneMask()) {
      return;
    }
    using FieldTag = op::get_field_tag<T, OrdinalTag>;
    auto&& src_ref = op::get<T, OrdinalTag>(src);
    auto&& dst_ref = op::get<T, OrdinalTag>(dst);
    bool srcHasValue = bool(op::get_value_or_null(src_ref));
    bool dstHasValue = bool(op::get_value_or_null(dst_ref));
    if (!srcHasValue && !dstHasValue) { // skip
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
    using FieldType = op::get_native_type<T, OrdinalTag>;
    if constexpr (is_thrift_struct_v<FieldType>) {
      // Field doesn't exist in src, so just clear dst with the mask.
      if (!srcHasValue) {
        clear_fields(next, *op::get_value_or_null(dst_ref));
        return;
      }
      // Field exists in both src and dst, so call copy recursively.
      if (dstHasValue) {
        copied |= copy_fields(
            next,
            *op::get_value_or_null(src_ref),
            *op::get_value_or_null(dst_ref));
        return;
      }
      // Field only exists in src. Need to construct object only if there's
      // a field to add.
      FieldType newObject;
      bool constructObject =
          copy_fields(next, *op::get_value_or_null(src_ref), newObject);
      if (constructObject) {
        moveObject(dst_ref, std::move(newObject));
        copied = true;
      }
    }
  });
  return copied;
}

// This converts ident list to a field mask with a single field.
template <typename T>
Mask path(const Mask& other) {
  // This is the base case as there is no more ident.
  return other;
}

template <typename T, typename Ident, typename... Idents>
Mask path(const Mask& other) {
  if constexpr (is_thrift_struct_v<T>) {
    Mask mask;
    using fieldId = op::get_field_id<T, Ident>;
    static_assert(fieldId::value != FieldId{});
    using FieldType = op::get_native_type<T, Ident>;
    mask.includes_ref().emplace()[static_cast<int16_t>(fieldId::value)] =
        path<FieldType, Idents...>(other);
    return mask;
  }
  throw std::runtime_error("field doesn't exist");
}

// Throws a runtime error if the mask contains a map mask.
void throwIfContainsMapMask(const Mask& mask);

template <typename T>
void compare_impl(const T& original, const T& modified, FieldIdToMask& mask) {
  op::for_each_field_id<T>([&](auto fieldIdTag) {
    using FieldIdTag = decltype(fieldIdTag);
    int16_t fieldId = static_cast<int16_t>(fieldIdTag());
    auto&& original_field = op::get<T, FieldIdTag>(original);
    auto&& modified_field = op::get<T, FieldIdTag>(modified);
    auto* original_ptr = op::get_value_or_null(original_field);
    auto* modified_ptr = op::get_value_or_null(modified_field);

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
    using FieldType = op::get_native_type<T, FieldIdTag>;
    if constexpr (is_thrift_struct_v<FieldType>) {
      compare_impl(
          *original_ptr, *modified_ptr, mask[fieldId].includes_ref().emplace());
      return;
    }
    // The values are different and not nested.
    mask[fieldId] = field_mask_constants::allMask();
  });
}
} // namespace apache::thrift::protocol::detail
