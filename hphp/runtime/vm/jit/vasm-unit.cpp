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

#include "hphp/runtime/vm/jit/vasm-unit.h"

#include <algorithm>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

Vlabel Vunit::makeBlock(AreaIndex area) {
  auto i = blocks.size();
  blocks.emplace_back(area);
  return Vlabel{i};
}

Vlabel Vunit::makeScratchBlock() {
  return makeBlock(AreaIndex::Main);
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

// helper for making constants, where T maps to the correct overloaded
// Vconst constructor.
template<class T> Vreg make_const(Vunit& unit, T c) {
  auto const vconst = Vconst{c};
  auto it = unit.constToReg.find(vconst);
  if (it != unit.constToReg.end()) return it->second;

  auto const reg = unit.makeReg();
  unit.constToReg.emplace(vconst, reg);
  unit.regToConst.emplace(reg, vconst);
  return reg;
}

Vreg Vunit::makeConst(bool v)     { return make_const(*this, v); }
Vreg Vunit::makeConst(uint32_t v) { return make_const(*this, v); }
Vreg Vunit::makeConst(uint64_t v) { return make_const(*this, v); }
Vreg Vunit::makeConst(Vptr v)     { return make_const(*this, v); }
Vreg Vunit::makeConst(double v)   { return make_const(*this, v); }
Vreg Vunit::makeConst(Vconst::Kind k) { return make_const(*this, k); }

bool Vunit::needsRegAlloc() const {
  if (next_vr > Vreg::V0) return true;

  for (auto& block : blocks) {
    for (auto& inst : block.code) {
      if (inst.op == Vinstr::copyargs) return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
