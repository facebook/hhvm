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

#ifndef incl_HPHP_JIT_VASM_REG_H_
#define incl_HPHP_JIT_VASM_REG_H_

#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/util/asm-x64.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct Vptr;
struct Vscaled;
struct VscaledDisp;

///////////////////////////////////////////////////////////////////////////////

/*
 * Vreg is like PhysReg, but numbers go beyond the physical register names.
 * Since it is unconstrained, it has predicates to test whether `rn' is a GPR,
 * XMM, or virtual register.
 */
struct Vreg {
  static constexpr unsigned kNumGP = PhysReg::kNumGP;
  static constexpr unsigned kNumXMM = PhysReg::kNumSIMD;
  static constexpr unsigned kNumSF = PhysReg::kNumSF;
  static constexpr unsigned G0 = PhysReg::kGPOffset;
  static constexpr unsigned X0 = PhysReg::kSIMDOffset;
  static constexpr unsigned S0 = PhysReg::kSFOffset;
  static constexpr unsigned V0 = PhysReg::kMaxRegs;
  static constexpr unsigned kInvalidReg = 0xffffffffU;

  /*
   * Constructors.
   */
  Vreg() {}
  explicit Vreg(size_t r) : rn(r) {}
  /* implicit */ Vreg(Reg64 r)  : rn(int(r)) {}
  /* implicit */ Vreg(Reg32 r)  : rn(int(r)) {}
  /* implicit */ Vreg(Reg8 r)   : rn(int(r)) {}
  /* implicit */ Vreg(RegXMM r) : rn(X0+int(r)) {}
  /* implicit */ Vreg(RegSF r)  : rn(S0+int(r)) {}
  /* implicit */ Vreg(PhysReg r);

  /*
   * Casts.
   */
  /* implicit */ operator size_t() const { return rn; }
  /* implicit */ operator Reg64() const;
  /* implicit */ operator RegXMM() const;
  /* implicit */ operator RegSF() const;
  /* implicit */ operator PhysReg() const { return physReg(); }

  PhysReg physReg() const;

  /*
   * Is-a checks.
   */
  bool isPhys() const {
    static_assert(G0 < V0 && X0 < V0 && S0 < V0 && V0 < kInvalidReg, "");
    return rn < V0;
  }
  bool isGP() const { return /* rn >= G0 && */ rn < G0+kNumGP; }
  bool isSIMD() const { return rn >= X0 && rn < X0+kNumXMM; }
  bool isSF() const { return rn >= S0 && rn < S0+kNumSF; }
  bool isVirt() const { return rn >= V0 && isValid(); }
  bool isValid() const { return rn != kInvalidReg; }

  /*
   * Comparisons.
   */
  bool operator==(Vreg r) const { return rn == r.rn; }
  bool operator!=(Vreg r) const { return rn != r.rn; }

  /*
   * Addressing.
   */
  Vptr operator[](int disp) const;
  Vptr operator[](ScaledIndex) const;
  Vptr operator[](ScaledIndexDisp) const;
  Vptr operator[](Vptr) const;
  Vptr operator[](DispReg) const;
  Vptr operator[](Vscaled) const;
  Vptr operator[](VscaledDisp) const;
  Vptr operator[](Vreg) const;

  Vptr operator*() const;
  Vscaled operator*(int scale) const;

  Vptr operator+(size_t) const;
  Vptr operator+(int32_t) const;
  Vptr operator+(intptr_t) const;

private:
  unsigned rn{kInvalidReg};
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Instantiations or distinct subclasses of Vr wrap virtual register numbers
 * in in a strongly typed wrapper that conveys physical-register constraints,
 * similar to Reg64, Reg32, RegXMM, etc.
 */
template<class Reg>
struct Vr {
  /*
   * Constructors.
   */
  explicit Vr(size_t rn) : rn(rn) {}
  /* implicit */ Vr(Vreg r);
  /* implicit */ Vr(Reg r) : Vr{Vreg(r)} {}
  /* implicit */ Vr(PhysReg pr) : Vr{Vreg(pr)} {}

  /*
   * Casting.
   */
  /* implicit */ operator size_t() const { return rn; }
  /* implicit */ operator Reg() const { return asReg(); }
  /* implicit */ operator Vreg() const { return Vreg(rn); }
  explicit operator PhysReg() const { return asReg(); }

  Reg asReg() const;

  /*
   * Is-a checks.
   */
  bool isPhys() const {
    static_assert(Vreg::G0 == 0, "");
    return rn < Vreg::V0;
  }
  bool isGP() const { return rn>=Vreg::G0 && rn<Vreg::G0+Vreg::kNumGP; }
  bool isSIMD() const { return rn>=Vreg::X0 && rn<Vreg::X0+Vreg::kNumXMM; }
  bool isSF() const { return rn>=Vreg::S0 && rn<Vreg::S0+Vreg::kNumSF; }
  bool isVirt() const { return rn >= Vreg::V0 && isValid(); }
  bool isValid() const { return rn != Vreg::kInvalidReg; }

