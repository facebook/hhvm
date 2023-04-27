/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/WeakRefCountedPtr.h>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

using namespace testing;

namespace proxygen { namespace test {

class TestClass : public EnableWeakRefCountedPtr<TestClass> {
 public:
  MOCK_METHOD(void, onWeakRefCountedPtrCreate, ());
  MOCK_METHOD(void, onWeakRefCountedPtrDestroy, ());
};

class TestDerivedClass : public TestClass {
 public:
  using TestClass::TestClass;
  void onlyDerivedFunc() const {
  }
};

TEST(WeakRefCountedPtrTest, JustTarget) {
  auto target = std::make_unique<StrictMock<TestClass>>();
}

TEST(WeakRefCountedPtrTest, DestroyTarget) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
  auto kaPtr = target->getWeakRefCountedPtr();
  EXPECT_TRUE(kaPtr);
  EXPECT_EQ(target.get(), kaPtr.get());
  target = nullptr;
  EXPECT_FALSE(kaPtr);
  EXPECT_EQ(nullptr, kaPtr.get());
}

TEST(WeakRefCountedPtrTest, DestroyPtr) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  {
    EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
    auto kaPtr = target->getWeakRefCountedPtr();
    EXPECT_TRUE(kaPtr);
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    EXPECT_EQ(target.get(), kaPtr.get());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy());
  }
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
  auto kaPtr = target->getWeakRefCountedPtr();
  EXPECT_TRUE(kaPtr);
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_EQ(target.get(), kaPtr.get());
  EXPECT_CALL(*target, onWeakRefCountedPtrDestroy());
  kaPtr.reset();
  EXPECT_FALSE(kaPtr);
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
  Mock::VerifyAndClearExpectations(target.get());
}

TEST(WeakRefCountedPtrTest, JustTargetDerived) {
  auto target = std::make_unique<StrictMock<TestDerivedClass>>();
  target->onlyDerivedFunc();
}

TEST(WeakRefCountedPtrTest, DestroyTargetDerived) {
  auto target = std::make_unique<StrictMock<TestDerivedClass>>();
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
  auto kaPtr = target->getWeakRefCountedPtr<TestDerivedClass>();
  EXPECT_TRUE(kaPtr);
  CHECK_NOTNULL(kaPtr.get())->onlyDerivedFunc();
  EXPECT_EQ(target.get(), kaPtr.get());
  target = nullptr;
  EXPECT_FALSE(kaPtr);
  EXPECT_EQ(nullptr, kaPtr.get());
}

TEST(WeakRefCountedPtrTest, DestroyPtrDerived) {
  auto target = std::make_unique<StrictMock<TestDerivedClass>>();
  {
    EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
    auto kaPtr = target->getWeakRefCountedPtr<TestDerivedClass>();
    EXPECT_TRUE(kaPtr);
    CHECK_NOTNULL(kaPtr.get())->onlyDerivedFunc();
    kaPtr->onlyDerivedFunc();
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    EXPECT_EQ(target.get(), kaPtr.get());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy());
  }
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
  {
    EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
    auto kaPtr = target->getWeakRefCountedPtr<TestDerivedClass>();
    EXPECT_TRUE(kaPtr);
    CHECK_NOTNULL(kaPtr.get())->onlyDerivedFunc();
    kaPtr->onlyDerivedFunc();
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    EXPECT_EQ(target.get(), kaPtr.get());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy());
  }
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, DestroyPtrMulti) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate()).Times(2);
  {
    auto kaPtr1 = target->getWeakRefCountedPtr();
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    EXPECT_EQ(target.get(), kaPtr1.get());
    auto kaPtr2 = target->getWeakRefCountedPtr();
    EXPECT_EQ(2, target->numWeakRefCountedPtrs());
    EXPECT_EQ(target.get(), kaPtr2.get());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy()).Times(2);
  }
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate()).Times(2);
  {
    auto kaPtr1 = target->getWeakRefCountedPtr();
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    EXPECT_EQ(target.get(), kaPtr1.get());
    auto kaPtr2 = target->getWeakRefCountedPtr();
    EXPECT_EQ(2, target->numWeakRefCountedPtrs());
    EXPECT_EQ(target.get(), kaPtr2.get());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy()).Times(2);
  }
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

