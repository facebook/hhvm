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

struct ExtractedMasksFromPatch {
  Mask read; // read mask from patch
  Mask write; // write mask from patch
};

inline ExtractedMasksFromPatch operator|(
    const ExtractedMasksFromPatch& lhs, const ExtractedMasksFromPatch& rhs) {
  ExtractedMasksFromPatch ret;
  ret.read = lhs.read | rhs.read;
  ret.write = lhs.write | rhs.write;
  return ret;
}

inline void insertTypeToMaskIfNotAllMask(Mask& mask, const type::Type& type) {
  if (isAllMask(mask)) {
    return;
  }
  mask.includes_type().ensure().emplace(type, allMask());
}

template <typename T>
const T& getKeyOrElem(const T& value) {
  return value;
}
template <typename K, typename V>
const K& getKeyOrElem(const std::pair<const K, V>& value) {
  return value.first;
}

// Put allMask() if the key was never included in the mask before.
inline void insertKeysToMask(Mask& mask, int64_t k) {
  mask.includes_map().ensure().emplace(k, allMask());
}
inline void insertKeysToMask(Mask& mask, std::string k) {
  mask.includes_string_map().ensure().emplace(std::move(k), allMask());
}
template <typename Container>
void insertKeysToMask(Mask& mask, const Container& c) {
  for (const auto& elem : c) {
    const auto& v = getKeyOrElem(elem);

    if (getArrayKeyFromValue(v) == ArrayKey::Integer) {
      mask.includes_map().ensure().emplace(
          static_cast<int64_t>(getMapIdFromValue(v)), allMask());
    } else {
      mask.includes_string_map().ensure().emplace(
          getStringFromValue(v), allMask());
    }
  }
}

template <typename Container>
void insertKeysToMaskIfNotAllMask(Mask& mask, const Container& c) {
  if (isAllMask(mask)) {
    return;
  }
  insertKeysToMask(mask, c);
}

inline void insertFieldsToMask(Mask& mask, FieldId id) {
  mask.includes().ensure().emplace(static_cast<int16_t>(id), allMask());
}
inline void insertFieldsToMask(Mask& mask, const Object& obj) {
  for (const auto& [id, value] : obj) {
    mask.includes().ensure().emplace(id, allMask());
  }
}

inline void insertFieldsToMaskIfNotAllMask(Mask& mask, FieldId id) {
  if (isAllMask(mask)) {
    return;
  }
  insertFieldsToMask(mask, id);
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
    mask.includes().ensure().emplace(id.as_i16(), allMask());
  }
}

// Inserts the next mask with union operator to getIncludesRef(mask)[id].
// Skips if mask is allMask (already includes all fields), or next is noneMask.
template <typename Id, typename F>
void insertMaskUnion(
    Mask& mask, Id id, const Mask& next, const F& getIncludesRef) {
  if (mask != allMask() && next != noneMask()) {
    Mask& current = getIncludesRef(mask)
                        .ensure()
                        .emplace(std::move(id), noneMask())
                        .first->second;
    current = current | next;
  }
}

template <typename Id, typename F>
void insertNextMask(
    ExtractedMasksFromPatch& masks,
    const ExtractedMasksFromPatch& nextMasks,
    Id readId,
    Id writeId,
    const F& getIncludesRef) {
  insertMaskUnion(
      masks.read, std::move(readId), nextMasks.read, getIncludesRef);
  insertMaskUnion(
      masks.write, std::move(writeId), nextMasks.write, getIncludesRef);
}

// Read mask should be always subset of write mask. If not, make read mask
// equal to write mask. This can happen for struct or map fields with patch
// operations that returns noneMask for read mask (i.e. assign).
inline void ensureRWMaskInvariant(ExtractedMasksFromPatch& masks) {
  if ((masks.read | masks.write) != masks.write) {
    masks.read = masks.write;
  }
}

} // namespace apache::thrift::protocol::detail
