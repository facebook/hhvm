/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/vasm-util.h"

#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

bool is_nop(const copy& i) { return i.s == i.d; }
bool is_nop(const copy2& i) { return i.s0 == i.d0 && i.s1 == i.d1; }

// movb r,r is a nop, however movl is not since it zeros upper bits.
bool is_nop(const movb& i) { return i.s == i.d; }

bool is_nop(const lea& i) {
  if (i.s.disp != 0) return false;
  return
    (i.s.base == i.d && !i.s.index.isValid()) ||
    (!i.s.base.isValid() && i.s.index == i.d && i.s.scale == 1 &&
      i.s.seg == Vptr::DS);
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

bool is_trivial_nop(const Vinstr& inst) {
  return
    (inst.op == Vinstr::copy && is_nop(inst.copy_)) ||
    (inst.op == Vinstr::copy2 && is_nop(inst.copy2_)) ||
    (inst.op == Vinstr::lea && is_nop(inst.lea_)) ||
    (inst.op == Vinstr::movb && is_nop(inst.movb_)) ||
    inst.op == Vinstr::nop;
}

//////////////////////////////////////////////////////////////////////

namespace {

/*
 * Add a jump from middleLabel to destLabel, taking into account any special
 * instructions at the beginning of destLabel.
 */
void forwardJmp(Vunit& unit, jit::flat_set<size_t>& catch_blocks,
                Vlabel middleLabel, Vlabel destLabel) {
  auto& middle = unit.blocks[middleLabel];
  auto& dest = unit.blocks[destLabel];

  auto& headInst = dest.code.front();
  auto origin = headInst.origin;
  if (headInst.op == Vinstr::phidef) {
    // We need to preserve any phidefs in the forwarding block if they're
    // present in the original destination block.
    auto tuple = headInst.phidef_.defs;
    auto forwardedRegs = unit.tuples[tuple];
    VregList regs(forwardedRegs.size());
    for (unsigned i = 0; i < forwardedRegs.size(); ++i) {
      regs[i] = unit.makeReg();
    }
    auto newTuple = unit.makeTuple(regs);
    middle.code.emplace_back(phidef{newTuple});
    middle.code.back().origin = origin;
    middle.code.emplace_back(phijmp{destLabel, newTuple});
    middle.code.back().origin = origin;
    return;
  } else if (headInst.op == Vinstr::landingpad) {
    // If the dest started with a landingpad, copy it to middle. The dest's
    // will be erased at the end of the pass.
    catch_blocks.insert(destLabel);
    assertx(middle.code.empty());
    middle.code.emplace_back(headInst);
  }

  middle.code.emplace_back(jmp{destLabel});
  middle.code.back().origin = origin;
}

}

/*
 * Splits the critical edges in `unit', if any.
 * Returns true iff the unit was modified.
 */
bool splitCriticalEdges(Vunit& unit) {
  jit::vector<unsigned> preds(unit.blocks.size());
  jit::flat_set<size_t> catch_blocks;

  for (size_t b = 0; b < unit.blocks.size(); b++) {
    auto succlist = succs(unit.blocks[b]);
    for (auto succ : succlist) {
      preds[succ]++;
    }
  }

  auto changed = false;
  for (size_t pred = 0; pred < unit.blocks.size(); pred++) {
    auto succlist = succs(unit.blocks[pred]);
    if (succlist.size() <= 1) continue;
    for (auto& succ : succlist) {
      if (preds[succ] <= 1) continue;
      // split the critical edge.
      auto middle = unit.makeBlock(unit.blocks[succ].area);
      forwardJmp(unit, catch_blocks, middle, succ);
      succ = middle;
      changed = true;
    }
  }

  // Remove any landingpad{} instructions that were hoisted to split edges.
  for (auto block : catch_blocks) {
    auto& code = unit.blocks[block].code;
    assertx(code.front().op == Vinstr::landingpad);
    code.front() = nop{};
  }

  return changed;
}

Vreg make_const(Vunit& unit, Type type) {
  if (type.subtypeOfAny(TUninit, TInitNull)) {
    // Return undefined value.
    return unit.makeConst(Vconst::Quad);
  }
  if (type <= TNullptr) return unit.makeConst(0);

  assertx(type.hasConstVal());
  if (type <= TBool) return unit.makeConst(type.boolVal());
  if (type <= TDbl) return unit.makeConst(type.dblVal());
  return unit.makeConst(type.rawVal());
}

//////////////////////////////////////////////////////////////////////

}}
