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

#include <initializer_list>
#include <string_view>

#include <folly/container/Access.h>
#include <thrift/conformance/if/gen-cpp2/type_types.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/type/UniversalName.h>
#include <thrift/lib/thrift/gen-cpp2/type_types.h>

namespace apache::thrift::conformance {

inline constexpr type::hash_size_t kTypeHashBytesNotSpecified = -1;

// The minimum and default number of bytes that can be used to identify
// a type.
//
// The expected number of types that can be hashed before a
// collision is 2^(8*{numBytes}/2).
// Which is ~4.3 billion types for the min, and ~18.45 quintillion
// types for the default.
inline constexpr type::hash_size_t kMinTypeHashBytes = 8;

// Creates an ThriftTypeInfo struct with the given names and configuration.
//
// The first name in names is set as the primary name, and all others are added
// as aliases.
ThriftTypeInfo createThriftTypeInfo(
    std::span<folly::cstring_view const> uris,
    type::hash_size_t typeHashBytes = kTypeHashBytesNotSpecified);

inline ThriftTypeInfo createThriftTypeInfo(
    std::initializer_list<folly::cstring_view> uris,
    type::hash_size_t typeHashBytes = kTypeHashBytesNotSpecified) {
  return createThriftTypeInfo(std::span(uris), typeHashBytes);
}

// Raises std::invalid_argument if invalid.
void validateThriftTypeInfo(const ThriftTypeInfo& type);

template <typename T>
const ThriftTypeInfo& getGeneratedThriftTypeInfo() {
  static const ThriftTypeInfo kInfo =
      createThriftTypeInfo({::apache::thrift::uri<T>()});
  return kInfo;
}

} // namespace apache::thrift::conformance
