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

#include "hphp/runtime/vm/jit/vasm-fold-imms.h"

#include "hphp/vixl/hphp-compat.h"

TRACE_SET_MOD(hhir)

namespace HPHP::jit {

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
    if (!vixl::Assembler::IsImmAddSub(imm64)) return false;
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
  void fold(testq& in, Vinstr& out) {
    // Copy `in` because it aliases `out` through the Vinstr union, and
    // writing a differently-laid-out variant (e.g. testqi64) to `out` can
    // corrupt fields read from `in`.
    auto const in_copy = in;
    int val;
    uint64_t bm;
    if (logical_imm(in_copy.s0, val))      { out = testqi{val, in_copy.s1, in_copy.sf}; }
    else if (logical_imm(in_copy.s1, val)) { out = testqi{val, in_copy.s0, in_copy.sf}; }
    else if (logical_bmsk(in_copy.s0, bm)) { out = testqi64{bm, in_copy.s1, in_copy.sf}; }
    else if (logical_bmsk(in_copy.s1, bm)) { out = testqi64{bm, in_copy.s0, in_copy.sf}; }
  }
  void fold(cmpb& in, Vinstr& out) { return fold_cmp<cmpbi>(in, out); }
  void fold(cmpw& in, Vinstr& out) { return fold_cmp<cmpwi>(in, out); }
  void fold(cmpl& in, Vinstr& out) { return fold_cmp<cmpli>(in, out); }
  void fold(cmpq& in, Vinstr& out) { return fold_cmp<cmpqi>(in, out); }

  void fold(andq& in, Vinstr& out) {
    // Copy `in` because it aliases `out` through the Vinstr union, and
    // writing a differently-laid-out variant (e.g. andqi64) to `out` can
    // corrupt fields read from `in`.
    auto const in_copy = in;
    if (!uses[in_copy.sf] && valid.test(in_copy.s0) && valid.test(in_copy.s1)) {
      out = ldimmq{vals[in_copy.s0] & vals[in_copy.s1], in_copy.d};
      return;
    }
    int val;
    uint64_t bm;
    if (logical_imm(in_copy.s0, val))      { out = andqi{val, in_copy.s1, in_copy.d, in_copy.sf}; }
    else if (logical_imm(in_copy.s1, val)) { out = andqi{val, in_copy.s0, in_copy.d, in_copy.sf}; }
    else if (logical_bmsk(in_copy.s0, bm)) { out = andqi64{bm, in_copy.s1, in_copy.d, in_copy.sf}; }
    else if (logical_bmsk(in_copy.s1, bm)) { out = andqi64{bm, in_copy.s0, in_copy.d, in_copy.sf}; }
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
  template<typename storepairT>
  void fold_storepair(storepairT& in, Vinstr& out, PhysReg zero) {
    auto s0 = in.s0;
    auto s1 = in.s1;
    auto changed = false;
    if (zero_imm(s0)) {
      s0 = zero;
      changed = true;
    }
    if (zero_imm(s1)) {
      s1 = zero;
      changed = true;
    }
    if (changed) out = storepairT{s0, s1, in.d};
  }
  void fold(storepair& in, Vinstr& out) {
    fold_storepair(in, out, PhysReg(vixl::xzr));
  }
  void fold(storepairl& in, Vinstr& out) {
    fold_storepair(in, out, PhysReg(vixl::wzr));
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
  void fold(xorw& in, Vinstr& out) {
    int val;
    if (logical_imm(in.s0, val)) {
      if (val == 0 && !uses[in.sf]) {
        out = copy{in.s1, in.d};
      } else {
        out = xorwi{val, in.s1, in.d, in.sf};
      }
    } else if (logical_imm(in.s1, val)) {
      if (val == 0 && !uses[in.sf]) {
        out = copy{in.s0, in.d};
      } else {
        out = xorwi{val, in.s0, in.d, in.sf};
      }
    }
  }
  void fold(xorq& in, Vinstr& out) {
    // Copy `in` because it aliases `out` through the Vinstr union, and
    // writing a differently-laid-out variant (e.g. xorqi64) to `out` can
    // corrupt fields read from `in`.
    auto const in_copy = in;
    int val;
    uint64_t bm;
    if (logical_imm(in_copy.s0, val)) {
      if (val == 0 && !uses[in_copy.sf]) {
        out = copy{in_copy.s1, in_copy.d};
      } else {
        out = xorqi{val, in_copy.s1, in_copy.d, in_copy.sf};
      }
    } else if (logical_imm(in_copy.s1, val)) {
      if (val == 0 && !uses[in_copy.sf]) {
        out = copy{in_copy.s0, in_copy.d};
      } else {
        out = xorqi{val, in_copy.s0, in_copy.d, in_copy.sf};
      }
    } else if (logical_bmsk(in_copy.s0, bm)) {
      out = xorqi64{bm, in_copy.s1, in_copy.d, in_copy.sf};
    } else if (logical_bmsk(in_copy.s1, bm)) {
      out = xorqi64{bm, in_copy.s0, in_copy.d, in_copy.sf};
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

template void foldImms<arm::ImmFolder>(Vunit& unit);

}
