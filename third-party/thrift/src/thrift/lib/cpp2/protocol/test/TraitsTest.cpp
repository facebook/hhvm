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

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>
#include <thrift/lib/cpp2/protocol/Traits.h>

#include <cstdint>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp/protocol/TType.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;

namespace tc = apache::thrift::type_class;

class TraitsTest : public testing::Test {
 public:
  template <typename T>
  struct indy_value {
    T value;
  };
  struct indy_access {
    template <typename T>
    constexpr auto operator()(T&& t) const noexcept
        -> decltype((static_cast<T&&>(t).value));
  };
  template <typename T>
  using indy_tag = detail::indirection_tag<T, indy_access>;
};

TEST_F(TraitsTest, protocol_type) {
  EXPECT_EQ(TType::T_BOOL, (protocol_type_v<tc::integral, bool>));
  EXPECT_EQ(TType::T_I08, (protocol_type_v<tc::integral, int8_t>));
  EXPECT_EQ(TType::T_I08, (protocol_type_v<tc::integral, uint8_t>));
  EXPECT_EQ(TType::T_I16, (protocol_type_v<tc::integral, int16_t>));
  EXPECT_EQ(TType::T_I16, (protocol_type_v<tc::integral, uint16_t>));
  EXPECT_EQ(TType::T_I32, (protocol_type_v<tc::integral, int32_t>));
  EXPECT_EQ(TType::T_I32, (protocol_type_v<tc::integral, uint32_t>));
  EXPECT_EQ(TType::T_I64, (protocol_type_v<tc::integral, int64_t>));
  EXPECT_EQ(TType::T_I64, (protocol_type_v<tc::integral, uint64_t>));
  EXPECT_EQ(TType::T_FLOAT, (protocol_type_v<tc::floating_point, float>));
  EXPECT_EQ(TType::T_DOUBLE, (protocol_type_v<tc::floating_point, double>));
  EXPECT_EQ(TType::T_I32, (protocol_type_v<tc::enumeration, void>));
  EXPECT_EQ(TType::T_STRING, (protocol_type_v<tc::string, void>));
  EXPECT_EQ(TType::T_STRING, (protocol_type_v<tc::binary, void>));
  EXPECT_EQ(TType::T_STRUCT, (protocol_type_v<tc::structure, void>));
  EXPECT_EQ(TType::T_STRUCT, (protocol_type_v<tc::variant, void>));
  EXPECT_EQ(TType::T_LIST, (protocol_type_v<tc::list<void>, void>));
  EXPECT_EQ(TType::T_SET, (protocol_type_v<tc::set<void>, void>));
  EXPECT_EQ(TType::T_MAP, (protocol_type_v<tc::map<void, void>, void>));
  EXPECT_EQ(
      TType::T_FLOAT,
      (protocol_type_v<indy_tag<tc::floating_point>, indy_value<float>>));
}

template <typename T>
class FixedCostSkipTest : public testing::Test {};

struct Struct {};
enum class Enum {};

using TypeClassesAndType = ::testing::Types<
    std::pair<tc::string, std::string>,
    std::pair<tc::integral, int8_t>,
    std::pair<tc::integral, int16_t>,
    std::pair<tc::integral, int32_t>,
    std::pair<tc::integral, int64_t>,
    std::pair<tc::enumeration, Enum>,
    std::pair<tc::floating_point, float>,
    std::pair<tc::floating_point, double>,
    std::pair<tc::structure, Struct>,
    std::pair<tc::variant, Struct>>;

TYPED_TEST_CASE(FixedCostSkipTest, TypeClassesAndType);

