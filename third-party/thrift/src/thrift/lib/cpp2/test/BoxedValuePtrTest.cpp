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

#include <stdexcept>
#include <type_traits>

#include <gtest/gtest.h>
#include <folly/Portability.h>
#include <thrift/lib/cpp2/BoxedValuePtr.h>

using namespace apache::thrift::detail;

struct TestStruct {
  using __fbthrift_cpp2_type = TestStruct;
  static constexpr bool __fbthrift_cpp2_is_union = false;

  TestStruct() : a(0) {}
  explicit TestStruct(int val) : a(val) {}
  void set_a(int b) { a = b; }
  int a;
};

constexpr bool operator==(const TestStruct& lhs, const TestStruct& rhs) {
  return lhs.a == rhs.a;
}
constexpr bool operator==(const TestStruct& lhs, int rhs) {
  return lhs.a == rhs;
}
constexpr bool operator==(int lhs, const TestStruct& rhs) {
  return lhs == rhs.a;
}

constexpr bool operator<(const TestStruct& lhs, const TestStruct& rhs) {
  return lhs.a < rhs.a;
}
constexpr bool operator<(const TestStruct& lhs, int rhs) {
  return lhs.a < rhs;
}
constexpr bool operator<(int lhs, const TestStruct& rhs) {
  return lhs < rhs.a;
}

TEST(BoxedValuePtrTest, DefaultConstructor) {
  boxed_value_ptr<TestStruct> a;
  EXPECT_FALSE(static_cast<bool>(a));
}

TEST(BoxedValuePtrTest, Constructor) {
  boxed_value_ptr<TestStruct> a(5);
  EXPECT_EQ(*a, 5);
}

TEST(BoxedValuePtrTest, CopyConstructor) {
  boxed_value_ptr<TestStruct> a(5);
  boxed_value_ptr<TestStruct> b(a);
  EXPECT_EQ(*b, 5);
  EXPECT_EQ(*a, 5);
}

TEST(BoxedValuePtrTest, CopyAssignment) {
  boxed_value_ptr<TestStruct> a(5);
  boxed_value_ptr<TestStruct> b;
  b = a;
  EXPECT_EQ(*b, 5);
  EXPECT_EQ(*a, 5);
}

TEST(BoxedValuePtrTest, MoveConstructor) {
  boxed_value_ptr<TestStruct> a(5);
  boxed_value_ptr<TestStruct> b(std::move(a));
  EXPECT_EQ(*b, 5);
}

TEST(BoxedValuePtrTest, MoveAssignment) {
  boxed_value_ptr<TestStruct> a(5);
  boxed_value_ptr<TestStruct> b;
  b = std::move(a);
  EXPECT_EQ(*b, 5);
}

TEST(BoxedValuePtrTest, EmptyAssignment) {
  boxed_value_ptr<TestStruct> a;
  boxed_value_ptr<TestStruct> b(5);
  EXPECT_EQ(*b, 5);
  b = a;
  EXPECT_FALSE(static_cast<bool>(b));
}

TEST(BoxedValuePtrTest, Emplace) {
  boxed_value_ptr<TestStruct> a;
  a.emplace(5);
  EXPECT_EQ(*a, 5);
  a.emplace(7);
  EXPECT_EQ(*a, 7);
}

TEST(BoxedValuePtrTest, Reset) {
  boxed_value_ptr<TestStruct> a(6);
  a.reset();
  EXPECT_FALSE(static_cast<bool>(a));
}

TEST(BoxedValuePtrTest, Assignment) {
  boxed_value_ptr<TestStruct> a;
  a = 6;
  EXPECT_EQ(*a, 6);
}

TEST(BoxedValuePtrTest, MoveOnlyType) {
  boxed_value_ptr<std::unique_ptr<int>> a;
  a = std::make_unique<int>(5);
  EXPECT_EQ(**a, 5);
  boxed_value_ptr<std::unique_ptr<int>> b(std::move(a));
  EXPECT_EQ(**b, 5);
}

