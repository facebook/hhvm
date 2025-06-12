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
#include <folly/Hash.h>
#include <thrift/lib/cpp2/type/detail/Wrap.h>
#include <thrift/lib/thrift/gen-cpp2/standard_types.h>
#include <thrift/lib/thrift/gen-cpp2/type_rep_types.h>
#include <thrift/lib/thrift/gen-cpp2/type_rep_types_custom_protocol.h>

namespace apache::thrift::type {

/**
 * A thrift protocol.
 *
 * All protocols can be represented uniquely by a name.
 *
 * For efficiency, a set of standard protocols can also be represented by
 * a value in the StandardProtocol enum class. The name of the enum value is the
 * name of that protocol. For example, Protocol::fromName("Binary") is identical
 * to Protocol(StandardProtocol::Binary).
 **/
class Protocol : public detail::Wrap<ProtocolUnion, union_t<ProtocolUnion>> {
  using Base = detail::Wrap<ProtocolUnion, union_t<ProtocolUnion>>;

 public:
  // Returns a protocol for the given name.
  //
  // Unlike the normal constructors, throws std::invalid_argument, if the name
  // is invalid.
  static Protocol fromName(std::string_view name);

  // Returns a static const value for the given standard protocol.
  //
  // The address of this value cannot be used for equality.
  template <StandardProtocol P>
  static FOLLY_EXPORT const Protocol& get() noexcept {
    static const auto* kValue = new Protocol(P);
    return *kValue;
  }

  Protocol() = default;
  Protocol(const Protocol&) = default;
  Protocol(Protocol&&) noexcept = default;
  explicit Protocol(const ProtocolUnion& other) : Base(other) { normalize(); }
  explicit Protocol(ProtocolUnion&& other) noexcept : Base(std::move(other)) {
    normalize();
  }
  Protocol& operator=(const Protocol&) = default;
  Protocol& operator=(Protocol&&) = default;

  /*implicit*/ Protocol(StandardProtocol standard) noexcept {
    if (standard != StandardProtocol::Custom) {
      data_.standard_ref() = standard;
    }
  }

  // Returns the name for the given protocol.
  //
  // The empty string is returned if the protocol is an unrecognized standard
  // protocol.
  std::string_view name() const noexcept;
  StandardProtocol standard() const noexcept {
    return data_.standard_ref().value_or(StandardProtocol::Custom);
  }
  std::string_view custom() const noexcept {
    if (data_.custom_ref().has_value()) {
      return *data_.custom_ref();
    }
    return {};
  }

  bool isStandard() const noexcept {
    return standard() != StandardProtocol::Custom;
  }
  bool isCustom() const noexcept { return !custom().empty(); }

 private:
  using Base::data_;
  explicit Protocol(std::string custom) {
    data_.custom_ref() = std::move(custom);
  }

  friend bool operator==(const Protocol& lhs, const Protocol& rhs) {
    return lhs.data_ == rhs.data_;
  }
  friend bool operator!=(const Protocol& lhs, const Protocol& rhs) {
    return lhs.data_ != rhs.data_;
  }
  friend bool operator<(const Protocol& lhs, const Protocol& rhs);

  void normalize();
};

// Raises std::invalid_argument on failure.
void validateProtocol(const Protocol& protocol);

FOLLY_PUSH_WARNING
FOLLY_CLANG_DISABLE_WARNING("-Wglobal-constructors")
inline const Protocol kNoProtocol = {};
FOLLY_POP_WARNING

} // namespace apache::thrift::type

FBTHRIFT_STD_HASH_WRAP_DATA(apache::thrift::type::Protocol)
