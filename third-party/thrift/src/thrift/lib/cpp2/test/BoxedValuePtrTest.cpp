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

#include <type_traits>

#include <folly/Portability.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/BoxedValuePtr.h>

using namespace apache::thrift::detail;

struct TestStruct {
  TestStruct() : a(0) {}
  explicit TestStruct(int val) : a(val) {}
  int a;
};

constexpr bool operator==(const TestStruct& lhs, int rhs) {
  return lhs.a == rhs;
}

constexpr bool operator==(int lhs, const TestStruct& rhs) {
  return lhs == rhs.a;
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
  static_assert(!std::is_copy_constructible<boxed_ptr<TestStruct>>::value);
  static_assert(!std::is_copy_assignable<boxed_ptr<TestStruct>>::value);
}

TEST(BoxedPtrTest, MoveConstructor) {
  const TestStruct t{5};
  boxed_ptr<TestStruct> a = boxed_ptr<TestStruct>::fromStaticConstant(&t);
  boxed_ptr<TestStruct> b(std::move(a));
  EXPECT_EQ(*b, 5);
}

TEST(BoxedPtrTest, MoveAssignment) {
  const TestStruct t{5};
  boxed_ptr<TestStruct> a = boxed_ptr<TestStruct>::fromStaticConstant(&t);
  boxed_ptr<TestStruct> b;
  b = std::move(a);
  EXPECT_EQ(*b, 5);
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

// TODO(dokwon): Enable this test for boxed_value.
// class LifecycleTester {
//  public:
//   using __fbthrift_cpp2_type = std::true_type;

//   LifecycleTester() : isACopy_(false), hasBeenCopied_(false) {}
//   explicit LifecycleTester(int) : isACopy_(false), hasBeenCopied_(false) {}

//   // NOLINTNEXTLINE(facebook-hte-NonConstCopyContructor)
//   LifecycleTester(LifecycleTester& other)
//       : isACopy_(true), hasBeenCopied_(false) {
//     other.hasBeenCopied_ = true;
//   }

//   LifecycleTester& operator=(LifecycleTester& other) {
//     LifecycleTester tmp{other};
//     std::swap(isACopy_, tmp.isACopy_);
//     std::swap(hasBeenCopied_, tmp.hasBeenCopied_);
//     other.hasBeenCopied_ = true;
//     return *this;
//   }

//   bool isACopy() const { return isACopy_; }

//   bool hasBeenCopied() const { return hasBeenCopied_; }

//  private:
//   bool isACopy_;
//   bool hasBeenCopied_;
// };

// TEST(LifecycleTest, ConstUnownedCopy) {
//   LifecycleTester lct;

//   boxed_ptr<LifecycleTester> boxed1 = &lct;
//   EXPECT_FALSE(boxed1->isACopy());
//   EXPECT_FALSE(boxed1->hasBeenCopied());

//   boxed_ptr<LifecycleTester> boxed2 = boxed1;
//   EXPECT_FALSE(boxed1->hasBeenCopied());
//   EXPECT_FALSE(boxed2->isACopy());

//   EXPECT_FALSE(lct.hasBeenCopied());
//   EXPECT_EQ(boxed1, boxed2);
// }

// TEST(LifecycleTest, MutOwnedRefCopy) {
//   LifecycleTester lct;

//   boxed_ptr<LifecycleTester> boxed{&lct};
//   EXPECT_FALSE(boxed->isACopy());
//   EXPECT_TRUE(boxed.mut()->isACopy()); // This mutable ref creates a copy.
//   EXPECT_TRUE(lct.hasBeenCopied());
// }

// TEST(LifecycleTest, copyOfMutableRefCreatesCopy) {
//   // Creating the object this way starts with a MutOwned state.
//   boxed_ptr<LifecycleTester> boxed1{0};
//   EXPECT_FALSE(boxed1->hasBeenCopied());
//   auto boxed2 = boxed1;
//   EXPECT_TRUE(boxed1->hasBeenCopied());
// }
