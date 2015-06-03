/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <gtest/gtest.h>

#include <boost/range/join.hpp>
#include <vector>

#include "hphp/runtime/test/test-context.h"

#include "hphp/runtime/vm/jit/alias-class.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

std::vector<AliasClass> generic_classes() {
  return {
    AFrameAny,
    APropAny,
    AHeapAny,
    AStackAny,
    AElemIAny,
    AElemSAny,
    AElemAny,
  };
}

std::vector<AliasClass> specialized_classes(IRUnit& unit) {
  auto const marker = BCMarker::Dummy();

  // Specialized test cases need some SSATmp*'s and similar things, so let's
  // make some instructions.
  auto const mainFP = unit.gen(DefFP, marker)->dst();
  auto const SP = unit.gen(
    DefSP, marker, FPInvOffsetData { FPInvOffset { 10 } }, mainFP)->dst();

  return {
    // Frame locals.
    AFrame { mainFP, 1 },
    AFrame { mainFP, 2 },
    AFrame { mainFP, 6 },

    // Some stack locations.
    AStack { SP, -1, 1 },
    AStack { SP, -2, 3 },

    // Frame-based 'canonicalized' stack locations.
    AStack { mainFP, -12, std::numeric_limits<int32_t>::max() },
    AStack { mainFP, -12, 4 },
    AStack { mainFP, -11, 3 },
    AStack { mainFP, -52, 10 },
  };
}

}

//////////////////////////////////////////////////////////////////////

TEST(AliasClass, Basic) {
  IRUnit unit{test_context};
  auto const specialized = specialized_classes(unit);
  auto const joined = boost::join(generic_classes(), specialized);

  for (auto cls : joined) {
    // Everything is a subclass of AUnknown and intersects AUnknown.
    EXPECT_TRUE(cls.maybe(AUnknown));
    EXPECT_TRUE(cls <= AUnknown);

    // Everything contains AEmpty, but AEmpty can't be anything.
    EXPECT_TRUE(AEmpty <= cls);
    EXPECT_FALSE(AEmpty.maybe(cls));
  }

  // maybe() is a symmetric relation
  for (auto c1 : joined) {
    EXPECT_EQ(c1.maybe(AUnknown), AUnknown.maybe(c1));
    EXPECT_EQ(c1.maybe(AEmpty), AEmpty.maybe(c1));
    for (auto c2 : joined) {
      EXPECT_EQ(c1.maybe(c2), c2.maybe(c1));
    }
  }

  // If one class is a subclass of another, and they aren't equal, the other
  // isn't a subclass of it.
  for (auto c1 : joined) {
    for (auto c2 : joined) {
      if (c1 != c2) {
        if (c1 <= c2) {
          EXPECT_FALSE(c2 <= c1);
        }
      }
    }
  }

  // == implies <=, and <= implies maybe
  for (auto c1 : joined) {
    for (auto c2 : joined) {
      if (c1 == c2) EXPECT_TRUE(c1 <= c2);
      if (c1 <= c2) EXPECT_TRUE(c1.maybe(c2));
    }
  }

  // Arguments to operator| are subclasses of the result, and operator| is
  // commutative.
  for (auto c1 : joined) {
    for (auto c2 : joined) {
      auto const res = c1 | c2;
      EXPECT_EQ(res, c2 | c1);
      EXPECT_TRUE(c1 <= res);
      EXPECT_TRUE(c2 <= res);
    }
  }
}

