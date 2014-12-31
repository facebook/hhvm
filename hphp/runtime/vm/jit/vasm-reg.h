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

#ifndef incl_HPHP_JIT_VASM_REG_H_
#define incl_HPHP_JIT_VASM_REG_H_

#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/vasm.h"

namespace HPHP { namespace jit {

struct Vptr;
struct Vscaled;

// Vreg is like physreg, but numbers go beyond the physical register names.
// Since it is unconstrained, it has predicates to test whether rn is
// a gpr, xmm, or virtual register.
struct Vreg {
  static constexpr auto kind = VregKind::Any;
  static const unsigned kNumGP{PhysReg::kSIMDOffset}; // 33
  static const unsigned kNumXMM{30};
  static const unsigned kNumSF{1};
  static const unsigned G0{0};
  static const unsigned X0{kNumGP};
  static const unsigned S0{X0+kNumXMM};
  static const unsigned V0{S0+kNumSF};
  static const unsigned kInvalidReg{0xffffffffU};
  Vreg() : rn(kInvalidReg) {}
  explicit Vreg(size_t r) : rn(r) {}
  /* implicit */ Vreg(Reg64 r) : rn(int(r)) {}
  /* implicit */ Vreg(Reg32 r) : rn(int(r)) {}
  /* implicit */ Vreg(Reg8 r) : rn(int(r)) {}
  /* implicit */ Vreg(RegXMM r) : rn(X0+int(r)) {}
  /* implicit */ Vreg(RegSF r) : rn(S0+int(r)) {}
  /* implicit */ Vreg(PhysReg r) {
    rn = (r == InvalidReg) ? kInvalidReg :
         r.isGP() ? G0+int(Reg64(r)) :
          r.isSIMD() ? X0+int(RegXMM(r)) :
          /* r.isSF() ? */ S0+int(RegSF(r));
  }
  /* implicit */ operator size_t() const { return rn; }
  /* implicit */ operator Reg64() const {
    assert(isGP());
    return Reg64(rn - G0);
  }
  /* implicit */ operator RegXMM() const {
    assert(isSIMD());
    return RegXMM(rn - X0);
  }
  /* implicit */ operator RegSF() const {
    assert(isSF());
    return RegSF(rn - S0);
  }
  /* implicit */ operator PhysReg() const { return physReg(); }
  bool isPhys() const {
    static_assert(G0 < V0 && X0 < V0 && S0 < V0 && V0 < kInvalidReg, "");
    return rn < V0;
  }
  bool isGP() const { return /* rn >= G0 && */ rn < G0+kNumGP; }
  bool isSIMD() const { return rn >= X0 && rn < X0+kNumXMM; }
  bool isSF() const { return rn >= S0 && rn < S0+kNumSF; }
  bool isVirt() const { return rn >= V0 && isValid(); }
  bool isValid() const { return rn != kInvalidReg; }
  bool operator==(Vreg r) const { return rn == r.rn; }
  bool operator!=(Vreg r) const { return rn != r.rn; }
  PhysReg physReg() const {
    assert(!isValid() || isPhys());
    return !isValid() ? InvalidReg :
           isGP() ? PhysReg(/* implicit */operator Reg64()) :
           isSIMD() ? PhysReg(/* implicit */operator RegXMM()) :
           /* isSF() ? */ PhysReg(/* implicit */operator RegSF());
  }
  Vptr operator[](int disp) const;
  Vptr operator[](ScaledIndex) const;
  Vptr operator[](ScaledIndexDisp) const;
  Vptr operator[](Vptr) const;
  Vptr operator[](DispReg) const;
  Vptr operator*() const;
  Vptr operator[](Vscaled) const;
  Vscaled operator*(int scale) const;
  Vptr operator[](Vreg) const;
  Vptr operator+(size_t d) const;
private:
  unsigned rn{kInvalidReg};
};

} }

#endif