TEST(BoxedValuePtrTest, Swap) {
  boxed_value_ptr<TestStruct> a(5);
  boxed_value_ptr<TestStruct> b(7);
  std::swap(a, b);
  EXPECT_EQ(*a, 7);
  EXPECT_EQ(*b, 5);
}

TEST(BoxedValuePtrTest, Equal) {
  boxed_value_ptr<TestStruct> a;
  boxed_value_ptr<TestStruct> b;
  EXPECT_TRUE(a == b);
  a = 5;
  EXPECT_FALSE(a == b);
  b = 5;
  EXPECT_FALSE(a == b);
  b = 7;
  EXPECT_FALSE(a == b);
}

TEST(BoxedPtrTest, DefaultConstructor) {
  boxed_ptr<TestStruct> a;
  EXPECT_FALSE(static_cast<bool>(a));
}

TEST(BoxedPtrTest, FromStaticConstant) {
  const TestStruct t{5};
  boxed_ptr<TestStruct> a = boxed_ptr<TestStruct>::fromStaticConstant(&t);
  EXPECT_EQ(*a, 5);
}

TEST(BoxedPtrTest, UniquePtrConstructor) {
  boxed_ptr<TestStruct> a{std::make_unique<TestStruct>(5)};
  EXPECT_EQ(*a, 5);
}

TEST(BoxedPtrTest, CopyConstructorAndAssignment) {
  static_assert(!std::is_copy_constructible_v<boxed_ptr<TestStruct>>);
  static_assert(!std::is_copy_assignable_v<boxed_ptr<TestStruct>>);
}

TEST(BoxedPtrTest, MoveConstructor) {
  const TestStruct t{5};
  boxed_ptr<TestStruct> a = boxed_ptr<TestStruct>::fromStaticConstant(&t);
  boxed_ptr<TestStruct> b(std::move(a));
  EXPECT_EQ(*b, 5);
}

TEST(BoxedPtrTest, MoveAssignment) {
  const TestStruct t{5};
  {
    boxed_ptr<TestStruct> unowned =
        boxed_ptr<TestStruct>::fromStaticConstant(&t);
    boxed_ptr<TestStruct> b;
    b = std::move(unowned);
    EXPECT_EQ(*b, 5);
  }
  {
    boxed_ptr<TestStruct> owned{std::make_unique<TestStruct>(6)};
    boxed_ptr<TestStruct> b;
    b = std::move(owned);
    EXPECT_EQ(*b, 6);
  }
  {
    boxed_ptr<TestStruct> unowned =
        boxed_ptr<TestStruct>::fromStaticConstant(&t);
    boxed_ptr<TestStruct> b{std::make_unique<TestStruct>(7)};
    b = std::move(unowned);
    EXPECT_EQ(*b, 5);
  }
  {
    boxed_ptr<TestStruct> owned{std::make_unique<TestStruct>(6)};
    boxed_ptr<TestStruct> b{std::make_unique<TestStruct>(7)};
    b = std::move(owned);
    EXPECT_EQ(*b, 6);
  }
}

TEST(BoxedPtrTest, Reset) {
  const TestStruct t{5};
  boxed_ptr<TestStruct> a = boxed_ptr<TestStruct>::fromStaticConstant(&t);
  a.reset();
  EXPECT_FALSE(static_cast<bool>(a));

  a.reset(std::make_unique<TestStruct>(6));
  EXPECT_TRUE(static_cast<bool>(a));
  EXPECT_EQ(*a, 6);
}

TEST(BoxedPtrTest, Swap) {
  const TestStruct t1{5};
  const TestStruct t2{7};

  boxed_ptr<TestStruct> a = boxed_ptr<TestStruct>::fromStaticConstant(&t1);
  boxed_ptr<TestStruct> b = boxed_ptr<TestStruct>::fromStaticConstant(&t2);

  std::swap(a, b);
  EXPECT_EQ(*a, 7);
  EXPECT_EQ(*b, 5);
}

