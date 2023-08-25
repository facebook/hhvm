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

// Note: Below methods don't work for class scoped enums

#ifndef THRIFT_UTIL_ENUMUTILS_H_
#define THRIFT_UTIL_ENUMUTILS_H_ 1

#include <cstring>

#include <folly/Conv.h>
#include <folly/Portability.h>
#include <folly/lang/Exception.h>

#include <thrift/lib/cpp/Thrift.h>

namespace apache {
namespace thrift {

namespace util {

/// Return whether EnumType is a thrift defined enum type
template <typename EnumType, typename = void>
FOLLY_INLINE_VARIABLE constexpr bool is_thrift_enum_v = false;

template <typename EnumType>
FOLLY_INLINE_VARIABLE constexpr bool is_thrift_enum_v<
    EnumType,
    folly::void_t<decltype(TEnumTraits<EnumType>::size)>> = true;

/**
 * Parses an enum name to the enum type
 */
template <typename EnumType>
bool tryParseEnum(folly::StringPiece name, EnumType* out) {
  return TEnumTraits<EnumType>::findValue(name, out);
}

/*
 * Same as tryParseEnum but throw an exception if the given name is not found in
 * enum
 */

template <typename EnumType>
EnumType enumValueOrThrow(folly::StringPiece name) {
  EnumType out;
  if (!tryParseEnum(name, &out)) {
    folly::throw_exception<std::out_of_range>("name not found in enum");
  }
  return out;
}
/**
 * Returns the human-readable name for an Enum type.
 * WARNING! By default it returns nullptr if the value is not in enum.
 */
template <typename EnumType>
const char* enumName(EnumType value, const char* defaultName = nullptr) {
  const char* name = TEnumTraits<EnumType>::findName(value);
  if (!name)
    return defaultName;
  return name;
}

/**
 * Same as enumName but returns the integer value converted to string
 * if it is not in enum, to avoid returning nullptr.
 */
template <typename EnumType>
std::string enumNameSafe(EnumType value) {
  auto under = folly::to_underlying(value);
  folly::StringPiece name;
  bool found = TEnumTraits<EnumType>::findName(value, &name);
  return found ? std::string(name) : folly::to<std::string>(under);
}

/*
 * Same as enumName but throw an exception if the given value is not found in
 * enum
 */
template <typename EnumType>
const char* enumNameOrThrow(EnumType value) {
  if (const char* name = enumName(value)) {
    return name;
  }
  folly::throw_exception<std::out_of_range>("value not found in enum");
}

} // namespace util
} // namespace thrift
} // namespace apache

#endif // THRIFT_UTIL_ENUMUTILS_H_ 1
