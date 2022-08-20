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
#include <thrift/lib/thrift/gen-cpp2/standard_constants.h>
#include <thrift/lib/thrift/gen-cpp2/type_types.h>

namespace apache::thrift::conformance {

inline constexpr type::hash_size_t kTypeHashBytesNotSpecified = -1;
inline constexpr type::hash_size_t kMinTypeHashBytes =
    type::standard_constants::minTypeHashBytes();
inline constexpr type::hash_size_t kDefaultTypeHashBytes =
    type::standard_constants::defaultTypeHashBytes();

// Creates an ThriftTypeInfo struct with the given names and configuration.
//
// The first name in names is set as the primary name, and all others are added
// as aliases.
template <typename C = std::initializer_list<std::string_view>>
ThriftTypeInfo createThriftTypeInfo(
    C&& names, type::hash_size_t typeHashBytes = kTypeHashBytesNotSpecified);

// Raises std::invalid_argument if invalid.
void validateThriftTypeInfo(const ThriftTypeInfo& type);

// Implementation

template <typename R>
ThriftTypeInfo createThriftTypeInfo(R&& uris, type::hash_size_t typeHashBytes) {
  ThriftTypeInfo type;
  if (typeHashBytes != kTypeHashBytesNotSpecified) {
    type.typeHashBytes() = typeHashBytes;
  }
  auto itr = folly::access::begin(std::forward<R>(uris));
  auto iend = folly::access::end(std::forward<R>(uris));
  if (itr == iend) {
    folly::throw_exception<std::invalid_argument>(
        "At least one name must be provided.");
  }
  type.uri() = std::string{std::forward<decltype(*itr)>(*itr++)};
  for (; itr != iend; ++itr) {
    type.altUris()->emplace(std::string{std::forward<decltype(*itr)>(*itr++)});
  }
  return type;
}

template <typename T>
const ThriftTypeInfo& getGeneratedThriftTypeInfo() {
  static const ThriftTypeInfo kInfo =
      createThriftTypeInfo({::apache::thrift::uri<T>()});
  return kInfo;
}

} // namespace apache::thrift::conformance
