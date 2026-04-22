/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/containers.h"
#ifdef __aarch64__
#include "hphp/runtime/vm/jit/abi-arm.h"
#endif
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include "hphp/util/immed.h"

#include <folly/portability/GTest.h>

namespace HPHP::jit {

namespace {

// Strip extraneous whitespace from vasm code strings.
std::string stripWhitespace(std::string str) {
  if (str.length() > 1 && str[0] == '\n' && str[1] == ' ') {
    str.erase(0, 2);
  }
  size_t spc, pos = 0;
  while ((spc = str.find("  ", pos)) != std::string::npos) {
    str.erase(spc, 1);
    pos = spc;
  }
  pos = 0;
  while ((spc = str.find(" \n", pos)) != std::string::npos) {
    str.erase(spc, 1);
    pos = spc;
  }
  pos = 0;
  while ((spc = str.find("\n ", pos)) != std::string::npos) {
    str.erase(spc + 1, 1);
    pos = spc;
  }
  return str;
}

template<class F>
void testPostRAWithTraceAndPrologueAbi(F f) {
  f(abi(CodeKind::Trace));
  f(abi(CodeKind::Prologue));
}

void testSetccXor() {
  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    auto sf = v.makeReg();
    auto dst = v.makeReg();
    auto xdst = v.makeReg();
    v << movl{unit.makeConst(42u), sf};
    v << setcc{CC_Z, sf, dst};
    v << xorbi{1, dst, xdst, v.makeReg()};

    simplify(unit);

    // Test that setcc/xor pair is collapsed.
    EXPECT_EQ(
      "B0 main (1)\n"
      "movl %131(42l) => %128\n"
      "setcc NE, %128 => %130\n",
      stripWhitespace(show(unit))
    );
  }

  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    auto sf = v.makeReg();
    auto dst = v.makeReg();
    auto xdst = v.makeReg();
    v << movl{unit.makeConst(42u), sf};
    v << setcc{CC_Z, sf, dst};
    v << xorbi{1, dst, xdst, v.makeReg()};
    v << movl{dst, v.makeReg()};

    simplify(unit);

    // Test that setcc/xor pair is not collapsed when setcc result
    // has more than one use.
    EXPECT_EQ(
      "B0 main (1)\n"
      "movl %131(42l) => %128\n"
      "setcc E, %128 => %129\n"
      "xorbi 1, %129 => %130, %132\n"
      "movl %129 => %133\n",
      stripWhitespace(show(unit))
    );
  }

  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    auto sf = v.makeReg();
    auto dst = v.makeReg();
    auto xdst = v.makeReg();
    v << movl{unit.makeConst(42u), sf};
    v << setcc{CC_NZ, sf, dst};
    v << xorbi{1, dst, xdst, v.makeReg()};

    simplify(unit);

    // Check that setcc/xor pair is collapsed with different condition.
    EXPECT_EQ(
      "B0 main (1)\n"
      "movl %131(42l) => %128\n"
      "setcc E, %128 => %130\n",
      stripWhitespace(show(unit))
    );
  }

  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    auto sf = v.makeReg();
    auto dst = v.makeReg();
    v << movl{unit.makeConst(42u), sf};
    v << setcc{CC_NZ, sf, dst};

    simplify(unit);

    // Make sure that setcc with no xor doesn't cause a buffer overrun.
    EXPECT_EQ(
      "B0 main (1)\n"
      "movl %130(42l) => %128\n"
      "setcc NE, %128 => %129\n",
      stripWhitespace(show(unit))
    );
  }

  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    auto sf = v.makeReg();
    auto dst = v.makeReg();
    auto xdst = v.makeReg();
    v << movl{unit.makeConst(42u), sf};
    v << setcc{CC_NZ, sf, dst};
    v << xorbi{2, dst, xdst, v.makeReg()};

    simplify(unit);

    // Make sure that setcc/xor with an non-1 xor constant is skipped.
    EXPECT_EQ(
      "B0 main (1)\n"
      "movl %131(42l) => %128\n"
      "setcc NE, %128 => %129\n"
      "xorbi 2, %129 => %130, %132\n",
      stripWhitespace(show(unit))
    );
  }

  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    auto sf = v.makeReg();
    auto dst = v.makeReg();
    auto xdst = v.makeReg();
    auto xsf = v.makeReg();
    v << movl{unit.makeConst(42u), sf};
    v << setcc{CC_NZ, sf, dst};
    v << xorbi{1, dst, xdst, xsf};
    v << movl{xsf, v.makeReg()};

    simplify(unit);

    // Make sure that setcc/xor with xor status flags being used is skipped.
    EXPECT_EQ(
      "B0 main (1)\n"
      "movl %132(42l) => %128\n"
      "setcc NE, %128 => %129\n"
      "xorbi 1, %129 => %130, %131\n"
      "movl %131 => %133\n",
      stripWhitespace(show(unit))
    );
  }

}

