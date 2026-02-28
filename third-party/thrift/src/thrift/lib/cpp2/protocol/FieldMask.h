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

#include <vector>

#include <folly/Range.h>
#include <thrift/lib/cpp2/protocol/FieldMaskRef.h>
#include <thrift/lib/cpp2/protocol/detail/FieldMask.h>
#include <thrift/lib/cpp2/protocol/detail/FieldMaskUtil.h>
#include <thrift/lib/cpp2/type/detail/Wrap.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_constants.h>
#include <thrift/lib/thrift/gen-cpp2/field_mask_types.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache::thrift::protocol {
inline constexpr auto allMask = field_mask_constants::allMask;
inline constexpr auto noneMask = field_mask_constants::noneMask;

// Constructs a new FieldMask that is reverse of the given mask.
Mask reverseMask(Mask mask);

// Removes masked fields in schemaless Thrift Object (Protocol Object).
// Throws a runtime exception if the mask and object are incompatible.
void clear(const Mask& mask, protocol::Object& t);

// Returns a new object that contains only the masked fields.
// Throws a runtime exception if the mask and objects are incompatible.
// Note: Masked structured (struct/union) fields will be pruned (i.e left unset)
// if no masked unstructued child fields exist in src.
protocol::Object filter(const Mask& mask, const protocol::Object& src);

/**
 * The API copy(protocol::Object, protocol::Object) is not provided here as
 * it can produce invalid outputs for union masks
 *
 * i.e. it's not possible to tell whether a given protocol::Object instance
 * is a struct or union - making it difficult to apply copy semantics
 */

// Returns whether field mask is compatible with thrift type tag.
//
// If it is a struct, it is incompatible if the mask contains a field that
// doesn't exist in the struct or that exists with a different type.
//
// If it is a map, it is incompatible only if mask is not a map mask, or any
// nested mask in the map mask is incompatible with mapped type.
//
// If it is not a struct or a map, it is only compatible when it is allMask or
// noneMask.
using detail::is_compatible_with;

// Ensures that the masked fields have value in the thrift struct.
// If it doesn't, it emplaces the field.
// Throws a runtime exception if the mask and struct are incompatible.
template <typename T>
void ensure(const Mask& mask, T& t) {
  static_assert(
      is_thrift_struct_v<T> || is_thrift_union_v<T>,
      "not a thrift struct or union");
  detail::throwIfContainsMapMask(mask);
  return detail::ensure_fields(MaskRef{mask, false}, t);
}

// Clears masked fields in the thrift struct.
// If the field doesn't have value, does nothing.
// Throws a runtime exception if the mask and struct are incompatible.
template <typename T>
void clear(const Mask& mask, T& t) {
  static_assert(
      is_thrift_struct_v<T> || is_thrift_union_v<T>,
      "not a thrift struct or union");
  detail::throwIfContainsMapMask(mask);
  return detail::clear_fields(MaskRef{mask, false}, t);
}

/**
 * Returns a new object that contains only the masked fields.
 * Throws a runtime exception if the mask and objects are incompatible.
 * Note: Masked structured (struct/union) fields will be pruned (i.e left unset
 * for optional/union fields, or set to default for unqualified fields) if no
 * masked unstructued child fields exist in src.
 *
 * Semantics for masks specified on data contained within thrift.Any:
 *  1. If the mask doesn't select the actual type contained within thrift.Any,
 * the returned object will be default inialized (i.e. cleared).
 *  2. If the mask selects the actual type contained within thrift.Any, but is a
 * noneMask, same behavior as (1).
 *  3. If the mask selects the actual type contained within thrift.Any, is a
 * non-noneMask, thrift.Any retains its type and the mask is used to filter the
 * contained data.
 */
template <typename T>
inline T filter(const Mask& mask, const T& src) {
  static_assert(
      is_thrift_struct_v<T> || is_thrift_union_v<T>,
      "not a thrift struct or union");
  detail::throwIfContainsMapMask(mask);
  T filtered;
  MaskRef mref{mask, false};
  detail::filter_fields(mref, src, filtered);
  return filtered;
}

// Logical operators that can construct a new mask
Mask operator&(const Mask&, const Mask&); // intersect
Mask operator|(const Mask&, const Mask&); // union
Mask operator-(const Mask&, const Mask&); // subtract

