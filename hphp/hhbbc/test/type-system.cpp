/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/hhbbc/type-system.h"

#include <gtest/gtest.h>
#include <boost/range/join.hpp>

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {

auto primitives = {
  TUninit, TInitNull, TFalse, TTrue, TInt, TDbl, TSStr,
  TCStr, TSArr, TCArr, TObj, TRes, TCls, TRef
};
auto unions = {
  TOptTrue, TOptFalse, TOptBool, TOptInt, TOptDbl, TOptSStr,
  TOptStr, TOptSArr, TOptArr, TOptObj, TOptRes, TInitCell,
  TCell, TInitGen, TGen, TNull, TBool, TStr, TArr,
  TInitUnc, TUnc, TTop
};
auto all = boost::join(primitives, unions);

}

//////////////////////////////////////////////////////////////////////

TEST(Type, Top) {
  // Everything is a subtype of Top, couldBe Top, and the union of Top
  // with anything is Top.
  for (auto& t : all) {
    EXPECT_TRUE(t.subtypeOf(TTop));
    EXPECT_TRUE(t.couldBe(TTop));
    EXPECT_TRUE(union_of(t, TTop) == TTop);
    EXPECT_TRUE(union_of(TTop, t) == TTop);
  }
}

TEST(Type, Bottom) {
  // Bottom is a subtype of everything, nothing couldBe Bottom, and
  // the union_of anything with Bottom is itself.
  for (auto& t : all) {
    EXPECT_TRUE(TBottom.subtypeOf(t));
    EXPECT_TRUE(!TBottom.couldBe(t));
    EXPECT_TRUE(union_of(t, TBottom) == t);
    EXPECT_TRUE(union_of(TBottom, t) == t);
  }
}

TEST(Type, Prims) {
  // All pairs of non-equivalent primitives are not related by either
  // subtypeOf or couldBe.
  for (auto& t1 : primitives) {
    for (auto& t2 : primitives) {
      if (t1 != t2) {
        EXPECT_TRUE(!t1.subtypeOf(t2) && !t2.subtypeOf(t1));
        EXPECT_TRUE(!t1.couldBe(t2));
        EXPECT_TRUE(!t2.couldBe(t1));
      }
    }
  }
}

TEST(Type, Relations) {
  // couldBe is symmetric and reflexive
  for (auto& t1 : all) {
    for (auto& t2 : all) {
      EXPECT_TRUE(t1.couldBe(t2) == t2.couldBe(t1));
    }
  }
  for (auto& t1 : all) EXPECT_TRUE(t1.couldBe(t1));

  // subtype is antisymmetric and reflexive
  for (auto& t1 : all) {
    for (auto& t2 : all) {
      if (t1 != t2) {
        EXPECT_TRUE(!(t1.subtypeOf(t2) && t2.subtypeOf(t1)));
      }
    }
  }
  for (auto& t1 : all) EXPECT_TRUE(t1.subtypeOf(t1));

  // union_of is commutative
  for (auto& t1 : all) {
    for (auto& t2 : all) {
      EXPECT_TRUE(union_of(t1, t2) == union_of(t2, t1));
    }
  }
}

TEST(Type, Unc) {
  EXPECT_TRUE(TInt.subtypeOf(TInitUnc));
  EXPECT_TRUE(TInt.subtypeOf(TUnc));
  EXPECT_TRUE(TDbl.subtypeOf(TInitUnc));
  EXPECT_TRUE(TDbl.subtypeOf(TUnc));
  EXPECT_TRUE(dval(3.0).subtypeOf(TInitUnc));
}

TEST(Type, DblNan) {
  auto const qnan = std::numeric_limits<double>::quiet_NaN();
  EXPECT_TRUE(dval(qnan).subtypeOf(dval(qnan)));
  EXPECT_TRUE(dval(qnan).couldBe(dval(qnan)));
  EXPECT_TRUE(dval(qnan) == dval(qnan));
}

