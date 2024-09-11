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

#include <map>
#include <string_view>
#include <unordered_set>

#include <thrift/lib/cpp2/protocol/detail/FieldMask.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types.h>

namespace apache::thrift::protocol {
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
  MaskRef get(detail::MapId id) const;

  // Get nested MaskRef with the given string key. If the string key does not
  // exist in the map, it returns noneMask or allMask depending on whether the
  // field should be included. Throws a runtime exception if the mask is not a
  // map mask.
  MaskRef get(const std::string& key) const;

  // Get nested MaskRef for the given type. If the type does not exist in the
  // map, it returns noneMask or allMask depending on whether the field should
  // be included. Throws a runtime exception if the mask is not a type map mask.
  MaskRef get(const type::Type& type) const;

  // This API is reserved for internal use only.
  MaskRef getViaIdenticalType_INTERNAL_DO_NOT_USE(const type::Type& type) const;

  // Returns whether the ref includes all fields.
  bool isAllMask() const;

  // Returns whether the ref includes no fields.
  bool isNoneMask() const;

  // Returns whether the ref includes all fields.
  bool isAllMapMask() const;

  // Returns whether the ref includes no fields.
  bool isNoneMapMask() const;

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

  // Returns true if the mask is a type-map mask (i.e. AnyMask)
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
    if (auto includes = mask.includes_ref()) {
      return includes->size();
    } else {
      return op::size_v<T> - mask.excludes_ref()->size();
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
