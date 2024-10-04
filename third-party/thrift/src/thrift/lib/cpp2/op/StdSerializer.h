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

#include <array>
#include <utility>

#include <thrift/lib/cpp2/op/Serializer.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/JSONProtocol.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>
#include <thrift/lib/cpp2/type/Protocol.h>
#include <thrift/lib/cpp2/type/TypeRegistry.h>

namespace apache::thrift::op {

namespace detail {
template <type::StandardProtocol protocol>
struct ProtocolSelector;
template <>
struct ProtocolSelector<type::StandardProtocol::Binary>
    : std::pair<BinaryProtocolReader, BinaryProtocolWriter> {};
template <>
struct ProtocolSelector<type::StandardProtocol::Compact>
    : std::pair<CompactProtocolReader, CompactProtocolWriter> {};

// NOTE: Deprecated in v1+
template <>
struct ProtocolSelector<type::StandardProtocol::Json>
    : std::pair<JSONProtocolReader, JSONProtocolWriter> {};
template <>
struct ProtocolSelector<type::StandardProtocol::SimpleJson>
    : std::pair<SimpleJSONProtocolReader, SimpleJSONProtocolWriter> {};

} // namespace detail

// A standard protocol serializer for any thrift structred type.
template <typename Tag, type::StandardProtocol P>
class StdSerializer : public ProtocolSerializer<
                          Tag,
                          typename detail::ProtocolSelector<P>::first_type,
                          typename detail::ProtocolSelector<P>::second_type> {
 public:
  const type::Protocol& getProtocol() const final {
    return type::Protocol::get<P>();
  }
};

template <typename Tag, type::StandardProtocol... Ps>
void registerStdSerializers(
    type::TypeRegistry& registry, bool skipDuplicates = false) {
  for (auto result :
       std::array<bool, sizeof...(Ps)>{{registry.registerSerializer(
           std::make_unique<StdSerializer<Tag, Ps>>(),
           type::Type::get<Tag>())...}}) {
    if (!result && !skipDuplicates) {
      folly::throw_exception<std::runtime_error>("Could not register type.");
    }
  }
}

} // namespace apache::thrift::op
