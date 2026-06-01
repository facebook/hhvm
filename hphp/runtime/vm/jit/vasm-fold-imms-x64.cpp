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

TRACE_SET_MOD(hhir)

namespace HPHP::jit {

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
  bool match_xbyte(Vreg r, int& val) {
    if (!valid.test(r)) return false;
    auto const imm64 = vals[r];
    val = static_cast<int8_t>(imm64);
    return true;
  }
  bool match_xword(Vreg r, int& val) {
    if (!valid.test(r)) return false;
    auto const imm64 = vals[r];
    val = static_cast<int16_t>(imm64);
    return true;
  }
  bool match_int(Vreg r, int& val) {
    if (!valid.test(r)) return false;
    auto const imm64 = vals[r];
    if (!deltaFits(imm64, sz::dword)) return false;
    val = imm64;
    return true;
  }
  bool match_uint(Vreg r, int& val) {
    if (!valid.test(r)) return false;
    auto const imm64 = vals[r];
    if (!magFits(imm64, sz::dword)) return false;
    val = imm64;
    return true;
  }
  bool match_xint(Vreg r, int& val) {
    if (!valid.test(r)) return false;
    val = vals[r];
    return true;
  }

  // folders
  template <class Inst>
  void fold(Inst&, Vinstr& /*out*/) {}
  void fold(addq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) {
      int val2;
      if (!uses[in.sf] && match_int(in.s1, val2)) {
        // Attempt to const-fold addition of two constants. NB: This
        // is safe from overflow because val1 and val2 are ints.
        auto const sum = int64_t(val) + int64_t(val2);
        out = copy{unit.makeConst(sum), in.d};
        if (in.d.isVirt()) {
          valid.set(in.d);
          vals[in.d] = sum;
        }
      } else if (val == 0 && !uses[in.sf]) { // nop sets no flags.
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
  void fold(testl& in, Vinstr& out) {
    int val;
    if (match_xint(in.s0, val)) {out = testli{val, in.s1, in.sf};}
    else if (match_xint(in.s1, val)) {out = testli{val, in.s0, in.sf};}
  }
  void fold(testw& in, Vinstr& out) {
    int val;
    if (match_xint(in.s0, val)) {out = testwi{int16_t(val), in.s1, in.sf};}
    else if (match_xint(in.s1, val)) {out = testwi{int16_t(val), in.s0, in.sf};}
  }
  void fold(testb& in, Vinstr& out) {
    int val;
    if (match_xint(in.s0, val)) {out = testbi{int8_t(val), in.s1, in.sf};}
    else if (match_xint(in.s1, val)) {out = testbi{int8_t(val), in.s0, in.sf};}
  }
  void fold(cmpb& in, Vinstr& out) {
    int val;
    if (match_xbyte(in.s0, val)) { out = cmpbi{val, in.s1, in.sf}; }
  }
  void fold(cmpbm& in, Vinstr& out) {
    int val;
    if (match_xbyte(in.s0, val)) { out = cmpbim{val, in.s1, in.sf}; }
  }
  void fold(cmpw& in, Vinstr& out) {
    int val;
    if (match_xword(in.s0, val)) { out = cmpwi{val, in.s1, in.sf}; }
  }
  void fold(cmpwm& in, Vinstr& out) {
    int val;
    if (match_xword(in.s0, val)) { out = cmpwim{val, in.s1, in.sf}; }
  }
  void fold(cmpq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = cmpqi{val, in.s1, in.sf}; }
  }
  void fold(cmpl& in, Vinstr& out) {
    int val;
    if (match_xint(in.s0, val)) { out = cmpli{val, in.s1, in.sf}; }
  }
  void fold(cmpqm& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) { out = cmpqim{val, in.s1, in.sf}; }
  }
  void fold(cmplm& in, Vinstr& out) {
    int val;
    if (match_xint(in.s0, val)) { out = cmplim{val, in.s1, in.sf}; }
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
    if (match_xbyte(in.s, val)) { out = storebi{val, in.m}; }
  }
  void fold(storebi& in, Vinstr& /*out*/) { foldVptr(in.m); }
  void fold(storew& in, Vinstr& out) {
    foldVptr(in.m);
    int val;
    if (match_xword(in.s, val)) { out = storewi{val, in.m}; }
  }
  void fold(storewi& in, Vinstr& /*out*/) { foldVptr(in.m); }
  void fold(storel& in, Vinstr& out) {
    foldVptr(in.m);
    int val;
    if (match_xint(in.s, val)) { out = storeli{val, in.m}; }
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
    if (match_xbyte(in.s0, val)) {
      if (val == 0 && !uses[in.sf]) { // copy doesn't set any flags.
        out = copy{in.s1, in.d};
      } else if (val == -1 && !uses[in.sf]) { // not doesn't set any flags.
        out = notb{in.s1, in.d};
      } else {
        out = xorbi{val, in.s1, in.d, in.sf};
      }
    } else if (match_xbyte(in.s1, val)) {
      if (val == 0 && !uses[in.sf]) { // copy doesn't set any flags.
        out = copy{in.s0, in.d};
      } else if (val == -1 && !uses[in.sf]) { // not doesn't set any flags.
        out = notb{in.s0, in.d};
      } else {
        out = xorbi{val, in.s0, in.d, in.sf};
      }
    }
  }
  void fold(xorw& in, Vinstr& out) {
    int val;
    // TODO(mcolavita): xor with -1 is a not
    if (match_xword(in.s0, val)) {
      if (val == 0 && !uses[in.sf]) { // copy doesn't set any flags.
        out = copy{in.s1, in.d};
      } else {
        out = xorwi{val, in.s1, in.d, in.sf};
      }
    } else if (match_xword(in.s1, val)) {
      if (val == 0 && !uses[in.sf]) { // copy doesn't set any flags.
        out = copy{in.s0, in.d};
      } else {
        out = xorwi{val, in.s0, in.d, in.sf};
      }
    }
  }
  void fold(xorq& in, Vinstr& out) {
    int val;
    if (match_int(in.s0, val)) {
      if (val == 0 && !uses[in.sf]) { // copy doesn't set any flags
        out = copy{in.s1, in.d};
      } else if (val == -1 && !uses[in.sf]) { // not doesn't set any flags
        out = not_{in.s1, in.d};
      } else {
        out = xorqi{val, in.s1, in.d, in.sf};
      }
    } else if (match_int(in.s1, val)) {
      if (val == 0 && !uses[in.sf]) { // copy doesn't set any flags
        out = copy{in.s0, in.d};
      } else if (val == -1 && !uses[in.sf]) { // not doesn't set any flags
        out = not_{in.s0, in.d};
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

  void fold(addqrm& in, Vinstr&) { foldVptr(in.m); }

  void fold(load& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadb& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadw& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadl& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadups& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadsbl& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadsbq& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadsd& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadzbl& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadzbq& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadzwq& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadzlq& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadtqb& in, Vinstr& /*out*/) { foldVptr(in.s); }
  void fold(loadtql& in, Vinstr& /*out*/) { foldVptr(in.s); }

  void fold(testbim& in, Vinstr&) { foldVptr(in.s1); }
  void fold(testwim& in, Vinstr&) { foldVptr(in.s1); }
  void fold(testlim& in, Vinstr&) { foldVptr(in.s1); }
  void fold(testqim& in, Vinstr&) { foldVptr(in.s1); }
  void fold(testbm& in, Vinstr&) { foldVptr(in.s1); }
  void fold(testwm& in, Vinstr&) { foldVptr(in.s1); }
  void fold(testlm& in, Vinstr&) { foldVptr(in.s1); }
  void fold(testqm& in, Vinstr&) { foldVptr(in.s1); }

  void fold(cmpbim& in, Vinstr&) { foldVptr(in.s1); }
  void fold(cmpwim& in, Vinstr&) { foldVptr(in.s1); }
  void fold(cmplim& in, Vinstr&) { foldVptr(in.s1); }
  void fold(cmpqim& in, Vinstr&) { foldVptr(in.s1); }
};
} // namespace x64

template void foldImms<x64::ImmFolder>(Vunit& unit);

}
