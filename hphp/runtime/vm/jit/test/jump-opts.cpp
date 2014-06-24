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
#include "hphp/runtime/vm/jit/opt.h"

#include "hphp/runtime/vm/jit/test/match.h"
#include "hphp/runtime/vm/jit/test/test-context.h"

#include <gtest/gtest.h>

namespace HPHP { namespace JIT {

TEST(JumpOpts, eliminateTrivial) {
  BCMarker marker = BCMarker::Dummy();

  // Trivial jumps are eliminated
  {
    IRUnit unit{test_context};

    Block* entry = unit.entry();
    Block* second = unit.defBlock();

    entry->push_back(unit.gen(Jmp, marker, second));

    // Make sure the DefLabel gets deleted
    second->push_back(unit.gen(DefLabel, marker));
    second->push_back(unit.gen(Halt, marker));

    optimizeJumps(unit);

    EXPECT_EQ(1, entry->instrs().size());
    EXPECT_MATCH(entry->front(), Halt);
    EXPECT_TRUE(entry->isExit());
  }

  // Jumps with arguments are also eliminated
  {
    IRUnit unit{test_context};

    Block* entry = unit.entry();
    Block* second = unit.defBlock();

    auto value = unit.gen(Conjure, marker, Type::Gen);
    entry->push_back(value);
    entry->push_back(unit.gen(Jmp, marker, second, value->dst()));
    second->push_back(unit.defLabel(1, marker, {0}));
    second->push_back(unit.gen(Halt, marker));

    optimizeJumps(unit);

    EXPECT_EQ(3, entry->instrs().size());
    EXPECT_MATCH(entry->back(), Halt);
    EXPECT_TRUE(entry->isExit());
  }

  // Jumps to blocks with other predecessors are not eliminated
  {
    IRUnit unit{test_context};

    Block* pred1 = unit.entry();
    Block* pred2 = unit.defBlock();
    Block* succ  = unit.defBlock();

    // pred2 is actually unreachable but this optimization pass shouldn't be
    // concerned about that
    pred1->push_back(unit.gen(Jmp, marker, succ));
    pred2->push_back(unit.gen(Jmp, marker, succ));
    succ->push_back(unit.gen(Halt, marker));

    optimizeJumps(unit);

    EXPECT_EQ(1, pred1->instrs().size());
    EXPECT_EQ(1, pred2->instrs().size());
    EXPECT_MATCH(pred1->back(), Jmp, succ);
    EXPECT_MATCH(pred2->back(), Jmp, succ);
  }
}

TEST(JumpOpts, optimizeCondTraceExit) {
  BCMarker marker = BCMarker::Dummy();

  IRUnit unit{test_context};

  Block* entry = unit.entry();
  Block* taken = unit.defBlock();
  Block* fallthru = unit.defBlock();

  // A conditional jump that goes to "SyncABIRegs; ReqBindJmp" on both edges
  // can be coalesced into a ReqBindJmpSomething.

  auto fp = unit.gen(DefFP, marker);
  auto sp = unit.gen(DefSP, marker, StackOffset(0), fp->dst());
  auto val = unit.gen(Conjure, marker, Type::Bool);
  auto jmp = unit.gen(JmpZero, marker, taken, val->dst());
  jmp->setNext(fallthru);
  entry->push_back({fp, sp, val, jmp});

  auto bcoff1 = 10;
  auto sync1 = unit.gen(SyncABIRegs, marker, fp->dst(), sp->dst());
  auto bind1 = unit.gen(ReqBindJmp, marker, ReqBindJmpData(bcoff1));
  taken->push_back({sync1, bind1});

  auto bcoff2 = 20;
  auto sync2 = unit.gen(SyncABIRegs, marker, fp->dst(), sp->dst());
  auto bind2 = unit.gen(ReqBindJmp, marker, ReqBindJmpData(bcoff2));
  fallthru->push_back({sync2, bind2});

  optimizeJumps(unit);

  EXPECT_EQ(nullptr, entry->next());
  EXPECT_EQ(nullptr, entry->taken());
  auto const& back = entry->back();
  auto const* data = back.extra<ReqBindJccData>();
  EXPECT_MATCH(back, ReqBindJmpZero, val->dst());
  EXPECT_EQ(bcoff1, data->taken);
  EXPECT_EQ(bcoff2, data->notTaken);
}

TEST(JumpOpts, optimizeSideExitJcc) {
  BCMarker marker = BCMarker::Dummy();
  IRUnit unit{test_context};

  Block* entry = unit.entry();
  Block* taken = unit.defBlock();
  Block* fallthru = unit.defBlock();

  // A conditional jump that goes to a "SyncABIRegs; ReqBindJmp" on the taken
  // edge only can turn into a SideExitJmpSomething.

  auto fp = unit.gen(DefFP, marker);
  auto sp = unit.gen(DefSP, marker, StackOffset(0), fp->dst());
  auto val = unit.gen(Conjure, marker, Type::Bool);
  auto jcc = unit.gen(JmpZero, marker, taken, val->dst());
  jcc->setNext(fallthru);
  entry->push_back({fp, sp, val, jcc});

  fallthru->push_back(unit.gen(Halt, marker));

  auto bcoff = 10;
  auto sync = unit.gen(SyncABIRegs, marker, fp->dst(), sp->dst());
  auto bind = unit.gen(ReqBindJmp, marker, ReqBindJmpData(bcoff));
  taken->push_back({sync, bind});

  optimizeJumps(unit);

  // This exercises trivial jump optimization too: since the JmpZero gets turned
  // into a non-branch, the entry block gets coalesced with the Halt block.
  EXPECT_EQ(nullptr, entry->next());
  EXPECT_EQ(nullptr, entry->taken());
  EXPECT_EQ(Halt, entry->back().op());
  auto const& sideExit = *(--entry->backIter());
  EXPECT_MATCH(sideExit, SideExitJmpZero, val->dst());
  EXPECT_EQ(bcoff, sideExit.extra<SideExitJccData>()->taken);
}

TEST(JumpOpts, optimizeSideExitCheck) {
  BCMarker marker = BCMarker::Dummy();
  IRUnit unit{test_context};

  Block* entry = unit.entry();
  Block* taken = unit.defBlock();
  Block* fallthru = unit.defBlock();

  // A conditional jump that goes to a "SyncABIRegs; ReqBindJmp" on the taken
  // edge only can turn into a SideExitJmpSomething.

  auto fp = unit.gen(DefFP, marker);
  auto sp = unit.gen(DefSP, marker, StackOffset(0), fp->dst());
  auto chk = unit.gen(CheckStk, marker, Type::Int, taken,
                      StackOffset(0), sp->dst());
  chk->setNext(fallthru);
  entry->push_back({fp, sp, chk});

  fallthru->push_back(unit.gen(Halt, marker));

  auto bcoff = 10;
  auto sync = unit.gen(SyncABIRegs, marker, fp->dst(), sp->dst());
  auto bind = unit.gen(ReqBindJmp, marker, ReqBindJmpData(bcoff));
  taken->push_back({sync, bind});

  optimizeJumps(unit);

  // This exercises trivial jump optimization too: since the JmpZero gets turned
  // into a non-branch, the entry block gets coalesced with the Halt block.
  EXPECT_EQ(nullptr, entry->next());
  EXPECT_EQ(nullptr, entry->taken());
  EXPECT_EQ(Halt, entry->back().op());
  auto const& sideExit = *(--entry->backIter());
  EXPECT_MATCH(sideExit, SideExitGuardStk);
  EXPECT_EQ(bcoff, sideExit.extra<SideExitGuardData>()->taken);
}


}}