TEST(AliasClass, StackBasics) {
  IRUnit unit{test_context};
  auto const marker = BCMarker::Dummy();
  auto const FP = unit.gen(DefFP, marker)->dst();
  auto const SP = unit.gen(
    DefSP, marker, FPInvOffsetData { FPInvOffset { 5 } }, FP)->dst();

  // Some basic canonicalization and maybe.
  {
    AliasClass const stk1 = AStack { SP, 0, 1 };
    AliasClass const stk2 = AStack { FP, -5, 1 };

    EXPECT_TRUE(stk1 <= AStackAny);
    EXPECT_TRUE(stk2 <= AStackAny);
    EXPECT_TRUE(stk1 != AStackAny);
    EXPECT_TRUE(stk2 != AStackAny);

    EXPECT_EQ(stk1, stk2);
    EXPECT_TRUE(stk1.maybe(stk2));
    EXPECT_TRUE(stk1 <= stk2);
  }

  // Stack ranges, with subtype and maybe.
  {
    AliasClass const stk1 = AStack { FP, -10, 1 };
    AliasClass const stk2 = AStack { FP, -10, 2 };
    EXPECT_TRUE(stk1 <= stk2);
    EXPECT_TRUE(stk1.maybe(stk2));

    AliasClass const stk3 = AStack { FP, -10, 5 };
    EXPECT_TRUE(stk1 <= stk3);
    EXPECT_TRUE(stk1.maybe(stk3));
    AliasClass const stk4 = AStack { FP, -15, 1 };
    AliasClass const stk5 = AStack { FP, -14, 1 };
    // stk4's slot is immediately below stk3's range, but stk5 is the last slot
    // of its range.
    EXPECT_FALSE(stk3.maybe(stk4));
    EXPECT_FALSE(stk3 <= stk4);
    EXPECT_TRUE(stk5.maybe(stk3));
    EXPECT_TRUE(stk5 <= stk3);
    EXPECT_FALSE(stk5.maybe(stk4));
    EXPECT_FALSE(stk5 <= stk4);
  }
}

TEST(AliasClass, SpecializedUnions) {
  IRUnit unit{test_context};
  auto const marker = BCMarker::Dummy();
  auto const FP = unit.gen(DefFP, marker)->dst();

  AliasClass const stk = AStack { FP, -10, 3 };
  AliasClass const unrelated_stk = AStack { FP, -14, 1 };
  AliasClass const related_stk = AStack { FP, -11, 2 };

  auto const stk_and_frame = stk | AFrameAny;
  EXPECT_TRUE(!stk_and_frame.is_stack());
  EXPECT_TRUE(AFrameAny <= stk_and_frame);
  EXPECT_TRUE(stk <= stk_and_frame);
  EXPECT_TRUE(AStackAny.maybe(stk_and_frame));
  EXPECT_TRUE(AFrameAny.maybe(stk_and_frame));
  EXPECT_FALSE(unrelated_stk <= stk_and_frame);
  EXPECT_FALSE(stk_and_frame.maybe(unrelated_stk));

  auto const stk_and_prop = stk | APropAny;
  EXPECT_TRUE(stk_and_prop.maybe(stk_and_frame));
  EXPECT_TRUE(stk_and_frame.maybe(stk_and_prop));
  EXPECT_FALSE(stk_and_prop <= stk_and_frame);
  EXPECT_FALSE(stk_and_frame <= stk_and_prop);
  EXPECT_TRUE(APropAny.maybe(stk_and_prop));
  EXPECT_TRUE(AStackAny.maybe(stk_and_prop));

  auto const unrelated_stk_and_prop = unrelated_stk | APropAny;
  EXPECT_FALSE(stk_and_frame.maybe(unrelated_stk_and_prop));
  EXPECT_FALSE(unrelated_stk_and_prop.maybe(stk_and_frame));
  EXPECT_TRUE(unrelated_stk_and_prop.maybe(stk_and_prop)); // because of prop
  EXPECT_FALSE(unrelated_stk_and_prop <= stk_and_prop);
  EXPECT_FALSE(stk_and_prop <= unrelated_stk_and_prop);
  EXPECT_FALSE(unrelated_stk_and_prop <= stk_and_frame);
  EXPECT_FALSE(stk_and_frame <= unrelated_stk_and_prop);

  EXPECT_FALSE(stk_and_prop <= AHeapAny);
  EXPECT_TRUE(stk_and_prop.maybe(AHeapAny));
  EXPECT_FALSE(stk_and_frame <= AHeapAny);
  EXPECT_FALSE(stk_and_frame.maybe(AHeapAny));

  auto const rel_stk_and_frame = related_stk | AFrameAny;
  EXPECT_TRUE(stk_and_frame.maybe(rel_stk_and_frame));
  EXPECT_TRUE(rel_stk_and_frame.maybe(stk_and_frame));
  EXPECT_TRUE(related_stk <= stk);
  EXPECT_TRUE(rel_stk_and_frame <= stk_and_frame);
  EXPECT_FALSE(stk_and_frame <= rel_stk_and_frame);
  EXPECT_TRUE(rel_stk_and_frame.maybe(stk_and_prop));
  EXPECT_TRUE(stk_and_prop.maybe(rel_stk_and_frame));
  EXPECT_FALSE(rel_stk_and_frame <= stk_and_prop);

  AliasClass const some_mis = AMIState { 0x10 };
  {
    AliasClass const some_heap = AElemIAny;
    auto const u1 = some_heap | some_mis;
    auto const u2 = AFrameAny | u1;
    EXPECT_TRUE((AHeapAny | some_heap) == AHeapAny);
    EXPECT_TRUE(AHeapAny <= (AHeapAny | u1));
    EXPECT_TRUE(AHeapAny <= (AHeapAny | u2));
  }

  auto const mis_stk = some_mis | stk;
  auto const mis_stk_any = AStackAny | mis_stk;
  EXPECT_TRUE(mis_stk_any == (AStackAny | AMIStateAny));
}

