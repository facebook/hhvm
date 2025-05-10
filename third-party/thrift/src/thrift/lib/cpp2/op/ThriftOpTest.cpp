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

#include <gtest/gtest.h>
#include <thrift/conformance/data/ValueGenerator.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Testing.h>

namespace apache::thrift::op {
namespace {
using conformance::data::ValueGenerator;
using test::IsEmpty;
using test::IsEqualTo;
using test::IsIdenticalTo;
using ::testing::Not;

// A test suite that check ops work correctly for a given test case.
template <typename OpTestCase>
class TypedOpTest : public testing::Test {};

// Helpers for defining test cases.
template <typename Tag, typename T = type::standard_type<Tag>>
struct BaseTestCase {
  using type_tag = Tag;
  using type = T;
};
template <typename Tag, typename T = type::standard_type<Tag>>
struct BaseDefaultTestCase : BaseTestCase<Tag, T> {
  const static inline T default_ = {};
};
template <typename Tag, typename T = type::standard_type<Tag>>
struct NumericTestCase : BaseDefaultTestCase<Tag, T> {
  constexpr static T one = 1;
  constexpr static T otherOne = 1;
  constexpr static std::array<T, 2> many = {2, static_cast<T>(-4)};
};

template <typename Tag, typename T = type::standard_type<Tag>>
struct StringTestCase : BaseTestCase<Tag, T> {
  const static inline T default_ = StringTraits<T>::fromStringLiteral("");
  const static inline T one = StringTraits<T>::fromStringLiteral("one");
  const static inline T otherOne = StringTraits<T>::fromStringLiteral("one");
  const static inline std::array<T, 2> many = {
      StringTraits<T>::fromStringLiteral("two"),
      StringTraits<T>::fromStringLiteral("three")};
};

template <
    typename VTagCase,
    typename T = type::standard_type<type::list<typename VTagCase::type_tag>>>
struct ListTestCase
    : BaseDefaultTestCase<type::list<typename VTagCase::type_tag>, T> {
  const static inline T one = {VTagCase::many.begin(), VTagCase::many.end()};
  const static inline T otherOne = {
      VTagCase::many.begin(), VTagCase::many.end()};
  const static inline std::array<T, 3> many = {
      T{VTagCase::many.rbegin(), VTagCase::many.rend()},
      T{VTagCase::one},
      T{VTagCase::default_},
  };
};

template <
    typename KTagCase,
    typename T = type::standard_type<type::set<typename KTagCase::type_tag>>>
struct SetTestCase
    : BaseDefaultTestCase<type::set<typename KTagCase::type_tag>, T> {
  const static inline T one = {KTagCase::many.begin(), KTagCase::many.end()};
  const static inline T otherOne = {
      KTagCase::many.rbegin(), KTagCase::many.rend()};
  const static inline std::array<T, 3> many = {
      T{KTagCase::default_},
      T{KTagCase::default_, KTagCase::one},
      T{KTagCase::one},
  };
};

template <typename KTagCase, typename VTagCase>
using map_type_tag =
    type::map<typename KTagCase::type_tag, typename VTagCase::type_tag>;

template <
    typename KTagCase,
    typename VTagCase,
    typename T = type::standard_type<map_type_tag<KTagCase, VTagCase>>>
struct MapTestCase : BaseDefaultTestCase<map_type_tag<KTagCase, VTagCase>, T> {
  const static inline T one = {
      {KTagCase::one, VTagCase::one}, {KTagCase::default_, VTagCase::one}};
  const static inline T otherOne = {
      {KTagCase::default_, VTagCase::one}, {KTagCase::one, VTagCase::one}};
  const static inline std::array<T, 3> many = {
      T{{KTagCase::one, VTagCase::one}},
      T{{KTagCase::default_, VTagCase::one}},
      T{{KTagCase::one, VTagCase::default_}},
  };
};

// The tests cases to run.
using OpTestCases = ::testing::Types<
    NumericTestCase<type::byte_t>,
    NumericTestCase<type::i16_t>,
    // TODO(dokwon): Add support to cpp_type
    // NumericTestCase<type::i16_t, uint16_t>,
    NumericTestCase<type::i32_t>,
    // TODO(dokwon): Add support to cpp_type
    // NumericTestCase<type::i32_t, uint32_t>,
    NumericTestCase<type::i64_t>,
    // TODO(dokwon): Add support to cpp_type
    // NumericTestCase<type::i64_t, uint64_t>,
    NumericTestCase<type::float_t>,
    NumericTestCase<type::double_t>,
    StringTestCase<type::string_t>,
    // TODO(dokwon): Add support to cpp_type
    // StringTestCase<type::binary_t, folly::IOBuf>,
    StringTestCase<type::binary_t>,
    // TODO(afuller): Fix 'copyability' for this type, so we can test this case.
    // StringTestCase<type::binary_t, std::unique_ptr<folly::IOBuf>>,
    ListTestCase<NumericTestCase<type::byte_t>>,
    ListTestCase<StringTestCase<type::binary_t>>,
    // TODO(afuller): Consider supporting non-default standard types in
    // the paramaterized types, for these tests.
    // ListTestCase<StringTestCase<type::binary_t, folly::IOBuf>>,
    SetTestCase<NumericTestCase<type::i32_t, uint32_t>>,
    MapTestCase<StringTestCase<type::string_t>, NumericTestCase<type::i32_t>>>;

TYPED_TEST_SUITE(TypedOpTest, OpTestCases);

TYPED_TEST(TypedOpTest, Equal) {
  using Tag = typename TypeParam::type_tag;

  EXPECT_THAT(TypeParam::default_, IsEqualTo<Tag>(TypeParam::default_));
  EXPECT_THAT(TypeParam::one, IsEqualTo<Tag>(TypeParam::one));
  EXPECT_THAT(TypeParam::one, IsEqualTo<Tag>(TypeParam::otherOne));

  EXPECT_THAT(TypeParam::default_, Not(IsEqualTo<Tag>(TypeParam::one)));
  EXPECT_THAT(TypeParam::one, Not(IsEqualTo<Tag>(TypeParam::default_)));
  for (const auto& other : TypeParam::many) {
    EXPECT_THAT(TypeParam::one, Not(IsEqualTo<Tag>(other)));
    EXPECT_THAT(other, Not(IsEqualTo<Tag>(TypeParam::one)));
  }
}

TYPED_TEST(TypedOpTest, Empty) {
  using Tag = typename TypeParam::type_tag;

  EXPECT_THAT(TypeParam::default_, IsEmpty<Tag>());
  EXPECT_THAT(TypeParam::one, Not(IsEmpty<Tag>()));
  for (const auto& other : TypeParam::many) {
    EXPECT_THAT(other, Not(IsEmpty<Tag>()));
  }
}

TYPED_TEST(TypedOpTest, Clear) {
  using T = typename TypeParam::type;
  using Tag = typename TypeParam::type_tag;

  T value = TypeParam::one;
  EXPECT_THAT(value, Not(IsEmpty<Tag>()));
  EXPECT_THAT(value, Not(IsIdenticalTo<Tag>(TypeParam::default_)));
  EXPECT_THAT(value, IsIdenticalTo<Tag>(TypeParam::one));
  clear<Tag>(value);
  EXPECT_THAT(value, IsEmpty<Tag>());
  EXPECT_THAT(value, IsIdenticalTo<Tag>(TypeParam::default_));
  EXPECT_THAT(value, Not(IsIdenticalTo<Tag>(TypeParam::one)));
}

TYPED_TEST(TypedOpTest, Hash) {
  using Tag = typename TypeParam::type_tag;
  EXPECT_NE(op::hash<Tag>(TypeParam::default_), op::hash<Tag>(TypeParam::one));
}

TYPED_TEST(TypedOpTest, HashQuality) {
  using Tag = typename TypeParam::type_tag;
  std::set<size_t> seen;
  // All the 'interesting' key values should not collide.
  for (const auto& val : ValueGenerator<Tag>::getKeyValues()) {
    EXPECT_TRUE(seen.insert(op::hash<Tag>(val.value)).second) << val.name;
  }
}

} // namespace
} // namespace apache::thrift::op