  /*
   * Comparisons.
   */
  bool operator==(Vr<Reg> r) const { return rn == r.rn; }
  bool operator!=(Vr<Reg> r) const { return rn != r.rn; }

  /*
   * Addressing.
   */
  Vptr operator[](int disp) const;
  Vptr operator[](ScaledIndex) const;
  Vptr operator[](ScaledIndexDisp) const;
  Vptr operator[](Vptr) const;
  Vptr operator[](DispReg) const;

  Vptr operator*() const;

  Vptr operator+(size_t) const;
  Vptr operator+(intptr_t) const;

private:
  unsigned rn;
};

using Vreg64  = Vr<Reg64>;
using Vreg32  = Vr<Reg32>;
using Vreg16  = Vr<Reg16>;
using Vreg8   = Vr<Reg8>;
using VregSF  = Vr<RegSF>;

struct VregDbl : Vr<RegXMM> {
  explicit VregDbl(size_t rn) : Vr<RegXMM>(rn) {}
  template<class... Args> /* implicit */ VregDbl(Args&&... args)
    : Vr<RegXMM>(std::forward<Args>(args)...) {}
  static bool allowable(Vreg r) { return r.isVirt() || r.isSIMD(); }
};

struct Vreg128 : Vr<RegXMM> {
  explicit Vreg128(size_t rn) : Vr<RegXMM>(rn) {}
  template<class... Args> /* implicit */ Vreg128(Args&&... args)
    : Vr<RegXMM>(std::forward<Args>(args)...)
  {}
};

inline Reg64 r64(Vreg64 r) { return r; }

///////////////////////////////////////////////////////////////////////////////

struct Vscaled {
  Vreg64 index;
  int scale;
};

struct VscaledDisp {
  Vscaled vs;
  int32_t disp;
};

VscaledDisp operator+(Vscaled, int32_t);

/*
 * Result of virtual register addressing: base + (index * scale) + disp.
 *    - base is optional, implying baseless address
 *    - index is optional
 */
struct Vptr {
  enum Segment : uint8_t { DS, FS, GS };

  Vptr()
    : base(Vreg{})
    , index(Vreg{})
    , disp(0)
  {}

  template<class Base>
  Vptr(Base b, int d)
    : base(b)
    , index(Vreg{})
    , scale(1)
    , disp(d)
  {}

  template<class Base, class Index>
  Vptr(Base b, Index i, int s, int d)
    : base(b)
    , index(i)
    , scale(s)
    , disp(d)
  {
    assert((scale == 0x1 || scale == 0x2 || scale == 0x4 || scale == 0x8) &&
           "Invalid index register scaling (must be 1,2,4 or 8).");
  }

  /* implicit */ Vptr(MemoryRef m, Segment s = DS)
    : base(m.r.base)
    , index(m.r.index)
    , scale(m.r.scale)
    , seg(s)
    , disp(m.r.disp)
  {}

  Vptr(const Vptr& o) = default;
  Vptr& operator=(const Vptr& o) = default;

  MemoryRef mr() const;
  /* implicit */ operator MemoryRef() const;

  bool operator==(const Vptr&) const;
  bool operator!=(const Vptr&) const;

  Vreg64 base;      // optional, for baseless mode
  Vreg64 index;     // optional
  uint8_t scale;    // 1,2,4,8
  Segment seg{DS};  // DS, FS or GS
  int32_t disp;
};

Vptr operator+(Vptr lhs, int32_t d);
Vptr operator+(Vptr lhs, intptr_t d);

Vptr baseless(VscaledDisp);

///////////////////////////////////////////////////////////////////////////////

/*
 * A Vloc is either a single or pair of vregs, for keeping track of where we
 * have stored an SSATmp.
 */
struct Vloc {
  enum Kind { kPair, kWide };

  /*
   * Constructors.
   */
  Vloc() {}
  explicit Vloc(Vreg r) { m_regs[0] = r; }
  Vloc(Vreg r0, Vreg r1) { m_regs[0] = r0; m_regs[1] = r1; }
  Vloc(Kind kind, Vreg r) : m_kind(kind) { m_regs[0] = r; }

  /*
   * Accessors.
   */
  bool hasReg(int i = 0) const;
  Vreg reg(int i = 0) const;
  int numAllocated() const;
  int numWords() const;
  bool isFullSIMD() const;

  /*
   * Comparisons.
   */
  bool operator==(Vloc other) const;
  bool operator!=(Vloc other) const;

private:
  Kind m_kind{kPair};
  Vreg m_regs[2];
};

///////////////////////////////////////////////////////////////////////////////
}}

#include "hphp/runtime/vm/jit/vasm-reg-inl.h"

#endif
