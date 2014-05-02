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

#include "hphp/util/asm-x64.h"

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/opt.h"

#include "hphp/runtime/vm/jit/test/test-context.h"

namespace HPHP { namespace JIT {

TEST(RefcountOpts, trivial) {
  BCMarker dummy = BCMarker::Dummy();
  IRUnit unit(test_context);
  Block* b = unit.entry();
  FrameState fs{unit, 0, nullptr, 0};

  auto fp  = unit.gen(DefFP, dummy);
  auto str = unit.gen(Conjure, dummy, Type::CountedStr);
  auto inc1 = unit.gen(IncRef, dummy, str->dst());
  auto inc2 = unit.gen(IncRef, dummy, str->dst());
  auto jonx = unit.gen(Conjure, dummy, Type::Gen);
  auto dec1 = unit.gen(DecRef, dummy, str->dst());
  auto dec2 = unit.gen(DecRef, dummy, str->dst());
  b->push_back({fp, str, inc1, inc2, jonx, dec1, dec2, unit.gen(Halt, dummy)});

  optimizeRefcounts(unit, std::move(fs));

  auto matcher = [] (Opcode op) {
    return [op] (const IRInstruction& i) { return i.op() == op; };
  };

  EXPECT_EQ(1, std::count_if(b->begin(), b->end(), matcher(IncRef)));
  EXPECT_EQ(1, std::count_if(b->begin(), b->end(), matcher(DecRef)));
}

}}
