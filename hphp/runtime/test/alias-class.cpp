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
    ANonFrame,
    ANonStack,
    AStackAny,
    AElemIAny,
    AElemAny,
  };
}

std::vector<AliasClass> specialized_classes(IRUnit& unit) {
  auto const marker = BCMarker::Dummy();

  // Specialized test cases need some SSATmp*'s and similar things, so let's
  // make some instructions.
  auto const mainFP = unit.gen(DefFP, marker)->dst();
  auto const SP = unit.gen(DefSP, marker, StackOffset { 10 }, mainFP)->dst();

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
  auto const SP = unit.gen(DefSP, marker, StackOffset { 5 }, FP)->dst();
  auto const SP2 = unit.gen(AdjustSP, marker, StackOffset { -2 }, SP)->dst();

  // Some basic canonicalization and maybe.
  {
    AliasClass const stk1 = AStack { SP, 0, 1 };
    AliasClass const stk2 = AStack { FP, -5, 1 };
    AliasClass const stk3 = AStack { SP2, -5, 1 };

    EXPECT_TRUE(stk1 <= AStackAny);
    EXPECT_TRUE(stk2 <= AStackAny);
    EXPECT_TRUE(stk3 <= AStackAny);
    EXPECT_TRUE(stk1 != AStackAny);
    EXPECT_TRUE(stk2 != AStackAny);
    EXPECT_TRUE(stk3 != AStackAny);

    EXPECT_NE(stk1, stk2);
    EXPECT_NE(stk2, stk3);
    EXPECT_NE(stk3, stk1);
    EXPECT_TRUE(stk1.maybe(stk2));
    EXPECT_TRUE(stk2.maybe(stk3));
    EXPECT_TRUE(stk3.maybe(stk1));
    EXPECT_FALSE(stk1 <= stk2);
    EXPECT_FALSE(stk2 <= stk3);
    EXPECT_FALSE(stk3 <= stk1);
    EXPECT_EQ(canonicalize(stk2), stk2); // already canonical
    EXPECT_EQ(canonicalize(stk1), stk2);
    EXPECT_NE(canonicalize(stk3), stk2);
    EXPECT_NE(canonicalize(stk3), stk1);
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

//////////////////////////////////////////////////////////////////////

}}