TEST(BoxedPtrTest, Equal) {
  const TestStruct t1{5};
  const TestStruct t2{7};

  boxed_ptr<TestStruct> a;
  boxed_ptr<TestStruct> b;
  EXPECT_TRUE(a == b);
  a = boxed_ptr<TestStruct>::fromStaticConstant(&t1);
  EXPECT_FALSE(a == b);
  b = boxed_ptr<TestStruct>::fromStaticConstant(&t1);
  EXPECT_TRUE(a == b);
  b = boxed_ptr<TestStruct>::fromStaticConstant(&t2);
  EXPECT_FALSE(a == b);
}

TEST(BoxedPtrTest, Mut) {
  const TestStruct t1{5};

  boxed_ptr<TestStruct> a = boxed_ptr<TestStruct>::fromStaticConstant(&t1);
  boxed_ptr<TestStruct> b = boxed_ptr<TestStruct>::fromStaticConstant(&t1);

  EXPECT_TRUE(a == b);

  (void)a.mut();

  EXPECT_FALSE(a == b);
}

TEST(BoxedPtrTest, MutFromDefaultConstructed) {
  boxed_ptr<TestStruct> a;

  EXPECT_FALSE(static_cast<bool>(a));
  if (folly::kIsDebug) {
    EXPECT_DEATH(a.mut(), "");
  }
}

TEST(BoxedPtrTest, Copy) {
  const TestStruct t1{5};

  boxed_ptr<TestStruct> a = boxed_ptr<TestStruct>::fromStaticConstant(&t1);
  // Shallow copy when 'boxed_ptr' does not own the value.
  boxed_ptr<TestStruct> b = a.copy();

  EXPECT_TRUE(a == b);

  (void)a.mut();
  // Deep copy when 'boxed_ptr' owns the value.
  boxed_ptr<TestStruct> c = a.copy();

  EXPECT_FALSE(a == c);
}

TEST(BoxedValueTest, DefaultConstructor) {
  boxed_value<TestStruct> a;
  EXPECT_FALSE(a.has_value());
}

TEST(BoxedValueTest, FromStaticConstant) {
  const TestStruct t{5};
  boxed_value<TestStruct> a = boxed_value<TestStruct>::fromStaticConstant(&t);
  EXPECT_EQ(a.value(), 5);
}

TEST(BoxedValueTest, UniquePtrConstructor) {
  boxed_value<TestStruct> a{std::make_unique<TestStruct>(5)};
  EXPECT_EQ(a.value(), 5);
}

TEST(BoxedValueTest, CopyConstructor) {
  const TestStruct t{5};
  boxed_value<TestStruct> a = boxed_value<TestStruct>::fromStaticConstant(&t);
  boxed_value<TestStruct> b{a};
  EXPECT_EQ(a.value(), 5);
  // shallow copy.
  EXPECT_EQ(b.get(), a.get());

  a.mut();
  boxed_value<TestStruct> c{a};
  // deep copy.
  EXPECT_NE(c.get(), a.get());
}

TEST(BoxedValueTest, CopyAssignment) {
  const TestStruct t{5};
  boxed_value<TestStruct> a = boxed_value<TestStruct>::fromStaticConstant(&t);
  boxed_value<TestStruct> b = a;
  EXPECT_EQ(a.value(), 5);
  // shallow copy.
  EXPECT_EQ(b.get(), a.get());

  a.mut();
  boxed_value<TestStruct> c = a;
  // deep copy.
  EXPECT_NE(c.get(), a.get());
}

TEST(BoxedValueTest, MoveConstructor) {
  const TestStruct t{5};
  boxed_value<TestStruct> a = boxed_value<TestStruct>::fromStaticConstant(&t);
  boxed_value<TestStruct> b(std::move(a));
  EXPECT_EQ(b.value(), 5);
}

TEST(BoxedValueTest, MoveAssignment) {
  const TestStruct t{5};
  boxed_value<TestStruct> a = boxed_value<TestStruct>::fromStaticConstant(&t);
  boxed_value<TestStruct> b;
  b = std::move(a);
  EXPECT_EQ(b.value(), 5);
}

