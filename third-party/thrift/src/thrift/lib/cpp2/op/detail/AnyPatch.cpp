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

#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/op/detail/AnyPatch.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/lib/cpp2/type/Type.h>

namespace apache::thrift::op::detail {

void throwDuplicatedType(const type::Type& type) {
  folly::throw_exception<std::runtime_error>(fmt::format(
      "duplicated key in AnyPatch: {}", debugStringViaEncode(type)));
}

void throwTypeNotValid(const type::Type& type) {
  folly::throw_exception<std::runtime_error>(
      fmt::format("Invalid type: {}", debugStringViaEncode(type)));
}

void throwAnyNotValid(const type::AnyStruct& any) {
  folly::throw_exception<std::runtime_error>(
      fmt::format("Invalid any: {}", debugStringViaEncode(any)));
}

void throwUnsupportedAnyProtocol(const type::AnyStruct& any) {
  folly::throw_exception<std::runtime_error>(fmt::format(
      "Unsupported serialization protocol for AnyPatch: {}",
      debugStringViaEncode(any.protocol().value())));
}

} // namespace apache::thrift::op::detail
