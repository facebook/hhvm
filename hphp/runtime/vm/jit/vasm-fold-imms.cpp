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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/vixl/a64/assembler-a64.h"

#include <boost/dynamic_bitset.hpp>

TRACE_SET_MOD(hhir);

namespace HPHP { namespace jit {

namespace x64 {
struct ImmFolder {
  jit::vector<bool> used;
  jit::vector<uint64_t> vals;
  boost::dynamic_bitset<> valid;

  explicit ImmFolder(jit::vector<bool>&& used_in)
  : used(std::move(used_in)) { }

  // helpers
  bool match_byte(Vreg r, int& val) {
    if (!valid.test(r)) return false;
    auto imm64 = vals[r];
    if (!deltaFits(imm64, sz::byte)) return false;
    val = imm64;
    return true;
  }
  bool match_int(Vreg r, int& val) {
    if (!valid.test(r)) return false;
    auto imm64 = vals[r];
    if (!deltaFits(imm64, sz::dword)) return false;
    val = imm64;
    return true;
  }
  // folders
  template<class Inst> void fold(Inst&, Vinstr& out) {}
  void fold(addq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) {
      if (val == 0 && !used[in.sf]) { // nop sets no flags.
        out = copy{in.s1, in.d};
      } else if (val == 1 && !used[in.sf]) { // CF not set by inc.
        out = incq{in.s1, in.d, in.sf};
      } else {
        out = addqi{val, in.s1, in.d, in.sf};
      }
    } else if (match_int(in.s1, val)) {
      if (val == 0 && !used[in.sf]) { // nop sets no flags.
        out = copy{in.s0, in.d};
      } else if (val == 1 && !used[in.sf]) { // CF not set by inc.
        out = incq{in.s0, in.d, in.sf};
      } else {
        out = addqi{val, in.s0, in.d, in.sf};
      }
    }
  }
  void fold(andq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = andqi{val, in.s1, in.d, in.sf}; }
    else if (match_int(in.s1, val)) { out = andqi{val, in.s0, in.d, in.sf}; }
  }
  void fold(cmpb& in, Vinstr& out) {
    int val;
    if (match_byte(in.s0, val)) { out = cmpbi{val, in.s1, in.sf}; }
  }
  void fold(cmpq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = cmpqi{val, in.s1, in.sf}; }
  }
  void fold(cmpqm& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = cmpqim{val, in.s1, in.sf}; }
  }
  void fold(cmplm& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = cmplim{val, in.s1, in.sf}; }
  }
  void fold(orq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = orqi{val, in.s1, in.d, in.sf}; }
    else if (match_int(in.s1, val)) { out = orqi{val, in.s0, in.d, in.sf}; }
  }
  void fold(storeb& in, Vinstr& out) {
    int val;
    if (out.origin && out.origin->marker().sk().prologue()) return;
    if (match_byte(in.s, val)) { out = storebi{val, in.m}; }
  }
  void fold(storel& in, Vinstr& out) {
    int val;
    if (match_int(in.s, val)) { out = storeli{val, in.m}; }
  }
  void fold(store& in, Vinstr& out) {
    int val;
    if (match_int(in.s, val)) { out = storeqi{val, in.d}; }
  }
  void fold(subq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) {
      if (val == 0 && !used[in.sf]) { // copy sets no flags.
        out = copy{in.s1, in.d};
      } else if (val == 1 && !used[in.sf]) { // CF not set by dec.
        out = decq{in.s1, in.d, in.sf};
      } else {
        out = subqi{val, in.s1, in.d, in.sf};
      }
    } else if (match_int(in.s1, val)) {
      if (val == 0) out = neg{in.s0, in.d, in.sf};
    }
  }
  void fold(subqi& in, Vinstr& out) {
    if (in.s0.l() == 0 && !used[in.sf]) {  // copy sets no flags.
      out = copy{in.s1, in.d};
    }
  }
  // xor clears CF, OF.  ZF, SF, PF set accordingly
  void fold(xorb& in, Vinstr& out) {
    int val;
    if (match_byte(in.s0, val)) {
      if (val == 0 && !used[in.sf]) { // copy doesn't set any flags.
        out = copy{in.s1, in.d};
      } else if (val == -1 && !used[in.sf]) { // not doesn't set any flags.
        out = notb{in.s1, in.d};
      } else {
        out = xorbi{val, in.s1, in.d, in.sf};
      }
    } else if (match_byte(in.s1, val)) {
      if (val == 0 && !used[in.sf]) { // copy doesn't set any flags.
        out = copy{in.s0, in.d};
      } else if (val == -1 && !used[in.sf]) { // not doesn't set any flags.
        out = notb{in.s0, in.d};
      } else {
        out = xorbi{val, in.s0, in.d, in.sf};
      }
    }
  }
  void fold(xorq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) {
      if (val == 0 && !used[in.sf]) { // copy doesn't set any flags
        out = copy{in.s1, in.d};
      } else if (val == -1 && !used[in.sf]) { // not doesn't set any flags
        out = not{in.s1, in.d};
      } else {
        out = xorqi{val, in.s1, in.d, in.sf};
      }
    } else if (match_int(in.s1, val)) {
      if (val == 0 && !used[in.sf]) { // copy doesn't set any flags
        out = copy{in.s0, in.d};
      } else if (val == -1 && !used[in.sf]) { // not doesn't set any flags
        out = not{in.s0, in.d};
      } else {
        out = xorqi{val, in.s0, in.d, in.sf};
      }
    }
  }
  void fold(movzbl& in, Vinstr& out) {
    int val;
    if (match_byte(in.s, val)) {
      out = copy{in.s, in.d};
      valid.set(in.d);
      vals[in.d] = vals[in.s];
    }
  }
  void fold(load& in, Vinstr& out) {
    int val;
    if (in.s.index.isValid() && in.s.scale == 1 && match_int(in.s.index, val) &&
        deltaFits(int64_t(in.s.disp) + val, sz::dword)) {
      // index is const: [base+disp+index*1] => [base+(disp+index)]
      in.s.index = Vreg{};
      in.s.disp += val;
    }
  }
};
} // namespace x64