// Move construct and assign tests

TEST(WeakRefCountedPtrTest, MoveConstruct) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  {
    EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
    auto kaPtr1 = target->getWeakRefCountedPtr();
    EXPECT_TRUE(kaPtr1);
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    WeakRefCountedPtr<TestClass> kaPtr2(std::move(kaPtr1));
    EXPECT_TRUE(kaPtr2);
    EXPECT_FALSE(kaPtr1);
    EXPECT_EQ(nullptr, kaPtr1.get());
    EXPECT_EQ(target.get(), kaPtr2.get());
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy());
  }
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, MoveConstructEmpty) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  {
    WeakRefCountedPtr<TestClass> kaPtr1;
    EXPECT_FALSE(kaPtr1);
    EXPECT_EQ(nullptr, kaPtr1.get());
    EXPECT_EQ(0, target->numWeakRefCountedPtrs());

    WeakRefCountedPtr<TestClass> kaPtr2(std::move(kaPtr1));
    EXPECT_FALSE(kaPtr1);
    EXPECT_FALSE(kaPtr2);
    EXPECT_EQ(nullptr, kaPtr1.get());
    EXPECT_EQ(nullptr, kaPtr2.get());
    EXPECT_EQ(0, target->numWeakRefCountedPtrs());
  }
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, MoveAssign) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  {
    EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
    auto kaPtr1 = target->getWeakRefCountedPtr();
    EXPECT_TRUE(kaPtr1);
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    auto kaPtr2 = std::move(kaPtr1);
    EXPECT_FALSE(kaPtr1);
    EXPECT_TRUE(kaPtr2);
    EXPECT_EQ(nullptr, kaPtr1.get());
    EXPECT_EQ(target.get(), kaPtr2.get());
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy());
  }
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, MoveAssignToExistingActivePtr) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate()).Times(2);
  {
    auto kaPtr1 = target->getWeakRefCountedPtr();
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    auto kaPtr2 = target->getWeakRefCountedPtr();
    EXPECT_TRUE(kaPtr1);
    EXPECT_TRUE(kaPtr2);
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(target.get(), kaPtr2.get());
    EXPECT_EQ(2, target->numWeakRefCountedPtrs());

    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy());
    kaPtr2 = std::move(kaPtr1);
    Mock::VerifyAndClearExpectations(target.get());
    EXPECT_FALSE(kaPtr1);
    EXPECT_TRUE(kaPtr2);
    EXPECT_EQ(nullptr, kaPtr1.get());
    EXPECT_EQ(target.get(), kaPtr2.get());
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy());
  }
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, MoveAssignToExistingInactivePtr) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  {
    EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
    auto kaPtr1 = target->getWeakRefCountedPtr();
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());

    WeakRefCountedPtr<TestClass> kaPtr2;
    kaPtr2 = std::move(kaPtr1);
    EXPECT_FALSE(kaPtr1);
    EXPECT_TRUE(kaPtr2);
    EXPECT_EQ(nullptr, kaPtr1.get());
    EXPECT_EQ(target.get(), kaPtr2.get());
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy());
  }
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, MoveAssignEmptyToExistingActivePtr) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  {
    EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
    auto kaPtr1 = target->getWeakRefCountedPtr();
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());

    WeakRefCountedPtr<TestClass> kaPtr2;
    EXPECT_TRUE(kaPtr1);
    EXPECT_FALSE(kaPtr2);
    EXPECT_EQ(nullptr, kaPtr2.get());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy());
    kaPtr1 = std::move(kaPtr2);
    Mock::VerifyAndClearExpectations(target.get());
    EXPECT_FALSE(kaPtr1);
    EXPECT_FALSE(kaPtr2);
    EXPECT_EQ(nullptr, kaPtr1.get());
    EXPECT_EQ(nullptr, kaPtr2.get());
    EXPECT_EQ(0, target->numWeakRefCountedPtrs());
  }
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, MoveAssignEmptyToExistingInactivePtr) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  {
    WeakRefCountedPtr<TestClass> kaPtr1;
    EXPECT_EQ(nullptr, kaPtr1.get());
    EXPECT_EQ(0, target->numWeakRefCountedPtrs());

    WeakRefCountedPtr<TestClass> kaPtr2;
    EXPECT_EQ(nullptr, kaPtr2.get());
    kaPtr1 = std::move(kaPtr2);
    EXPECT_FALSE(kaPtr1);
    EXPECT_FALSE(kaPtr2);
    EXPECT_EQ(nullptr, kaPtr1.get());
    EXPECT_EQ(nullptr, kaPtr2.get());
    EXPECT_EQ(0, target->numWeakRefCountedPtrs());
  }
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, MoveAssignDestroyTarget) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
  auto kaPtr1 = target->getWeakRefCountedPtr();
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_EQ(target.get(), kaPtr1.get());
  auto kaPtr2 = std::move(kaPtr1);
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_FALSE(kaPtr1);
  EXPECT_TRUE(kaPtr2);
  EXPECT_EQ(nullptr, kaPtr1.get());
  EXPECT_EQ(target.get(), kaPtr2.get());
  target = nullptr;
  EXPECT_FALSE(kaPtr1);
  EXPECT_FALSE(kaPtr2);
  EXPECT_EQ(nullptr, kaPtr1.get());
  EXPECT_EQ(nullptr, kaPtr2.get());
}

