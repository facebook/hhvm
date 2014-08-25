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
#include "folly/Format.h"

#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/code-gen.h"

#include "hphp/runtime/vm/jit/test/test-context.h"

namespace HPHP { namespace jit {

// basic liveness of a use of a const inside a loop
TEST(Loops, Counting) {
  // init:
  //   cnt0 = 0
  //   sum0 = 0
  //   Jmp loop (cnt0, sum0)
  // loop:
  //   cnt1, sum1 = DefLabel<2>
  //   cnt2 = AddInt cnt1, 1
  //   sum2 = AddInt cnt1, sum1
  //   JmpLtInt cnt2, 100, exit, back
  // back:
  //   Jmp loop (cnt2, sum2)
  // exit:
  //   Halt
  BCMarker marker = BCMarker::Dummy();
  IRUnit unit{test_context};
  auto init = unit.entry();
  auto loop = unit.defBlock();
  auto exit = unit.defBlock();
  auto back = unit.defBlock();

  auto loop_label = unit.defLabel(2, marker, {0, 0});
  loop->push_back(loop_label);

  auto cnt0 = unit.cns(0);
  auto sum0 = unit.cns(0);
  auto jmp0 = unit.gen(Jmp, marker, loop, cnt0, sum0);
  init->push_back(jmp0);

  auto cnt1 = loop_label->dst(0);
  auto sum1 = loop_label->dst(1);
  loop->push_back(unit.gen(AddInt, marker, cnt1, unit.cns(1)));
  auto cnt2 = loop->back().dst();
  loop->push_back(unit.gen(AddInt, marker, cnt1, sum1));
  auto sum2 = loop->back().dst();
  loop->push_back(unit.gen(JmpLtInt, marker, back, cnt2, unit.cns(100)));
  loop->back().setNext(exit);

  back->push_back(unit.gen(Jmp, marker, loop, cnt2, sum2));

  exit->push_back(unit.gen(Halt, marker));

  reflowTypes(unit);
  auto regs = allocateRegs(unit);
  ASSERT_TRUE(checkRegisters(unit, regs));
}

// make sure computeLiveRegs handles loops too.  This defines
// sum0 before the loop so it gets a register, and checks to
// ensure the register is live across a subsequent call inside
// the loop body.
TEST(Loops, ComputeLive) {
  // init:
  //   cnt0 = 0
  //   sum0 = Sqrt(1.1)
  //   Jmp loop (cnt0, sum0)
  // loop:
  //   cnt1, sum1 = DefLabel<2>
  //   cnt2 = AddInt cnt1, 1
  //   sum2 = AddDbl sum1, sum0
  //   JmpLtInt cnt2, 100, exit, back
  // back:
  //   sum3 = Floor(sum2)
  //   Jmp loop (cnt2, sum3)
  // exit:
  //   Halt
  BCMarker marker = BCMarker::Dummy();
  IRUnit unit{test_context};
  auto init = unit.entry();
  auto loop = unit.defBlock();
  auto exit = unit.defBlock();
  auto back = unit.defBlock();

  auto loop_label = unit.defLabel(2, marker, {0, 0});
  loop->push_back(loop_label);

  auto cnt0 = unit.cns(0);
  init->push_back(unit.gen(Sqrt, marker, unit.cns(2.0)));
  auto sum0 = init->back().dst();
  auto jmp0 = unit.gen(Jmp, marker, loop, cnt0, sum0);
  init->push_back(jmp0);

  auto cnt1 = loop_label->dst(0);
  auto sum1 = loop_label->dst(1);
  loop->push_back(unit.gen(AddInt, marker, cnt1, unit.cns(1)));
  auto cnt2 = loop->back().dst();
  loop->push_back(unit.gen(AddDbl, marker, sum1, sum0));
  auto sum2 = loop->back().dst();
  loop->push_back(unit.gen(JmpLtInt, marker, back, cnt2, unit.cns(100)));
  loop->back().setNext(exit);

  back->push_back(unit.gen(Floor, marker, sum2));
  auto sum3 = back->back().dst();
  back->push_back(unit.gen(Jmp, marker, loop, cnt2, sum3));

  exit->push_back(unit.gen(Halt, marker));

  reflowTypes(unit);
  auto regs = allocateRegs(unit);
  ASSERT_TRUE(checkRegisters(unit, regs));

  // check that the register assigned to sum0 is live across the Floor
  auto floor = sum3->inst();
  auto sum0reg = regs[sum0->inst()].dst(0).reg();

  auto live = computeLiveRegs(unit, regs);
  ASSERT_TRUE(live[floor].contains(sum0reg));
}

}}