TEST(Type, Option) {
  EXPECT_TRUE(TTrue.subtypeOf(TOptTrue));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptTrue));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptTrue));

  EXPECT_TRUE(TFalse.subtypeOf(TOptFalse));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptFalse));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptFalse));

  EXPECT_TRUE(TFalse.subtypeOf(TOptBool));
  EXPECT_TRUE(TTrue.subtypeOf(TOptBool));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptBool));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptBool));

  EXPECT_TRUE(ival(3).subtypeOf(TOptInt));
  EXPECT_TRUE(TInt.subtypeOf(TOptInt));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptInt));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptInt));

  EXPECT_TRUE(TDbl.subtypeOf(TOptDbl));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptDbl));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptDbl));
  EXPECT_TRUE(dval(3.0).subtypeOf(TOptDbl));

  const StaticString s_test("test");

  EXPECT_TRUE(sval(s_test.get()).subtypeOf(TOptSStr));
  EXPECT_TRUE(TSStr.subtypeOf(TOptSStr));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptSStr));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptSStr));
  EXPECT_TRUE(!TStr.subtypeOf(TOptSStr));
  EXPECT_TRUE(TStr.couldBe(TOptSStr));

  EXPECT_TRUE(TStr.subtypeOf(TOptStr));
  EXPECT_TRUE(TSStr.subtypeOf(TOptStr));
  EXPECT_TRUE(sval(s_test.get()).subtypeOf(TOptStr));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptStr));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptStr));

  EXPECT_TRUE(TSArr.subtypeOf(TOptSArr));
  EXPECT_TRUE(!TArr.subtypeOf(TOptSArr));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptSArr));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptSArr));

  EXPECT_TRUE(TArr.subtypeOf(TOptArr));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptArr));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptArr));

  EXPECT_TRUE(TObj.subtypeOf(TOptObj));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptObj));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptObj));

  EXPECT_TRUE(TRes.subtypeOf(TOptRes));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptRes));
  EXPECT_TRUE(!TUninit.subtypeOf(TOptRes));
}

TEST(Type, SpecificExamples) {
  // Random examples to stress option types, values, etc:

  EXPECT_TRUE(!TInt.subtypeOf(ival(1)));

  EXPECT_TRUE(TInitCell.couldBe(ival(1)));
  EXPECT_TRUE(TInitCell.subtypeOf(TGen));
  EXPECT_TRUE(ival(2).subtypeOf(TInt));
  EXPECT_TRUE(!ival(2).subtypeOf(TBool));
  EXPECT_TRUE(ival(3).subtypeOf(TOptInt));
  EXPECT_TRUE(TInt.subtypeOf(TOptInt));
  EXPECT_TRUE(!TBool.subtypeOf(TOptInt));
  EXPECT_TRUE(TInitNull.subtypeOf(TOptInt));
  EXPECT_TRUE(!TNull.subtypeOf(TOptInt));
  EXPECT_TRUE(TNull.couldBe(TOptInt));
  EXPECT_TRUE(TNull.couldBe(TOptBool));

  EXPECT_TRUE(TInitNull.subtypeOf(TInitCell));
  EXPECT_TRUE(TInitNull.subtypeOf(TCell));
  EXPECT_TRUE(!TUninit.subtypeOf(TInitNull));

  EXPECT_TRUE(ival(3).subtypeOf(TOptInt));
  EXPECT_TRUE(ival(3).subtypeOf(opt(ival(3))));
  EXPECT_TRUE(ival(3).couldBe(opt(ival(3))));
  EXPECT_TRUE(ival(3).couldBe(TInt));
  EXPECT_TRUE(TInitNull.couldBe(opt(ival(3))));
  EXPECT_TRUE(TNull.couldBe(opt(ival(3))));
  EXPECT_TRUE(TInitNull.subtypeOf(opt(ival(3))));
  EXPECT_TRUE(!TNull.subtypeOf(opt(ival(3))));
}

//////////////////////////////////////////////////////////////////////

}}
