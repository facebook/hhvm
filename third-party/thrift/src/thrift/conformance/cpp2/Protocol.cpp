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

#include <thrift/conformance/cpp2/Protocol.h>

#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/type/UniversalName.h>

namespace apache::thrift::conformance {
using type::validateUniversalName;

Protocol::Protocol(ProtocolStruct protocolStruct)
    : standard_(*protocolStruct.standard()) {
  if (standard_ == StandardProtocol::Custom &&
      protocolStruct.custom().has_value()) {
    custom_ = std::move(protocolStruct.custom().value_unchecked());
  }
}

Protocol Protocol::fromName(const char* name) {
  StandardProtocol standard;
  if (TEnumTraits<StandardProtocol>::findValue(name, &standard)) {
    return Protocol(standard);
  }
  validateUniversalName(name);
  return Protocol(name);
}

std::string_view Protocol::name() const noexcept {
  if (!custom_.empty()) {
    return custom_;
  }
  if (const char* name =
          apache::thrift::TEnumTraits<StandardProtocol>::findName(standard_)) {
    return name;
  }

  // Unknown standard protocol.
  return {};
}

ProtocolStruct Protocol::asStruct() const noexcept {
  ProtocolStruct result;
  switch (standard_) {
    case StandardProtocol::Compact:
      // Leave unset.
      break;
    case StandardProtocol::Custom:
      result.custom().emplace(custom_);
      [[fallthrough]];
    default:
      result.standard() = standard_;
      break;
  }
  return result;
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

} // namespace apache::thrift::conformance
