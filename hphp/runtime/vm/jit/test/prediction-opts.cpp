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
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/opt.h"

#include "hphp/util/asm-x64.h"

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
    auto& ctm = entry->back();
    EXPECT_EQ(CheckTypeMem, ctm.op());
    EXPECT_EQ(Type::Int, ctm.typeParam());
    EXPECT_EQ(taken, ctm.taken());
    EXPECT_EQ(end, ctm.next());
  }

  {
    ASSERT_EQ(3, taken->instrs().size());
    auto takenIt = taken->begin();
    auto& ldmemGen = *takenIt;
    EXPECT_EQ(LdMem, ldmemGen.op());
    EXPECT_EQ(Type::Gen, ldmemGen.typeParam());
    auto& increfGen = *(++takenIt);
    EXPECT_EQ(IncRef, increfGen.op());
    EXPECT_EQ(ldmemGen.dst(), increfGen.src(0));
  }

  {
    ASSERT_EQ(4, end->instrs().size());
    auto endIt = end->begin();
    auto& ldmemInt = *endIt;
    EXPECT_EQ(LdMem, ldmemInt.op());
    EXPECT_EQ(Type::Int, ldmemInt.typeParam());
    auto& increfInt = *(++endIt);
    EXPECT_EQ(IncRef, increfInt.op());
    EXPECT_EQ(ldmemInt.dst(), increfInt.src(0));
    auto& mov = *(++endIt);
    EXPECT_EQ(Mov, mov.op());
    EXPECT_EQ(ldmemInt.dst(), mov.src(0));
    EXPECT_EQ(ckt->dst(), mov.dst());
  }
}

}}
