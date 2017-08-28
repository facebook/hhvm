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
  Vunit& unit;
  jit::vector<uint8_t> uses;
  jit::vector<uint64_t> vals;
  boost::dynamic_bitset<> valid;

  explicit ImmFolder(Vunit& u, jit::vector<uint8_t>&& uses_in)
      : unit(u), uses(std::move(uses_in)) { }

  template <typename T>
  static uint64_t mask(T r) {
    auto bytes = static_cast<int>(width(r));
    assertx(bytes && !(bytes & (bytes - 1)) && bytes <= 8);
    return bytes == 8 ? -1uLL : (1uLL << (bytes * 8)) - 1;
  }

  template <typename T>
  void extend_truncate_impl(T& in, Vinstr& out) {
    if (!valid.test(in.s)) return;
    auto const val = vals[in.s] & mask(in.s);
    // For the extend cases, we're done, and val is the value of the
    // constant that we want.
    // For the truncate cases, we don't actually care about the upper
    // bits of the register, but if we choose a signed value, sized to
    // the destination there's more chance that it will be folded with
    // a subsequent operation.
    auto const m = mask(in.d);
    auto const m1 = m >> 1;
    auto const top = m ^ m1;
    auto dval = int64_t(val & m1) - int64_t(val & top);
    out = copy{unit.makeConst(dval), in.d};
    if (in.d.isVirt()) {
      valid.set(in.d);
      vals[in.d] = dval;
    }
  }

  // helpers
  bool match_byte(Vreg r, int& val) {
    if (!valid.test(r)) return false;
    auto imm64 = vals[r];
    if (!deltaFits(imm64, sz::byte)) return false;
    val = imm64;
    return true;
  }
  bool match_word(Vreg r, int& val) {
    if (!valid.test(r)) return false;
    auto imm64 = vals[r];
    if (!deltaFits(imm64, sz::word)) return false;
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
  bool match_uint(Vreg r, int& val) {
    if (!valid.test(r)) return false;
    auto imm64 = vals[r];
    if (!magFits(imm64, sz::dword)) return false;
    val = imm64;
    return true;
  }
  // folders
  template <class Inst>
  void fold(Inst&, Vinstr& /*out*/) {}
  void fold(addq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) {
      if (val == 0 && !uses[in.sf]) { // nop sets no flags.
        out = copy{in.s1, in.d};
      } else if (val == 1 && !uses[in.sf]) { // CF not set by inc.
        out = incq{in.s1, in.d, in.sf};
      } else {
        out = addqi{val, in.s1, in.d, in.sf};
      }
    } else if (match_int(in.s1, val)) {
      if (val == 0 && !uses[in.sf]) { // nop sets no flags.
        out = copy{in.s0, in.d};
      } else if (val == 1 && !uses[in.sf]) { // CF not set by inc.
        out = incq{in.s0, in.d, in.sf};
      } else {
        out = addqi{val, in.s0, in.d, in.sf};
      }
    }
  }
  void fold(andq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) {
      out = andqi{val, in.s1, in.d, in.sf};
    } else if (match_int(in.s1, val)) {
      out = andqi{val, in.s0, in.d, in.sf};
    } else {
      auto rep = [&](Vreg64 s) {
        if (val == -1 && !uses[in.sf]) {
          out = movzlq{Reg32(s), in.d};
        } else {
          out = andli{val, Reg32(s), Reg32(in.d), in.sf};
        }
      };
      if (match_uint(in.s0, val)) {
        rep(in.s1);
      } else if (match_uint(in.s1, val)) {
        rep(in.s0);
      }
    }
  }
  void fold(testq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) {out = testqi{val, in.s1, in.sf};}
    else if (match_int(in.s1, val)) {out = testqi{val, in.s0, in.sf};}
    else if (match_uint(in.s0, val)) {out = testli{val, Reg32(in.s1), in.sf};}
    else if (match_uint(in.s1, val)) {out = testli{val, Reg32(in.s0), in.sf};}
  }
  void fold(cmpb& in, Vinstr& out) {
    int val;
    if (match_byte(in.s0, val)) { out = cmpbi{val, in.s1, in.sf}; }
  }
  void fold(cmpbm& in, Vinstr& out) {
    int val;
    if (match_byte(in.s0, val)) { out = cmpbim{val, in.s1, in.sf}; }
  }
  void fold(cmpw& in, Vinstr& out) {
    int val;
    if (match_word(in.s0, val)) { out = cmpwi{val, in.s1, in.sf}; }
  }
  void fold(cmpwm& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = cmpwim{val, in.s1, in.sf}; }
  }
  void fold(cmpq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = cmpqi{val, in.s1, in.sf}; }
  }
  void fold(cmpl& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = cmpli{val, in.s1, in.sf}; }
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
    foldVptr(in.m);
    if (out.origin && out.origin->marker().sk().prologue() && uses[in.s] > 1) {
      return;
    }
    int val;
    if (match_byte(in.s, val)) { out = storebi{val, in.m}; }
  }
  void fold(storebi& in, Vinstr& /*out*/) { foldVptr(in.m); }
  void fold(storew& in, Vinstr& out) {
    foldVptr(in.m);
    int val;
    if (match_word(in.s, val)) { out = storewi{val, in.m}; }
  }
  void fold(storewi& in, Vinstr& /*out*/) { foldVptr(in.m); }
  void fold(storel& in, Vinstr& out) {
    foldVptr(in.m);
    int val;
    if (match_int(in.s, val)) { out = storeli{val, in.m}; }
  }
  void fold(storeli& in, Vinstr& /*out*/) { foldVptr(in.m); }
  void fold(store& in, Vinstr& out) {
    foldVptr(in.d);
    int val;
    if (match_int(in.s, val)) { out = storeqi{val, in.d}; }
  }
  void fold(storeqi& in, Vinstr& /*out*/) { foldVptr(in.m); }
  void fold(subq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) {
      if (val == 0 && !uses[in.sf]) { // copy sets no flags.
        out = copy{in.s1, in.d};
      } else if (val == 1 && !uses[in.sf]) { // CF not set by dec.
        out = decq{in.s1, in.d, in.sf};
      } else {
        out = subqi{val, in.s1, in.d, in.sf};
      }
    } else if (match_int(in.s1, val)) {
      if (val == 0) out = neg{in.s0, in.d, in.sf};
    }
  }
  void fold(subqi& in, Vinstr& out) {
    if (in.s0.l() == 0 && !uses[in.sf]) {  // copy sets no flags.
      out = copy{in.s1, in.d};
    }
  }
  // xor clears CF, OF.  ZF, SF, PF set accordingly
  void fold(xorb& in, Vinstr& out) {
    int val;
    if (match_byte(in.s0, val)) {
      if (val == 0 && !uses[in.sf]) { // copy doesn't set any flags.
        out = copy{in.s1, in.d};
      } else if (val == -1 && !uses[in.sf]) { // not doesn't set any flags.
        out = notb{in.s1, in.d};
      } else {
        out = xorbi{val, in.s1, in.d, in.sf};
      }
    } else if (match_byte(in.s1, val)) {
      if (val == 0 && !uses[in.sf]) { // copy doesn't set any flags.
        out = copy{in.s0, in.d};
      } else if (val == -1 && !uses[in.sf]) { // not doesn't set any flags.
        out = notb{in.s0, in.d};
      } else {
        out = xorbi{val, in.s0, in.d, in.sf};
      }
    }
  }
  void fold(xorq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) {
      if (val == 0 && !uses[in.sf]) { // copy doesn't set any flags
        out = copy{in.s1, in.d};
      } else if (val == -1 && !uses[in.sf]) { // not doesn't set any flags
        out = not{in.s1, in.d};
      } else {
        out = xorqi{val, in.s1, in.d, in.sf};
      }
    } else if (match_int(in.s1, val)) {
      if (val == 0 && !uses[in.sf]) { // copy doesn't set any flags
        out = copy{in.s0, in.d};
      } else if (val == -1 && !uses[in.sf]) { // not doesn't set any flags
        out = not{in.s0, in.d};
      } else {
        out = xorqi{val, in.s0, in.d, in.sf};
      }
    }
  }
  void fold(movzbw& in, Vinstr& out) {
    extend_truncate_impl(in, out);
  }
  void fold(movzbl& in, Vinstr& out) {
    extend_truncate_impl(in, out);
  }
  void fold(movzbq& in, Vinstr& out) {
    extend_truncate_impl(in, out);
  }
  void fold(movzlq& in, Vinstr& out) {
    extend_truncate_impl(in, out);
  }
  void fold(movzwl& in, Vinstr& out) {
    extend_truncate_impl(in, out);
  }
  void fold(movzwq& in, Vinstr& out) {
    extend_truncate_impl(in, out);
  }
  void fold(movtql& in, Vinstr& out) {
    extend_truncate_impl(in, out);
  }
  void fold(movtqw& in, Vinstr& out) {
    extend_truncate_impl(in, out);
  }
  void fold(movtqb& in, Vinstr& out) {
    extend_truncate_impl(in, out);
  }
  void fold(copy& in, Vinstr& /*out*/) {
    if (in.d.isVirt() && valid.test(in.s)) {
      valid.set(in.d);
      vals[in.d] = vals[in.s];
    }
  }
  void foldVptr(Vptr& mem) {
    int val;
    if (mem.index.isValid() && match_int(mem.index, val)) {
      mem.validate();
      if (deltaFits(mem.disp + int64_t(val) * mem.scale, sz::dword)) {
        // index is const: [base+disp+index*scale] => [base+(disp+index)]
        mem.index = Vreg{};
        mem.disp += int64_t(val) * mem.scale;
      }
    }
    if (mem.base.isValid() && match_int(mem.base, val)) {
      mem.validate();
      if (deltaFits(mem.disp + int64_t(val), sz::dword)) {
        mem.base = Vreg{};
        mem.disp += val;
      }
    }
  }
  void fold(load& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadb& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadw& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadl& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadups& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadsd& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadzbl& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadzlq& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadtqb& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadtql& in, Vinstr& /*out*/) { foldVptr(in.s); }
};
} // namespace x64