TEST(BoxedValueTest, Reset) {
  const TestStruct t{5};
  boxed_value<TestStruct> a = boxed_value<TestStruct>::fromStaticConstant(&t);
  a.reset();
  EXPECT_FALSE(a.has_value());

  a.reset(std::make_unique<TestStruct>(6));
  EXPECT_TRUE(a.has_value());
  EXPECT_EQ(*a, 6);
}

TEST(BoxedValueTest, Swap) {
  const TestStruct t1{5};
  const TestStruct t2{7};

  boxed_value<TestStruct> a = boxed_value<TestStruct>::fromStaticConstant(&t1);
  boxed_value<TestStruct> b = boxed_value<TestStruct>::fromStaticConstant(&t2);

  std::swap(a, b);
  EXPECT_EQ(*a, 7);
  EXPECT_EQ(*b, 5);
}

TEST(BoxedValueTest, Mut) {
  const TestStruct t1{5};

  boxed_value<TestStruct> a = boxed_value<TestStruct>::fromStaticConstant(&t1);
  boxed_value<TestStruct> b = boxed_value<TestStruct>::fromStaticConstant(&t1);

  a.mut().set_a(1);
  std::move(b).mut().set_a(2);

  EXPECT_EQ(a.value(), 1);

  // The following statement technically accesses a moved-from object (`b`),
  // but is nevertheless valid: it effectively asserts that given that both
  // `mut()` and `set_a()` do not invalidate the state of any r-value object
  // they are called on, then the following assertion still stands.
  // Note that this does not violate the typical expectation that "moved-from
  // objects shall be placed in a valid but unspecified state": it merely goes
  // further by specifying said state.
  EXPECT_EQ(b.value(), 2);
}

TEST(BoxedValueTest, MutFromDefaultConstructed) {
  boxed_value<TestStruct> a;
  EXPECT_THROW(a.mut(), std::logic_error);
  EXPECT_THROW(std::move(a).mut(), std::logic_error);
}

TEST(BoxedValueTest, Comparison) {
  const TestStruct t1{1};
  const TestStruct t2{2};

  boxed_value<TestStruct> a = boxed_value<TestStruct>::fromStaticConstant(&t1);
  boxed_value<TestStruct> b = boxed_value<TestStruct>::fromStaticConstant(&t2);

  EXPECT_FALSE(a == b);
  EXPECT_TRUE(a != b);
  EXPECT_TRUE(a < b);
  EXPECT_TRUE(a <= b);
  EXPECT_FALSE(a > b);
  EXPECT_FALSE(a >= b);

  EXPECT_FALSE(a == t2);
  EXPECT_TRUE(a != t2);
  EXPECT_TRUE(a < t2);
  EXPECT_TRUE(a <= t2);
  EXPECT_FALSE(a > t2);
  EXPECT_FALSE(a >= t2);

  EXPECT_FALSE(t1 == b);
  EXPECT_TRUE(t1 != b);
  EXPECT_TRUE(t1 < b);
  EXPECT_TRUE(t1 <= b);
  EXPECT_FALSE(t1 > b);
  EXPECT_FALSE(t1 >= b);

  EXPECT_TRUE(t2 == b);
  EXPECT_FALSE(t2 != b);
  EXPECT_FALSE(t2 < b);
  EXPECT_TRUE(t2 <= b);
  EXPECT_FALSE(t2 > b);
  EXPECT_TRUE(t2 >= b);
}

TEST(BoxedValueTest, ComparisonWithNoValue) {
  const TestStruct t1{1};

  boxed_value<TestStruct> a = boxed_value<TestStruct>::fromStaticConstant(&t1);
  boxed_value<TestStruct> b;
  boxed_value<TestStruct> c;

  EXPECT_FALSE(a == b);
  EXPECT_TRUE(a != b);
  EXPECT_FALSE(a < b);
  EXPECT_FALSE(a <= b);
  EXPECT_TRUE(a > b);
  EXPECT_TRUE(a >= b);

  EXPECT_FALSE(b == t1);
  EXPECT_TRUE(b != t1);
  EXPECT_TRUE(b < t1);
  EXPECT_TRUE(b <= t1);
  EXPECT_FALSE(b > t1);
  EXPECT_FALSE(b >= t1);

  EXPECT_FALSE(t1 == b);
  EXPECT_TRUE(t1 != b);
  EXPECT_FALSE(t1 < b);
  EXPECT_FALSE(t1 <= b);
  EXPECT_TRUE(t1 > b);
  EXPECT_TRUE(t1 >= b);
}