namespace arm {
struct ImmFolder {
  jit::vector<uint64_t> vals;
  boost::dynamic_bitset<> valid;

  explicit ImmFolder(jit::vector<bool>&&) {}

  bool arith_imm(Vreg r, int32_t& out) {
    if (!valid.test(r)) return false;
    auto imm64 = vals[r];
    if (!vixl::Assembler::IsImmArithmetic(imm64)) return false;
    out = safe_cast<int32_t>(imm64);
    return true;
  }
  bool logical_imm(Vreg r, int32_t& out) {
    if (!valid.test(r)) return false;
    auto imm64 = vals[r];
    if (!vixl::Assembler::IsImmLogical(imm64, vixl::kXRegSize)) return false;
    if (!deltaFits(imm64, sz::word)) return false;
    out = safe_cast<int32_t>(imm64);
    return true;
  }
  bool zero_imm(Vreg r) {
    if (!valid.test(r)) return false;
    return vals[r] == 0;
  }

  template<typename Inst>
  void fold(Inst& i, Vinstr& out) {}

  void fold(addq& in, Vinstr& out) {
    int val;
    if (arith_imm(in.s0, val)) { out = addqi{val, in.s1, in.d, in.sf}; }
    else if (arith_imm(in.s1, val)) { out = addqi{val, in.s0, in.d, in.sf}; }
  }
  void fold(andq& in, Vinstr& out) {
    int val;
    if (logical_imm(in.s0, val)) { out = andqi{val, in.s1, in.d, in.sf}; }
    else if (logical_imm(in.s1, val)) { out = andqi{val, in.s0, in.d, in.sf}; }
  }
  void fold(cmpl& in, Vinstr& out) {
    int val;
    if (arith_imm(in.s0, val)) { out = cmpli{val, in.s1, in.sf}; }
  }
  void fold(cmpq& in, Vinstr& out) {
    int val;
    if (arith_imm(in.s0, val)) { out = cmpqi{val, in.s1, in.sf}; }
  }
  void fold(orq& in, Vinstr& out) {
    int val;
    if (logical_imm(in.s0, val)) { out = orqi{val, in.s1, in.d, in.sf}; }
    else if (logical_imm(in.s1, val)) { out = orqi{val, in.s0, in.d, in.sf}; }
  }
  void fold(store& in, Vinstr& out) {
    if (zero_imm(in.s)) out = store{PhysReg(vixl::xzr), in.d};
  }
  void fold(storeqi& in, Vinstr& out) {
    if (in.s.q() == 0) out = store{PhysReg(vixl::xzr), in.m};
  }
  void fold(storel& in, Vinstr& out) {
    if (zero_imm(in.s)) out = storel{PhysReg(vixl::wzr), in.m};
  }
  void fold(storeli& in, Vinstr& out) {
    if (in.s.l() == 0) out = storel{PhysReg(vixl::wzr), in.m};
  }
  void fold(subq& in, Vinstr& out) {
    int val;
    if (arith_imm(in.s0, val)) { out = subqi{val, in.s1, in.d, in.sf}; }
  }
  void fold(xorq& in, Vinstr& out) {
    int val;
    if (logical_imm(in.s0, val)) { out = xorqi{val, in.s1, in.d, in.sf}; }
    else if (logical_imm(in.s1, val)) { out = xorqi{val, in.s0, in.d, in.sf}; }
  }
};
}

// Immediate-folding. If an instruction takes a register operand defined
// as a constant, and there is valid immediate-form of that instruction,
// then change the instruction and embed the immediate.
template<typename Folder>
void foldImms(Vunit& unit) {
  assertx(check(unit)); // especially, SSA
  // block order doesn't matter, but only visit reachable blocks.
  auto blocks = sortBlocks(unit);

  // Use flag for each registers.  If a SR is used then
  // certain optimizations will not fire since they do not
  // set the condition codes as the original instruction(s)
  // would.
  jit::vector<bool> used(unit.next_vr);
  for (auto b : blocks) {
    for (auto& inst : unit.blocks[b].code) {
      visitUses(unit, inst, [&](Vreg r) { used[r] = true; });
    }
  }

  Folder folder(std::move(used));
  folder.vals.resize(unit.next_vr);
  folder.valid.resize(unit.next_vr);
  // figure out which Vregs are constants and stash their values.
  for (auto& entry : unit.constToReg) {
    folder.valid.set(entry.second);
    folder.vals[entry.second] = entry.first.val;
  }
  // now mutate instructions
  for (auto b : blocks) {
    for (auto& inst : unit.blocks[b].code) {
      switch (inst.op) {
#define O(name, imms, uses, defs)\
        case Vinstr::name: {\
          auto origin = inst.origin;\
          folder.fold(inst.name##_, inst);\
          inst.origin = origin;\
          break;\
        }
        VASM_OPCODES
#undef O
      }
    }
  }
  printUnit(kVasmImmsLevel, "after foldImms", unit);
}

template void foldImms<x64::ImmFolder>(Vunit& unit);
template void foldImms<arm::ImmFolder>(Vunit& unit);

}}
