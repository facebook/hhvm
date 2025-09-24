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

#include <thrift/lib/thrift/detail/TypeSystemAdapter.h>

#include <thrift/lib/thrift/gen-cpp2/type_system_types.h>

#include <folly/hash/Hash.h>

#include <fmt/core.h>

using apache::thrift::type_system::FieldId;
using apache::thrift::type_system::FieldIdentity;
using apache::thrift::type_system::FieldName;

std::size_t std::hash<FieldIdentity>::operator()(
    const FieldIdentity& identity) const noexcept {
  return folly::hash::hash_combine(
      std::hash<FieldId>{}(identity.id()),
      std::hash<FieldName>{}(identity.name()));
}

fmt::format_context::iterator fmt::formatter<FieldIdentity>::format(
    const FieldIdentity& identity, fmt::format_context& ctx) const {
  return fmt::format_to(
      ctx.out(),
      "({}, '{}')",
      folly::to_underlying(identity.id()),
      identity.name());
}