void testPostRACopyFold() {
  // Use register numbers valid on both x64 (0-15) and ARM (0-30),
  // avoiding rsp (4 on x64, 31 on ARM).
  auto const r0 = Vreg{Reg64{0}};
  auto const r1 = Vreg{Reg64{1}};
  auto const r8 = Vreg{Reg64{8}};
  auto const r9 = Vreg{Reg64{9}};
  auto const r10 = Vreg{Reg64{10}};

  // Post-RA fold: shrli{2, r8, r8}; copy{r8, r0} -> shrli{2, r8, r0}
  testPostRAWithTraceAndPrologueAbi([&] (const Abi& postRAAbi) {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    auto const sf = v.makeReg();
    v << shrli{2, Vreg32(r8), Vreg32(r8), sf, 0};
    v << copy{r8, r0};

    postRASimplify(unit, postRAAbi);

    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(1, code.size());
    EXPECT_EQ(Vinstr::shrli, code[0].op);
    EXPECT_EQ(Vreg32(r0), code[0].shrli_.d);
  });

  // Post-RA fold: load{[r10-0x10], r8}; copy{r8, r0} -> load{[r10-0x10], r0}
  testPostRAWithTraceAndPrologueAbi([&] (const Abi& postRAAbi) {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    v << load{r10[-0x10], r8};
    v << copy{r8, r0};

    postRASimplify(unit, postRAAbi);

    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(1, code.size());
    EXPECT_EQ(Vinstr::load, code[0].op);
    EXPECT_EQ(r0, code[0].load_.d);
  });

  // Negative: copy source used after the copy — should NOT fold.
  testPostRAWithTraceAndPrologueAbi([&] (const Abi& postRAAbi) {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    auto const sf = v.makeReg();
    v << shrli{2, Vreg32(r8), Vreg32(r8), sf, 0};
    v << copy{r8, r0};
    v << copy{r8, r1};  // r8 still used

    postRASimplify(unit, postRAAbi);

    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(3, code.size());
    EXPECT_EQ(Vinstr::shrli, code[0].op);
    EXPECT_EQ(Vreg32(r8), code[0].shrli_.d);
    EXPECT_EQ(Vinstr::copy, code[1].op);
    EXPECT_EQ(Vinstr::copy, code[2].op);
  });

  // Negative: intervening instruction reads cp.d — should NOT fold.
  testPostRAWithTraceAndPrologueAbi([&] (const Abi& postRAAbi) {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    auto const sf = v.makeReg();
    v << shrli{2, Vreg32(r8), Vreg32(r8), sf, 0};
    v << copy{r0, r10};  // reads r0 (the copy dest)
    v << copy{r8, r0};

    postRASimplify(unit, postRAAbi);

    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(3, code.size());
    EXPECT_EQ(Vinstr::shrli, code[0].op);
    EXPECT_EQ(Vreg32(r8), code[0].shrli_.d);
    EXPECT_EQ(Vinstr::copy, code[1].op);
    EXPECT_EQ(Vinstr::copy, code[2].op);
  });

  // Negative: intervening call may clobber cp.d implicitly — should NOT fold.
  testPostRAWithTraceAndPrologueAbi([&] (const Abi& postRAAbi) {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    auto const sf = v.makeReg();
    v << shrli{2, Vreg32(r8), Vreg32(r8), sf, 0};
    v << callr{Vreg64(r9), RegSet{}};
    v << copy{r8, r0};

    postRASimplify(unit, postRAAbi);

    auto const& code = unit.blocks[unit.entry].code;
    ASSERT_EQ(3, code.size());
    EXPECT_EQ(Vinstr::shrli, code[0].op);
    EXPECT_EQ(Vreg32(r8), code[0].shrli_.d);
    EXPECT_EQ(Vinstr::callr, code[1].op);
    EXPECT_EQ(Vinstr::copy, code[2].op);
  });
}