TEST(BoxedValueTest, ValueComparison) {
  const TestStruct t{1};

  boxed_value<TestStruct> a = boxed_value<TestStruct>::fromStaticConstant(&t);
  boxed_value<TestStruct> b = boxed_value<TestStruct>::fromStaticConstant(&t);
  b.mut();

  // 'b' points to shared value; meanwhile, 'c' owns the same value.
  EXPECT_NE(a.get(), b.get());
  EXPECT_EQ(a, b);
}

TEST(BoxedValueTest, Ensure) {
  const TestStruct t{1};
  boxed_value<TestStruct> a = boxed_value<TestStruct>::fromStaticConstant(&t);
  boxed_value<TestStruct> b = boxed_value<TestStruct>::fromStaticConstant(&t);
  boxed_value<TestStruct> c;

  EXPECT_EQ(a.get(), b.get());
  a.ensure();
  EXPECT_EQ(a.get(), b.get());

  c.ensure();
  c.mut().set_a(1);
  EXPECT_EQ(a, c);
}

// Why alignas(2)? Without it, the (natural) alignment of LifecycleTester is 1.
// However, in order for values of LifecycleTester to be usable with boxed_value
// below, they cannot be single-byte aligned, as that would prevent
// boxed_value_ptr fom using the least-significant bits of their address to
// capture ownership information (a fundamental requirement).
class alignas(2) LifecycleTester {
 public:
  using __fbthrift_cpp2_type = LifecycleTester;

  LifecycleTester() : isACopy_(false), hasBeenCopied_(false) {}
  explicit LifecycleTester(int) : isACopy_(false), hasBeenCopied_(false) {}

  // NOLINTNEXTLINE(facebook-hte-NonConstCopyContructor)
  LifecycleTester(LifecycleTester& other)
      : isACopy_(true), hasBeenCopied_(false) {
    other.hasBeenCopied_ = true;
  }

  LifecycleTester& operator=(LifecycleTester& other) {
    LifecycleTester tmp{other};
    std::swap(isACopy_, tmp.isACopy_);
    std::swap(hasBeenCopied_, tmp.hasBeenCopied_);
    other.hasBeenCopied_ = true;
    return *this;
  }

  bool isACopy() const { return isACopy_; }

  bool hasBeenCopied() const { return hasBeenCopied_; }

 private:
  bool isACopy_;
  bool hasBeenCopied_;
};

TEST(LifecycleTest, ConstUnownedCopy) {
  const LifecycleTester lct;

  boxed_value<LifecycleTester> boxed1 =
      boxed_value<LifecycleTester>::fromStaticConstant(&lct);
  EXPECT_FALSE(boxed1->isACopy());
  EXPECT_FALSE(boxed1->hasBeenCopied());

  boxed_value<LifecycleTester> boxed2 = boxed1;
  EXPECT_FALSE(boxed1->hasBeenCopied());
  EXPECT_FALSE(boxed2->isACopy());
}

TEST(LifecycleTest, MutOwnedRefCopy) {
  const LifecycleTester lct;

  boxed_value<LifecycleTester> boxed =
      boxed_value<LifecycleTester>::fromStaticConstant(&lct);
  EXPECT_FALSE(boxed->isACopy());
  EXPECT_TRUE(boxed.mut().isACopy()); // This mutable ref creates a copy.
}

TEST(LifecycleTest, copyOfMutableRefCreatesCopy) {
  // Creating the object this way starts with a MutOwned state.
  boxed_value<LifecycleTester> boxed1;
  boxed1.ensure();
  EXPECT_FALSE(boxed1->hasBeenCopied());
  auto boxed2 = boxed1;
  EXPECT_TRUE(boxed1->hasBeenCopied());
}
