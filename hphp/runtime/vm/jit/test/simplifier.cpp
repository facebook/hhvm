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

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/simplifier.h"

#include <gtest/gtest.h>

#define EXPECT_SINGLE_OP(result, opc) \
  EXPECT_EQ((result).dst, nullptr); \
  ASSERT_EQ((result).instrs.size(), 1); \
  EXPECT_EQ((result).instrs[0]->op(), (opc));

#define EXPECT_NO_CHANGE(result) \
  EXPECT_EQ((result).dst, nullptr); \
  EXPECT_EQ((result).instrs.size(), 0);

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

  OptionSetter<bool> r(RuntimeOption::EvalHHIRBytecodeControlFlow, false);

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

  // Don't do this if bytecode control flow is on.
  {
    OptionSetter<bool> r(RuntimeOption::EvalHHIRBytecodeControlFlow, true);

    auto jmp = unit.gen(JmpNZero, dummy, unit.defBlock(), unit.cns(false));
    auto result = sim.simplify(jmp, false);
    EXPECT_NO_CHANGE(result);
    ASSERT_TRUE(RuntimeOption::EvalHHIRBytecodeControlFlow);
  }
  ASSERT_FALSE(RuntimeOption::EvalHHIRBytecodeControlFlow);

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

TEST(Simplifier, JumpFuse) {
  BCMarker dummy = BCMarker::Dummy();
  IRUnit unit(0);
  Simplifier sim(unit);

  OptionSetter<bool> r(RuntimeOption::EvalHHIRBytecodeControlFlow, false);

  {
    // JmpZero(Eq(X, true)) --> JmpEq(X, false)
    auto lhs = unit.cns(true);
    auto rhs = unit.gen(Conjure, dummy, Type::Bool);
    auto eq  = unit.gen(Eq, dummy, lhs, rhs->dst());
    auto jmp = unit.gen(JmpZero, dummy, unit.defBlock(), eq->dst());
    auto result = sim.simplify(jmp, false);

    EXPECT_EQ(result.dst, nullptr);
    EXPECT_EQ(result.instrs.size(), 2);

    // This is a dead Eq instruction; an artifact of weirdness in the
    // implementation. Should go away.
    EXPECT_FALSE(result.instrs[0]->isControlFlow());

    EXPECT_EQ(result.instrs[1]->op(), JmpEq);
    EXPECT_EQ(result.instrs[1]->src(0), rhs->dst());
    EXPECT_EQ(result.instrs[1]->src(1)->boolVal(), false);
  }
}


}}