void testSinkDefsMovesIntoMergeAfterPhidef() {
  for (auto const kind : {CodeKind::Trace, CodeKind::Prologue}) {
    SCOPED_TRACE(kind == CodeKind::Trace ? "trace_abi" : "prologue_abi");

    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    auto const pred = unit.makeBlock(AreaIndex::Main, 1);
    auto const merge = unit.makeBlock(AreaIndex::Main, 1);

    Vout v(unit, unit.entry);
    Vout vp(unit, pred);
    Vout vm(unit, merge);

    auto const cand = Vreg64{v.makeReg()};
    auto const phiIn = Vreg64{vp.makeReg()};
    auto const phi = Vreg64{vm.makeReg()};
    auto const out = Vreg64{vm.makeReg()};

    v << ldimmq{Immed64{42}, cand};
    v << jmp{pred};

    vp << ldimmq{Immed64{1}, phiIn};
    vp << phijmp{merge, vp.makeTuple({phiIn})};

    vm << phidef{vm.makeTuple({phi})};
    vm << copy{cand, out};
    vm << ret{};

    sinkDefs(unit, abi(kind));

    auto const& entryCode = unit.blocks[unit.entry].code;
    ASSERT_EQ(entryCode.size(), 1);
    ASSERT_EQ(entryCode[0].op, Vinstr::jmp);
    EXPECT_EQ(entryCode[0].jmp_.target, pred);

    auto const& mergeCode = unit.blocks[merge].code;
    ASSERT_EQ(mergeCode.size(), 4);
    ASSERT_EQ(mergeCode[0].op, Vinstr::phidef);
    ASSERT_EQ(mergeCode[1].op, Vinstr::ldimmq);
    EXPECT_EQ(static_cast<uint64_t>(mergeCode[1].ldimmq_.s.q()), 42);
    EXPECT_EQ(mergeCode[1].ldimmq_.d, cand);
    ASSERT_EQ(mergeCode[2].op, Vinstr::copy);
    EXPECT_EQ(mergeCode[2].copy_.s, Vreg{cand});
    EXPECT_EQ(mergeCode[2].copy_.d, Vreg{out});
    ASSERT_EQ(mergeCode[3].op, Vinstr::ret);
  }
}

void testSinkDefsKeepsJoinPointDefsInPlace() {
  for (auto const kind : {CodeKind::Trace, CodeKind::Prologue}) {
    SCOPED_TRACE(kind == CodeKind::Trace ? "trace_abi" : "prologue_abi");

    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    auto const left = unit.makeBlock(AreaIndex::Main, 1);
    auto const right = unit.makeBlock(AreaIndex::Main, 1);

    Vout v(unit, unit.entry);
    Vout vl(unit, left);
    Vout vr(unit, right);

    auto const cand = Vreg64{v.makeReg()};
    auto const sf = v.makeReg();
    auto const leftUse = Vreg64{vl.makeReg()};
    auto const rightUse = Vreg64{vr.makeReg()};

    v << ldimmq{Immed64{42}, cand};
    v << cmpqi{Immed{0}, Vreg64{Reg64{0}}, sf, Vflags{}};
    v << jcc{CC_E, sf, {left, right}, StringTag{}};

    vl << copy{cand, leftUse};
    vl << ret{};

    vr << copy{cand, rightUse};
    vr << ret{};

    sinkDefs(unit, abi(kind));

    auto const& entryCode = unit.blocks[unit.entry].code;
    ASSERT_EQ(entryCode.size(), 3);
    ASSERT_EQ(entryCode[0].op, Vinstr::ldimmq);
    EXPECT_EQ(static_cast<uint64_t>(entryCode[0].ldimmq_.s.q()), 42);
    EXPECT_EQ(entryCode[0].ldimmq_.d, cand);
    ASSERT_EQ(entryCode[2].op, Vinstr::jcc);

    auto const& leftCode = unit.blocks[left].code;
    ASSERT_EQ(leftCode.size(), 2);
    ASSERT_EQ(leftCode[0].op, Vinstr::copy);
    EXPECT_EQ(leftCode[0].copy_.s, Vreg{cand});

    auto const& rightCode = unit.blocks[right].code;
    ASSERT_EQ(rightCode.size(), 2);
    ASSERT_EQ(rightCode[0].op, Vinstr::copy);
    EXPECT_EQ(rightCode[0].copy_.s, Vreg{cand});
  }
}

