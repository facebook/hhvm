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

#include <array>

#include <thrift/lib/cpp2/op/Compare.h>

#include <folly/portability/GTest.h>
#include <thrift/conformance/if/gen-cpp2/object_types.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/Module_types_custom_protocol.h>

namespace apache::thrift::op {
namespace {
using conformance::Value;

TEST(CompareTest, Double) {
  // 1 is equal and identical to itself.
  EXPECT_TRUE(equal<type::double_t>(1.0, 1.0));
  EXPECT_TRUE(identical<type::double_t>(1.0, 1.0));

  // 1 is neither equal or identical to 2.
  EXPECT_FALSE(equal<type::double_t>(1.0, 2.0));
  EXPECT_FALSE(identical<type::double_t>(1.0, 2.0));

  // -0 is equal to, but not identical to 0.
  EXPECT_TRUE(equal<type::double_t>(-0.0, +0.0));
  EXPECT_FALSE(identical<type::double_t>(-0.0, +0.0));

  // NaN is identical to, but not equal to itself.
  EXPECT_FALSE(equal<type::double_t>(
      std::numeric_limits<double>::quiet_NaN(),
      std::numeric_limits<double>::quiet_NaN()));
  EXPECT_TRUE(identical<type::double_t>(
      std::numeric_limits<double>::quiet_NaN(),
      std::numeric_limits<double>::quiet_NaN()));
}

TEST(CompareTest, Float) {
  // 1 is equal and identical to itself.
  EXPECT_TRUE(equal<type::float_t>(1.0f, 1.0f));
  EXPECT_TRUE(identical<type::float_t>(1.0f, 1.0f));

  // 1 is neither equal or identical to 2.
  EXPECT_FALSE(equal<type::float_t>(1.0f, 2.0f));
  EXPECT_FALSE(identical<type::float_t>(1.0f, 2.0f));

  // -0 is equal to, but not identical to 0.
  EXPECT_TRUE(equal<type::float_t>(-0.0f, +0.0f));
  EXPECT_FALSE(identical<type::float_t>(-0.0f, +0.0f));

  // NaN is identical to, but not equal to itself.
  EXPECT_FALSE(equal<type::float_t>(
      std::numeric_limits<float>::quiet_NaN(),
      std::numeric_limits<float>::quiet_NaN()));
  EXPECT_TRUE(identical<type::float_t>(
      std::numeric_limits<float>::quiet_NaN(),
      std::numeric_limits<float>::quiet_NaN()));
}

TEST(CompareTest, StructWithFloat) {
  Value lhs;
  Value rhs;
  EqualTo<type::struct_t<Value>> equal;
  IdenticalTo<type::struct_t<Value>> identical;

  lhs.floatValue_ref().ensure() = std::numeric_limits<float>::quiet_NaN();
  rhs.floatValue_ref().ensure() = std::numeric_limits<float>::quiet_NaN();
  EXPECT_FALSE(equal(lhs, rhs));
  EXPECT_FALSE(identical(lhs, rhs)); // Should be true!

  lhs.floatValue_ref().ensure() = -0.0f;
  rhs.floatValue_ref().ensure() = +0.0f;
  EXPECT_TRUE(equal(lhs, rhs));
  EXPECT_TRUE(identical(lhs, rhs)); // Should be false!
}

TEST(CompareTest, ListWithDouble) {
  EqualTo<type::list<type::double_t>> equal;
  IdenticalTo<type::list<type::double_t>> identical;

  EXPECT_FALSE(equal(
      {1, std::numeric_limits<float>::quiet_NaN()},
      {1, std::numeric_limits<float>::quiet_NaN()}));
  EXPECT_TRUE(identical(
      {1, std::numeric_limits<float>::quiet_NaN()},
      {1, std::numeric_limits<float>::quiet_NaN()}));

  EXPECT_TRUE(equal({-0.0, 2.0}, {+0.0, 2.0}));
  EXPECT_FALSE(identical({-0.0, 2.0}, {+0.0, 2.0}));
}

TEST(CompareTest, SetWithDouble) {
  EqualTo<type::set<type::double_t>> equal;
  IdenticalTo<type::set<type::double_t>> identical;

  // Note: NaN in a set is undefined behavior.
  EXPECT_TRUE(equal({-0.0, 2.0}, {+0.0, 2.0}));
  EXPECT_FALSE(identical({-0.0, 2.0}, {+0.0, 2.0}));
}

TEST(CompareTest, MapWithDouble) {
  EqualTo<type::map<type::double_t, type::float_t>> equal;
  IdenticalTo<type::map<type::double_t, type::float_t>> identical;

  // Note: NaN in a map keys is undefined behavior.
  EXPECT_FALSE(equal(
      {{1, std::numeric_limits<float>::quiet_NaN()}},
      {{1, std::numeric_limits<float>::quiet_NaN()}}));
  EXPECT_TRUE(identical(
      {{1, std::numeric_limits<float>::quiet_NaN()}},
      {{1, std::numeric_limits<float>::quiet_NaN()}}));

  EXPECT_TRUE(equal({{-0.0, 2.0}}, {{+0.0, 2.0}}));
  EXPECT_FALSE(identical({{-0.0, 2.0}}, {{+0.0, 2.0}}));
  EXPECT_TRUE(equal({{2.0, +0.0}}, {{2.0, -0.0}}));
  EXPECT_FALSE(identical({{2.0, +0.0}}, {{2.0, -0.0}}));
  EXPECT_TRUE(equal({{-0.0, +0.0}}, {{+0.0, -0.0}}));
  EXPECT_FALSE(identical({{-0.0, +0.0}}, {{+0.0, -0.0}}));
}

// Sets and maps that use representational uniqueness.
template <typename KeyTag, typename T = type::native_type<KeyTag>>
using InternSet = std::unordered_set<T, Hash<KeyTag>, IdenticalTo<KeyTag>>;
template <
    typename KeyTag,
    typename ValTag,
    typename K = type::native_type<KeyTag>,
    typename V = type::native_type<ValTag>>
using InternMap = std::unordered_map<K, V, Hash<KeyTag>, IdenticalTo<KeyTag>>;

TEST(CompareTest, Collisions) {
  // Use intern set/map's of 0.0 and -0.0 to make it easy to cause a collision,
  // as op::hash(0.0) must equal op::hash(-0.0).
  auto hash = op::hash<type::double_t>;
  EXPECT_EQ(hash(0.0), hash(-0.0));
  EXPECT_FALSE(identical<type::double_t>(0.0, -0.0));
  using set_t = type::set<type::double_t>;
  using InternDblSet = InternSet<type::double_t>;
  InternDblSet set{0.0, -0.0};
  EXPECT_EQ(set.size(), 2);

  // identical and equal to a copy of itself.
  EXPECT_TRUE(identical<set_t>(set, InternDblSet(set)));
  EXPECT_TRUE(equal<set_t>(set, InternDblSet(set)));

  using map_t = type::map<type::float_t, type::float_t>;
  using InternFloatMap = InternMap<type::float_t, type::float_t>;
  InternFloatMap map{{0.0f, 0.0f}, {-0.0f, 0.0f}};
  EXPECT_EQ(map.size(), 2);

  // identical and equal to a copy of itself.
  EXPECT_TRUE(identical<map_t>(map, InternFloatMap(map)));
  EXPECT_TRUE(equal<map_t>(map, InternFloatMap(map)));

  // Equal, but not identical to a map with equal but not identical values.
  InternFloatMap otherMap{{0.0f, 0.0f}, {-0.0f, -0.0f}};
  EXPECT_FALSE(identical<map_t>(map, otherMap));
  EXPECT_TRUE(equal<map_t>(map, otherMap));
}

} // namespace
} // namespace apache::thrift::op
