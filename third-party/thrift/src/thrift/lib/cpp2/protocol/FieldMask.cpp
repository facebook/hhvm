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
      auto tmp = std::move(mask.includes_ref().value());
      mask.excludes_ref() = std::move(tmp);
      return mask;
    }
    case Mask::Type::excludes: {
      auto tmp = std::move(mask.excludes_ref().value());
      mask.includes_ref() = std::move(tmp);
      return mask;
    }
    case Mask::Type::includes_map: {
      auto tmp = std::move(mask.includes_map_ref().value());
      mask.excludes_map_ref() = std::move(tmp);
      return mask;
    }
    case Mask::Type::excludes_map: {
      auto tmp = std::move(mask.excludes_map_ref().value());
      mask.includes_map_ref() = std::move(tmp);
      return mask;
    }
    case Mask::Type::includes_string_map: {
      auto tmp = std::move(mask.includes_string_map_ref().value());
      mask.excludes_string_map_ref() = std::move(tmp);
      return mask;
    }
    case Mask::Type::excludes_string_map: {
      auto tmp = std::move(mask.excludes_string_map_ref().value());
      mask.includes_string_map_ref() = std::move(tmp);
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

void copy(
    const Mask& mask, const protocol::Object& src, protocol::Object& dst) {
  (MaskRef{mask, false}).copy(src, dst);
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

template <class Func>
Mask apply(const Mask& lhs, const Mask& rhs, Func&& func) {
  using detail::getFieldMask;
  using detail::getIntegerMapMask;
  using detail::getStringMapMask;

  Mask mask;

  if (getStringMapMask(lhs) || getStringMapMask(rhs)) {
    mask.includes_string_map_ref() =
        func(toStringMapMask(lhs), toStringMapMask(rhs));
    return mask;
  }

  // If one of them is an integer map mask, the other one must be either integer
  // map mask, or allMask/noneMask which either mask the whole integer map, or
  // nothing.
  if (getIntegerMapMask(lhs) || getIntegerMapMask(rhs)) {
    mask.includes_map_ref() =
        func(toIntegerMapMask(lhs), toIntegerMapMask(rhs));
    return mask;
  }

  mask.includes_ref() = func(*getFieldMask(lhs), *getFieldMask(rhs));
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

bool isInclusive(const Mask& mask) {
  return folly::to_underlying(mask.getType()) % 2 == 0;
}

} // namespace

Mask operator&(const Mask& lhs, const Mask& rhs) {
  if (isInclusive(lhs)) {
    if (isInclusive(rhs)) { // lhs=includes rhs=includes
      return intersectMask(lhs, rhs);
    }
    // lhs=includes rhs=excludes
    return subtractMask(lhs, rhs);
  }
  if (isInclusive(rhs)) { // lhs=excludes rhs=includes
    return subtractMask(rhs, lhs);
  }
  // lhs=excludes rhs=excludes
  return reverseMask(unionMask(lhs, rhs));
}
Mask operator|(const Mask& lhs, const Mask& rhs) {
  if (isInclusive(lhs)) {
    if (isInclusive(rhs)) { // lhs=includes rhs=includes
      return unionMask(lhs, rhs);
    }
    // lhs=includes rhs=excludes
    return reverseMask(subtractMask(rhs, lhs));
  }
  if (isInclusive(rhs)) { // lhs=excludes rhs=includes
    return reverseMask(subtractMask(lhs, rhs));
  }
  // lhs=excludes rhs=excludes
  return reverseMask(intersectMask(lhs, rhs));
}
Mask operator-(const Mask& lhs, const Mask& rhs) {
  if (isInclusive(lhs)) {
    if (isInclusive(rhs)) { // lhs=includes rhs=includes
      return subtractMask(lhs, rhs);
    }
    // lhs=includes rhs=excludes
    return intersectMask(lhs, rhs);
  }
  if (isInclusive(rhs)) { // lhs=excludes rhs=includes
    return reverseMask(unionMask(lhs, rhs));
  }
  // lhs=excludes rhs=excludes
  return subtractMask(rhs, lhs);
}
} // namespace apache::thrift::protocol
