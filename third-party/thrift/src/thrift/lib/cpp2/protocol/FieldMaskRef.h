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

#include <optional>
#include <string_view>

#include <thrift/lib/cpp2/protocol/detail/FieldMask.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache::thrift::protocol {
// MaskRef struct represents the Field Mask and whether the mask is coming from
// excludes mask. MaskRef is used for inputs and outputs for Field Mask
// methods to determine the status of the mask, because if the mask is coming
// from excludes mask, the mask actually represents the complement set.
class MaskRef {
 public:
  // TODO(dokwon): Consider requiring is_exclusion to explicitly specified in
  // the constructor.
  explicit MaskRef(const Mask& m) : mask(m), is_exclusion(false) {}
  MaskRef(const Mask& m, bool exclusion) : mask(m), is_exclusion(exclusion) {}

  const Mask& mask;
  bool is_exclusion; // Whether the mask comes from excludes mask

  // Get nested MaskRef with the given field id. If the id does not exist in the
  // field mask, it returns noneMask or allMask depending on whether the field
  // should be included.
  //
  // Throws a runtime exception if the mask is not a field mask.
  MaskRef get(FieldId id) const;

  // Get nested MaskRef with the given map id. If the id does not exist in the
  // integer map mask, it returns noneMask or allMask depending on whether the
  // field should be included.
  //
  // Throws a runtime exception if the mask is not an integer map mask.
  MaskRef get(detail::MapId id) const;

  // Get nested MaskRef with the given string key. If the string key does not
  // exist in the string map mask, it returns noneMask or allMask depending on
  // whether the field should be included.
  //
  // Throws a runtime exception if the mask is not a string map mask.
  MaskRef get(const std::string& key) const;
  MaskRef get(std::string_view key) const;

  // Get nested MaskRef for the given type. If the type does not exist in the
  // type mask, it returns noneMask or allMask depending on whether the field
  // should be included.
  //
  // Throws a runtime exception if the mask is not a type mask.
  MaskRef get(const type::Type& type) const;

  // Get nested MaskRef with the given field id. If the id does not exist in the
  // field mask, it returns empty optional.
  //
  // Throws a runtime exception if the mask is not a field mask.
  std::optional<MaskRef> tryGet(FieldId id) const;

  // Get nested MaskRef with the given map id. If the id does not exist in the
  // integer map mask, it returns empty optional.
  //
  // Throws a runtime exception if the mask is not an integer map mask.
  std::optional<MaskRef> tryGet(detail::MapId id) const;

  // Get nested MaskRef with the given string key. If the string key does not
  // exist in the string map mask, it returns empty optional.
  //
  // Throws a runtime exception if the mask is not a string map mask.
  std::optional<MaskRef> tryGet(const std::string& key) const;

  // Get nested MaskRef for the given type. If the type does not exist in the
  // type mask, it returns empty optional.
  //
  // Throws a runtime exception if the mask is not a type mask.
  std::optional<MaskRef> tryGet(const type::Type& type) const;

  // This API is reserved for internal use only.
  MaskRef getViaIdenticalType_INTERNAL_DO_NOT_USE(const type::Type& type) const;

  // Returns whether the ref includes all fields.
  bool isAllMask() const;

  // Returns whether the ref includes no fields.
  bool isNoneMask() const;

  // Returns whether the ref includes all map keys.
  //
  // Note, allMask is not considered here.
  bool isAllMapMask() const;

  // Returns whether the ref includes no map keys.
  //
  // Note, noneMask is not considered here.
  bool isNoneMapMask() const;

  // Returns whether the ref includes all type entries.
  //
  // Note, allMask is not considered here.
  bool isAllTypeMask() const;

  // Returns whether the ref includes no type entries.
  //
  // Note, noneMask is not considered here.
  bool isNoneTypeMask() const;

  // Returns whether the ref is logically exclusive in context.
  bool isExclusive() const;

  // Returns true if the mask is a field mask.
  //
  // Note, allMask and noneMask are also field masks.
  bool isFieldMask() const;

  // Returns true if the mask is a map mask.
  //
  // Note, allMask and noneMask are not a map mask.
  bool isMapMask() const;

  // Returns true if the mask is an integer map mask.
  //
  // Note, allMask and noneMask are not an integer map mask.
  bool isIntegerMapMask() const;

  // Returns true if the mask is a string map mask.
  //
  // Note, allMask and noneMask are not a string map mask.
  bool isStringMapMask() const;

  // Returns true if the mask is a type-map mask (i.e. AnyMask)
  //
  // Note, allMask and noneMask are not a type mask.
  bool isTypeMask() const;

  // Removes masked fields in schemaless Thrift Value
  // Throws a runtime exception if the mask and object are incompatible.
  void clear(protocol::Value& t) const;

  // Removes masked fields in schemaless Thrift Object (Protocol Object).
  // Throws a runtime exception if the mask and object are incompatible.
  void clear(protocol::Object& t) const;

  // Removes masked fields in a map value in schemaless Thrift Object.
  // Throws a runtime exception if the mask and object are incompatible.
  void clear(folly::F14FastMap<Value, Value>& map) const;

  /**
   * The API copy(protocol::Object, protocol::Object) is not provided here as
   * it can produce invalid outputs for union masks
   *
   * i.e. it's not possible to tell whether a given protocol::Object instance
   * is a struct or union - making it difficult to apply copy semantics
   */

  // Returns a new object that contains only the masked fields.
  // Throws a runtime exception if the mask and objects are incompatible.
  // See docblock on protocol::filter for exact semantics.
  protocol::Value filter(const protocol::Value& src) const;
  protocol::Object filter(const protocol::Object& src) const;
  folly::F14FastMap<Value, Value> filter(
      const folly::F14FastMap<Value, Value>& src) const;

  // Requires isFieldMask() == true
  // Returns the number of fields that are set in this mask.
  template <typename T>
  size_t numFieldsSet() {
    throwIfNotFieldMask();
    if (auto includes = mask.includes()) {
      return includes->size();
    } else {
      return op::num_fields<T> - mask.excludes()->size();
    }
  }

 private:
  void throwIfNotFieldMask() const;
  void throwIfNotMapMask() const;
  void throwIfNotIntegerMapMask() const;
  void throwIfNotStringMapMask() const;
  void throwIfNotTypeMask() const;
};
} // namespace apache::thrift::protocol
