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

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

TEST(Simplifier, JumpConstFold) {
  BCMarker dummy = BCMarker::Dummy();
  IRUnit unit(test_context);

  // Folding JmpZero and JmpNZero.
  {
    auto tester = [&] (SSATmp* val, Opcode op) {
      auto jmp = unit.gen(op, dummy, unit.defBlock(), val);
      return simplify(unit, jmp, false);
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
  BCMarker marker = BCMarker::Dummy();

  // Folding Conv*ToBool
  {
    auto val = unit.gen(Conjure, marker, TInt);
    auto cnv = unit.gen(ConvIntToBool, marker, val->dst());
    auto jcc = unit.gen(JmpZero, marker, unit.defBlock(), cnv->dst());

    auto result = simplify(unit, jcc, false);

    EXPECT_EQ(nullptr, result.dst);
    ASSERT_EQ(1, result.instrs.size());
    EXPECT_MATCH(result.instrs[0], JmpZero, val->dst());
  }

  // Folding in negation
  {
    auto val = unit.gen(Conjure, marker, TBool);
    auto neg = unit.gen(XorBool, marker, val->dst(), unit.cns(true));
    auto jcc = unit.gen(JmpZero, marker, unit.defBlock(), neg->dst());

    auto result = simplify(unit, jcc, false);

    EXPECT_EQ(nullptr, result.dst);
    ASSERT_EQ(1, result.instrs.size());
    EXPECT_MATCH(result.instrs[0], JmpNZero, val->dst());
  }
}

TEST(Simplifier, DoubleCmp) {
  IRUnit unit{test_context};
  BCMarker dummy = BCMarker::Dummy();

  // Lt(X:Dbl, Y:Int) --> LtDbl(X, ConvIntToDbl(Y))
  {
    auto x = unit.gen(Conjure, dummy, TDbl);
    auto y = unit.gen(Conjure, dummy, TInt);
    auto lt = unit.gen(Lt, dummy, x->dst(), y->dst());
    auto result = simplify(unit, lt, false);

    auto conv = result.instrs[0];
    EXPECT_MATCH(conv, ConvIntToDbl, y->dst());
    EXPECT_MATCH(result.instrs[1], LtDbl, x->dst(), conv->dst());
    EXPECT_EQ(result.instrs[1]->dst(), result.dst);
  }

  // Lt(X:Dbl, 10) --> LtDbl(X, 10.0)
  {
    auto x  = unit.gen(Conjure, dummy, TDbl);
    auto lt = unit.gen(Lt, dummy, x->dst(), unit.cns(10));
    auto result = simplify(unit, lt, false);

    EXPECT_MATCH(result.instrs[0], LtDbl, x->dst(), unit.cns(10.0));
  }
}

TEST(Simplifier, Count) {
  IRUnit unit{test_context};
  BCMarker dummy = BCMarker::Dummy();

  // Count($null) --> 0
  {
    auto null = unit.gen(Conjure, dummy, TNull);
    auto count = unit.gen(Count, dummy, null->dst());
    auto result = simplify(unit, count, false);

    EXPECT_NE(nullptr, result.dst);
    EXPECT_EQ(0, result.instrs.size());
    EXPECT_EQ(0, result.dst->intVal());
  }

  // Count($bool_int_dbl_str) --> 1
  {
    auto ty = TBool | TInt | TDbl | TStr | TRes;
    auto val = unit.gen(Conjure, dummy, ty);
    auto count = unit.gen(Count, dummy, val->dst());
    auto result = simplify(unit, count, false);

    EXPECT_NE(nullptr, result.dst);
    EXPECT_EQ(0, result.instrs.size());
    EXPECT_EQ(1, result.dst->intVal());
  }

  // Count($array_no_kind) --> CountArray($array_no_kind)
  {
    auto arr = unit.gen(Conjure, dummy, TArr);
    auto count = unit.gen(Count, dummy, arr->dst());
    auto result = simplify(unit, count, false);

    EXPECT_NE(nullptr, result.dst);
    EXPECT_EQ(1, result.instrs.size());
    EXPECT_MATCH(result.instrs[0], CountArray, arr->dst());
  }

  // Count($array_packed) --> CountArrayFast($array_packed)
  {
    auto ty = Type::Array(ArrayData::kPackedKind);
    auto arr = unit.gen(Conjure, dummy, ty);
    auto count = unit.gen(Count, dummy, arr->dst());
    auto result = simplify(unit, count, false);

    EXPECT_NE(nullptr, result.dst);
    EXPECT_EQ(1, result.instrs.size());
    EXPECT_MATCH(result.instrs[0], CountArrayFast, arr->dst());
  }

  // Count($some_obj) --> Count($some_obj)
  {
    auto obj = unit.gen(Conjure, dummy, TObj);
    auto count = unit.gen(Count, dummy, obj->dst());
    auto result = simplify(unit, count, false);
    EXPECT_NO_CHANGE(result);
  }

}

TEST(Simplifier, LdObjClass) {
  IRUnit unit{test_context};
  auto const dummy = BCMarker::Dummy();
  auto const cls = SystemLib::s_IteratorClass;

  // LdObjClass t1:Obj<=C doesn't simplify
  {
    auto sub = Type::SubObj(cls);
    auto obj = unit.gen(Conjure, dummy, sub);
    auto load = unit.gen(LdObjClass, dummy, obj->dst());
    auto result = simplify(unit, load, false);
    EXPECT_NO_CHANGE(result);
  }

  // LdObjClass t1:Obj=C --> Cls(C)
  {
    auto exact = Type::ExactObj(cls);
    auto obj = unit.gen(Conjure, dummy, exact);
    auto load = unit.gen(LdObjClass, dummy, obj->dst());
    auto result = simplify(unit, load, false);
    EXPECT_EQ(result.dst->clsVal(), cls);
    EXPECT_EQ(result.instrs.size(), 0);
  }
}

TEST(Simplifier, LdObjInvoke) {
  IRUnit unit{test_context};
  auto const dummy = BCMarker::Dummy();
  auto const taken = unit.defBlock();

  // LdObjInvoke t1:Cls doesn't simplify
  {
    auto type = TCls;
    auto cls = unit.gen(Conjure, dummy, type);
    auto load = unit.gen(LdObjInvoke, dummy, taken, cls->dst());
    auto result = simplify(unit, load, false);
    EXPECT_NO_CHANGE(result);
  }

  // LdObjInvoke t1:Cls(C), where C is persistent but has no __invoke
  // doesn't simplify.
  {
    auto type = Type::cns(SystemLib::s_IteratorClass);
    auto cls = unit.gen(Conjure, dummy, type);
    auto load = unit.gen(LdObjInvoke, dummy, taken, cls->dst());
    auto result = simplify(unit, load, false);
    EXPECT_NO_CHANGE(result);
  }
}

}}
