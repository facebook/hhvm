// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "hphp/util/ptr-impl.h"

#include <gtest/gtest.h>

namespace HPHP {

#ifdef NDEBUG
#define ASSERT_DEBUG_DEATH(statement, regex) \
  GTEST_EXECUTE_STATEMENT_(statement, regex)
#else
#define ASSERT_DEBUG_DEATH(statement, regex) \
  ASSERT_DEATH(statement, regex)
#endif

static uintptr_t makePtr(int shift) {
  return uintptr_t(1) << shift;
}

template <typename T>
using SmallPtr = ptrimpl::PtrImpl<T, ptrimpl::Normal, ptrimpl::UInt32>;

template <typename T>
using AtomicSmallPtr = ptrimpl::PtrImpl<T, ptrimpl::Normal, ptrimpl::UInt32>;

template <typename T>
using PackedPtr = ptrimpl::PtrImpl<T, ptrimpl::Normal, ptrimpl::UInt32Packed>;

template <typename T>
using AtomicPackedPtr = ptrimpl::PtrImpl<T, ptrimpl::Normal, ptrimpl::UInt32Packed>;

template <typename T>
using FullPtr = ptrimpl::PtrImpl<T, ptrimpl::Normal, ptrimpl::UInt64>;

template <typename T>
using AtomicFullPtr = ptrimpl::PtrImpl<T, ptrimpl::Normal, ptrimpl::UInt64>;

class PackedPtrGoodTest : public testing::TestWithParam<uintptr_t> {};

TEST_P(PackedPtrGoodTest, AcceptablePtr) {
  uintptr_t input = GetParam();

  int* a = reinterpret_cast<int*>(input);
  PackedPtr<int> p(a);
  EXPECT_EQ(a, p.get());
}

INSTANTIATE_TEST_SUITE_P(PtrTestSuite, PackedPtrGoodTest, testing::Values(makePtr(3), makePtr(4), makePtr(31), makePtr(32), makePtr(33), makePtr(34)));

class AtomicPackedPtrGoodTest : public testing::TestWithParam<uintptr_t> {};

TEST_P(AtomicPackedPtrGoodTest, AcceptablePtr) {
  uintptr_t input = GetParam();

  int* a = reinterpret_cast<int*>(input);
  AtomicPackedPtr<int> p(a);
  EXPECT_EQ(a, p.get());
}

INSTANTIATE_TEST_SUITE_P(PtrTestSuite, AtomicPackedPtrGoodTest, testing::Values(makePtr(3), makePtr(4), makePtr(31), makePtr(32), makePtr(33), makePtr(34)));

class FullPtrGoodTest : public testing::TestWithParam<uintptr_t> {};

TEST_P(FullPtrGoodTest, AcceptablePtr) {
  uintptr_t input = GetParam();

  int* a = reinterpret_cast<int*>(input);
  FullPtr<int> p(a);
  EXPECT_EQ(a, p.get());
}

INSTANTIATE_TEST_SUITE_P(PtrTestSuite, FullPtrGoodTest, testing::Values(makePtr(0), makePtr(1), makePtr(2), makePtr(3), makePtr(4), makePtr(31), makePtr(32), makePtr(33), makePtr(34), makePtr(35), makePtr(36), makePtr(63)));

class AtomicFullPtrGoodTest : public testing::TestWithParam<uintptr_t> {};

TEST_P(AtomicFullPtrGoodTest, AcceptablePtr) {
  uintptr_t input = GetParam();

  int* a = reinterpret_cast<int*>(input);
  AtomicFullPtr<int> p(a);
  EXPECT_EQ(a, p.get());
}

INSTANTIATE_TEST_SUITE_P(PtrTestSuite, AtomicFullPtrGoodTest, testing::Values(makePtr(0), makePtr(1), makePtr(2), makePtr(3), makePtr(4), makePtr(31), makePtr(32), makePtr(33), makePtr(34), makePtr(35), makePtr(36), makePtr(63)));

class SmallPtrGoodTest : public testing::TestWithParam<uintptr_t> {};

TEST_P(SmallPtrGoodTest, AcceptablePtr) {
  uintptr_t input = GetParam();

  int* a = reinterpret_cast<int*>(input);
  SmallPtr<int> p(a);
  EXPECT_EQ(a, p.get());
}

INSTANTIATE_TEST_SUITE_P(PtrTestSuite, SmallPtrGoodTest, testing::Values(makePtr(0), makePtr(1), makePtr(2), makePtr(3), makePtr(4), makePtr(31)));

class AtomicSmallPtrGoodTest : public testing::TestWithParam<uintptr_t> {};

TEST_P(AtomicSmallPtrGoodTest, AcceptablePtr) {
  uintptr_t input = GetParam();

  int* a = reinterpret_cast<int*>(input);
  AtomicSmallPtr<int> p(a);
  EXPECT_EQ(a, p.get());
}

INSTANTIATE_TEST_SUITE_P(PtrTestSuite, AtomicSmallPtrGoodTest, testing::Values(makePtr(0), makePtr(1), makePtr(2), makePtr(3), makePtr(4), makePtr(31)));

class PackedPtrBadTest : public testing::TestWithParam<uintptr_t> {};

TEST_P(PackedPtrBadTest, NotAcceptablePtr) {
  uintptr_t input = GetParam();

  int* a = reinterpret_cast<int*>(input);
  ASSERT_DEBUG_DEATH(
    { PackedPtr<int> p(a); },
    ".*validatePtr.*"
  );
}

INSTANTIATE_TEST_SUITE_P(PtrTestSuite, PackedPtrBadTest, testing::Values(makePtr(0), makePtr(1), makePtr(2), makePtr(35), makePtr(36), makePtr(63)));

class AtomicPackedPtrBadTest : public testing::TestWithParam<uintptr_t> {};

TEST_P(AtomicPackedPtrBadTest, NotAcceptablePtr) {
  uintptr_t input = GetParam();

  int* a = reinterpret_cast<int*>(input);
  ASSERT_DEBUG_DEATH(
    { AtomicPackedPtr<int> p(a); },
    ".*validatePtr.*"
  );
}

INSTANTIATE_TEST_SUITE_P(PtrTestSuite, AtomicPackedPtrBadTest, testing::Values(makePtr(0), makePtr(1), makePtr(2), makePtr(35), makePtr(36), makePtr(63)));

class SmallPtrBadTest : public testing::TestWithParam<uintptr_t> {};

TEST_P(SmallPtrBadTest, NotAcceptablePtr) {
  uintptr_t input = GetParam();

  int* a = reinterpret_cast<int*>(input);
  ASSERT_DEBUG_DEATH(
    { SmallPtr<int> p(a); },
    ".*validatePtr.*"
  );
}

INSTANTIATE_TEST_SUITE_P(PtrTestSuite, SmallPtrBadTest, testing::Values(makePtr(32), makePtr(33), makePtr(62), makePtr(63)));

class AtomicSmallPtrBadTest : public testing::TestWithParam<uintptr_t> {};

TEST_P(AtomicSmallPtrBadTest, NotAcceptablePtr) {
  uintptr_t input = GetParam();

  int* a = reinterpret_cast<int*>(input);
  ASSERT_DEBUG_DEATH(
    { AtomicSmallPtr<int> p(a); },
    ".*validatePtr.*"
  );
}

INSTANTIATE_TEST_SUITE_P(PtrTestSuite, AtomicSmallPtrBadTest, testing::Values(makePtr(32), makePtr(33), makePtr(62), makePtr(63)));

}
