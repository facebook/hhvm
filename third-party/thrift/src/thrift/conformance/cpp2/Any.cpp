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

#include <thrift/conformance/cpp2/Any.h>

#include <thrift/lib/cpp2/type/UniversalName.h>

namespace apache::thrift::conformance {
using type::validateUniversalName;

Protocol getProtocol(const Any& any) noexcept {
  if (!any.protocol()) {
    return getStandardProtocol<StandardProtocol::Compact>();
  }
  if (*any.protocol() != StandardProtocol::Custom) {
    return Protocol(*any.protocol());
  }
  if (any.customProtocol()) {
    return Protocol(any.customProtocol().value_unchecked());
  }
  return {};
}

bool hasProtocol(const Any& any, const Protocol& protocol) noexcept {
  if (!any.protocol()) {
    return protocol.standard() == StandardProtocol::Compact;
  }
  if (*any.protocol() != StandardProtocol::Custom) {
    return *any.protocol() == protocol.standard();
  }
  if (any.customProtocol() && !any.customProtocol().value_unchecked().empty()) {
    return any.customProtocol().value_unchecked() == protocol.custom();
  }
  // `any` has no protocol.
  return protocol.isNone();
}

void setProtocol(const Protocol& protocol, Any& any) noexcept {
  switch (protocol.standard()) {
    case StandardProtocol::Compact:
      any.protocol().reset();
      any.customProtocol().reset();
      break;
    case StandardProtocol::Custom:
      any.protocol().emplace(StandardProtocol::Custom);
      any.customProtocol() = protocol.custom();
      break;
    default:
      any.protocol().emplace(protocol.standard());
      any.customProtocol().reset();
      break;
  }
}

void validateAny(const Any& any) {
  if (any.type().has_value() && !any.type().value_unchecked().empty()) {
    validateUniversalName(any.type().value_unchecked());
  }
  if (any.customProtocol().has_value() &&
      !any.customProtocol().value_unchecked().empty()) {
    validateUniversalName(any.customProtocol().value_unchecked());
  }
}

} // namespace apache::thrift::conformance
