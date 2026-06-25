/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)   |
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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include "hphp/util/configs/jit.h"

#include <folly/portability/GTest.h>

namespace HPHP::jit {

namespace {

Vreg128 makeSimd(Vout& v) {
  return Vreg128{v.makeReg()};
}

}

// Two identical 128-bit immediates collapse: the second materialization is
// replaced by a copy of the first.
TEST(Vasm, ReuseImmqDeduplicatesIdentical128) {
  if (Cfg::Jit::LdimmqSpan <= 0) GTEST_SKIP() << "ldimmq reuse disabled";

  Vunit unit;
  unit.entry = unit.makeBlock(AreaIndex::Main, 1);
  Vout v(unit, unit.entry);

  auto const d0 = makeSimd(v);
  auto const d1 = makeSimd(v);

  v << ldimm128{0xdeadbeef, 0xcafef00d, d0};
  v << ldimm128{0xdeadbeef, 0xcafef00d, d1};
  v << ret{};

  reuseImmq(unit);

  auto const& code = unit.blocks[unit.entry].code;
  ASSERT_EQ(code[0].op, Vinstr::ldimm128);
  ASSERT_EQ(code[1].op, Vinstr::copy);
  EXPECT_EQ(code[1].copy_.s, Vreg{d0});
  EXPECT_EQ(code[1].copy_.d, Vreg{d1});
}

// Constants that differ in only one 64-bit lane are not deduplicated.
TEST(Vasm, ReuseImmqKeepsDistinct128) {
  if (Cfg::Jit::LdimmqSpan <= 0) GTEST_SKIP() << "ldimmq reuse disabled";

  Vunit unit;
  unit.entry = unit.makeBlock(AreaIndex::Main, 1);
  Vout v(unit, unit.entry);

  auto const d0 = makeSimd(v);
  auto const d1 = makeSimd(v);

  v << ldimm128{0xdeadbeef, 0x1111, d0};
  v << ldimm128{0xdeadbeef, 0x2222, d1};
  v << ret{};

  reuseImmq(unit);

  auto const& code = unit.blocks[unit.entry].code;
  ASSERT_EQ(code[0].op, Vinstr::ldimm128);
  EXPECT_EQ(code[1].op, Vinstr::ldimm128);
}

// A 128-bit immediate is never reused from a same-valued quad immediate; the
// widths must match.
TEST(Vasm, ReuseImmqDoesNotReuse128FromQuad) {
  if (Cfg::Jit::LdimmqSpan <= 0) GTEST_SKIP() << "ldimmq reuse disabled";

  Vunit unit;
  unit.entry = unit.makeBlock(AreaIndex::Main, 1);
  Vout v(unit, unit.entry);

  auto const q = v.makeReg();
  auto const d0 = makeSimd(v);

  v << ldimmq{0xdeadbeef, q};
  v << ldimm128{0xdeadbeef, 0, d0};
  v << ret{};

  reuseImmq(unit);

  auto const& code = unit.blocks[unit.entry].code;
  EXPECT_EQ(code[1].op, Vinstr::ldimm128);
}

// Identical 128-bit immediates separated by more than the reuse span are not
// deduplicated: the first is evicted from the leaky bucket before the second.
TEST(Vasm, ReuseImmqKeeps128BeyondSpan) {
  if (Cfg::Jit::LdimmqSpan <= 0) GTEST_SKIP() << "ldimmq reuse disabled";

  Vunit unit;
  unit.entry = unit.makeBlock(AreaIndex::Main, 1);
  Vout v(unit, unit.entry);

  auto const d0 = makeSimd(v);
  auto const d1 = makeSimd(v);

  v << ldimm128{0xdeadbeef, 0xcafef00d, d0};
  // Emit enough filler to push the first materialization out of the
  // leaky-bucket window before the second one is processed.
  for (auto i = 0; i < Cfg::Jit::LdimmqSpan; ++i) {
    v << nop{};
  }
  v << ldimm128{0xdeadbeef, 0xcafef00d, d1};
  v << ret{};

  reuseImmq(unit);

  size_t ldimm128s = 0;
  size_t copies = 0;
  for (auto const& inst : unit.blocks[unit.entry].code) {
    if (inst.op == Vinstr::ldimm128) ++ldimm128s;
    if (inst.op == Vinstr::copy) ++copies;
  }
  // Both remain full materializations; no copy is introduced.
  EXPECT_EQ(ldimm128s, 2);
  EXPECT_EQ(copies, 0);
}

// Reusing a previous 128-bit materialization refreshes the leaky bucket with the
// copy destination, so later matches can reuse the copied value after the
// original materialization expires.
TEST(Vasm, ReuseImmqRefreshes128OnReuse) {
  if (Cfg::Jit::LdimmqSpan < 2) {
    GTEST_SKIP() << "ldimmq reuse span is too small";
  }

  Vunit unit;
  unit.entry = unit.makeBlock(AreaIndex::Main, 1);
  Vout v(unit, unit.entry);

  auto const d0 = makeSimd(v);
  auto const d1 = makeSimd(v);
  auto const d2 = makeSimd(v);

  v << ldimm128{0xdeadbeef, 0xcafef00d, d0};
  v << ldimm128{0xdeadbeef, 0xcafef00d, d1};
  for (auto i = 2; i <= Cfg::Jit::LdimmqSpan; ++i) {
    v << nop{};
  }
  v << ldimm128{0xdeadbeef, 0xcafef00d, d2};
  v << ret{};

  reuseImmq(unit);

  auto const& code = unit.blocks[unit.entry].code;
  ASSERT_EQ(code[1].op, Vinstr::copy);
  EXPECT_EQ(code[1].copy_.s, Vreg{d0});
  EXPECT_EQ(code[1].copy_.d, Vreg{d1});

  auto const finalCopyIdx = static_cast<size_t>(Cfg::Jit::LdimmqSpan) + 1;
  ASSERT_EQ(code[finalCopyIdx].op, Vinstr::copy);
  EXPECT_EQ(code[finalCopyIdx].copy_.s, Vreg{d1});
  EXPECT_EQ(code[finalCopyIdx].copy_.d, Vreg{d2});

  size_t ldimm128s = 0;
  size_t copies = 0;
  for (auto const& inst : code) {
    if (inst.op == Vinstr::ldimm128) ++ldimm128s;
    if (inst.op == Vinstr::copy) ++copies;
  }
  EXPECT_EQ(ldimm128s, 1);
  EXPECT_EQ(copies, 2);
}

}
