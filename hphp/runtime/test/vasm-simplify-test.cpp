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
#include "hphp/runtime/test/test-context.h"
#include "hphp/runtime/test/vasm-test-helpers.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/immed.h"

#include <folly/portability/GTest.h>

#include <optional>

namespace HPHP::jit {

namespace {

IRInstruction* makeLdLocOrigin(IRUnit& unit,
                               BCContext bcctx,
                               SSATmp* fp,
                               uint32_t locId) {
  return unit.gen(LdLoc, bcctx, TCell, LocalId{locId}, fp);
}

IRInstruction* makeStLocOrigin(IRUnit& unit,
                               BCContext bcctx,
                               SSATmp* fp,
                               uint32_t locId) {
  auto const value = unit.gen(Conjure, bcctx, TCell)->dst();
  return unit.gen(StLoc, bcctx, LocalId{locId}, fp, value);
}

IRInstruction* makeLdStkOrigin(IRUnit& unit,
                               BCContext bcctx,
                               IRSPRelOffset offset) {
  auto const sp = unit.gen(
    DefRegSP,
    bcctx,
    DefStackData{SBInvOffset{0}, SBInvOffset{0}}
  )->dst();
  return unit.gen(LdStk, bcctx, TCell, IRSPRelOffsetData{offset}, sp);
}

IRInstruction* makeEnterInlineFrameOrigin(IRUnit& unit, BCContext bcctx) {
  auto const sp = unit.gen(
    DefRegSP,
    bcctx,
    DefStackData{SBInvOffset{0}, SBInvOffset{0}}
  )->dst();
  auto const calleeFP = unit.gen(
    DefCalleeFP,
    bcctx,
    DefCalleeFPData{
      IRSPRelOffset{0},
      nullptr,
      1,
      SrcKey{},
      IRSPRelOffset{0},
      SBInvOffset{0},
      0
    },
    sp
  )->dst();
  return unit.gen(EnterInlineFrame, bcctx, calleeFP);
}

// Used only as a stable direct-call address for Vasm call/vinvoke tests.
void sinkDefsTestHelper() {}

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

void testSinkDefsKeepsDefsOutOfHotterBlocks() {
  auto const test = [] (AreaIndex srcArea,
                        uint64_t srcWeight,
                        AreaIndex targetArea,
                        uint64_t targetWeight) {
    for (auto const kind : {CodeKind::Trace, CodeKind::Prologue}) {
      SCOPED_TRACE(kind == CodeKind::Trace ? "trace_abi" : "prologue_abi");

      Vunit unit;
      unit.entry = unit.makeBlock(AreaIndex::Main, 1);
      auto const src = unit.makeBlock(srcArea, srcWeight);
      auto const target = unit.makeBlock(targetArea, targetWeight);

      Vout v(unit, unit.entry);
      Vout vs(unit, src);
      Vout vt(unit, target);

      auto const cand = Vreg64{vs.makeReg()};
      auto const out = Vreg64{vt.makeReg()};

      v << jmp{src};

      vs << ldimmq{Immed64{42}, cand};
      vs << jmp{target};

      vt << copy{cand, out};
      vt << ret{};

      sinkDefs(unit, abi(kind));

      auto const& srcCode = unit.blocks[src].code;
      ASSERT_EQ(srcCode.size(), 2);
      ASSERT_EQ(srcCode[0].op, Vinstr::ldimmq);
      EXPECT_EQ(static_cast<uint64_t>(srcCode[0].ldimmq_.s.q()), 42);
      EXPECT_EQ(srcCode[0].ldimmq_.d, cand);
      ASSERT_EQ(srcCode[1].op, Vinstr::jmp);

      auto const& targetCode = unit.blocks[target].code;
      ASSERT_EQ(targetCode.size(), 2);
      ASSERT_EQ(targetCode[0].op, Vinstr::copy);
      EXPECT_EQ(targetCode[0].copy_.s, Vreg{cand});
      ASSERT_EQ(targetCode[1].op, Vinstr::ret);
    }
  };

  test(AreaIndex::Main, 1, AreaIndex::Main, 10);
  test(AreaIndex::Cold, 10, AreaIndex::Main, 10);
}

void testSinkDefsFallsBackToLegalDominator() {
  for (auto const kind : {CodeKind::Trace, CodeKind::Prologue}) {
    SCOPED_TRACE(kind == CodeKind::Trace ? "trace_abi" : "prologue_abi");

    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    auto const src = unit.makeBlock(AreaIndex::Main, 10);
    auto const mid = unit.makeBlock(AreaIndex::Main, 10);
    auto const target = unit.makeBlock(AreaIndex::Main, 20);

    Vout v(unit, unit.entry);
    Vout vs(unit, src);
    Vout vm(unit, mid);
    Vout vt(unit, target);

    auto const cand = Vreg64{vs.makeReg()};
    auto const out = Vreg64{vt.makeReg()};

    v << jmp{src};

    vs << ldimmq{Immed64{42}, cand};
    vs << jmp{mid};

    vm << jmp{target};

    vt << copy{cand, out};
    vt << ret{};

    sinkDefs(unit, abi(kind));

    auto const& srcCode = unit.blocks[src].code;
    ASSERT_EQ(srcCode.size(), 1);
    ASSERT_EQ(srcCode[0].op, Vinstr::jmp);
    EXPECT_EQ(srcCode[0].jmp_.target, mid);

    auto const& midCode = unit.blocks[mid].code;
    ASSERT_EQ(midCode.size(), 2);
    ASSERT_EQ(midCode[0].op, Vinstr::ldimmq);
    EXPECT_EQ(static_cast<uint64_t>(midCode[0].ldimmq_.s.q()), 42);
    EXPECT_EQ(midCode[0].ldimmq_.d, cand);
    ASSERT_EQ(midCode[1].op, Vinstr::jmp);
    EXPECT_EQ(midCode[1].jmp_.target, target);

    auto const& targetCode = unit.blocks[target].code;
    ASSERT_EQ(targetCode.size(), 2);
    ASSERT_EQ(targetCode[0].op, Vinstr::copy);
    EXPECT_EQ(targetCode[0].copy_.s, Vreg{cand});
    EXPECT_EQ(targetCode[0].copy_.d, Vreg{out});
    ASSERT_EQ(targetCode[1].op, Vinstr::ret);
  }
}

void testSinkDefsMovesDefsWithDeadSF() {
  for (auto const kind : {CodeKind::Trace, CodeKind::Prologue}) {
    SCOPED_TRACE(kind == CodeKind::Trace ? "trace_abi" : "prologue_abi");

    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    auto const slow = unit.makeBlock(AreaIndex::Main, 1);

    Vout v(unit, unit.entry);
    Vout vs(unit, slow);

    auto const src = Vreg64{v.makeReg()};
    auto const shifted = Vreg64{v.makeReg()};
    auto const out = Vreg64{vs.makeReg()};
    auto const sf = v.makeReg();

    v << ldimmq{Immed64{42}, src};
    v << shrqi{Immed{5}, src, shifted, sf, Vflags{}};
    v << jmp{slow};

    vs << copy{shifted, out};
    vs << ret{};

    sinkDefs(unit, abi(kind));

    auto const& entryCode = unit.blocks[unit.entry].code;
    ASSERT_EQ(entryCode.size(), 1);
    ASSERT_EQ(entryCode[0].op, Vinstr::jmp);
    EXPECT_EQ(entryCode[0].jmp_.target, slow);

    auto const& slowCode = unit.blocks[slow].code;
    ASSERT_EQ(slowCode.size(), 4);
    ASSERT_EQ(slowCode[0].op, Vinstr::ldimmq);
    EXPECT_EQ(slowCode[0].ldimmq_.d, src);
    ASSERT_EQ(slowCode[1].op, Vinstr::shrqi);
    EXPECT_EQ(slowCode[1].shrqi_.s1, src);
    EXPECT_EQ(slowCode[1].shrqi_.d, shifted);
    ASSERT_EQ(slowCode[2].op, Vinstr::copy);
    EXPECT_EQ(slowCode[2].copy_.s, Vreg{shifted});
    EXPECT_EQ(slowCode[2].copy_.d, Vreg{out});
    ASSERT_EQ(slowCode[3].op, Vinstr::ret);
  }
}

struct PureLoadSinkLinearContext {
  Vunit& unit;
  Vlabel mid;
  Vlabel exit;
  Vreg64 base;
  Vreg64 cand;
};

template<class MakeLoadOrigin, class EmitMid, class Verify>
void testSinkDefsPureLoadLinear(MakeLoadOrigin makeLoadOrigin,
                                EmitMid emitMid,
                                Verify verify) {
  for (auto const kind : {CodeKind::Trace, CodeKind::Prologue}) {
    SCOPED_TRACE(kind == CodeKind::Trace ? "trace_abi" : "prologue_abi");

    IRUnit irUnit{test_context};
    auto const bcctx = BCContext{BCMarker::Dummy(), 0};
    auto const fp = irUnit.gen(DefFP, bcctx, DefFPData{std::nullopt})->dst();
    auto const loadOrigin = makeLoadOrigin(irUnit, bcctx, fp);

    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    auto const mid = unit.makeBlock(AreaIndex::Main, 1);
    auto const exit = unit.makeBlock(AreaIndex::Main, 1);

    Vout v(unit, unit.entry);
    Vout vm(unit, mid);
    Vout ve(unit, exit);

    auto const base = Vreg64{v.makeReg()};
    auto const keepBase = Vreg64{v.makeReg()};
    auto const cand = Vreg64{v.makeReg()};
    auto const out = Vreg64{ve.makeReg()};

    v << ldimmq{Immed64{0x100}, base};
    v.setOrigin(loadOrigin);
    v << load{base[0], cand};
    v.setOrigin(nullptr);
    v << copy{base, keepBase};
    v << jmp{mid};

    emitMid(irUnit, bcctx, fp, vm, base, exit);

    ve << copy{cand, out};
    ve << ret{};

    sinkDefs(unit, abi(kind));

    verify(PureLoadSinkLinearContext{unit, mid, exit, base, cand});
  }
}

template<class EmitMid, class Verify>
void testSinkDefsLocalPureLoadLinear(uint32_t loadLoc,
                                     EmitMid emitMid,
                                     Verify verify) {
  testSinkDefsPureLoadLinear(
    [=] (IRUnit& irUnit, BCContext bcctx, SSATmp* fp) {
      return makeLdLocOrigin(irUnit, bcctx, fp, loadLoc);
    },
    emitMid,
    verify
  );
}

std::optional<size_t> findLoadDef(const Vblock& block, Vreg def) {
  for (auto i = size_t{0}; i < block.code.size(); ++i) {
    auto const& inst = block.code[i];
    if (inst.op == Vinstr::load && inst.load_.d == def) return i;
  }
  return std::nullopt;
}

std::optional<size_t> findUse(const Vunit& unit,
                              const Vblock& block,
                              Vreg use,
                              size_t begin = 0) {
  for (auto i = begin; i < block.code.size(); ++i) {
    auto found = false;
    visitUses(unit, block.code[i], [&] (Vreg r) {
      if (r == use) found = true;
    });
    if (found) return i;
  }
  return std::nullopt;
}

size_t countLoadDefs(const Vunit& unit, Vreg def) {
  auto count = size_t{0};
  for (auto const& block : unit.blocks) {
    for (auto const& inst : block.code) {
      if (inst.op == Vinstr::load && inst.load_.d == def) ++count;
    }
  }
  return count;
}

std::optional<size_t> expectOnlyLoadDefInBlock(const Vunit& unit,
                                               Vlabel block,
                                               Vreg def) {
  EXPECT_EQ(size_t{1}, countLoadDefs(unit, def))
    << stripWhitespace(show(unit));

  auto const pos = findLoadDef(unit.blocks[block], def);
  EXPECT_TRUE(pos.has_value()) << stripWhitespace(show(unit));
  return pos;
}

void expectPureLoadSunkToExit(const PureLoadSinkLinearContext& ctx) {
  auto const cand = Vreg{ctx.cand};
  auto const& entry = ctx.unit.blocks[ctx.unit.entry];
  auto const& exit = ctx.unit.blocks[ctx.exit];

  EXPECT_FALSE(findLoadDef(entry, cand).has_value())
    << stripWhitespace(show(ctx.unit));

  auto const exitLoad = expectOnlyLoadDefInBlock(ctx.unit, ctx.exit, cand);
  ASSERT_TRUE(exitLoad.has_value()) << stripWhitespace(show(ctx.unit));
  ASSERT_TRUE(findUse(ctx.unit, exit, cand, *exitLoad + 1).has_value())
    << stripWhitespace(show(ctx.unit));
}

void expectPureLoadKeptInEntry(const PureLoadSinkLinearContext& ctx) {
  auto const cand = Vreg{ctx.cand};
  auto const& exit = ctx.unit.blocks[ctx.exit];

  ASSERT_TRUE(expectOnlyLoadDefInBlock(ctx.unit, ctx.unit.entry, cand)
                .has_value()) << stripWhitespace(show(ctx.unit));
  EXPECT_FALSE(findLoadDef(exit, cand).has_value())
    << stripWhitespace(show(ctx.unit));
  ASSERT_TRUE(findUse(ctx.unit, exit, cand).has_value())
    << stripWhitespace(show(ctx.unit));
}

void expectPureLoadSunkToMid(const PureLoadSinkLinearContext& ctx) {
  auto const cand = Vreg{ctx.cand};
  auto const& entry = ctx.unit.blocks[ctx.unit.entry];
  auto const& exit = ctx.unit.blocks[ctx.exit];

  EXPECT_FALSE(findLoadDef(entry, cand).has_value())
    << stripWhitespace(show(ctx.unit));
  ASSERT_TRUE(expectOnlyLoadDefInBlock(ctx.unit, ctx.mid, cand).has_value())
    << stripWhitespace(show(ctx.unit));
  EXPECT_FALSE(findLoadDef(exit, cand).has_value())
    << stripWhitespace(show(ctx.unit));
  ASSERT_TRUE(findUse(ctx.unit, exit, cand).has_value())
    << stripWhitespace(show(ctx.unit));
}

void testSinkDefsPureLoadSinksAcrossUnrelatedStoreOrigin() {
  testSinkDefsLocalPureLoadLinear(
    0,
    [] (IRUnit& irUnit,
        BCContext bcctx,
        SSATmp* fp,
        Vout& vm,
        Vreg64 base,
        Vlabel exit) {
      auto const storeOrigin = makeStLocOrigin(irUnit, bcctx, fp, 1);
      auto const tmp = Vreg64{vm.makeReg()};
      vm.setOrigin(storeOrigin);
      vm << copy{base, tmp};
      vm.setOrigin(nullptr);
      vm << jmp{exit};
    },
    [] (const PureLoadSinkLinearContext& ctx) {
      expectPureLoadSunkToExit(ctx);

      auto const& midCode = ctx.unit.blocks[ctx.mid].code;
      ASSERT_EQ(midCode.size(), 2);
      ASSERT_EQ(midCode[0].op, Vinstr::copy);
      ASSERT_EQ(midCode[1].op, Vinstr::jmp);
    }
  );
}

void testSinkDefsPureLoadSinksAfterUserMoves() {
  for (auto const kind : {CodeKind::Trace, CodeKind::Prologue}) {
    SCOPED_TRACE(kind == CodeKind::Trace ? "trace_abi" : "prologue_abi");

    IRUnit irUnit{test_context};
    auto const bcctx = BCContext{BCMarker::Dummy(), 0};
    auto const fp = irUnit.gen(DefFP, bcctx, DefFPData{std::nullopt})->dst();
    auto const loadOrigin = makeLdLocOrigin(irUnit, bcctx, fp, 0);

    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    auto const exit = unit.makeBlock(AreaIndex::Main, 1);

    Vout v(unit, unit.entry);
    Vout ve(unit, exit);

    auto const base = Vreg64{v.makeReg()};
    auto const keepBase = Vreg64{v.makeReg()};
    auto const cand = Vreg64{v.makeReg()};
    auto const copied = Vreg64{v.makeReg()};
    auto const out = Vreg64{ve.makeReg()};

    v << ldimmq{Immed64{0x100}, base};
    v.setOrigin(loadOrigin);
    v << load{base[0], cand};
    v.setOrigin(nullptr);
    v << copy{base, keepBase};
    v << copy{cand, copied};
    v << jmp{exit};

    ve << copy{copied, out};
    ve << ret{};

    sinkDefs(unit, abi(kind));

    auto const candReg = Vreg{cand};
    auto const& entry = unit.blocks[unit.entry];
    auto const& exitBlock = unit.blocks[exit];

    EXPECT_FALSE(findLoadDef(entry, candReg).has_value())
      << stripWhitespace(show(unit));
    ASSERT_EQ(size_t{1}, countLoadDefs(unit, candReg))
      << stripWhitespace(show(unit));

    auto const exitLoad = findLoadDef(exitBlock, candReg);
    ASSERT_TRUE(exitLoad.has_value()) << stripWhitespace(show(unit));

    auto const movedUser = findUse(unit, exitBlock, candReg, *exitLoad + 1);
    ASSERT_TRUE(movedUser.has_value()) << stripWhitespace(show(unit));
    ASSERT_EQ(exitBlock.code[*movedUser].op, Vinstr::copy);
    EXPECT_EQ(exitBlock.code[*movedUser].copy_.s, candReg);
    EXPECT_EQ(exitBlock.code[*movedUser].copy_.d, Vreg{copied});
  }
}

void testSinkDefsPureLoadStopsBeforeClobberingStore() {
  testSinkDefsLocalPureLoadLinear(
    0,
    [] (IRUnit& irUnit,
        BCContext bcctx,
        SSATmp* fp,
        Vout& vm,
        Vreg64 base,
        Vlabel exit) {
      auto const storeOrigin = makeStLocOrigin(irUnit, bcctx, fp, 0);
      auto const storeVal = Vreg64{vm.makeReg()};
      vm << ldimmq{Immed64{7}, storeVal};
      vm.setOrigin(storeOrigin);
      vm << store{storeVal, base[0]};
      vm.setOrigin(nullptr);
      vm << jmp{exit};
    },
    [] (const PureLoadSinkLinearContext& ctx) {
      expectPureLoadSunkToMid(ctx);

      auto const& midCode = ctx.unit.blocks[ctx.mid].code;
      ASSERT_EQ(midCode.size(), 4);
      ASSERT_EQ(midCode[0].op, Vinstr::load);
      EXPECT_EQ(midCode[0].load_.d, Vreg{ctx.cand});
      ASSERT_EQ(midCode[1].op, Vinstr::ldimmq);
      ASSERT_EQ(midCode[2].op, Vinstr::store);
      ASSERT_EQ(midCode[3].op, Vinstr::jmp);
    }
  );
}

void testSinkDefsPureLoadStopsBeforeEnterInlineFrame() {
  testSinkDefsPureLoadLinear(
    [] (IRUnit& irUnit, BCContext bcctx, SSATmp*) {
      return makeLdStkOrigin(irUnit, bcctx, IRSPRelOffset{0});
    },
    [] (IRUnit& irUnit,
        BCContext bcctx,
        SSATmp*,
        Vout& vm,
        Vreg64,
        Vlabel exit) {
      auto const enterOrigin = makeEnterInlineFrameOrigin(irUnit, bcctx);
      vm.setOrigin(enterOrigin);
      vm << inlinestart{};
      vm.setOrigin(nullptr);
      vm << jmp{exit};
    },
    [] (const PureLoadSinkLinearContext& ctx) {
      expectPureLoadSunkToMid(ctx);

      auto const& midCode = ctx.unit.blocks[ctx.mid].code;
      ASSERT_EQ(midCode.size(), 3);
      ASSERT_EQ(midCode[0].op, Vinstr::load);
      EXPECT_EQ(midCode[0].load_.d, Vreg{ctx.cand});
      ASSERT_EQ(midCode[1].op, Vinstr::inlinestart);
      ASSERT_EQ(midCode[2].op, Vinstr::jmp);
    }
  );
}

void testSinkDefsPureLoadStopsAtSelfLoopClobber() {
  for (auto const kind : {CodeKind::Trace, CodeKind::Prologue}) {
    SCOPED_TRACE(kind == CodeKind::Trace ? "trace_abi" : "prologue_abi");

    IRUnit irUnit{test_context};
    auto const bcctx = BCContext{BCMarker::Dummy(), 0};
    auto const fp = irUnit.gen(DefFP, bcctx, DefFPData{std::nullopt})->dst();
    auto const loadOrigin = makeLdLocOrigin(irUnit, bcctx, fp, 0);
    auto const storeOrigin = makeStLocOrigin(irUnit, bcctx, fp, 0);

    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    auto const loop = unit.makeBlock(AreaIndex::Main, 1);

    Vout v(unit, unit.entry);
    Vout vl(unit, loop);

    auto const base = Vreg64{v.makeReg()};
    auto const cand = Vreg64{v.makeReg()};
    auto const storeVal = Vreg64{vl.makeReg()};
    auto const out = Vreg64{vl.makeReg()};

    v << ldimmq{Immed64{0x100}, base};
    v.setOrigin(loadOrigin);
    v << load{base[0], cand};
    v.setOrigin(nullptr);
    v << jmp{loop};

    vl << ldimmq{Immed64{7}, storeVal};
    vl.setOrigin(storeOrigin);
    vl << store{storeVal, base[0]};
    vl.setOrigin(nullptr);
    vl << copy{cand, out};
    vl << jmp{loop};

    sinkDefs(unit, abi(kind));

    auto const& entryCode = unit.blocks[unit.entry].code;
    ASSERT_EQ(entryCode.size(), 3);
    ASSERT_EQ(entryCode[0].op, Vinstr::ldimmq);
    ASSERT_EQ(entryCode[1].op, Vinstr::load);
    EXPECT_EQ(entryCode[1].load_.d, Vreg{cand});
    ASSERT_EQ(entryCode[2].op, Vinstr::jmp);

    auto const& loopCode = unit.blocks[loop].code;
    ASSERT_EQ(loopCode.size(), 4);
    ASSERT_EQ(loopCode[0].op, Vinstr::ldimmq);
    ASSERT_EQ(loopCode[1].op, Vinstr::store);
    ASSERT_EQ(loopCode[2].op, Vinstr::copy);
    EXPECT_EQ(loopCode[2].copy_.s, Vreg{cand});
    ASSERT_EQ(loopCode[3].op, Vinstr::jmp);
    EXPECT_EQ(loopCode[3].jmp_.target, loop);
  }
}

void testSinkDefsPureLoadStopsAtClobberingBranchPath() {
  for (auto const kind : {CodeKind::Trace, CodeKind::Prologue}) {
    SCOPED_TRACE(kind == CodeKind::Trace ? "trace_abi" : "prologue_abi");

    IRUnit irUnit{test_context};
    auto const bcctx = BCContext{BCMarker::Dummy(), 0};
    auto const fp = irUnit.gen(DefFP, bcctx, DefFPData{std::nullopt})->dst();
    auto const loadOrigin = makeLdLocOrigin(irUnit, bcctx, fp, 0);
    auto const storeOrigin = makeStLocOrigin(irUnit, bcctx, fp, 0);

    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    auto const left = unit.makeBlock(AreaIndex::Main, 1);
    auto const right = unit.makeBlock(AreaIndex::Main, 1);
    auto const merge = unit.makeBlock(AreaIndex::Main, 1);

    Vout v(unit, unit.entry);
    Vout vl(unit, left);
    Vout vr(unit, right);
    Vout vm(unit, merge);

    auto const base = Vreg64{v.makeReg()};
    auto const cand = Vreg64{v.makeReg()};
    auto const sf = v.makeReg();
    auto const storeVal = Vreg64{vl.makeReg()};
    auto const out = Vreg64{vm.makeReg()};

    v << ldimmq{Immed64{0x100}, base};
    v.setOrigin(loadOrigin);
    v << load{base[0], cand};
    v.setOrigin(nullptr);
    v << cmpqi{Immed{0}, Vreg64{Reg64{0}}, sf, Vflags{}};
    v << jcc{CC_E, sf, {left, right}, StringTag{}};

    vl << ldimmq{Immed64{7}, storeVal};
    vl.setOrigin(storeOrigin);
    vl << store{storeVal, base[0]};
    vl.setOrigin(nullptr);
    vl << jmp{merge};

    vr << jmp{merge};

    vm << copy{cand, out};
    vm << ret{};

    sinkDefs(unit, abi(kind));

    auto const& entryCode = unit.blocks[unit.entry].code;
    ASSERT_EQ(entryCode.size(), 4);
    ASSERT_EQ(entryCode[0].op, Vinstr::ldimmq);
    ASSERT_EQ(entryCode[1].op, Vinstr::load);
    EXPECT_EQ(entryCode[1].load_.d, Vreg{cand});
    ASSERT_EQ(entryCode[2].op, Vinstr::cmpqi);
    ASSERT_EQ(entryCode[3].op, Vinstr::jcc);

    auto const& mergeCode = unit.blocks[merge].code;
    ASSERT_EQ(mergeCode.size(), 2);
    ASSERT_EQ(mergeCode[0].op, Vinstr::copy);
    EXPECT_EQ(mergeCode[0].copy_.s, Vreg{cand});
    ASSERT_EQ(mergeCode[1].op, Vinstr::ret);
  }
}

void testSinkDefsPureLoadStopsBeforeClobberingBlockEnd() {
  for (auto const kind : {CodeKind::Trace, CodeKind::Prologue}) {
    SCOPED_TRACE(kind == CodeKind::Trace ? "trace_abi" : "prologue_abi");

    IRUnit irUnit{test_context};
    auto const bcctx = BCContext{BCMarker::Dummy(), 0};
    auto const fp = irUnit.gen(DefFP, bcctx, DefFPData{std::nullopt})->dst();
    auto const loadOrigin = makeLdLocOrigin(irUnit, bcctx, fp, 0);

    Vunit unit;
    unit.entry = unit.makeBlock(AreaIndex::Main, 1);
    auto const mid = unit.makeBlock(AreaIndex::Main, 1);
    auto const done = unit.makeBlock(AreaIndex::Main, 1);
    auto const catchBlock = unit.makeBlock(AreaIndex::Main, 1);

    Vout v(unit, unit.entry);
    Vout vm(unit, mid);
    Vout vd(unit, done);
    Vout vc(unit, catchBlock);

    auto const base = Vreg64{v.makeReg()};
    auto const cand = Vreg64{v.makeReg()};
    auto const out = Vreg64{vd.makeReg()};

    v << ldimmq{Immed64{0x100}, base};
    v.setOrigin(loadOrigin);
    v << load{base[0], cand};
    v.setOrigin(nullptr);
    v << jmp{mid};

    vm << vinvoke{
      CallSpec::direct(sinkDefsTestHelper),
      vm.makeVcallArgs({{}}),
      vm.makeTuple({}),
      {done, catchBlock},
      Fixup::none(),
      DestType::None
    };

    vd << copy{cand, out};
    vd << ret{};

    vc << ret{};

    sinkDefs(unit, abi(kind));

    auto const& entryCode = unit.blocks[unit.entry].code;
    ASSERT_EQ(entryCode.size(), 1);
    ASSERT_EQ(entryCode[0].op, Vinstr::jmp);

    auto const& midCode = unit.blocks[mid].code;
    ASSERT_EQ(midCode.size(), 3);
    ASSERT_EQ(midCode[0].op, Vinstr::ldimmq);
    EXPECT_EQ(midCode[0].ldimmq_.d, base);
    ASSERT_EQ(midCode[1].op, Vinstr::load);
    EXPECT_EQ(midCode[1].load_.d, Vreg{cand});
    ASSERT_EQ(midCode[2].op, Vinstr::vinvoke);

    auto const& doneCode = unit.blocks[done].code;
    ASSERT_EQ(doneCode.size(), 2);
    ASSERT_EQ(doneCode[0].op, Vinstr::copy);
    EXPECT_EQ(doneCode[0].copy_.s, Vreg{cand});
    ASSERT_EQ(doneCode[1].op, Vinstr::ret);
  }
}

}

TEST(Vasm, Simplifier) {
  testSetccXor();
  testPostRACopyFold();
  testSinkDefsMovesIntoMergeAfterPhidef();
  testSinkDefsKeepsJoinPointDefsInPlace();
  testSinkDefsKeepsDefsOutOfHotterBlocks();
  testSinkDefsFallsBackToLegalDominator();
  testSinkDefsMovesDefsWithDeadSF();
  testSinkDefsPureLoadSinksAcrossUnrelatedStoreOrigin();
  testSinkDefsPureLoadSinksAfterUserMoves();
  testSinkDefsPureLoadStopsBeforeClobberingStore();
  testSinkDefsPureLoadStopsBeforeEnterInlineFrame();
  testSinkDefsPureLoadStopsAtSelfLoopClobber();
  testSinkDefsPureLoadStopsAtClobberingBranchPath();
  testSinkDefsPureLoadStopsBeforeClobberingBlockEnd();
}

}
