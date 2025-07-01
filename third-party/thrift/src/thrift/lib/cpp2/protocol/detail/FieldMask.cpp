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
#include <string_view>
#include <type_traits>
#include <unordered_set>

#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/protocol/detail/FieldMask.h>
#include <thrift/lib/cpp2/protocol/detail/Object.h>

namespace field_mask_constants = apache::thrift::protocol::field_mask_constants;

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

[[nodiscard]] const MapTypeToMask* FOLLY_NULLABLE
getTypeMask(const Mask& mask) {
  if (mask.includes_type_ref()) {
    return &*mask.includes_type_ref();
  }

  if (mask.excludes_type_ref()) {
    return &*mask.excludes_type_ref();
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

Value getValueAs(MapId id, const Value& as) {
  auto key = static_cast<int64_t>(id);
  if (as.is_byte()) {
    if (key > std::numeric_limits<int8_t>::max() ||
        key < std::numeric_limits<int8_t>::min()) {
      folly::throw_exception<std::runtime_error>(
          "MapId overflows the provided type.");
    }
    return asValueStruct<type::byte_t>(key);
  }
  if (as.is_i16()) {
    if (key > std::numeric_limits<int16_t>::max() ||
        key < std::numeric_limits<int16_t>::min()) {
      folly::throw_exception<std::runtime_error>(
          "MapId overflows the provided type.");
    }
    return asValueStruct<type::i16_t>(key);
  }
  if (as.is_i32()) {
    if (key > std::numeric_limits<int32_t>::max() ||
        key < std::numeric_limits<int32_t>::min()) {
      folly::throw_exception<std::runtime_error>(
          "MapId overflows the provided type.");
    }
    return asValueStruct<type::i32_t>(key);
  }
  if (as.is_i64()) {
    return asValueStruct<type::i64_t>(key);
  }
  folly::throw_exception<std::runtime_error>(
      "Provided value contains a non-integer.");
}

Value getValueAs(std::string key, const Value& as) {
  if (as.is_binary()) {
    return asValueStruct<type::binary_t>(key);
  }
  if (as.is_string()) {
    return asValueStruct<type::string_t>(key);
  }
  folly::throw_exception<std::runtime_error>(
      "Provided value contains a non-string.");
}

void throwIfContainsMapMask(const Mask& mask) {
  if (mask.includes_map_ref() || mask.excludes_map_ref() ||
      mask.includes_string_map_ref() || mask.excludes_string_map_ref()) {
    folly::throw_exception<std::runtime_error>("map mask is not implemented");
  }
  if (auto* typeMapPtr = getTypeMask(mask)) {
    for (const auto& [_, nestedMask] : *typeMapPtr) {
      throwIfContainsMapMask(nestedMask);
    }
    return;
  }
  for (const auto& [_, nestedMask] : *CHECK_NOTNULL(getFieldMask(mask))) {
    throwIfContainsMapMask(nestedMask);
  }
}

void validateSinglePath(const Mask& mask) {
  if (isAllMask(mask)) {
    return;
  }
  if (isExclusive(mask)) {
    folly::throw_exception<std::runtime_error>(
        "Field mask should not contain any exclusive mask.");
  }
  op::invoke_by_field_id<Mask>(
      static_cast<FieldId>(mask.getType()),
      [&](auto id) {
        using Id = decltype(id);
        auto& m = op::get<Id>(mask).value();
        if (m.size() > 1) {
          folly::throw_exception<std::runtime_error>(
              "Field mask expresses more than one path.");
        }
        if (m.size() == 0) {
          folly::throw_exception<std::runtime_error>(
              "The terminal path is not indicated with allMask.");
        }
        validateSinglePath(m.begin()->second);
      },
      [] {
        folly::throw_exception<std::runtime_error>(
            "Invalid mask to represent a single path.");
      });
}

} // namespace apache::thrift::protocol::detail