namespace arm {
struct ImmFolder {
  jit::vector<uint8_t> uses;
  jit::vector<uint64_t> vals;
  boost::dynamic_bitset<> valid;

  explicit ImmFolder(Vunit& /*unit*/, jit::vector<uint8_t>&& uses_in)
      : uses(std::move(uses_in)) {}

  bool arith_imm(Vreg r, int32_t& out) {
    if (!valid.test(r)) return false;
    auto imm64 = vals[r];
    if (!vixl::Assembler::IsImmArithmetic(imm64)) return false;
    out = imm64;
    return true;
  }

  bool logical_imm(Vreg r, int32_t& out) {
    if (!valid.test(r)) return false;
    auto imm64 = vals[r];
    if (!vixl::Assembler::IsImmLogical(imm64, vixl::kXRegSize)) return false;
    if (!deltaFits(imm64, sz::word)) return false;
    out = imm64;
    return true;
  }

  bool logical_bmsk(Vreg r, uint64_t& out) {
    if (!valid.test(r)) return false;
    auto imm64 = vals[r];
    if (!vixl::Assembler::IsImmLogical(imm64, vixl::kXRegSize)) return false;
    out = imm64;
    return true;
  }

  bool zero_imm(Vreg r) {
    if (!valid.test(r)) return false;
    return vals[r] == 0;
  }