TYPED_TEST(FixedCostSkipTest, Compact) {
  using P = CompactProtocolReader;
  using TC = typename TypeParam::first_type;
  using T = typename TypeParam::second_type;

  EXPECT_EQ(
      (!std::is_same_v<TC, tc::structure> && !std::is_same_v<TC, tc::variant>),
      (fixed_cost_skip_v<P, TC, T>));

  EXPECT_EQ(
      (std::is_same_v<T, int8_t> || //
       std::is_same_v<T, float> || //
       std::is_same_v<T, double>),
      (fixed_cost_skip_v<P, tc::list<TC>, std::vector<T>>));

  EXPECT_EQ(
      (std::is_same_v<T, int8_t> || //
       std::is_same_v<T, float> || //
       std::is_same_v<T, double>),
      (fixed_cost_skip_v<P, tc::set<TC>, std::set<T>>));

  EXPECT_EQ(
      (fixed_cost_skip_v<P, tc::list<TC>, std::vector<T>>),
      (fixed_cost_skip_v<
          P,
          tc::map<tc::integral, TC>,
          std::map<std::int8_t, T>>));

  EXPECT_FALSE( //
      (fixed_cost_skip_v<
          P,
          tc::map<tc::integral, TC>,
          std::map<std::int16_t, T>>));

  EXPECT_FALSE((
      fixed_cost_skip_v<P, tc::map<tc::string, TC>, std::map<std::string, T>>));
}

TYPED_TEST(FixedCostSkipTest, Binary) {
  using P = BinaryProtocolReader;
  using TC = typename TypeParam::first_type;
  using T = typename TypeParam::second_type;

  EXPECT_EQ(
      (!std::is_same_v<TC, tc::structure> && !std::is_same_v<TC, tc::variant>),
      (fixed_cost_skip_v<P, TC, T>));

  EXPECT_EQ(
      (std::is_same_v<TC, tc::integral> ||
       std::is_same_v<TC, tc::floating_point> ||
       std::is_same_v<TC, tc::enumeration>),
      (fixed_cost_skip_v<P, tc::list<TC>, std::vector<T>>));

  EXPECT_EQ(
      (std::is_same_v<TC, tc::integral> ||
       std::is_same_v<TC, tc::floating_point> ||
       std::is_same_v<TC, tc::enumeration>),
      (fixed_cost_skip_v<P, tc::set<TC>, std::set<T>>));

  EXPECT_EQ(
      (fixed_cost_skip_v<P, tc::list<TC>, std::vector<T>>),
      (fixed_cost_skip_v<
          P,
          tc::map<tc::integral, TC>,
          std::map<std::int8_t, T>>));

  EXPECT_EQ(
      (fixed_cost_skip_v<P, tc::list<TC>, std::vector<T>>),
      (fixed_cost_skip_v<
          P,
          tc::map<tc::integral, TC>,
          std::map<std::int16_t, T>>));

  EXPECT_FALSE((
      fixed_cost_skip_v<P, tc::map<tc::string, TC>, std::map<std::string, T>>));
}

TYPED_TEST(FixedCostSkipTest, SimpleJSON) {
  using P = SimpleJSONProtocolReader;
  using TC = typename TypeParam::first_type;
  using T = typename TypeParam::second_type;

  EXPECT_EQ(
      (std::is_same_v<TC, tc::integral> ||
       std::is_same_v<TC, tc::floating_point> ||
       std::is_same_v<TC, tc::enumeration>),
      (fixed_cost_skip_v<P, TC, T>));

  EXPECT_FALSE((fixed_cost_skip_v<P, tc::list<TC>, std::vector<T>>));

  EXPECT_FALSE((fixed_cost_skip_v<P, tc::set<TC>, std::set<T>>));

  EXPECT_FALSE((fixed_cost_skip_v<
                P,
                tc::map<tc::integral, TC>,
                std::map<std::int8_t, T>>));

  EXPECT_FALSE((fixed_cost_skip_v<
                P,
                tc::map<tc::integral, TC>,
                std::map<std::int16_t, T>>));

  EXPECT_FALSE((
      fixed_cost_skip_v<P, tc::map<tc::string, TC>, std::map<std::string, T>>));
}
