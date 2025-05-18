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

#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include <algorithm>

namespace HPHP::jit {
///////////////////////////////////////////////////////////////////////////////

constexpr int Vframe::Top;
constexpr int Vframe::Root;

int Vunit::allocFrame(const Vinstr& inst, int parent_frame,
                      uint64_t entry_weight) {
  assertx(inst.op == Vinstr::inlinestart);
  assertx(inst.origin->is(EnterInlineFrame));

  auto const fp = inst.origin->src(0);
  auto const [it, inserted] = fpToFrame.emplace(fp, frames.size());
  if (!inserted) {
    always_assert(frames[it->second].parent == parent_frame);
    frames[it->second].entry_weight += entry_weight;
    return it->second;
  }

  for (auto f = parent_frame; f != Vframe::Top; f = frames[f].parent) {
    frames[f].inclusive_cost += inst.inlinestart_.cost;
    frames[f].num_inner_frames++;
  }

  auto const sbToRootSbOff =
    frames[parent_frame].sbToRootSbOff +
    inst.origin->marker().bcSPOff().offset +
    inst.inlinestart_.func->numSlotsInFrame();

  frames.emplace_back(
    inst.inlinestart_.func,
    inst.origin->marker().bcOff(),
    sbToRootSbOff,
    parent_frame,
    inst.inlinestart_.cost,
    entry_weight
  );

  return it->second;
}

Vlabel Vunit::makeBlock(AreaIndex area, uint64_t weight) {
  auto i = blocks.size();
  blocks.emplace_back(area, weight);
  return Vlabel{i};
}

Vlabel Vunit::makeBlock(AreaIndex area) {
  return makeBlock(area, areaWeightFactor(area));
}

Vlabel Vunit::makeScratchBlock() {
  return makeBlock(AreaIndex::Main, 1);
}

void Vunit::freeScratchBlock(Vlabel l) {
  // This will leak blocks if anything's been added since the corresponding
  // call to makeScratchBlock(), but it's harmless.
  if (l == blocks.size() - 1) blocks.pop_back();
}

Vtuple Vunit::makeTuple(VregList&& regs) {
  auto i = tuples.size();
  tuples.emplace_back(std::move(regs));
  return Vtuple{i};
}

Vtuple Vunit::makeTuple(const VregList& regs) {
  auto i = tuples.size();
  tuples.emplace_back(regs);
  return Vtuple{i};
}

VcallArgsId Vunit::makeVcallArgs(VcallArgs&& args) {
  VcallArgsId i(vcallArgs.size());
  vcallArgs.emplace_back(std::move(args));
  return i;
}

Vreg Vunit::makeConst(Vconst vconst) {
  auto it = constToReg.find(vconst);
  if (it != constToReg.end()) return it->second;

  auto const reg = makeReg();
  constToReg.emplace(vconst, reg);
  regToConst.emplace(reg, vconst);
  return reg;
}

bool Vunit::needsRegAlloc() const {
  if (next_vr > Vreg::V0) return true;

  for (auto& block : blocks) {
    for (auto& inst : block.code) {
      if (inst.op == Vinstr::copyargs) return true;
    }
  }

  return false;
}

bool Vunit::needsFramesComputed() const {
  return frames.empty();
}

///////////////////////////////////////////////////////////////////////////////
}
