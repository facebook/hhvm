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

#include <thrift/lib/cpp2/detail/FieldMask.h>
#include <thrift/lib/cpp2/type/detail/Wrap.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_constants.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_types.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache::thrift::protocol {
// FieldMask constants
inline const Mask& allMask() {
  return field_mask_constants::allMask();
}
inline const Mask& noneMask() {
  return field_mask_constants::noneMask();
}

// Constructs a new FieldMask that is reverse of the given mask.
Mask reverseMask(const Mask& mask);

// Removes masked fields in schemaless Thrift Object (Protocol Object).
// Throws a runtime exception if the mask and object are incompatible.
void clear(const Mask& mask, protocol::Object& t);

// Copies masked fields from one object to another (schemaless).
// If the masked field doesn't exist in src, the field in dst will be removed.
// Throws a runtime exception if the mask and objects are incompatible.
void copy(const Mask& mask, const protocol::Object& src, protocol::Object& dst);

// Returns whether field mask is compatible with thrift struct T.
// It is incompatible if the mask contains a field that doesn't exist in the
// struct or that exists with a different type.
template <typename T>
bool is_compatible_with(const Mask& mask) {
  static_assert(is_thrift_struct_v<T>, "not a thrift struct");
  detail::throwIfContainsMapMask(mask);
  detail::MaskRef ref{mask, false};
  if (ref.isAllMask() || ref.isNoneMask()) {
    return true;
  }
  return detail::validate_fields<T>(ref);
}

// Ensures that the masked fields have value in the thrift struct.
// If it doesn't, it emplaces the field.
// Throws a runtime exception if the mask and struct are incompatible.
template <typename T>
void ensure(const Mask& mask, T& t) {
  static_assert(is_thrift_struct_v<T>, "not a thrift struct");
  detail::throwIfContainsMapMask(mask);
  detail::errorIfNotCompatible<T>(mask);
  return detail::ensure_fields(detail::MaskRef{mask, false}, t);
}

// Clears masked fields in the thrift struct.
// If the field doesn't have value, does nothing.
// Throws a runtime exception if the mask and struct are incompatible.
template <typename T>
void clear(const Mask& mask, T& t) {
  static_assert(is_thrift_struct_v<T>, "not a thrift struct");
  detail::throwIfContainsMapMask(mask);
  detail::errorIfNotCompatible<T>(mask);
  return detail::clear_fields(detail::MaskRef{mask, false}, t);
}

// Copys masked fields from one thrift struct to another.
// If the masked field doesn't exist in src, the field in dst will be removed.
// Throws a runtime exception if the mask and objects are incompatible.
template <typename T>
void copy(const Mask& mask, const T& src, T& dst) {
  static_assert(is_thrift_struct_v<T>, "not a thrift struct");
  detail::throwIfContainsMapMask(mask);
  detail::errorIfNotCompatible<T>(mask);
  detail::copy_fields(detail::MaskRef{mask, false}, src, dst);
}

// Logical operators that can construct a new mask
Mask operator&(const Mask&, const Mask&); // intersect
Mask operator|(const Mask&, const Mask&); // union
Mask operator-(const Mask&, const Mask&); // subtract

// This converts ident/ field name to field id automatically which makes
// FieldMask easier for end-users to construct and use.
// Example:
//   MaskBuilder<T> mask(allMask()); // start with allMask
//   mask.includes<ident1, ident2>(anotherMask);
//   mask.excludes({"fieldname"}, anotherMask);
//   mask.toThrift();  // --> reference to the underlying FieldMask.

template <typename T>
struct MaskBuilder : type::detail::Wrap<Mask> {
  MaskBuilder() { data_ = Mask{}; }
  /* implicit */ MaskBuilder(Mask mask) { data_ = mask; }

  // Includes the field specified by the list of Idents/field names with
  // the given mask.
  // The field is t.field1().field2() ...
  // Throws runtime exception if the field doesn't exist.
  template <typename... Ident>
  void includes(const Mask& mask = allMask()) {
    data_ = data_ | detail::path<T, Ident...>(mask);
  }

  void includes(
      const std::vector<folly::StringPiece>& fieldNames,
      const Mask& mask = allMask()) {
    data_ = data_ | detail::path<T>(fieldNames, 0, mask);
  }

  // Excludes the field specified by the list of Idents/field names with
  // the given mask.
  // The field is t.field1().field2() ...
  // Throws runtime exception if the field doesn't exist.
  template <typename... Ident>
  void excludes(const Mask& mask = allMask()) {
    data_ = data_ - detail::path<T, Ident...>(mask);
  }

  void excludes(
      const std::vector<folly::StringPiece>& fieldNames,
      const Mask& mask = allMask()) {
    data_ = data_ - detail::path<T>(fieldNames, 0, mask);
  }
};

template <typename T>
using MaskAdapter = InlineAdapter<MaskBuilder<T>>;

// Constructs a FieldMask object that includes the fields that are
// different in the given two Thrift structs.
// TODO: support map mask
template <typename T>
Mask compare(const T& original, const T& modified) {
  static_assert(is_thrift_struct_v<T>, "not a thrift struct");
  Mask result;
  detail::compare_impl(original, modified, result.includes_ref().emplace());
  return result;
}

} // namespace apache::thrift::protocol