  template<typename arithi, typename arith>
  void fold_arith(arith& in, Vinstr& out) {
    int val;
    if (arith_imm(in.s0, val)) {
      if (val == 0 && !uses[in.sf]) {
        out = copy{in.s1,in.d};
      } else {
        out = arithi{val, in.s1, in.d, in.sf};
      }
    } else if (arith_imm(in.s1, val)) {
      if (val == 0 && !uses[in.sf]) {
        out = copy{in.s0, in.d};
      } else {
        out = arithi{val, in.s0, in.d, in.sf};
      }
    }
  }

  template<typename logicali, typename logical>
  void fold_logical(logical& in, Vinstr& out) {
    int val;
    if (logical_imm(in.s0, val)) { out = logicali{val, in.s1, in.d, in.sf}; }
    else if (logical_imm(in.s1, val)) { out = logicali{val, in.s0, in.d, in.sf}; }
  }

  template<typename testi, typename test>
  void fold_test(test& in, Vinstr& out) {
    int val;
    if (logical_imm(in.s0, val)) { out = testi{val, in.s1, in.sf}; }
    else if (logical_imm(in.s1, val)) { out = testi{val, in.s0, in.sf}; }
  }

  template<typename cmpi, typename cmp>
  void fold_cmp(cmp& in, Vinstr& out) {
    int val;
    if (arith_imm(in.s0, val)) { out = cmpi{val, in.s1, in.sf}; }
  }

