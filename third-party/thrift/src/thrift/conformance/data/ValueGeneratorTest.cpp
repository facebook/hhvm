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

#include <thrift/conformance/data/ValueGenerator.h>

#include <cmath>
#include <ostream>
#include <set>
#include <type_traits>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/type/Name.h>

namespace apache::thrift::conformance::data {

template <typename... Tags>
struct types {
  // Converts the type list to a type list of the given types.
  template <template <typename...> class T>
  using as = T<Tags...>;

  template <typename F>
  using filter = typename fatal::filter<types, F>;

  template <typename CTag>
  using of = filter<type::bound::is_a<CTag>>;
};

using primitive_types = types<
    type::bool_t,
    type::byte_t,
    type::i16_t,
    type::i32_t,
    type::i64_t,
    type::float_t,
    type::double_t,
    type::enum_c,
    type::string_t,
    type::binary_t>;

template <typename T>
std::ostream& operator<<(std::ostream& os, const NamedValue<T>& value) {
  return os << ::testing::PrintToString(value.value) << ":" << value.name;
}

namespace {

template <typename C>
::testing::AssertionResult includesNan(const C& container) {
  for (const auto& value : container) {
    if (std::isnan(value.value)) {
      return ::testing::AssertionSuccess();
    }
  }
  return ::testing::AssertionFailure();
}

template <typename C>
::testing::AssertionResult includesNegZero(const C& container) {
  for (const auto& value : container) {
    if (value.value == 0 && std::signbit(value.value)) {
      return ::testing::AssertionSuccess();
    }
  }
  return ::testing::AssertionFailure();
}

template <typename C>
::testing::AssertionResult allUnique(const C& container) {
  for (auto outer = container.begin(); outer != container.end(); ++outer) {
    for (auto inner = outer + 1; inner != container.end(); ++inner) {
      if (inner->value == outer->value) {
        return ::testing::AssertionFailure()
            << *inner << " at " << std::distance(container.begin(), inner)
            << " in " << testing::PrintToString(container);
      }
    }
  }
  return ::testing::AssertionSuccess();
}

template <typename Tag>
class ValueGeneratorTest : public ::testing::Test {
 public:
  using standard_type = type::standard_type<Tag>;
  static inline const auto values = ValueGenerator<Tag>::getInterestingValues();
  static inline const auto keyValues = ValueGenerator<Tag>::getKeyValues();
};

template <typename T>
class FloatingPointGeneratorTest : public ValueGeneratorTest<T> {};

// TODO(afuller): Expand the set of types tested.
using ValueGeneratorTestTypes =
    primitive_types::filter<type::bound::is_concrete>::as<::testing::Types>;
TYPED_TEST_CASE(ValueGeneratorTest, ValueGeneratorTestTypes);
using FloatingPointTestTypes =
    primitive_types::of<type::floating_point_c>::as<::testing::Types>;
TYPED_TEST_CASE(FloatingPointGeneratorTest, FloatingPointTestTypes);

TYPED_TEST(ValueGeneratorTest, UniqueKeys) {
  SCOPED_TRACE(type::getName<TypeParam>());
  EXPECT_TRUE(allUnique(this->keyValues));
}

TYPED_TEST(ValueGeneratorTest, List_Interesting) {
  SCOPED_TRACE(type::getName<TypeParam>());
  auto lists = ValueGenerator<type::list<TypeParam>>::getInterestingValues();
  size_t count = this->values.size();
  size_t extra = count > 2 ? 2 : (count == 2 ? 1 : 0);
  ASSERT_EQ(lists.size(), 3 + extra);
  EXPECT_EQ(lists[0].value.size(), 0); // empty
  EXPECT_EQ(lists[1].value.size(), count); // all values.
  EXPECT_EQ(lists[2].value.size(), 2 * count); // all values twice.
  // Different orderings.
  for (auto itr = lists.begin() + 3; itr != lists.end(); ++itr) {
    EXPECT_EQ(itr->value.size(), count);
  }
}

TYPED_TEST(ValueGeneratorTest, List_Key) {
  SCOPED_TRACE(type::getName<TypeParam>());
  auto lists = ValueGenerator<type::list<TypeParam>>::getKeyValues();
  size_t count = this->keyValues.size();
  size_t extra = count > 2 ? 2 : (count == 2 ? 1 : 0);
  ASSERT_EQ(lists.size(), 3 + extra);
  EXPECT_EQ(lists[0].value.size(), 0); // empty
  EXPECT_EQ(lists[1].value.size(), count); // all values.
  EXPECT_EQ(lists[2].value.size(), 2 * count); // all values twice.
  // Different orderings.
  for (auto itr = lists.begin() + 3; itr != lists.end(); ++itr) {
    EXPECT_EQ(itr->value.size(), count);
  }
  EXPECT_TRUE(allUnique(lists));
}

TYPED_TEST(ValueGeneratorTest, Set) {
  SCOPED_TRACE(type::getName<TypeParam>());
  auto sets = ValueGenerator<type::set<TypeParam>>::getInterestingValues();
  EXPECT_EQ(ValueGenerator<type::set<TypeParam>>::getKeyValues(), sets);
  ASSERT_EQ(sets.size(), 2 + this->keyValues.size());
  EXPECT_EQ(sets[0].value.size(), 0);
  EXPECT_EQ(sets[1].value.size(), this->keyValues.size());
  for (size_t i = 2; i < sets.size(); ++i) {
    EXPECT_EQ(sets[i].value.size(), 1);
  }
  EXPECT_TRUE(allUnique(sets));
}

TYPED_TEST(ValueGeneratorTest, Map_Interesting) {
  SCOPED_TRACE(type::getName<TypeParam>());
  size_t valueCount = this->values.size();
  size_t keyCount = this->keyValues.size();
  auto maps =
      ValueGenerator<type::map<TypeParam, TypeParam>>::getInterestingValues();
  ASSERT_EQ(maps.size(), 1 + valueCount + valueCount * keyCount);
  EXPECT_EQ(maps[0].value.size(), 0);
  for (size_t i = 1; i < maps.size(); ++i) {
    // keyCount of key -> all values followed by 1 all keys -> value
    size_t expected = ((i - 1) % (keyCount + 1)) == keyCount ? keyCount : 1;
    EXPECT_EQ(maps[i].value.size(), expected) << i;
  }
}

TYPED_TEST(ValueGeneratorTest, Map_Keys) {
  SCOPED_TRACE(type::getName<TypeParam>());
  size_t valueCount = this->keyValues.size();
  size_t keyCount = this->keyValues.size();
  auto maps = ValueGenerator<type::map<TypeParam, TypeParam>>::getKeyValues();
  ASSERT_EQ(maps.size(), 1 + valueCount + valueCount * keyCount);
  EXPECT_EQ(maps[0].value.size(), 0);
  for (size_t i = 1; i < maps.size(); ++i) {
    // keyCount of key -> all values followed by 1 all keys -> value
    size_t expected = ((i - 1) % (keyCount + 1)) == keyCount ? keyCount : 1;
    EXPECT_EQ(maps[i].value.size(), expected) << i;
  }
  EXPECT_TRUE(allUnique(maps));
}

TYPED_TEST(FloatingPointGeneratorTest, Nan) {
  SCOPED_TRACE(type::getName<TypeParam>());
  EXPECT_TRUE(includesNan(this->values));
  EXPECT_FALSE(includesNan(this->keyValues));
}

TYPED_TEST(FloatingPointGeneratorTest, NegZero) {
  SCOPED_TRACE(type::getName<TypeParam>());
  EXPECT_TRUE(includesNegZero(this->values));
  EXPECT_FALSE(includesNegZero(this->keyValues));
}

} // namespace
} // namespace apache::thrift::conformance::data
