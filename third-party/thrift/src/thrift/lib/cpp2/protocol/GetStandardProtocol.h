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
#include <thrift/lib/thrift/gen-cpp2/standard_types.h>

namespace apache {
namespace thrift {
namespace protocol {
namespace detail {

using type::StandardProtocol;

template <typename>
struct StandardProtocolHelper;

template <>
struct StandardProtocolHelper<BinaryProtocolReader> {
  static constexpr StandardProtocol value = StandardProtocol::Binary;
};

template <>
struct StandardProtocolHelper<BinaryProtocolWriter> {
  static constexpr StandardProtocol value = StandardProtocol::Binary;
};

template <>
struct StandardProtocolHelper<CompactProtocolReader> {
  static constexpr StandardProtocol value = StandardProtocol::Compact;
};

template <>
struct StandardProtocolHelper<CompactProtocolWriter> {
  static constexpr StandardProtocol value = StandardProtocol::Compact;
};

template <typename T>
inline constexpr StandardProtocol get_standard_protocol =
    StandardProtocolHelper<T>::value;

} // namespace detail
} // namespace protocol
} // namespace thrift
} // namespace apache
