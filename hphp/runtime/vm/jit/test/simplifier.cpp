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

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/simplifier.h"

#include "hphp/runtime/vm/jit/test/match.h"

#include <gtest/gtest.h>

#define EXPECT_SINGLE_OP(result, opc) \
  EXPECT_EQ(nullptr, (result).dst); \
  ASSERT_EQ(1, (result).instrs.size()); \
  EXPECT_EQ(opc, (result).instrs[0]->op());

#define EXPECT_NO_CHANGE(result) \
  EXPECT_EQ(nullptr, (result).dst); \
  EXPECT_EQ(0, (result).instrs.size());

namespace HPHP { namespace JIT {

//////////////////////////////////////////////////////////////////////

namespace {

template<typename T>
struct OptionSetter {
  OptionSetter(T& o, T newVal) : opt(o), oldVal(o) {
    opt = newVal;
  }
  ~OptionSetter() {
    opt = oldVal;
  }

  T& opt;
  T oldVal;
};

}

//////////////////////////////////////////////////////////////////////

TEST(Simplifier, JumpConstFold) {
  BCMarker dummy = BCMarker::Dummy();
  IRUnit unit(0);
  Simplifier sim(unit);

  // Folding JmpZero and JmpNZero.
  {
    auto tester = [&] (SSATmp* val, Opcode op) {
      auto jmp = unit.gen(op, dummy, unit.defBlock(), val);
      return sim.simplify(jmp, false);
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

  // Folding query jumps.
  {
    auto jmpeqTaken = unit.gen(JmpEq, dummy, unit.cns(10), unit.cns(10));
    auto result = sim.simplify(jmpeqTaken, false);
    EXPECT_SINGLE_OP(result, Jmp);

    auto jmpeqNTaken = unit.gen(JmpEq, dummy, unit.cns(10), unit.cns(400));
    result = sim.simplify(jmpeqNTaken, false);
    EXPECT_SINGLE_OP(result, Nop);
  }
}

TEST(Simplifier, CondJmp) {
  IRUnit unit{0};
  Simplifier sim{unit};
  BCMarker marker = BCMarker::Dummy();

  // Folding Conv*ToBool
  {
    auto val = unit.gen(Conjure, marker, Type::Int);
    auto cnv = unit.gen(ConvIntToBool, marker, val->dst());
    auto jcc = unit.gen(JmpZero, marker, unit.defBlock(), cnv->dst());

    auto result = sim.simplify(jcc, false);

    EXPECT_EQ(nullptr, result.dst);
    ASSERT_EQ(1, result.instrs.size());
    EXPECT_MATCH(result.instrs[0], JmpZero, val->dst());
  }

  // Folding in negation
  {
    auto val = unit.gen(Conjure, marker, Type::Bool);
    auto neg = unit.gen(XorBool, marker, val->dst(), unit.cns(true));
    auto jcc = unit.gen(JmpZero, marker, unit.defBlock(), neg->dst());

    auto result = sim.simplify(jcc, false);

    EXPECT_EQ(nullptr, result.dst);
    ASSERT_EQ(1, result.instrs.size());
    EXPECT_MATCH(result.instrs[0], JmpNZero, val->dst());
  }
}

TEST(Simplifier, JumpFuse) {
  BCMarker dummy = BCMarker::Dummy();
  IRUnit unit(0);
  Simplifier sim(unit);

  {
    // JmpZero(Eq(X, true)) --> JmpEq(X, false)
    auto taken = unit.defBlock();
    auto lhs = unit.cns(true);
    auto rhs = unit.gen(Conjure, dummy, Type::Bool);
    auto eq  = unit.gen(Eq, dummy, lhs, rhs->dst());
    auto jmp = unit.gen(JmpZero, dummy, taken, eq->dst());
    auto result = sim.simplify(jmp, false);

    EXPECT_EQ(nullptr, result.dst);
    EXPECT_EQ(2, result.instrs.size());

    // This is a dead Eq instruction; an artifact of weirdness in the
    // implementation. Should go away.
    EXPECT_FALSE(result.instrs[0]->isControlFlow());

    EXPECT_MATCH(result.instrs[1], JmpEq, taken, rhs->dst(), unit.cns(false));
  }

  {
    // JmpNZero(Neq(X, Y)) --> JmpNeq(X, Y)
    auto taken = unit.defBlock();
    auto x = unit.gen(Conjure, dummy, Type::Dbl);
    auto y = unit.gen(Conjure, dummy, Type::Dbl);

    auto neq = unit.gen(Neq, dummy, x->dst(), y->dst());
    auto jmp = unit.gen(JmpNZero, dummy, taken, neq->dst());
    auto result = sim.simplify(jmp, false);

    EXPECT_EQ(nullptr, result.dst);
    EXPECT_EQ(1, result.instrs.size());
    EXPECT_MATCH(result.instrs[0], JmpNeq, taken, x->dst(), y->dst());
  }
}


}}
