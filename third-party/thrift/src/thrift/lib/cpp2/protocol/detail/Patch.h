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

#include <thrift/lib/cpp2/protocol/FieldMask.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/type/Type.h>

namespace apache::thrift::protocol::detail {

inline void insertTypeToMaskIfNotAllMask(Mask& mask, const type::Type& type) {
  if (isAllMask(mask)) {
    return;
  }
  mask.includes_type_ref().ensure().emplace(type, allMask());
}

template <typename T>
const T& getKeyOrElem(const T& value) {
  return value;
}
template <typename K, typename V>
const K& getKeyOrElem(const std::pair<const K, V>& value) {
  return value.first;
}

// Put allMask() if the key was never included in the mask before. `view`
// specifies whether to use address of Value to populate map mask (deprecated).
template <typename Container>
void insertKeysToMask(Mask& mask, const Container& c, bool view) {
  if (view) {
    auto writeValueIndex = buildValueIndex(mask);
    for (const auto& elem : c) {
      const auto& v = getKeyOrElem(elem);
      auto id = static_cast<int64_t>(
          getMapIdValueAddressFromIndex(writeValueIndex, v));
      mask.includes_map_ref().ensure().emplace(id, allMask());
    }
    return;
  }

  for (const auto& elem : c) {
    const auto& v = getKeyOrElem(elem);

    if (getArrayKeyFromValue(v) == ArrayKey::Integer) {
      mask.includes_map_ref().ensure().emplace(
          static_cast<int64_t>(getMapIdFromValue(v)), allMask());
    } else {
      mask.includes_string_map_ref().ensure().emplace(
          getStringFromValue(v), allMask());
    }
  }
}

template <typename Container>
void insertKeysToMaskIfNotAllMask(Mask& mask, const Container& c, bool view) {
  if (isAllMask(mask)) {
    return;
  }
  insertKeysToMask(mask, c, view);
}

inline void insertFieldsToMask(Mask& mask, const Object& obj) {
  for (const auto& [id, value] : obj) {
    mask.includes_ref().ensure().emplace(id, allMask());
  }
}

inline void insertFieldsToMaskIfNotAllMask(Mask& mask, const Object& obj) {
  if (isAllMask(mask)) {
    return;
  }
  insertFieldsToMask(mask, obj);
}

inline void insertFieldsToMaskIfNotAllMask(
    Mask& mask, const std::vector<Value>& ids) {
  if (isAllMask(mask)) {
    return;
  }
  for (const Value& id : ids) {
    mask.includes_ref().ensure().emplace(id.as_i16(), allMask());
  }
}

} // namespace apache::thrift::protocol::detail
