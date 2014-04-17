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
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/opt.h"

#include "hphp/runtime/vm/jit/test/match.h"

#include <gtest/gtest.h>

namespace HPHP { namespace JIT {

TEST(PredictionOpts, basic) {
  UNUSED BCMarker marker = BCMarker::Dummy();
  IRUnit unit{0};

  Block* entry = unit.entry();
  Block* taken = unit.defBlock();
  Block* end = unit.defBlock();

  auto ptr = unit.gen(Conjure, marker, Type::PtrToGen);
  auto ldm = unit.gen(LdMem, marker, Type::Gen, ptr->dst(), unit.cns(0));
  auto inc = unit.gen(IncRef, marker, ldm->dst());
  auto ckt = unit.gen(CheckType, marker, Type::Int, taken, ldm->dst());
  ckt->setNext(end);
  entry->push_back({ptr, ldm, inc, ckt});

  taken->push_back(unit.gen(Halt, marker));
  end->push_back(unit.gen(Halt, marker));

  optimizePredictions(unit);

  // It should have pushed LdMem and IncRef to each successor block, with the
  // narrowed type on the fallthrough block.
  {
    ASSERT_EQ(2, entry->instrs().size());
    EXPECT_MATCH(entry->back(), CheckTypeMem, Type::Int, taken);
    EXPECT_EQ(end, entry->back().next());
  }

  {
    ASSERT_EQ(3, taken->instrs().size());
    auto takenIt = taken->begin();
    auto& ldmem = *takenIt;
    auto& incref = *(++takenIt);
    EXPECT_MATCH(ldmem, LdMem, Type::Gen, ptr->dst());
    EXPECT_MATCH(incref, IncRef, ldmem.dst());
  }

  {
    ASSERT_EQ(4, end->instrs().size());
    auto endIt = end->begin();
    auto& ldmem = *endIt;
    auto& incref = *(++endIt);
    auto& mov = *(++endIt);
    EXPECT_MATCH(ldmem, LdMem, Type::Int, ptr->dst());
    EXPECT_MATCH(incref, IncRef, ldmem.dst());
    EXPECT_MATCH(mov, Mov, ldmem.dst());
    EXPECT_EQ(ckt->dst(), mov.dst());
  }
}

}}