void testArmLeaLowering() {
#ifndef __aarch64__
  GTEST_SKIP() << "ARM-specific lea lowering test";
#else
  auto const test = [] (Vptr ptr) {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    auto const dst = arm::rret(0);
    v << lea{ptr, Vreg64{dst}};
    v << ret{RegSet{dst}};

    optimize(unit, arm::abi(), false);
    return stripWhitespace(show(unit));
  };

  auto const base = Vreg64{arm::rarg(2)};
  auto const index = Vreg64{arm::rarg(3)};

  auto const scaled = test(Vptr{base, index, 8, 0});
  EXPECT_EQ(scaled.find("shlqi"), std::string::npos);
  EXPECT_NE(scaled.find("lea ["), std::string::npos);
  EXPECT_NE(scaled.find("* 8"), std::string::npos);

  auto const scaledDisp = test(Vptr{base, index, 8, 16});
  EXPECT_EQ(scaledDisp.find("shlqi"), std::string::npos);
  EXPECT_NE(scaledDisp.find("lea ["), std::string::npos);
  EXPECT_NE(scaledDisp.find("* 8"), std::string::npos);
  EXPECT_NE(scaledDisp.find("0x10"), std::string::npos);
#endif
}

}

TEST(Vasm, Simplifier) {
  testSetccXor();
  testPostRACopyFold();
  testSinkDefsMovesIntoMergeAfterPhidef();
  testSinkDefsKeepsJoinPointDefsInPlace();
}

TEST(Vasm, ArmLeaLowering) {
  testArmLeaLowering();
}

void testArmLoadPairNoClobberBase() {
#ifndef __aarch64__
  GTEST_SKIP() << "ARM-specific loadpair test";
#else
  // Test that two adjacent loads are NOT combined when the first load's
  // destination overwrites the second load's base register.
  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    auto const base = v.makeReg();
    auto const dst2 = v.makeReg();
    v << ldimmq{uintptr_t(0x1000), base};
    // First load writes to 'base', clobbering the second load's base.
    v << load{base[0x10], base};
    v << load{base[0x18], dst2};
    v << ret{RegSet{base} | RegSet{dst2}};

    simplify(unit);

    auto const result = stripWhitespace(show(unit));
    // Should NOT be combined — first load overwrites the base of the second.
    EXPECT_EQ(result.find("loadpair"), std::string::npos)
      << "Should not combine dependent loads (base), got:\n" << result;
  }

  // Test that two adjacent loads are NOT combined when the first load's
  // destination overwrites the second load's index register.
  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    auto const base = v.makeReg();
    auto const idx = v.makeReg();
    auto const dst2 = v.makeReg();
    v << ldimmq{uintptr_t(0x1000), base};
    v << ldimmq{uintptr_t(0x8), idx};
    // First load writes to 'idx', clobbering the second load's index.
    v << load{Vptr{base, idx, 1, 0x10}, idx};
    v << load{Vptr{base, idx, 1, 0x18}, dst2};
    v << ret{RegSet{idx} | RegSet{dst2}};

    simplify(unit);

    auto const result = stripWhitespace(show(unit));
    // Should NOT be combined — first load overwrites the index of the second.
    EXPECT_EQ(result.find("loadpair"), std::string::npos)
      << "Should not combine dependent loads (index), got:\n" << result;
  }

  // Test that two adjacent loads with the same destination are NOT combined,
  // and the dead first load is removed by DCE.
  {
    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    Vout v(unit, unit.entry);

    auto const base = v.makeReg();
    auto const dst = v.makeReg();
    v << ldimmq{uintptr_t(0x1000), base};
    v << load{base[0x10], dst};
    v << load{base[0x18], dst};
    v << ret{RegSet{dst}};

    simplify(unit);

    auto result = stripWhitespace(show(unit));
    // Should NOT be fused — both loads write to the same destination.
    EXPECT_EQ(result.find("loadpair"), std::string::npos)
      << "Should not fuse loads with same destination, got:\n" << result;

    // The first load's destination is redefined by the second load and has
    // no other uses, so DCE should remove it.
    removeDeadCode(unit);

    result = stripWhitespace(show(unit));
    // Only one load should remain (the second one at offset 0x18).
    size_t pos = 0;
    int loadCount = 0;
    while ((pos = result.find("load ", pos)) != std::string::npos) {
      loadCount++;
      pos++;
    }
    EXPECT_EQ(loadCount, 1)
      << "Expected DCE to remove first dead load, got:\n" << result;
  }
#endif
}

TEST(Vasm, ArmLoadPairNoClobberBase) {
  testArmLoadPairNoClobberBase();
}

}
