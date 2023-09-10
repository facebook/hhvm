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

#include <functional>
#include <map>
#include <set>
#include <string_view>
#include <type_traits>
#include <unordered_set>

#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/protocol/detail/FieldMask.h>

using apache::thrift::protocol::field_mask_constants;

namespace apache::thrift::protocol::detail {

const FieldIdToMask* FOLLY_NULLABLE getFieldMask(const Mask& mask) {
  if (mask.includes_ref()) {
    return &*mask.includes_ref();
  }

  if (mask.excludes_ref()) {
    return &*mask.excludes_ref();
  }

  return nullptr;
}

const MapIdToMask* FOLLY_NULLABLE getIntegerMapMask(const Mask& mask) {
  if (mask.includes_map_ref()) {
    return &*mask.includes_map_ref();
  }

  if (mask.excludes_map_ref()) {
    return &*mask.excludes_map_ref();
  }

  return nullptr;
}

const MapStringToMask* FOLLY_NULLABLE getStringMapMask(const Mask& mask) {
  if (mask.includes_string_map_ref()) {
    return &*mask.includes_string_map_ref();
  }

  if (mask.excludes_string_map_ref()) {
    return &*mask.excludes_string_map_ref();
  }

  return nullptr;
}

ArrayKey getArrayKeyFromValue(const Value& v) {
  if (v.is_byte() || v.is_i16() || v.is_i32() || v.is_i64()) {
    return ArrayKey::Integer;
  }
  if (v.is_binary() || v.is_string()) {
    return ArrayKey::String;
  }
  folly::throw_exception<std::runtime_error>(
      "Value contains a non-integer or non-string key.");
}

MapId getMapIdFromValue(const Value& v) {
  if (v.is_byte()) {
    return MapId{v.as_byte()};
  }
  if (v.is_i16()) {
    return MapId{v.as_i16()};
  }
  if (v.is_i32()) {
    return MapId{v.as_i32()};
  }
  if (v.is_i64()) {
    return MapId{v.as_i64()};
  }
  folly::throw_exception<std::runtime_error>(
      "Value contains a non-integer key.");
}

std::string getStringFromValue(const Value& v) {
  if (v.is_binary()) {
    return v.as_binary().to<std::string>();
  }
  if (v.is_string()) {
    return v.as_string();
  }
  folly::throw_exception<std::runtime_error>(
      "Value contains a non-string key.");
}

void throwIfContainsMapMask(const Mask& mask) {
  if (mask.includes_map_ref() || mask.excludes_map_ref() ||
      mask.includes_string_map_ref() || mask.excludes_string_map_ref()) {
    folly::throw_exception<std::runtime_error>("map mask is not implemented");
  }
  const FieldIdToMask& map = mask.includes_ref() ? mask.includes_ref().value()
                                                 : mask.excludes_ref().value();
  for (auto& [_, nestedMask] : map) {
    throwIfContainsMapMask(nestedMask);
  }
}

MapId findMapIdByValueAddress(const Mask& mask, const Value& newKey) {
  MapId mapId = MapId{reinterpret_cast<int64_t>(&newKey)};
  if (!(mask.includes_map_ref() || mask.excludes_map_ref())) {
    return mapId;
  }
  const auto& mapIdToMask = mask.includes_map_ref() ? *mask.includes_map_ref()
                                                    : *mask.excludes_map_ref();
  auto it = std::find_if(
      mapIdToMask.begin(), mapIdToMask.end(), [&newKey](const auto& kv) {
        return *(reinterpret_cast<Value*>(kv.first)) == newKey;
      });
  return it == mapIdToMask.end() ? mapId : MapId{it->first};
}
} // namespace apache::thrift::protocol::detail
