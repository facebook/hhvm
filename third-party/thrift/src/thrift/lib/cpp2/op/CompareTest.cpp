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
#include <thrift/lib/cpp2/op/detail/Compare.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/Module_types_custom_protocol.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_types_custom_protocol.h>

namespace apache::thrift::op {
namespace {

TEST(CompareTest, IOBuf) {
  using STag = type::cpp_type<folly::IOBuf, type::string_t>;
  using BTag = type::cpp_type<folly::IOBuf, type::binary_t>;

  auto one = IOBuf::wrapBufferAsValue("a", 1);
  auto two = IOBuf::wrapBufferAsValue("ab", 2);

  EXPECT_TRUE(op::equal<STag>(one, one));
  EXPECT_FALSE(op::equal<BTag>(one, two));
  EXPECT_FALSE((op::equal<STag, BTag>(two, one)));
  EXPECT_TRUE((op::equal<BTag, STag>(two, two)));

  EXPECT_FALSE(op::less<STag>(one, one));
  EXPECT_TRUE(op::less<BTag>(one, two));
  EXPECT_FALSE((op::less<STag, BTag>(two, one)));
  EXPECT_FALSE((op::less<BTag, STag>(two, two)));

  EXPECT_EQ(op::compare<STag>(one, one), folly::ordering::eq);
  EXPECT_EQ(op::compare<BTag>(one, two), folly::ordering::lt);
  EXPECT_EQ((op::compare<STag, BTag>(two, one)), folly::ordering::gt);
  EXPECT_EQ((op::compare<BTag, STag>(two, two)), folly::ordering::eq);
}

TEST(CompareTest, Double) {
  auto hash = op::hash<type::double_t>;
  EXPECT_EQ(hash(0.0), hash(-0.0));

  // 1 is identical and equal to itself.
  EXPECT_TRUE(identical<double>(1.0, 1.0));
  EXPECT_TRUE(equal<double>(1.0, 1.0));
  EXPECT_FALSE(less<double>(1.0, 1.0));
  EXPECT_EQ(compare<double>(1.0, 1.0), folly::ordering::eq);

  // 1 is neither identical or equal to 2.
  EXPECT_FALSE(identical<type::double_t>(1.0, 2.0));
  EXPECT_FALSE(equal<type::double_t>(1.0, 2.0));
  EXPECT_TRUE(less<type::double_t>(1.0, 2.0));
  EXPECT_EQ(compare<type::double_t>(1.0, 2.0), folly::ordering::lt);

  // -0 is equal to, but not identical to 0.
  EXPECT_FALSE(identical<>(-0.0, +0.0));
  EXPECT_TRUE(equal<>(-0.0, +0.0));
  EXPECT_FALSE(less<>(-0.0, +0.0));
  EXPECT_EQ(compare<>(-0.0, +0.0), folly::ordering::eq);

  // NaN is identical to, but not equal to itself.
  EXPECT_TRUE(identical<type::double_t>(
      std::numeric_limits<double>::quiet_NaN(),
      std::numeric_limits<double>::quiet_NaN()));
  EXPECT_FALSE(equal<type::double_t>(
      std::numeric_limits<double>::quiet_NaN(),
      std::numeric_limits<double>::quiet_NaN()));
  EXPECT_FALSE(less<type::double_t>(
      std::numeric_limits<double>::quiet_NaN(),
      std::numeric_limits<double>::quiet_NaN()));
  EXPECT_EQ(
      compare<type::double_t>(
          std::numeric_limits<double>::quiet_NaN(),
          std::numeric_limits<double>::quiet_NaN()),
      folly::ordering::gt);
}

TEST(CompareTest, Float) {
  auto hash = op::hash<type::float_t>;
  EXPECT_EQ(hash(0.0f), hash(-0.0f));

  // 1 is equal and identical to itself.
  EXPECT_TRUE(equal<float>(1.0f, 1.0f));
  EXPECT_TRUE(identical<float>(1.0f, 1.0f));

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
  protocol::Value lhs;
  protocol::Value rhs;
  EqualTo<type::struct_t<protocol::Value>> equal;
  IdenticalTo<type::struct_t<protocol::Value>> identical;

  lhs.floatValue_ref().ensure() = std::numeric_limits<float>::quiet_NaN();
  rhs.floatValue_ref().ensure() = std::numeric_limits<float>::quiet_NaN();
  EXPECT_FALSE(op::equal<protocol::Value>(lhs, rhs));
  EXPECT_FALSE(op::identical<protocol::Value>(lhs, rhs)); // Should be true!

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
template <typename KTag, typename T = type::native_type<KTag>>
using InternSet = std::unordered_set<T, Hash<KTag>, IdenticalTo<KTag>>;
template <typename KTag>
using InternSetTag = type::cpp_type<InternSet<KTag>, type::set<KTag>>;

TEST(CompareTest, InternSet_Dbl) {
  using Tag = InternSetTag<type::double_t>;
  using SetT = type::native_type<Tag>;
  static_assert(detail::less_than_comparable_v<Tag>, "");
  static_assert(detail::comparable_v<Tag>, "");

  EXPECT_FALSE(identical<Tag>({0.0}, {-0.0}));
  EXPECT_TRUE(equal<Tag>({0.0}, {-0.0}));
  EXPECT_TRUE(less<Tag>({0}, {1}));
  EXPECT_FALSE(less<Tag>({0, 1}, {1, 0}));

  SetT set{0.0, -0.0};
  EXPECT_EQ(set.size(), 2);
  EXPECT_TRUE(identical<Tag>(set, SetT(set)));
  EXPECT_TRUE(equal<Tag>(set, SetT(set)));
  EXPECT_FALSE(less<Tag>(set, SetT(set)));
}

template <
    typename KTag,
    typename VTag,
    typename K = type::native_type<KTag>,
    typename V = type::native_type<VTag>>
using InternMap = std::unordered_map<K, V, Hash<KTag>, IdenticalTo<KTag>>;
template <typename KTag, typename VTag>
using InternMapTag =
    type::cpp_type<InternMap<KTag, VTag>, type::map<KTag, VTag>>;

TEST(CompareTest, InternMap_Flt) {
  using Tag = InternMapTag<type::float_t, type::float_t>;
  using MapT = type::native_type<Tag>;
  static_assert(detail::less_than_comparable_v<Tag>, "");
  static_assert(detail::comparable_v<Tag>, "");

  MapT map{{0.0f, 0.0f}, {-0.0f, 0.0f}};
  EXPECT_EQ(map.size(), 2);

  // identical and equal to a copy of itself.
  EXPECT_TRUE(identical<Tag>(map, MapT(map)));
  EXPECT_TRUE(equal<Tag>(map, MapT(map)));
  EXPECT_FALSE(less<Tag>(map, MapT(map)));

  // Equal, but not identical to a map with equal but not identical values.
  MapT otherMap{{0.0f, 0.0f}, {-0.0f, -0.0f}};
  EXPECT_FALSE(identical<Tag>(map, otherMap));
  EXPECT_TRUE(equal<Tag>(map, otherMap));
  EXPECT_FALSE(less<Tag>(map, otherMap));

  MapT largerMap{{1.0f, 0.0f}};
  EXPECT_FALSE(identical<Tag>(map, largerMap));
  EXPECT_FALSE(equal<Tag>(map, largerMap));
  EXPECT_TRUE(less<Tag>(map, largerMap));
}

TEST(CompareTest, Struct) {
  detail::StructEquality equalTo;
  detail::StructLessThan lessThan;
  test::OneOfEach lhs;
  test::OneOfEach rhs;
  EXPECT_TRUE(equalTo(lhs, rhs));
  EXPECT_FALSE(lessThan(lhs, rhs));

  --*lhs.myStruct()->mySubI64();
  EXPECT_FALSE(equalTo(lhs, rhs));
  EXPECT_TRUE(lessThan(lhs, rhs));

  --*rhs.myStruct()->mySubI64();
  EXPECT_TRUE(equalTo(lhs, rhs));
  EXPECT_FALSE(lessThan(lhs, rhs));

  // compare lhs with lhs
  EXPECT_TRUE(equalTo(lhs, lhs));
  EXPECT_FALSE(lessThan(lhs, lhs));
}

TEST(CompareTest, UnorderedFields) {
  detail::StructEquality equalTo;
  detail::StructLessThan lessThan;

  test::CppTemplateListField lhs;
  test::CppTemplateListField rhs;
  EXPECT_TRUE(equalTo(lhs, rhs));
  EXPECT_FALSE(lessThan(lhs, rhs));

  lhs.f1()->push_front("0");
  EXPECT_FALSE(equalTo(lhs, rhs));
  EXPECT_TRUE(lessThan(lhs, rhs));

  rhs.f1()->push_front("0");
  EXPECT_TRUE(equalTo(lhs, rhs));
  EXPECT_FALSE(lessThan(lhs, rhs));

  // compare lhs with lhs
  EXPECT_TRUE(equalTo(lhs, lhs));
  EXPECT_FALSE(lessThan(lhs, lhs));
}

TEST(CompareTest, Union) {
  detail::UnionEquality equalTo;
  detail::UnionLessThan lessThan;
  test::UnionIntegers empty, u1, u2;
  u1.myI16_ref() = 10;
  u2.myI32_ref() = 1;
  EXPECT_TRUE(lessThan(empty, u1));
  EXPECT_FALSE(equalTo(empty, u1));
  EXPECT_TRUE(lessThan(u1, u2));
  EXPECT_FALSE(equalTo(u1, u2));
  EXPECT_FALSE(lessThan(u1, u1));
  EXPECT_TRUE(equalTo(u1, u1));
  EXPECT_FALSE(lessThan(u2, u1));
  EXPECT_FALSE(equalTo(u2, u1));
  EXPECT_FALSE(lessThan(empty, empty));
  EXPECT_TRUE(equalTo(empty, empty));

  u2.myI16_ref() = 10;
  EXPECT_FALSE(lessThan(u1, u2));
  EXPECT_TRUE(equalTo(u1, u2));
}

TEST(CompareTest, ListIOBufCompare) {
  detail::StructEquality equalTo;
  detail::StructLessThan lessThan;

  test::ListIOBuf lhs, rhs;
  EXPECT_FALSE(lessThan(lhs, rhs));
  EXPECT_TRUE(equalTo(lhs, rhs));

  lhs.field()->push_back(*folly::IOBuf::copyBuffer("a", 1));
  rhs.field()->push_back(*folly::IOBuf::copyBuffer("b", 1));
  EXPECT_TRUE(lessThan(lhs, rhs));
  EXPECT_FALSE(equalTo(lhs, rhs));

  lhs.field()->insert(lhs.field()->begin(), *folly::IOBuf::copyBuffer("b", 1));
  rhs.field()->push_back(*folly::IOBuf::copyBuffer("a", 1));
  EXPECT_FALSE(lessThan(lhs, rhs));
  EXPECT_TRUE(equalTo(lhs, rhs));
}

TEST(CompareTest, MapIOBufCompare) {
  detail::StructEquality equalTo;
  detail::StructLessThan lessThan;

  test::ListIOBuf lhs, rhs;
  EXPECT_FALSE(lessThan(lhs, rhs));
  EXPECT_TRUE(equalTo(lhs, rhs));

  lhs.field_2()["10"];
  rhs.field_2()["10"] = *folly::IOBuf::copyBuffer("a", 1);
  EXPECT_TRUE(lessThan(lhs, rhs));
  EXPECT_FALSE(equalTo(lhs, rhs));

  lhs.field_2()["10"] = *folly::IOBuf::copyBuffer("a", 1);
  EXPECT_FALSE(lessThan(lhs, rhs));
  EXPECT_TRUE(equalTo(lhs, rhs));

  rhs.field_2()["11"];
  EXPECT_TRUE(lessThan(lhs, rhs));
  EXPECT_FALSE(equalTo(lhs, rhs));
}

TEST(CompareTest, Nan) {
  test::OneOfEach value;
  value.myDouble() = NAN;
  EXPECT_FALSE(detail::StructEquality{}(value, value));
  EXPECT_FALSE(detail::StructLessThan{}(value, value));
}

} // namespace
} // namespace apache::thrift::op
