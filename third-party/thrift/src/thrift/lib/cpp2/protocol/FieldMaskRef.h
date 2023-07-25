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

  // Removes masked fields in schemaless Thrift Object (Protocol Object).
  // Throws a runtime exception if the mask and object are incompatible.
  void clear(protocol::Object& t) const;

  // Removes masked fields in a map value in schemaless Thrift Object.
  // Throws a runtime exception if the mask and object are incompatible.
  void clear(folly::F14FastMap<Value, Value>& map) const;

  // Copies masked fields from one object to another (schemaless).
  // If the masked field doesn't exist in src, the field in dst will be removed.
  // Throws a runtime exception if the mask and objects are incompatible.
  void copy(const protocol::Object& src, protocol::Object& dst) const;
  void copy(
      const folly::F14FastMap<Value, Value>& src,
      folly::F14FastMap<Value, Value>& dst) const;

 private:
  // Gets all fields/ keys that need to be copied from src to dst.
  std::unordered_set<FieldId> getFieldsToCopy(
      const protocol::Object& src, const protocol::Object& dst) const;

  std::set<std::reference_wrapper<const Value>, std::less<Value>> getKeysToCopy(
      const folly::F14FastMap<Value, Value>& src,
      const folly::F14FastMap<Value, Value>& dst) const;

  void throwIfNotFieldMask() const;
  void throwIfNotMapMask() const;
  void throwIfNotIntegerMapMask() const;
  void throwIfNotStringMapMask() const;
};
} // namespace apache::thrift::protocol
