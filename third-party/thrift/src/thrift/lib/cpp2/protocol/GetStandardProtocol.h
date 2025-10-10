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

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/JSONProtocol.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/standard_types.h>

#include <folly/Traits.h>

#include <concepts>

namespace apache::thrift::protocol {

namespace detail {

template <typename T>
consteval type::StandardProtocol standardProtocolFor() {
  if constexpr (std::same_as<T, BinaryProtocolReader>) {
    return type::StandardProtocol::Binary;
  } else if constexpr (std::same_as<T, BinaryProtocolWriter>) {
    return type::StandardProtocol::Binary;
  } else if constexpr (std::same_as<T, CompactProtocolReader>) {
    return type::StandardProtocol::Compact;
  } else if constexpr (std::same_as<T, CompactProtocolWriter>) {
    return type::StandardProtocol::Compact;
  } else if constexpr (std::same_as<T, JSONProtocolReader>) {
    return type::StandardProtocol::Json;
  } else if constexpr (std::same_as<T, JSONProtocolWriter>) {
    return type::StandardProtocol::Json;
  } else if constexpr (std::same_as<T, SimpleJSONProtocolReader>) {
    return type::StandardProtocol::SimpleJson;
  } else if constexpr (std::same_as<T, SimpleJSONProtocolWriter>) {
    return type::StandardProtocol::SimpleJson;
  } else {
    static_assert(folly::always_false<T>, "Unknown Protocol");
  }
}

template <type::StandardProtocol ProtocolType>
consteval auto protocolReaderFor() {
  if constexpr (ProtocolType == type::StandardProtocol::Binary) {
    return std::type_identity<BinaryProtocolReader>{};
  } else if constexpr (ProtocolType == type::StandardProtocol::Compact) {
    return std::type_identity<CompactProtocolReader>{};
  } else if constexpr (ProtocolType == type::StandardProtocol::Json) {
    return std::type_identity<JSONProtocolReader>{};
  } else if constexpr (ProtocolType == type::StandardProtocol::SimpleJson) {
    return std::type_identity<SimpleJSONProtocolReader>{};
  } else {
    static_assert(!bool(ProtocolType), "Unknown Protocol");
  }
}

template <type::StandardProtocol ProtocolType>
consteval auto protocolWriterFor() {
  if constexpr (ProtocolType == type::StandardProtocol::Binary) {
    return std::type_identity<BinaryProtocolWriter>{};
  } else if constexpr (ProtocolType == type::StandardProtocol::Compact) {
    return std::type_identity<CompactProtocolWriter>{};
  } else if constexpr (ProtocolType == type::StandardProtocol::Json) {
    return std::type_identity<JSONProtocolWriter>{};
  } else if constexpr (ProtocolType == type::StandardProtocol::SimpleJson) {
    return std::type_identity<SimpleJSONProtocolWriter>{};
  } else {
    static_assert(!bool(ProtocolType), "Unknown Protocol");
  }
}

} // namespace detail

template <typename T>
inline constexpr type::StandardProtocol get_standard_protocol =
    detail::standardProtocolFor<T>();

template <type::StandardProtocol ProtocolType>
using ProtocolReaderFor =
    typename decltype(detail::protocolReaderFor<ProtocolType>())::type;

template <type::StandardProtocol ProtocolType>
using ProtocolWriterFor =
    typename decltype(detail::protocolWriterFor<ProtocolType>())::type;

} // namespace apache::thrift::protocol
