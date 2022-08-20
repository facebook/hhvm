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

#include <thrift/lib/cpp2/FieldMask.h>

using apache::thrift::protocol::FieldIdToMask;

namespace apache::thrift::protocol {

Mask reverseMask(const Mask& mask) {
  detail::throwIfContainsMapMask(mask);
  Mask reverse;
  if (mask.includes_ref()) {
    reverse.excludes_ref() = mask.includes_ref().value();
  } else {
    reverse.includes_ref() = mask.excludes_ref().value();
  }
  return reverse;
}

void clear(const Mask& mask, protocol::Object& obj) {
  (detail::MaskRef{mask, false}).clear(obj);
}

void copy(
    const Mask& mask, const protocol::Object& src, protocol::Object& dst) {
  (detail::MaskRef{mask, false}).copy(src, dst);
}

void insertIfNotNoneMask(
    FieldIdToMask& map, int16_t fieldId, const Mask& mask) {
  // This doesn't use isNoneMask() as we just want to remove includes{} mask
  // rather than masks that logically contain no fields.
  if (mask != noneMask()) {
    map[fieldId] = mask;
  }
}

// Implementation of FieldMask's Logical Operators
// -----------------------------------------------
// To implement the logical operators for Field Mask while keeping the code
// manageable is to assume the Field Mask is not complement first, and apply
// boolean algebra such as De Morgan's laws to handle all cases.
// Logical operators for FieldMask easily by applying laws of boolean algebra.
// If the result is a complement set, we can set that to excludes mask.
// P and Q are the set of fields in the mask for lhs and rhs.

// Intersect
// - lhs = includes, rhs = includes : P·Q
// - lhs = includes, rhs = excludes : P·~Q​=P-Q
// - lhs = excludes, rhs = includes: ~P·Q=Q-P
// - lhs = excludes, rhs = excludes: ~P·~Q​=~(P+Q)​
// Union
// - lhs = includes, rhs = includes : P+Q
// - lhs = includes, rhs = excludes : P+~Q​=~(~P·Q)​=~(Q-P)​
// - lhs = excludes, rhs = includes: ~P+Q=~(P·~Q)​​=~(P-Q)​
// - lhs = excludes, rhs = excludes: ~P+~Q​=~(P·Q)​
// Subtract
// - lhs = includes, rhs = includes : P-Q
// - lhs = includes, rhs = excludes : P-~Q​=P·Q
// - lhs = excludes, rhs = includes: ~P-Q=~P·~Q​=~(P+Q)​
// - lhs = excludes, rhs = excludes: ~P-~Q​=(~P·Q)=Q/P

// Returns the intersection, union, or subtraction of the given FieldIdToMasks.
// This basically treats the maps as inclusive sets.
FieldIdToMask intersectMask(
    const FieldIdToMask& lhs, const FieldIdToMask& rhs) {
  FieldIdToMask map;
  for (auto& [fieldId, lhsMask] : lhs) {
    if (!rhs.contains(fieldId)) { // Only lhs contains the field.
      continue;
    }
    // Both maps have the field, so the mask is their intersection.
    insertIfNotNoneMask(map, fieldId, lhsMask & rhs.at(fieldId));
  }
  return map;
}
FieldIdToMask unionMask(const FieldIdToMask& lhs, const FieldIdToMask& rhs) {
  FieldIdToMask map;
  for (auto& [fieldId, lhsMask] : lhs) {
    if (!rhs.contains(fieldId)) { // Only lhs contains the field.
      insertIfNotNoneMask(map, fieldId, lhsMask);
      continue;
    }
    // Both maps have the field, so the mask is their union.
    insertIfNotNoneMask(map, fieldId, lhsMask | rhs.at(fieldId));
  }
  for (auto& [fieldId, rhsMask] : rhs) {
    if (!lhs.contains(fieldId)) { // Only rhs contains the field.
      insertIfNotNoneMask(map, fieldId, rhsMask);
    }
  }
  return map;
}
FieldIdToMask subtractMask(const FieldIdToMask& lhs, const FieldIdToMask& rhs) {
  FieldIdToMask map;
  for (auto& [fieldId, lhsMask] : lhs) {
    if (!rhs.contains(fieldId)) { // Only lhs contains the field.
      insertIfNotNoneMask(map, fieldId, lhsMask);
      continue;
    }
    // Both maps have the field, so the mask is their subtraction.
    insertIfNotNoneMask(map, fieldId, lhsMask - rhs.at(fieldId));
  }
  return map;
}

Mask createIncludesMask(FieldIdToMask&& map) {
  Mask mask;
  mask.includes_ref() = map;
  return mask;
}
Mask createExcludesMask(FieldIdToMask&& map) {
  Mask mask;
  mask.excludes_ref() = map;
  return mask;
}

Mask operator&(const Mask& lhs, const Mask& rhs) {
  detail::throwIfContainsMapMask(lhs);
  detail::throwIfContainsMapMask(rhs);
  if (lhs.includes_ref()) {
    if (rhs.includes_ref()) { // lhs=includes rhs=includes
      return createIncludesMask(intersectMask(
          lhs.includes_ref().value(), rhs.includes_ref().value()));
    }
    // lhs=includes rhs=excludes
    return createIncludesMask(
        subtractMask(lhs.includes_ref().value(), rhs.excludes_ref().value()));
  }
  if (rhs.includes_ref()) { // lhs=excludes rhs=includes
    return createIncludesMask(
        subtractMask(rhs.includes_ref().value(), lhs.excludes_ref().value()));
  }
  // lhs=excludes rhs=excludes
  return createExcludesMask(
      unionMask(lhs.excludes_ref().value(), rhs.excludes_ref().value()));
}
Mask operator|(const Mask& lhs, const Mask& rhs) {
  detail::throwIfContainsMapMask(lhs);
  detail::throwIfContainsMapMask(rhs);
  if (lhs.includes_ref()) {
    if (rhs.includes_ref()) { // lhs=includes rhs=includes
      return createIncludesMask(
          unionMask(lhs.includes_ref().value(), rhs.includes_ref().value()));
    }
    // lhs=includes rhs=excludes
    return createExcludesMask(
        subtractMask(rhs.excludes_ref().value(), lhs.includes_ref().value()));
  }
  if (rhs.includes_ref()) { // lhs=excludes rhs=includes
    return createExcludesMask(
        subtractMask(lhs.excludes_ref().value(), rhs.includes_ref().value()));
  }
  // lhs=excludes rhs=excludes
  return createExcludesMask(
      intersectMask(lhs.excludes_ref().value(), rhs.excludes_ref().value()));
}
Mask operator-(const Mask& lhs, const Mask& rhs) {
  detail::throwIfContainsMapMask(lhs);
  detail::throwIfContainsMapMask(rhs);
  if (lhs.includes_ref()) {
    if (rhs.includes_ref()) { // lhs=includes rhs=includes
      return createIncludesMask(
          subtractMask(lhs.includes_ref().value(), rhs.includes_ref().value()));
    }
    // lhs=includes rhs=excludes
    return createIncludesMask(
        intersectMask(lhs.includes_ref().value(), rhs.excludes_ref().value()));
  }
  if (rhs.includes_ref()) { // lhs=excludes rhs=includes
    return createExcludesMask(
        unionMask(lhs.excludes_ref().value(), rhs.includes_ref().value()));
  }
  // lhs=excludes rhs=excludes
  return createIncludesMask(
      subtractMask(rhs.excludes_ref().value(), lhs.excludes_ref().value()));
}
} // namespace apache::thrift::protocol