TEST(WeakRefCountedPtrTest, MoveConstructDestroyTarget) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
  auto kaPtr1 = target->getWeakRefCountedPtr();
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_EQ(target.get(), kaPtr1.get());
  auto kaPtr2(std::move(kaPtr1));
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_FALSE(kaPtr1);
  EXPECT_TRUE(kaPtr2);
  EXPECT_EQ(nullptr, kaPtr1.get());
  EXPECT_EQ(target.get(), kaPtr2.get());
  target = nullptr;
  EXPECT_EQ(nullptr, kaPtr1.get());
  EXPECT_EQ(nullptr, kaPtr2.get());
}

TEST(WeakRefCountedPtrTest, MoveAssignDestroySrcDestroyTarget) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
  auto kaPtr1 = std::make_unique<WeakRefCountedPtr<TestClass>>(
      target->getWeakRefCountedPtr());
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_EQ(target.get(), kaPtr1->get());
  WeakRefCountedPtr<TestClass> kaPtr2(std::move(*kaPtr1));
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_FALSE(*kaPtr1);
  EXPECT_TRUE(kaPtr2);
  EXPECT_EQ(nullptr, kaPtr1->get());
  EXPECT_EQ(target.get(), kaPtr2.get());
  kaPtr1 = nullptr;
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_TRUE(kaPtr2);
  EXPECT_EQ(target.get(), kaPtr2.get());
  target = nullptr;
  EXPECT_EQ(nullptr, kaPtr2.get());
}

TEST(WeakRefCountedPtrTest, MoveConstructDestroySrcDestroyTarget) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
  auto kaPtr1 = std::make_unique<WeakRefCountedPtr<TestClass>>(
      target->getWeakRefCountedPtr());
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_EQ(target.get(), kaPtr1->get());
  auto kaPtr2 = std::move(*kaPtr1);
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_FALSE(*kaPtr1);
  EXPECT_TRUE(kaPtr2);
  EXPECT_EQ(nullptr, kaPtr1->get());
  EXPECT_EQ(target.get(), kaPtr2.get());
  kaPtr1 = nullptr;
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_TRUE(kaPtr2);
  EXPECT_EQ(target.get(), kaPtr2.get());
  target = nullptr;
  EXPECT_EQ(nullptr, kaPtr2.get());
}

// Copy construct and assign tests

