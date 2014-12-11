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

#include "hphp/runtime/test/match.h"
#include "hphp/runtime/test/test-context.h"

#include <gtest/gtest.h>

namespace HPHP { namespace jit {

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

}}
