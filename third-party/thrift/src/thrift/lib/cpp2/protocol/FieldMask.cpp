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

#include <folly/MapUtil.h>
#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/protocol/FieldMask.h>

using apache::thrift::protocol::FieldIdToMask;

namespace apache::thrift::protocol {

Mask reverseMask(Mask mask) {
  switch (mask.getType()) {
    case Mask::Type::includes: {
      // We need to move the data to temporary variable since the mask is a
      // union, we can't move it from one field to another directly.
      auto tmp = std::move(mask.includes().value());
      mask.excludes() = std::move(tmp);
      return mask;
    }
    case Mask::Type::excludes: {
      auto tmp = std::move(mask.excludes().value());
      mask.includes() = std::move(tmp);
      return mask;
    }
    case Mask::Type::includes_map: {
      auto tmp = std::move(mask.includes_map().value());
      mask.excludes_map() = std::move(tmp);
      return mask;
    }
    case Mask::Type::excludes_map: {
      auto tmp = std::move(mask.excludes_map().value());
      mask.includes_map() = std::move(tmp);
      return mask;
    }
    case Mask::Type::includes_string_map: {
      auto tmp = std::move(mask.includes_string_map().value());
      mask.excludes_string_map() = std::move(tmp);
      return mask;
    }
    case Mask::Type::excludes_string_map: {
      auto tmp = std::move(mask.excludes_string_map().value());
      mask.includes_string_map() = std::move(tmp);
      return mask;
    }
    case Mask::Type::includes_type: {
      auto tmp = std::move(mask.includes_type().value());
      mask.excludes_type() = std::move(tmp);
      return mask;
    }
    case Mask::Type::excludes_type: {
      auto tmp = std::move(mask.excludes_type().value());
      mask.includes_type() = std::move(tmp);
      return mask;
    }
    case Mask::Type::__EMPTY__:
      folly::throw_exception<std::runtime_error>("Can not reverse empty masks");
  }
  // This should be never reached.
  std::terminate();
}

void clear(const Mask& mask, protocol::Object& obj) {
  (MaskRef{mask, false}).clear(obj);
}

protocol::Object filter(const Mask& mask, const protocol::Object& src) {
  return (MaskRef{mask, false}).filter(src);
}

namespace {

template <class Map, class Key>
void insertIfNotNoneMask(Map& map, Key key, const Mask& mask) {
  // This doesn't use isNoneMask() as we just want to remove includes{} mask
  // rather than masks that logically contain no fields.
  if (!detail::isNoneMask(mask)) {
    map[key] = mask;
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

// Returns the intersection, union, or subtraction of the given maps.
// This basically treats the maps as inclusive sets.
struct IntersectMaskImpl {
  template <class Map>
  Map operator()(const Map& lhs, const Map& rhs) const {
    Map map;
    for (auto& [id, lhsMask] : lhs) {
      auto* p = folly::get_ptr(rhs, id);
      // Only lhs contains the id.
      if (p == nullptr) {
        continue;
      }
      // Both maps have the id, so the mask is their intersection.
      insertIfNotNoneMask(map, id, lhsMask & *p);
    }
    return map;
  }
};

struct UnionMaskImpl {
  template <class Map>
  Map operator()(const Map& lhs, const Map& rhs) const {
    Map map;
    for (auto& [id, lhsMask] : lhs) {
      auto* p = folly::get_ptr(rhs, id);
      // Only lhs contains the id.
      if (p == nullptr) {
        insertIfNotNoneMask(map, id, lhsMask);
        continue;
      }
      // Both maps have the id, so the mask is their union.
      insertIfNotNoneMask(map, id, lhsMask | *p);
    }
    for (auto& [id, rhsMask] : rhs) {
      if (lhs.find(id) == lhs.end()) { // Only rhs contains the id.
        insertIfNotNoneMask(map, id, rhsMask);
      }
    }
    return map;
  }
};

struct SubtractMaskImpl {
  template <class Map>
  Map operator()(const Map& lhs, const Map& rhs) const {
    Map map;
    for (auto& [id, lhsMask] : lhs) {
      auto* p = folly::get_ptr(rhs, id);
      // Only lhs contains the id.
      if (p == nullptr) {
        insertIfNotNoneMask(map, id, lhsMask);
        continue;
      }
      // Both maps have the id, so the mask is their subtraction.
      insertIfNotNoneMask(map, id, lhsMask - *p);
    }
    return map;
  }
};

const FieldIdToMask& toFieldMask(const Mask& mask) {
  if (const auto* fieldMask = detail::getFieldMask(mask)) {
    return *fieldMask;
  }

  folly::throw_exception<std::runtime_error>("Incompatible masks");
}

const MapStringToMask& toStringMapMask(const Mask& mask) {
  using detail::getStringMapMask;

  if (const auto* mapMask = getStringMapMask(mask)) {
    return *mapMask;
  }

  if (detail::isAllMask(mask) || detail::isNoneMask(mask)) {
    static const MapStringToMask map;
    return map;
  }

  // The mask can't be applied to a string map since it's not a string map mask
  // or allMask/noneMask
  folly::throw_exception<std::runtime_error>("Incompatible masks");
}

const MapIdToMask& toIntegerMapMask(const Mask& mask) {
  using detail::getIntegerMapMask;

  if (const auto* mapMask = getIntegerMapMask(mask)) {
    return *mapMask;
  }

  if (detail::isAllMask(mask) || detail::isNoneMask(mask)) {
    static const MapIdToMask map;
    return map;
  }

  // The mask can't be applied to an integer map since it's not an integer map
  // mask or allMask/noneMask
  folly::throw_exception<std::runtime_error>("Incompatible masks");
}

const MapTypeToMask& toTypeMask(const Mask& mask) {
  if (const auto* mapMask = detail::getTypeMask(mask)) {
    return *mapMask;
  }

  if (detail::isAllMask(mask) || detail::isNoneMask(mask)) {
    static const MapTypeToMask map;
    return map;
  }

  // The mask can't be applied to Any since it's not a type mask or
  // allMask/noneMask
  folly::throw_exception<std::runtime_error>("Incompatible masks");
}

template <class Func>
Mask apply(const Mask& lhs, const Mask& rhs, Func&& func) {
  using detail::getFieldMask;
  using detail::getIntegerMapMask;
  using detail::getStringMapMask;
  using detail::getTypeMask;

  Mask mask;

  if (getStringMapMask(lhs) || getStringMapMask(rhs)) {
    mask.includes_string_map() =
        func(toStringMapMask(lhs), toStringMapMask(rhs));
    return mask;
  }

  // If one of them is an integer map mask, the other one must be either integer
  // map mask, or allMask/noneMask which either mask the whole integer map, or
  // nothing.
  if (getIntegerMapMask(lhs) || getIntegerMapMask(rhs)) {
    mask.includes_map() = func(toIntegerMapMask(lhs), toIntegerMapMask(rhs));
    return mask;
  }

  if (getTypeMask(lhs) || getTypeMask(rhs)) {
    mask.includes_type() = func(toTypeMask(lhs), toTypeMask(rhs));
    return mask;
  }

  mask.includes() = func(toFieldMask(lhs), toFieldMask(rhs));
  return mask;
}

Mask intersectMask(const Mask& lhs, const Mask& rhs) {
  return apply(lhs, rhs, IntersectMaskImpl{});
}

Mask unionMask(const Mask& lhs, const Mask& rhs) {
  return apply(lhs, rhs, UnionMaskImpl{});
}

Mask subtractMask(const Mask& lhs, const Mask& rhs) {
  return apply(lhs, rhs, SubtractMaskImpl{});
}

} // namespace

Mask operator&(const Mask& lhs, const Mask& rhs) {
  if (detail::isAllMask(lhs) || detail::isNoneMask(rhs)) {
    return rhs;
  }
  if (detail::isNoneMask(lhs) || detail::isAllMask(rhs)) {
    return lhs;
  }

  if (detail::isInclusive(lhs)) {
    if (detail::isInclusive(rhs)) { // lhs=includes rhs=includes
      return intersectMask(lhs, rhs);
    }
    // lhs=includes rhs=excludes
    return subtractMask(lhs, rhs);
  }
  if (detail::isInclusive(rhs)) { // lhs=excludes rhs=includes
    return subtractMask(rhs, lhs);
  }
  // lhs=excludes rhs=excludes
  return reverseMask(unionMask(lhs, rhs));
}
Mask operator|(const Mask& lhs, const Mask& rhs) {
  if (detail::isAllMask(lhs) || detail::isNoneMask(rhs)) {
    return lhs;
  }
  if (detail::isNoneMask(lhs) || detail::isAllMask(rhs)) {
    return rhs;
  }

  if (detail::isInclusive(lhs)) {
    if (detail::isInclusive(rhs)) { // lhs=includes rhs=includes
      return unionMask(lhs, rhs);
    }
    // lhs=includes rhs=excludes
    return reverseMask(subtractMask(rhs, lhs));
  }
  if (detail::isInclusive(rhs)) { // lhs=excludes rhs=includes
    return reverseMask(subtractMask(lhs, rhs));
  }
  // lhs=excludes rhs=excludes
  return reverseMask(intersectMask(lhs, rhs));
}
Mask operator-(const Mask& lhs, const Mask& rhs) {
  if (detail::isAllMask(lhs)) {
    return reverseMask(rhs);
  }
  if (detail::isNoneMask(rhs)) {
    return lhs;
  }
  if (detail::isNoneMask(lhs) || detail::isAllMask(rhs)) {
    return noneMask();
  }

  if (detail::isInclusive(lhs)) {
    if (detail::isInclusive(rhs)) { // lhs=includes rhs=includes
      return subtractMask(lhs, rhs);
    }
    // lhs=includes rhs=excludes
    return intersectMask(lhs, rhs);
  }
  if (detail::isInclusive(rhs)) { // lhs=excludes rhs=includes
    return reverseMask(unionMask(lhs, rhs));
  }
  // lhs=excludes rhs=excludes
  return subtractMask(rhs, lhs);
}
} // namespace apache::thrift::protocol
