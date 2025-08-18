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

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/protocol/CompactV1Protocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/Module_types.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/Module_types_custom_protocol.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::test;

namespace {

template <typename T>
struct action_traits_impl;
template <typename C, typename A>
struct action_traits_impl<void (C::*)(A&) const> {
  using arg_type = A;
};
template <typename C, typename A>
struct action_traits_impl<void (C::*)(A&)> {
  using arg_type = A;
};
template <typename F>
using action_traits = action_traits_impl<decltype(&F::operator())>;
template <typename F>
using arg = typename action_traits<F>::arg_type;

template <typename F>
arg<F> returning(F&& f) {
  arg<F> ret;
  f(ret);
  return ret;
}

using CompactV1Serializer =
    Serializer<CompactV1ProtocolReader, CompactV1ProtocolWriter>;

class CompactV1ProtocolTest : public testing::Test {};

} // namespace

TEST_F(CompactV1ProtocolTest, roundtrip) {
  const auto orig = OneOfEach{};
  const auto serialized = CompactV1Serializer::serialize<std::string>(orig);
  const auto deserialized_size = returning([&](tuple<OneOfEach, uint32_t>& _) {
    get<1>(_) = CompactV1Serializer::deserialize(serialized, get<0>(_));
  });
  EXPECT_EQ(serialized.size(), get<1>(deserialized_size));
  EXPECT_EQ(orig, get<0>(deserialized_size));
}

TEST_F(CompactV1ProtocolTest, double_byteswap) {
  const auto orig = OneOfEach{};
  const auto serialized = CompactV1Serializer::serialize<std::string>(orig);
  auto deserialized_size = returning([&](tuple<OneOfEach, uint32_t>& _) {
    get<1>(_) = CompactSerializer::deserialize(serialized, get<0>(_));
  });
  EXPECT_EQ(serialized.size(), get<1>(deserialized_size));
  uint64_t double_rep;
  std::memcpy(
      &double_rep,
      &(*get<0>(deserialized_size).myDouble()),
      sizeof(double_rep));
  double_rep = folly::Endian::swap(double_rep);
  std::memcpy(
      &(*get<0>(deserialized_size).myDouble()),
      &double_rep,
      sizeof(double_rep));
  EXPECT_EQ(orig, get<0>(deserialized_size));
}