  template <typename Inst>
  void fold(Inst& /*i*/, Vinstr& /*out*/) {}
  void fold(addl& in, Vinstr& out) { return fold_arith<addli>(in, out); }
  void fold(addq& in, Vinstr& out) { return fold_arith<addqi>(in, out); }
  void fold(andb& in, Vinstr& out) { return fold_logical<andbi>(in, out); }
  void fold(andl& in, Vinstr& out) { return fold_logical<andli>(in, out); }
  void fold(orq& in, Vinstr& out) { return fold_logical<orqi>(in, out); }

  void fold(testb& in, Vinstr& out) { return fold_test<testbi>(in, out); }
  void fold(testw& in, Vinstr& out) { return fold_test<testwi>(in, out); }
  void fold(testl& in, Vinstr& out) { return fold_test<testli>(in, out); }
  void fold(testq& in, Vinstr& out) { return fold_test<testqi>(in, out); }
  void fold(cmpb& in, Vinstr& out) { return fold_cmp<cmpbi>(in, out); }
  void fold(cmpw& in, Vinstr& out) { return fold_cmp<cmpwi>(in, out); }
  void fold(cmpl& in, Vinstr& out) { return fold_cmp<cmpli>(in, out); }
  void fold(cmpq& in, Vinstr& out) { return fold_cmp<cmpqi>(in, out); }

  void fold(andq& in, Vinstr& out) {
    if (!uses[in.sf] && valid.test(in.s0) && valid.test(in.s1)) {
      out = ldimmq{vals[in.s0] & vals[in.s1], in.d};
      return;
    }
    int val;
    uint64_t bm;
    if (logical_imm(in.s0, val)) { out = andqi{val, in.s1, in.d, in.sf}; }
    else if (logical_imm(in.s1, val)) { out = andqi{val, in.s0, in.d, in.sf}; }
    else if (logical_bmsk(in.s0, bm)) { out = andqi64{bm, in.s1, in.d, in.sf}; }
    else if (logical_bmsk(in.s1, bm)) { out = andqi64{bm, in.s0, in.d, in.sf}; }
  }

  void fold(andqi& in, Vinstr& out) {
    if (uses[in.sf]) return;
    if (!valid.test(in.s1)) return;
    out = ldimmq{vals[in.s1] & in.s0.q(), in.d};
  }