TEST(AliasClass, StackUnions) {
  IRUnit unit{test_context};
  auto const marker = BCMarker::Dummy();
  auto const FP = unit.gen(DefFP, marker)->dst();
  auto const SP = unit.gen(
    DefSP, marker, FPInvOffsetData { FPInvOffset { 1 } }, FP)->dst();

  {
    AliasClass const stk1  = AStack { FP, -3, 1 };
    AliasClass const stk2  = AStack { FP, -4, 1 };
    AliasClass const stk3  = AStack { FP, -5, 1 };
    AliasClass const stk12 = AStack { FP, -3, 2 };
    AliasClass const stk23 = AStack { FP, -4, 2 };
    AliasClass const stk13 = AStack { FP, -3, 3 };
    EXPECT_EQ(stk1 | stk2, stk12);
    EXPECT_EQ(stk2 | stk3, stk23);
    EXPECT_EQ(stk1 | stk3, stk13);
  }

  // Same as above but with some other bits.
  {
    AliasClass const stk1  = AHeapAny | AStack { FP, -3, 1 };
    AliasClass const stk2  = AHeapAny | AStack { FP, -4, 1 };
    AliasClass const stk3  = AHeapAny | AStack { FP, -5, 1 };
    AliasClass const stk12 = AHeapAny | AStack { FP, -3, 2 };
    AliasClass const stk23 = AHeapAny | AStack { FP, -4, 2 };
    AliasClass const stk13 = AHeapAny | AStack { FP, -3, 3 };
    EXPECT_EQ(stk1 | stk2, stk12);
    EXPECT_EQ(stk2 | stk3, stk23);
    EXPECT_EQ(stk1 | stk3, stk13);
  }

  {
    AliasClass const stk1 = AStack { FP, -1, 1 };
    AliasClass const stk2 = AStack { SP, -2, 1 };
    AliasClass const true_union = AStack { FP, -1, 3 };
    EXPECT_NE(stk1 | stk2, AStackAny);
    EXPECT_EQ(stk1 | stk2, true_union);
  }

  {
    auto const imax = std::numeric_limits<int32_t>::max();
    AliasClass const deep_stk1 = AStack { FP, -10, imax };
    AliasClass const deep_stk2 = AStack { FP, -14, imax };
    EXPECT_EQ(deep_stk1 | deep_stk2, deep_stk1);
  }
}