TEST(WeakRefCountedPtrTest, CopyConstruct) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate()).Times(2);
  {
    auto kaPtr1 = target->getWeakRefCountedPtr();
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    WeakRefCountedPtr<TestClass> kaPtr2(kaPtr1);
    EXPECT_TRUE(kaPtr1);
    EXPECT_TRUE(kaPtr2);
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(target.get(), kaPtr2.get());
    EXPECT_EQ(2, target->numWeakRefCountedPtrs());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy()).Times(2);
  }
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, CopyConstructEmpty) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  {
    WeakRefCountedPtr<TestClass> kaPtr1;
    EXPECT_EQ(nullptr, kaPtr1.get());
    EXPECT_EQ(0, target->numWeakRefCountedPtrs());

    WeakRefCountedPtr<TestClass> kaPtr2(kaPtr1);
    EXPECT_FALSE(kaPtr1);
    EXPECT_FALSE(kaPtr2);
    EXPECT_EQ(nullptr, kaPtr1.get());
    EXPECT_EQ(nullptr, kaPtr2.get());
    EXPECT_EQ(0, target->numWeakRefCountedPtrs());
  }
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, CopyAssign) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  {
    EXPECT_CALL(*target, onWeakRefCountedPtrCreate()).Times(2);
    auto kaPtr1 = target->getWeakRefCountedPtr();
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    auto kaPtr2 = kaPtr1;
    EXPECT_TRUE(kaPtr1);
    EXPECT_TRUE(kaPtr2);
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(target.get(), kaPtr2.get());
    EXPECT_EQ(2, target->numWeakRefCountedPtrs());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy()).Times(2);
  }
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, CopyAssignToExistingActivePtr) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  {
    EXPECT_CALL(*target, onWeakRefCountedPtrCreate()).Times(3);
    auto kaPtr1 = target->getWeakRefCountedPtr();
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());
    auto kaPtr2 = target->getWeakRefCountedPtr();
    EXPECT_TRUE(kaPtr1);
    EXPECT_TRUE(kaPtr2);
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(target.get(), kaPtr2.get());
    EXPECT_EQ(2, target->numWeakRefCountedPtrs());

    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy());
    kaPtr2 = kaPtr1;
    Mock::VerifyAndClearExpectations(target.get());
    EXPECT_TRUE(kaPtr1);
    EXPECT_TRUE(kaPtr2);
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(target.get(), kaPtr2.get());
    EXPECT_EQ(2, target->numWeakRefCountedPtrs());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy()).Times(2);
  }
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, CopyAssignToExistingInactivePtr) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate()).Times(2);
  {
    auto kaPtr1 = target->getWeakRefCountedPtr();
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());

    WeakRefCountedPtr<TestClass> kaPtr2;
    kaPtr2 = kaPtr1;
    EXPECT_TRUE(kaPtr1);
    EXPECT_TRUE(kaPtr2);
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(target.get(), kaPtr2.get());
    EXPECT_EQ(2, target->numWeakRefCountedPtrs());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy()).Times(2);
  }
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, CopyAssignEmptyToExistingActivePtr) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  {
    EXPECT_CALL(*target, onWeakRefCountedPtrCreate());
    auto kaPtr1 = target->getWeakRefCountedPtr();
    EXPECT_EQ(target.get(), kaPtr1.get());
    EXPECT_EQ(1, target->numWeakRefCountedPtrs());

    WeakRefCountedPtr<TestClass> kaPtr2;
    EXPECT_EQ(nullptr, kaPtr2.get());
    EXPECT_CALL(*target, onWeakRefCountedPtrDestroy());
    kaPtr1 = kaPtr2;
    Mock::VerifyAndClearExpectations(target.get());
    EXPECT_FALSE(kaPtr1);
    EXPECT_FALSE(kaPtr2);
    EXPECT_EQ(nullptr, kaPtr1.get());
    EXPECT_EQ(nullptr, kaPtr2.get());
    EXPECT_EQ(0, target->numWeakRefCountedPtrs());
  }
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, CopyAssignEmptyToExistingInactivePtr) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  {
    WeakRefCountedPtr<TestClass> kaPtr1;
    EXPECT_EQ(nullptr, kaPtr1.get());
    EXPECT_EQ(0, target->numWeakRefCountedPtrs());

    WeakRefCountedPtr<TestClass> kaPtr2;
    kaPtr2 = kaPtr1;
    EXPECT_FALSE(kaPtr1);
    EXPECT_FALSE(kaPtr2);
    EXPECT_EQ(nullptr, kaPtr1.get());
    EXPECT_EQ(nullptr, kaPtr2.get());
    EXPECT_EQ(0, target->numWeakRefCountedPtrs());
  }
  EXPECT_EQ(0, target->numWeakRefCountedPtrs());
}

