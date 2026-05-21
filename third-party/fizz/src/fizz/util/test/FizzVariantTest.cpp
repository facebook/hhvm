/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/util/Status.h>
#include <fizz/util/Variant.h>

namespace fizz {
namespace test {

struct Cat {
  Cat() = default;
  Cat(Cat&&) {
    adopted = true;
  }
  std::string meow() {
    return "meow";
  }
  bool adopted{false};
};

struct Dog {
  Dog() = default;
  Dog(Dog&&) {
    adopted = true;
  }
  std::string woof() {
    return "woof";
  }
  bool adopted{false};
};

bool operator==(const Cat& lhs, const Cat& rhs) {
  return lhs.adopted == rhs.adopted;
}

bool operator==(const Dog& lhs, const Dog& rhs) {
  return lhs.adopted == rhs.adopted;
}

#define FIZZ_TEST_VARIANT(F, ...) \
  F(Cat, __VA_ARGS__)             \
  F(Dog, __VA_ARGS__)

FIZZ_DECLARE_VARIANT_TYPE(TestVariant, FIZZ_TEST_VARIANT)

TEST(VariantTest, TypeMatch) {
  TestVariant aDog{Dog()};
  TestVariant aCat{Cat()};
  EXPECT_EQ(TestVariant::Type::Cat_E, aCat.type());
  EXPECT_EQ(TestVariant::Type::Dog_E, aDog.type());
  EXPECT_EQ(nullptr, aDog.asCat());
  EXPECT_EQ(nullptr, aCat.asDog());
  EXPECT_EQ("meow", aCat.asCat()->meow());
  EXPECT_EQ("woof", aDog.asDog()->woof());
}

TEST(VariantTest, Move) {
  Dog aDog;
  EXPECT_FALSE(aDog.adopted);
  Cat aCat;
  EXPECT_FALSE(aCat.adopted);
  TestVariant hotDog{std::move(aDog)};
  TestVariant koolCat{std::move(aCat)};
  EXPECT_TRUE(koolCat.asCat()->adopted);
  EXPECT_TRUE(hotDog.asDog()->adopted);
}

TEST(VariantTest, Magic) {
  TestVariant aDog{Dog()};
  TestVariant aCat{Cat()};
  EXPECT_EQ(TestVariant::Type::Dog_E, aDog.type());
  EXPECT_EQ(TestVariant::Type::Cat_E, aCat.type());
  aDog = std::move(aCat);
  EXPECT_EQ(TestVariant::Type::Cat_E, aDog.type());
}

TEST(VariantTest, TryAs) {
  TestVariant aDog{Dog()};
  TestVariant aCat{Cat()};
  Error err;
  Cat* catPtr = nullptr;
  EXPECT_THROW(
      FIZZ_THROW_ON_ERROR(aDog.tryAsCat(catPtr, err), err), std::runtime_error);
  Dog* dogPtr = nullptr;
  EXPECT_THROW(
      FIZZ_THROW_ON_ERROR(aCat.tryAsDog(dogPtr, err), err), std::runtime_error);

  EXPECT_EQ(aCat.tryAsCat(catPtr, err), Status::Success);
  EXPECT_EQ("meow", catPtr->meow());
  EXPECT_EQ(aDog.tryAsDog(dogPtr, err), Status::Success);
  EXPECT_EQ("woof", dogPtr->woof());
}

} // namespace test
} // namespace fizz