TEST(AliasClass, IterUnion) {
  IRUnit unit{test_context};
  auto const marker = BCMarker::Dummy();
  auto const FP = unit.gen(DefFP, marker)->dst();

  {
    AliasClass const iterP0 = AIterPos { FP, 0 };
    AliasClass const iterP1 = AIterPos { FP, 1 };
    auto const u1 = iterP0 | iterP1;
    EXPECT_EQ(u1, AIterPosAny);
    EXPECT_TRUE(iterP0 <= AIterPosAny);
    EXPECT_FALSE(iterP0 <= AIterBaseAny);
  }

  {
    AliasClass const iterP0 = AIterPos { FP, 0 };
    AliasClass const iterB0 = AIterBase { FP, 0 };
    AliasClass const iterP1 = AIterPos { FP, 1 };
    auto const u1 = iterP0 | iterB0;
    EXPECT_TRUE(iterP0 <= u1);
    EXPECT_TRUE(iterB0 <= u1);
    EXPECT_FALSE(u1 <= AIterPosAny);
    EXPECT_FALSE(u1 <= AIterBaseAny);
    EXPECT_TRUE(u1 <= (AIterPosAny | AIterBaseAny));
    EXPECT_FALSE(iterP1 <= u1);
    EXPECT_FALSE(iterP1 <= iterP0);
    EXPECT_FALSE(iterP1 <= iterB0);

    EXPECT_TRUE(!!u1.iterPos());
    EXPECT_TRUE(!!u1.iterBase());
    EXPECT_TRUE(!u1.is_iterPos());
    EXPECT_TRUE(!u1.is_iterBase());
  }

  {
    AliasClass const local = AFrame { FP, 0 };
    AliasClass const iter  = AIterPos { FP, 0 };
    auto const u1 = local | iter;
    EXPECT_TRUE(local <= u1);
    EXPECT_TRUE(iter <= u1);
    EXPECT_FALSE(!!u1.is_iterPos());
    EXPECT_FALSE(!!u1.is_frame());
    EXPECT_TRUE(!!u1.frame());  // locals are preferred in unions to iters
    EXPECT_FALSE(!!u1.iterPos());
  }

  {
    AliasClass const iterP0 = AIterPos { FP, 0 };
    AliasClass const iterB0 = AIterBase { FP, 0 };
    AliasClass const iterP1 = AIterPos { FP, 1 };
    AliasClass const iterB1 = AIterBase { FP, 1 };

    EXPECT_FALSE(iterP0.maybe(iterP1));
    EXPECT_FALSE(iterB0.maybe(iterB1));

    auto const u1 = iterP0 | iterB0;
    auto const u2 = iterP1 | iterB1;
    EXPECT_FALSE(u1 == u2);
    EXPECT_FALSE(u1.maybe(u2));
    EXPECT_FALSE(u1 <= u2);
    EXPECT_FALSE(u2 <= u1);

    EXPECT_TRUE(iterB1 <= u2);
    EXPECT_TRUE(iterP1 <= u2);
    EXPECT_FALSE(iterP0 <= u2);
    EXPECT_FALSE(iterB0 <= u2);

    auto const u3 = u1 | iterP1;
    EXPECT_FALSE(!!u3.iterPos());
    EXPECT_FALSE(!!u3.iterBase());
    EXPECT_TRUE(iterP1 <= u3);
    EXPECT_TRUE(iterP0 <= u3);
    EXPECT_TRUE(iterB0 <= u3);
    EXPECT_TRUE(u1 <= u3);
    EXPECT_TRUE(u2.maybe(u3));

    // u2 <= u3 isn't 'really' true, but operator| is conservative and makes u3
    // too big for that right now.
    EXPECT_TRUE(!u1.precise_union(iterP1));
  }
}

//////////////////////////////////////////////////////////////////////

}}
