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

#include <string>
#include <string_view>

#include <folly/CPortability.h>
#include <thrift/conformance/if/gen-cpp2/protocol_types.h>

namespace apache::thrift::conformance {

/**
 * A serialization protocol.
 *
 * All protocols can be represented uniquely by a name.
 *
 * For efficiency, a set of standard protocols can also be represented by
 * a value in the StandardProtocol enum class. The name of the enum value is the
 * name of that protocol. For example Protocol("Binary") is identical to
 * Protocol(StandardProtocol::Binary).
 **/
class Protocol {
 public:
  Protocol() noexcept = default;
  Protocol(const Protocol&) = default;
  Protocol(Protocol&&) noexcept = default;

  explicit Protocol(StandardProtocol standard) noexcept : standard_(standard) {}
  explicit Protocol(std::string custom) noexcept : custom_(std::move(custom)) {}
  explicit Protocol(ProtocolStruct protocol) noexcept;

  // Returns a protocol for the given name.
  //
  // Unlike the normal constructors, throws std::invalid_argument, if the name
  // is invalid.
  static Protocol fromName(const char* name);
  static Protocol fromName(const std::string& name) {
    return fromName(name.c_str());
  }

  Protocol& operator=(const Protocol&) = default;
  Protocol& operator=(Protocol&&) = default;

  // Returns the name for the given protocol.
  //
  // The empty string is returned if the protocol is an unrecognized standard
  // protocol.
  std::string_view name() const noexcept;

  StandardProtocol standard() const noexcept { return standard_; }

  const std::string& custom() const noexcept { return custom_; }

  bool isStandard() const noexcept {
    return standard_ != StandardProtocol::Custom;
  }
  bool isCustom() const noexcept { return !custom_.empty(); }
  bool isNone() const noexcept { return !isStandard() && !isCustom(); }

  ProtocolStruct asStruct() const noexcept;

 private:
  StandardProtocol standard_ = StandardProtocol::Custom;
  std::string custom_;
};

inline bool operator==(const Protocol& lhs, const Protocol& rhs) {
  return lhs.standard() == rhs.standard() && lhs.custom() == rhs.custom();
}

bool operator<(const Protocol& lhs, const Protocol& rhs);

inline bool operator!=(const Protocol& lhs, const Protocol& rhs) {
  return !(lhs == rhs);
}

// Raises std::invalid_argument on failure.
void validateProtocol(const Protocol& protocol);

// Returns a static const value for the given standard protocol.
//
// The address of this value cannot be used for equality.
template <StandardProtocol protocol>
FOLLY_EXPORT const Protocol& getStandardProtocol() noexcept {
  static const auto* kValue = new Protocol(protocol);
  return *kValue;
}

inline const Protocol kNoProtocol = {};

} // namespace apache::thrift::conformance

// custom specialization of std::hash can be injected in namespace std
namespace std {
template <>
struct hash<apache::thrift::conformance::Protocol> {
  size_t operator()(
      const apache::thrift::conformance::Protocol& protocol) const noexcept {
    size_t h1 = std::hash<apache::thrift::conformance::StandardProtocol>{}(
        protocol.standard());
    size_t h2 = std::hash<std::string>{}(protocol.custom());
    // Since h1 and h2 are mutually exclusive, we can just add them.
    return h1 + h2;
  }
};

} // namespace std
