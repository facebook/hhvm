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

#include <thrift/conformance/cpp2/ThriftTypeInfo.h>

namespace apache::thrift::conformance {
using type::validateUniversalHashBytes;
using type::validateUniversalName;

void validateThriftTypeInfo(const ThriftTypeInfo& type) {
  validateUniversalName(*type.uri());
  for (const auto& uri : *type.altUris()) {
    validateUniversalName(uri);
  }
  if (type.altUris()->find(*type.uri()) != type.altUris()->end()) {
    folly::throw_exception<std::invalid_argument>(
        "duplicate uri: " + *type.uri());
  }

  if (type.typeHashBytes()) {
    validateUniversalHashBytes(type.typeHashBytes().value(), kMinTypeHashBytes);
  }
}

[[FOLLY_ATTR_GNU_COLD]] ThriftTypeInfo createThriftTypeInfo(
    std::span<folly::cstring_view const> uris,
    type::hash_size_t typeHashBytes) {
  ThriftTypeInfo type;
  if (typeHashBytes != kTypeHashBytesNotSpecified) {
    type.typeHashBytes() = typeHashBytes;
  }
  auto itr = folly::access::begin(uris);
  auto iend = folly::access::end(uris);
  if (itr == iend) {
    folly::throw_exception<std::invalid_argument>(
        "At least one name must be provided.");
  }
  type.uri() = std::string{*itr++};
  for (; itr != iend; ++itr) {
    type.altUris()->emplace(std::string{*itr++});
  }
  return type;
}

} // namespace apache::thrift::conformance
