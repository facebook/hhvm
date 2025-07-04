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

#include <initializer_list>
#include <optional>
#include <set>

#include <boost/mp11.hpp>

#include <folly/io/IOBufQueue.h>
#include <thrift/conformance/cpp2/AnyRegistry.h>
#include <thrift/conformance/cpp2/Protocol.h>
#include <thrift/conformance/if/gen-cpp2/test_suite_types.h>
#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace apache::thrift::conformance::data::detail {

using PrimaryTypeTags = boost::mp11::mp_list<
    type::bool_t,
    type::byte_t,
    type::i16_t,
    type::i32_t,
    type::i64_t,
    type::float_t,
    type::double_t,
    type::string_t,
    type::binary_t>;

using KeyTypeTags = boost::mp11::mp_list<type::i64_t, type::string_t>;

template <typename C>
std::set<Protocol> toProtocols(const C& protocolCtorArgs) {
  std::set<Protocol> result;
  for (const auto& arg : protocolCtorArgs) {
    result.emplace(arg);
  }
  return result;
}

constexpr std::initializer_list<StandardProtocol> kDefaultProtocols = {
    StandardProtocol::Binary, StandardProtocol::Compact};

[[nodiscard]] std::unique_ptr<folly::IOBuf> serializeThriftStruct(
    const apache::thrift::protocol::Object& a, const Protocol& protocol);

template <class ThriftStruct>
[[nodiscard]] std::
    enable_if_t<is_thrift_class_v<ThriftStruct>, std::unique_ptr<folly::IOBuf>>
    serializeThriftStruct(const ThriftStruct& s, const Protocol& protocol) {
  switch (auto p = protocol.standard()) {
    case StandardProtocol::Compact:
      return apache::thrift::CompactSerializer::serialize<folly::IOBufQueue>(s)
          .move();
    case StandardProtocol::Binary:
      return apache::thrift::BinarySerializer::serialize<folly::IOBufQueue>(s)
          .move();
    default:
      throw std::invalid_argument(
          "Unsupported protocol: " + util::enumNameSafe(p));
  }
}

// For Compatibility tests, we set the request to thrift.Any of new struct, then
// replace the underlying IOBuf to serialized old struct.
//
// This way, the Thrift.Any will be deserialized with old struct and the layout
// of new struct
template <class Old, class New>
[[nodiscard]] TestCase genCompatibilityRoundTripTestCase(
    const Protocol& protocol,
    std::string name,
    const Old& oldData,
    const New& newData,
    std::optional<std::string> description = {}) {
  RoundTripTestCase roundTrip;
  auto newAny = AnyRegistry::generated().store(newData, protocol);
  roundTrip.request()->value() = newAny;
  roundTrip.request()->value()->data() =
      *serializeThriftStruct(oldData, protocol);
  roundTrip.expectedResponse().emplace().value() = std::move(newAny);

  TestCase testCase;
  testCase.name() = std::move(name);
  testCase.test()->roundTrip() = std::move(roundTrip);
  testCase.description().from_optional(description);
  return testCase;
}

} // namespace apache::thrift::conformance::data::detail
