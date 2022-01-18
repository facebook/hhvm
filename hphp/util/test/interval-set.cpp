/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/util/interval-set.h"

#include "hphp/util/assertions.h"
#include "hphp/util/compilation-flags.h"

#include <gtest/gtest.h>
#include <initializer_list>

namespace HPHP {

namespace {

using vtype = IntervalSet::value_type;
using PairInterval = std::pair<vtype, vtype>;

UNUSED constexpr vtype NEG_INF = IntervalSet::NEG_INF;
UNUSED constexpr vtype POS_INF = IntervalSet::POS_INF;

IntervalSet build(std::initializer_list<PairInterval> init) {
  auto ret = IntervalSet::None();
  for (auto const& i : init) {
    ret |= std::apply(IntervalSet::Inclusive, i);
  }
  return ret;
}

}  // namespace anonymous

TEST(IntervalSet, ComplexBinary) {
  auto const i1 = build({
    {0, 0}, {2, 5}, {7, 9}, {11, POS_INF},
  });
  auto const i2 = build({
    {1, 1}, {3, 3}, {5, 7}, {9, 9},
  });
  EXPECT_EQ((i1 | i2).toString(), "{[0, 10), [11, +INF)}");
  EXPECT_EQ((i1 & i2).toString(), "{[3, 4), [5, 6), [7, 8), [9, 10)}");
}

TEST(IntervalSet, ObservedFailures) {
  auto const i1 = build({{-128, -1}});
  auto const i2 = build({{NEG_INF, -33}});
  auto const i3 = build({{-32, POS_INF}});
  EXPECT_EQ((~i2).toString(), "{[-32, +INF)}");
  EXPECT_EQ((i1 & i3).toString(), "{[-32, 0)}");
  EXPECT_EQ((i1 - i2).toString(), "{[-32, 0)}");
  EXPECT_EQ((i2 - i1).toString(), "{(-INF, -128)}");
}

TEST(IntervalSet, Constructors) {
  EXPECT_EQ(IntervalSet::None().toString(), "{}");
  EXPECT_EQ(IntervalSet::All().toString(), "{(-INF, +INF)}");
  EXPECT_EQ(IntervalSet::Point(17).toString(), "{[17, 18)}");
  EXPECT_EQ(IntervalSet::Range(42, 53).toString(), "{[42, 53)}");
  EXPECT_EQ(IntervalSet::Below(42).toString(), "{(-INF, 42)}");
  EXPECT_EQ(IntervalSet::Above(42).toString(), "{[42, +INF)}");
  EXPECT_EQ(IntervalSet::Inclusive(42, 53).toString(), "{[42, 54)}");
}

TEST(IntervalSet, InclusiveRelations) {
  auto const i1 = IntervalSet::Inclusive(4, 6);
  auto const i2 = IntervalSet::Inclusive(4, 6);
  auto const i3 = IntervalSet::Inclusive(5, 8);
  auto const i4 = IntervalSet::Inclusive(8, 10);
  auto const i5 = IntervalSet::Inclusive(12, POS_INF);
  EXPECT_TRUE(i1 == i1);
  EXPECT_TRUE(i1 == i2);
  EXPECT_TRUE(i1 != i3);
  EXPECT_TRUE(i1 != i4);
  EXPECT_TRUE(i1 != i5);

  EXPECT_FALSE(i1 != i1);
  EXPECT_FALSE(i1 != i2);
  EXPECT_FALSE(i1 == i3);
  EXPECT_FALSE(i1 == i4);
  EXPECT_FALSE(i1 == i5);
}

TEST(IntervalSet, PointRelations) {
  auto const i1 = IntervalSet::Point(4);
  auto const i2 = IntervalSet::Point(4);
  auto const i3 = IntervalSet::Point(5);
  auto const i4 = IntervalSet::Point(POS_INF);
  EXPECT_TRUE(i1 == i1);
  EXPECT_TRUE(i1 == i2);
  EXPECT_TRUE(i1 != i3);
  EXPECT_TRUE(i1 != i4);
  EXPECT_FALSE(i1 != i1);
  EXPECT_FALSE(i1 != i2);
  EXPECT_FALSE(i1 == i3);
  EXPECT_FALSE(i1 == i4);

  EXPECT_TRUE((i1 & i2) == i1);
  EXPECT_TRUE((i1 & i3) == IntervalSet::None());
  EXPECT_TRUE((i1 & i4) == IntervalSet::None());

  EXPECT_TRUE((i1 | i2) == i1);
  EXPECT_TRUE((i1 | i3) == build({{4, 5}}));
  EXPECT_TRUE((i1 | i4) == build({{4, 4}, {POS_INF, POS_INF}}));
}

} // namespace HPHP