TEST(WeakRefCountedPtrTest, CopyAssignDestroyTarget) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate()).Times(2);
  auto kaPtr1 = target->getWeakRefCountedPtr();
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_EQ(target.get(), kaPtr1.get());
  auto kaPtr2 = kaPtr1;
  EXPECT_EQ(2, target->numWeakRefCountedPtrs());
  EXPECT_TRUE(kaPtr1);
  EXPECT_TRUE(kaPtr2);
  EXPECT_EQ(target.get(), kaPtr1.get());
  EXPECT_EQ(target.get(), kaPtr2.get());
  target = nullptr;
  EXPECT_FALSE(kaPtr1);
  EXPECT_FALSE(kaPtr2);
  EXPECT_EQ(nullptr, kaPtr1.get());
  EXPECT_EQ(nullptr, kaPtr2.get());
}

TEST(WeakRefCountedPtrTest, CopyConstructDestroyTarget) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate()).Times(2);
  auto kaPtr1 = target->getWeakRefCountedPtr();
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_EQ(target.get(), kaPtr1.get());
  auto kaPtr2(kaPtr1);
  EXPECT_EQ(2, target->numWeakRefCountedPtrs());
  EXPECT_TRUE(kaPtr1);
  EXPECT_TRUE(kaPtr2);
  EXPECT_EQ(target.get(), kaPtr1.get());
  EXPECT_EQ(target.get(), kaPtr2.get());
  target = nullptr;
  EXPECT_FALSE(kaPtr1);
  EXPECT_FALSE(kaPtr2);
  EXPECT_EQ(nullptr, kaPtr1.get());
  EXPECT_EQ(nullptr, kaPtr2.get());
}

TEST(WeakRefCountedPtrTest, CopyAssignDestroySrcDestroyTarget) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate()).Times(2);
  auto kaPtr1 = std::make_unique<WeakRefCountedPtr<TestClass>>(
      target->getWeakRefCountedPtr());
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_EQ(target.get(), kaPtr1->get());
  WeakRefCountedPtr<TestClass> kaPtr2(*kaPtr1);
  EXPECT_EQ(2, target->numWeakRefCountedPtrs());
  EXPECT_TRUE(*kaPtr1);
  EXPECT_TRUE(kaPtr2);
  EXPECT_EQ(target.get(), kaPtr1->get());
  EXPECT_EQ(target.get(), kaPtr2.get());
  EXPECT_CALL(*target, onWeakRefCountedPtrDestroy());
  kaPtr1 = nullptr;
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_TRUE(kaPtr2);
  EXPECT_EQ(target.get(), kaPtr2.get());
  target = nullptr;
  EXPECT_FALSE(kaPtr2);
  EXPECT_EQ(nullptr, kaPtr2.get());
}

TEST(WeakRefCountedPtrTest, CopyConstructDestroySrcDestroyTarget) {
  auto target = std::make_unique<StrictMock<TestClass>>();
  EXPECT_CALL(*target, onWeakRefCountedPtrCreate()).Times(2);
  auto kaPtr1 = std::make_unique<WeakRefCountedPtr<TestClass>>(
      target->getWeakRefCountedPtr());
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_EQ(target.get(), kaPtr1->get());
  auto kaPtr2 = *kaPtr1;
  EXPECT_EQ(2, target->numWeakRefCountedPtrs());
  EXPECT_TRUE(*kaPtr1);
  EXPECT_TRUE(kaPtr2);
  EXPECT_EQ(target.get(), kaPtr1->get());
  EXPECT_EQ(target.get(), kaPtr2.get());
  EXPECT_CALL(*target, onWeakRefCountedPtrDestroy());
  kaPtr1 = nullptr;
  Mock::VerifyAndClearExpectations(target.get());
  EXPECT_EQ(1, target->numWeakRefCountedPtrs());
  EXPECT_TRUE(kaPtr2);
  EXPECT_EQ(target.get(), kaPtr2.get());
  target = nullptr;
  EXPECT_FALSE(kaPtr2);
  EXPECT_EQ(nullptr, kaPtr2.get());
}

}} // namespace proxygen::test
