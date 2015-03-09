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

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include <gtest/gtest.h>

namespace HPHP { namespace jit {

const Abi test_abi{};

TEST(Vasm, OptimizeCopies) {
  Vunit unit;
  unit.entry = unit.makeBlock(AreaIndex::Main);
  Vout v(unit, unit.entry);

  auto v1 = v.makeReg(), v2 = v.makeReg();
  v << ldimmq{0, v1};
  v << copy2{v1, reg::rbx, reg::rbx, v2};
  v << store{v2, reg::rbp[0x10]};
  v << store{v1, reg::rbp[0x20]};
  v << fallthru{};

  auto it = unit.blocks[unit.entry].code.end();
  --it; --it;
  EXPECT_EQ(Vinstr::store, it->op);
  EXPECT_EQ(v1, it->store_.s);

  --it;
  EXPECT_EQ(Vinstr::store, it->op);
  EXPECT_EQ(v2, it->store_.s);

  optimizeCopies(unit, test_abi);

  // 'store v1, rbp[0x20]' should be 'store rbx, rbp[0x20]'
  it = unit.blocks[unit.entry].code.end();
  --it; --it;
  EXPECT_EQ(Vinstr::store, it->op);
  EXPECT_EQ(Vreg{reg::rbx}, it->store_.s);

  // 'store v2, rbp[0x10]' should be unchanged
  --it;
  EXPECT_EQ(Vinstr::store, it->op);
  EXPECT_EQ(v2, it->store_.s);
}

}}
