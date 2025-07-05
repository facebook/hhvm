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

#include <thrift/lib/cpp2/type/Protocol.h>

#include <thrift/lib/cpp2/type/UniversalName.h>

namespace apache::thrift::type {

Protocol Protocol::fromName(std::string_view name) {
  StandardProtocol standard;
  if (TEnumTraits<StandardProtocol>::findValue(name, &standard)) {
    return Protocol(standard);
  }
  validateUniversalName(name);
  return Protocol(std::string(name));
}

std::string_view Protocol::name() const noexcept {
  if (isCustom()) {
    return custom();
  }
  std::string_view name;
  if (apache::thrift::TEnumTraits<StandardProtocol>::findName(
          standard(), &name)) {
    return name;
  }

  // Unknown standard protocol.
  return {};
}

void Protocol::normalize() {
  switch (data_.getType()) {
    case ProtocolUnion::Type::standard:
      if (data_.standard() == StandardProtocol::Custom) {
        reset();
      }
      break;
    case ProtocolUnion::Type::custom:
      if (data_.custom()->empty()) {
        reset();
      }
      break;
    default:
      break;
  }
}

bool operator<(const Protocol& lhs, const Protocol& rhs) {
  if (lhs.standard() < rhs.standard()) {
    return true;
  }
  if (lhs.standard() > rhs.standard()) {
    return false;
  }
  return lhs.custom() < rhs.custom();
}

void validateProtocol(const Protocol& protocol) {
  if (protocol.isCustom()) {
    validateUniversalName(protocol.custom());
  }
}

} // namespace apache::thrift::type