// MaskBuilder provides convenient type-safe APIs to use Thrift Mask easier for
// end-users to consturct and use. This converts Id/ field name to field id
// automatically. Id can be Ordinal, FieldId, Ident, TypeTag, and FieldTag.
//
// Example:
//   MaskBuilder<Struct> mask(allMask()); // start with allMask
//   mask.includes<id1, id2>(anotherMask);
//   mask.excludes({"fieldname"}, anotherMask);
//   mask.toThrift();  // --> reference to the underlying FieldMask.
template <typename T>
struct MaskBuilder : type::detail::Wrap<Mask> {
 private:
  using Tag = type::infer_tag<T>;

 public:
  static_assert(
      is_thrift_struct_v<T> || is_thrift_union_v<T>, "not a struct or union");
  MaskBuilder() { data_ = Mask{}; }
  /* implicit */ MaskBuilder(Mask mask) {
    detail::errorIfNotCompatible<Tag>(mask);
    data_ = std::move(mask);
  }

  MaskBuilder& reset_to_none() {
    data_ = noneMask();
    return *this;
  }

  MaskBuilder& reset_to_all() {
    data_ = allMask();
    return *this;
  }

  MaskBuilder& invert() {
    data_ = reverseMask(std::move(data_));
    return *this;
  }

  // Includes the field specified by the list of Ids/field names with
  // the given mask.
  // The field is t.field1().field2() ...
  // Throws runtime exception if the field doesn't exist.
  template <typename... Id>
  MaskBuilder& includes(const Mask& mask = allMask()) {
    data_ = data_ | detail::path<Tag, Id...>(mask);
    return *this;
  }
  MaskBuilder& includes(
      const std::vector<folly::StringPiece>& fieldNames,
      const Mask& mask = allMask()) {
    data_ = data_ | detail::path<Tag>(fieldNames, 0, mask);
    return *this;
  }

  template <typename... Id>
  MaskBuilder& includes_map_element(int64_t key, const Mask& mask = allMask()) {
    Mask map;
    map.includes_map().emplace()[key] = mask;
    return includes<Id...>(map);
  }

  template <typename... Id>
  MaskBuilder& includes_map_element(
      std::string key, const Mask& mask = allMask()) {
    Mask map;
    map.includes_string_map().emplace()[std::move(key)] = mask;
    return includes<Id...>(map);
  }

  template <typename... Id>
  MaskBuilder& includes_type(type::Type type, const Mask& mask = allMask()) {
    Mask typeMap;
    typeMap.includes_type().emplace().emplace(std::move(type), mask);
    return includes<Id...>(typeMap);
  }

  template <typename... Id, typename TypeTag>
  MaskBuilder& includes_type(TypeTag, const Mask& mask = allMask()) {
    detail::errorIfNotCompatible<TypeTag>(mask);
    return includes_type<Id...>(type::Type(TypeTag{}), mask);
  }

  // Excludes the field specified by the list of Ids/field names with
  // the given mask.
  // The field is t.field1().field2() ...
  // Throws runtime exception if the field doesn't exist.
  template <typename... Id>
  MaskBuilder& excludes(const Mask& mask = allMask()) {
    data_ = data_ - detail::path<Tag, Id...>(mask);
    return *this;
  }
  MaskBuilder& excludes(
      const std::vector<folly::StringPiece>& fieldNames,
      const Mask& mask = allMask()) {
    data_ = data_ - detail::path<Tag>(fieldNames, 0, mask);
    return *this;
  }

  template <typename... Id>
  MaskBuilder& excludes_map_element(int64_t key, const Mask& mask = allMask()) {
    Mask map;
    map.includes_map().emplace()[key] = mask;
    return excludes<Id...>(map);
  }

  template <typename... Id>
  MaskBuilder& excludes_map_element(
      std::string key, const Mask& mask = allMask()) {
    Mask map;
    map.includes_string_map().emplace()[std::move(key)] = mask;
    return excludes<Id...>(map);
  }

  template <typename... Id>
  MaskBuilder& excludes_type(type::Type type, const Mask& mask = allMask()) {
    Mask typeMap;
    typeMap.includes_type().emplace().emplace(std::move(type), mask);
    return excludes<Id...>(typeMap);
  }

