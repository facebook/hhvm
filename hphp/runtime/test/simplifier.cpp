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
#include <gtest/gtest.h>

#include "hphp/runtime/base/array-init.h"

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/simplify.h"

#include "hphp/runtime/test/match.h"
#include "hphp/runtime/test/test-context.h"

#define EXPECT_SINGLE_OP(result, opc) \
  EXPECT_EQ(nullptr, (result).dst); \
  ASSERT_EQ(1, (result).instrs.size()); \
  EXPECT_EQ(opc, (result).instrs[0]->op());

#define EXPECT_NO_CHANGE(result) \
  EXPECT_EQ(nullptr, (result).dst); \
  EXPECT_EQ(0, (result).instrs.size());

namespace HPHP::jit {

//////////////////////////////////////////////////////////////////////

TEST(Simplifier, JumpConstFold) {
  auto const dummy = BCContext { BCMarker::Dummy(), 0 };
  IRUnit unit(test_context);

  // Folding JmpZero and JmpNZero.
  {
    auto tester = [&] (SSATmp* val, Opcode op) {
      auto jmp = unit.gen(op, dummy, unit.defBlock(), val);
      return simplify(unit, jmp);
    };

    auto resultFalseZero  = tester(unit.cns(false),  JmpZero);
    auto resultFalseNZero = tester(unit.cns(false), JmpNZero);
    auto resultTrueZero   = tester(unit.cns(true),   JmpZero);
    auto resultTrueNZero  = tester(unit.cns(true),  JmpNZero);

    EXPECT_SINGLE_OP(resultFalseNZero, Nop);
    EXPECT_SINGLE_OP(resultTrueZero, Nop);

    EXPECT_SINGLE_OP(resultFalseZero, Jmp);
    EXPECT_SINGLE_OP(resultTrueNZero, Jmp);
  }
}

TEST(Simplifier, CondJmp) {
  IRUnit unit{test_context};
  auto const bcctx = BCContext { BCMarker::Dummy(), 0 };

  // Folding Conv*ToBool
  {
    auto val = unit.gen(Conjure, bcctx, TInt);
    auto cnv = unit.gen(ConvIntToBool, bcctx, val->dst());
    auto jcc = unit.gen(JmpZero, bcctx, unit.defBlock(), cnv->dst());

    auto result = simplify(unit, jcc);

    EXPECT_EQ(nullptr, result.dst);
    ASSERT_EQ(1, result.instrs.size());
    EXPECT_MATCH(result.instrs[0], JmpZero, val->dst());
  }

  // Folding in negation
  {
    auto val = unit.gen(Conjure, bcctx, TBool);
    auto neg = unit.gen(XorBool, bcctx, val->dst(), unit.cns(true));
    auto jcc = unit.gen(JmpZero, bcctx, unit.defBlock(), neg->dst());

    auto result = simplify(unit, jcc);

    EXPECT_EQ(nullptr, result.dst);
    ASSERT_EQ(1, result.instrs.size());
    EXPECT_MATCH(result.instrs[0], JmpNZero, val->dst());
  }
}

TEST(Simplifier, Count) {
  IRUnit unit{test_context};
  auto const dummy = BCContext { BCMarker::Dummy(), 0 };

  // Count($null) --> 0
  {
    auto const null = unit.gen(Conjure, dummy, TNull);
    auto const count = unit.gen(Count, dummy, null->dst());
    auto const result = simplify(unit, count);

    EXPECT_NE(nullptr, result.dst);
    EXPECT_EQ(0, result.instrs.size());
    EXPECT_EQ(0, result.dst->intVal());
  }

  // Count($bool_int_dbl_str) --> 1
  {
    auto const ty = TBool | TInt | TDbl | TStr | TRes;
    auto const val = unit.gen(Conjure, dummy, ty);
    auto const count = unit.gen(Count, dummy, val->dst());
    auto const result = simplify(unit, count);

    EXPECT_NE(nullptr, result.dst);
    EXPECT_EQ(0, result.instrs.size());
    EXPECT_EQ(1, result.dst->intVal());
  }

  // Count($vec) --> CountVec($vec)
  {
    auto const arr = unit.gen(Conjure, dummy, TVec);
    auto const count = unit.gen(Count, dummy, arr->dst());
    auto const result = simplify(unit, count);

    EXPECT_NE(nullptr, result.dst);
    EXPECT_EQ(1, result.instrs.size());
    EXPECT_MATCH(result.instrs[0], CountVec, arr->dst());
  }

  // Count($dict) --> CountDict($dict)
  {
    auto const arr = unit.gen(Conjure, dummy, TDict);
    auto const count = unit.gen(Count, dummy, arr->dst());
    auto const result = simplify(unit, count);

    EXPECT_NE(nullptr, result.dst);
    EXPECT_EQ(1, result.instrs.size());
    EXPECT_MATCH(result.instrs[0], CountDict, arr->dst());
  }

  // Count($keyset) --> CountKeyset($keyset)
  {
    auto const arr = unit.gen(Conjure, dummy, TKeyset);
    auto const count = unit.gen(Count, dummy, arr->dst());
    auto const result = simplify(unit, count);

    EXPECT_NE(nullptr, result.dst);
    EXPECT_EQ(1, result.instrs.size());
    EXPECT_MATCH(result.instrs[0], CountKeyset, arr->dst());
  }

  // Count($some_obj) --> Count($some_obj)
  {
    auto const obj = unit.gen(Conjure, dummy, TObj);
    auto const count = unit.gen(Count, dummy, obj->dst());
    auto const result = simplify(unit, count);
    EXPECT_NO_CHANGE(result);
  }

}

TEST(Simplifier, LdObjClass) {
  IRUnit unit{test_context};
  auto const dummy = BCContext { BCMarker::Dummy(), 0 };
  auto const cls = SystemLib::getHH_IteratorClass();

  // LdObjClass t1:Obj<=C doesn't simplify
  {
    auto sub = Type::SubObj(cls);
    auto obj = unit.gen(Conjure, dummy, sub);
    auto load = unit.gen(LdObjClass, dummy, obj->dst());
    auto result = simplify(unit, load);
    EXPECT_NO_CHANGE(result);
  }

  // LdObjClass t1:Obj=C --> Cls(C)
  {
    auto exact = Type::ExactObj(cls);
    auto obj = unit.gen(Conjure, dummy, exact);
    auto load = unit.gen(LdObjClass, dummy, obj->dst());
    auto result = simplify(unit, load);
    EXPECT_EQ(result.dst->clsVal(), cls);
    EXPECT_EQ(result.instrs.size(), 0);
  }
}

TEST(Simplifier, LdObjInvoke) {
  IRUnit unit{test_context};
  auto const dummy = BCContext { BCMarker::Dummy(), 0 };

  // LdObjInvoke t1:Cls doesn't simplify
  {
    auto type = TCls;
    auto cls = unit.gen(Conjure, dummy, type);
    auto load = unit.gen(LdObjInvoke, dummy, cls->dst());
    auto result = simplify(unit, load);
    EXPECT_NO_CHANGE(result);
  }

  // LdObjInvoke t1:Cls(C), where C is persistent but has no __invoke
  // simplifies to nullptr.
  {
    auto type = Type::cns(SystemLib::getHH_IteratorClass());
    auto cls = unit.gen(Conjure, dummy, type);
    auto load = unit.gen(LdObjInvoke, dummy, cls->dst());
    auto result = simplify(unit, load);
    EXPECT_EQ(result.dst->type(), TNullptr);
    EXPECT_EQ(result.instrs.size(), 0);
  }
}

}