  void fold(storeb& in, Vinstr& out) {
    if (zero_imm(in.s)) out = storeb{PhysReg(vixl::wzr), in.m};
  }
  void fold(storebi& in, Vinstr& out) {
    if (in.s.l() == 0) out = storeb{PhysReg(vixl::wzr), in.m};
  }
  void fold(storew& in, Vinstr& out) {
    if (zero_imm(in.s)) out = storew{PhysReg(vixl::wzr), in.m};
  }
  void fold(storewi& in, Vinstr& out) {
    if (in.s.l() == 0) out = storew{PhysReg(vixl::wzr), in.m};
  }
  void fold(storel& in, Vinstr& out) {
    if (zero_imm(in.s)) out = storel{PhysReg(vixl::wzr), in.m};
  }
  void fold(storeli& in, Vinstr& out) {
    if (in.s.l() == 0) out = storel{PhysReg(vixl::wzr), in.m};
  }
  void fold(store& in, Vinstr& out) {
    if (zero_imm(in.s)) out = store{PhysReg(vixl::xzr), in.d};
  }
  void fold(storeqi& in, Vinstr& out) {
    if (in.s.q() == 0) out = store{PhysReg(vixl::xzr), in.m};
  }
  void fold(subl& in, Vinstr& out) {
    int val;
    if (arith_imm(in.s0, val)) {
      if (val == 0 && !uses[in.sf]) {
        out = copy{in.s1, in.d};
      } else {
        out = subli{val, in.s1, in.d, in.sf};
      }
    }
  }
  void fold(subli& in, Vinstr& out) {
    if (in.s0.l() == 0 && !uses[in.sf]) {  // copy sets no flags.
      out = copy{in.s1, in.d};
    }
  }
  void fold(subq& in, Vinstr& out) {
    int val;
    if (arith_imm(in.s0, val)) {
      if (val == 0 && !uses[in.sf]) {
        out = copy{in.s1, in.d};
      } else {
        out = subqi{val, in.s1, in.d, in.sf};
      }
    }
  }
  void fold(subqi& in, Vinstr& out) {
    if (in.s0.l() == 0 && !uses[in.sf]) {  // copy sets no flags.
      out = copy{in.s1, in.d};
    }
  }
  void fold(xorb& in, Vinstr& out) {
    int val;
    if (logical_imm(in.s0, val)) {
      if (val == 0 && !uses[in.sf]) {
        out = copy{in.s1, in.d};
      } else {
        out = xorbi{val, in.s1, in.d, in.sf};
      }
    } else if (logical_imm(in.s1, val)) {
      if (val == 0 && !uses[in.sf]) {
        out = copy{in.s0, in.d};
      } else {
        out = xorbi{val, in.s0, in.d, in.sf};
      }
    }
  }
  void fold(xorq& in, Vinstr& out) {
    int val;
    if (logical_imm(in.s0, val)) {
      if (val == 0 && !uses[in.sf]) {
        out = copy{in.s1, in.d};
      } else {
        out = xorqi{val, in.s1, in.d, in.sf};
      }
    } else if (logical_imm(in.s1, val)) {
      if (val == 0 && !uses[in.sf]) {
        out = copy{in.s0, in.d};
      } else {
        out = xorqi{val, in.s0, in.d, in.sf};
      }
    }
  }
  void fold(copy& in, Vinstr& /*out*/) {
    if (in.d.isVirt() && valid.test(in.s)) {
      valid.set(in.d);
      vals[in.d] = vals[in.s];
    }
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

  // Use flag for each registers.  If a SR is uses then
  // certain optimizations will not fire since they do not
  // set the condition codes as the original instruction(s)
  // would.
  jit::vector<uint8_t> uses(unit.next_vr);
  for (auto b : blocks) {
    for (auto& inst : unit.blocks[b].code) {
      visitUses(unit, inst, [&](Vreg r) {
          auto& u = uses[r];
          if (u != 255) ++u;
        });
    }
  }

  Folder folder(unit, std::move(uses));
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
#define O(name, imms, uses, defs)           \
        case Vinstr::name: {                \
          auto const irctx = inst.irctx();  \
          folder.fold(inst.name##_, inst);  \
          inst.set_irctx(irctx);            \
          break;                            \
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