  template <typename... Id, typename TypeTag>
  MaskBuilder& excludes_type(TypeTag, const Mask& mask = allMask()) {
    detail::errorIfNotCompatible<TypeTag>(mask);
    return excludes_type<Id...>(type::Type(TypeTag{}), mask);
  }

  // Mask APIs
  void ensure(T& obj) const { protocol::ensure(data_, obj); }
  void clear(T& obj) const { protocol::clear(data_, obj); }
  T filter(const T& src) const { return protocol::filter(data_, src); }
};

template <typename Struct>
using MaskAdapter = InlineAdapter<MaskBuilder<Struct>>;

// DynamicMaskBuilder provides convenient APIs to use Thrift Mask easier for
// end-users to consturct and use. Use MaskBuilder if possible for type-safety.
//
//  Example:
//   DynamicMaskBuilder mask(allMask()); // start with allMask
//   mask.includes({id1, id2}, anotherMask);
//   mask.excludes({id3}, anotherMask);
//   mask.toThrift();  // --> reference to the underlying FieldMask.
struct DynamicMaskBuilder : type::detail::Wrap<Mask> {
 public:
  DynamicMaskBuilder() { data_ = Mask{}; }
  /* implicit */ DynamicMaskBuilder(Mask mask) { data_ = std::move(mask); }

  DynamicMaskBuilder& reset_to_none() {
    data_ = noneMask();
    return *this;
  }

  DynamicMaskBuilder& reset_to_all() {
    data_ = allMask();
    return *this;
  }

  DynamicMaskBuilder& invert() {
    data_ = reverseMask(std::move(data_));
    return *this;
  }

  DynamicMaskBuilder& includes(
      const detail::FieldPath& path, const Mask& mask = allMask()) {
    data_ = data_ | detail::fieldPathToMask(path, mask);
    return *this;
  }

  DynamicMaskBuilder& includes_map_element(
      const detail::FieldPath& path,
      int64_t key,
      const Mask& mask = allMask()) {
    Mask map;
    map.includes_map().emplace()[key] = mask;
    return includes(path, map);
  }

  DynamicMaskBuilder& includes_map_element(
      const detail::FieldPath& path,
      std::string key,
      const Mask& mask = allMask()) {
    Mask map;
    map.includes_string_map().emplace().emplace(std::move(key), mask);
    return includes(path, map);
  }

  DynamicMaskBuilder& includes_type(
      const detail::FieldPath& path,
      type::Type type,
      const Mask& mask = allMask()) {
    Mask typeMap;
    typeMap.includes_type().emplace().emplace(std::move(type), mask);
    return includes(path, typeMap);
  }

  DynamicMaskBuilder& excludes(
      const detail::FieldPath& path, const Mask& mask = allMask()) {
    data_ = data_ - detail::fieldPathToMask(path, mask);
    return *this;
  }

  DynamicMaskBuilder& excludes_map_element(
      const detail::FieldPath& path,
      int64_t key,
      const Mask& mask = allMask()) {
    Mask map;
    map.includes_map().emplace()[key] = mask;
    return excludes(path, map);
  }

  DynamicMaskBuilder& excludes_map_element(
      const detail::FieldPath& path,
      std::string key,
      const Mask& mask = allMask()) {
    Mask map;
    map.includes_string_map().emplace().emplace(std::move(key), mask);
    return excludes(path, map);
  }

  DynamicMaskBuilder& excludes_type(
      const detail::FieldPath& path,
      type::Type type,
      const Mask& mask = allMask()) {
    Mask typeMap;
    typeMap.includes_type().emplace().emplace(std::move(type), mask);
    return excludes(path, typeMap);
  }

 private:
};

// Constructs a FieldMask object that includes the fields that are
// different in the given two Thrift structs/unions.
// TODO: support map mask
template <typename T>
Mask compare(const T& original, const T& modified) {
  static_assert(
      is_thrift_class_v<T> || is_thrift_union_v<T>,
      "not a thrift struct or union");
  Mask result;
  detail::compare_impl(original, modified, result.includes().emplace());
  return result;
}

} // namespace apache::thrift::protocol
