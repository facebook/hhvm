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

#ifndef incl_HPHP_JIT_VASM_X64_H_
#define incl_HPHP_JIT_VASM_X64_H_

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/phys-loc.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/util/asm-x64.h"
#include <folly/Range.h>
#include <boost/dynamic_bitset.hpp>

namespace HPHP { namespace jit { namespace x64 {

struct Vptr;
struct Vscaled;

// Vreg is like physreg, but numbers go beyond the physical register names.
// Since it is unconstrained, it has predicates to test whether rn is
// a gpr, xmm, or virtual register.
struct Vreg {
  static constexpr auto kind = VregKind::Any;
  static const unsigned G0{0}, X0{16}, V0{32};
  Vreg() : rn(0xffffffff) {}
  explicit Vreg(size_t r) : rn(r) {}
  /* implicit */ Vreg(Reg64 r) : rn(int(r)) {}
  /* implicit */ Vreg(Reg32 r) : rn(int(r)) {}
  /* implicit */ Vreg(Reg8 r) : rn(int(r)) {}
  /* implicit */ Vreg(RegXMM r) : rn(X0+int(r)) {}
  /* implicit */ Vreg(PhysReg r) {
    rn = r == InvalidReg ? 0xffffffff :
         r.isGP() ? G0+int(Reg64(r)) :
         X0+int(RegXMM(r));
  }
  /* implicit */ operator size_t() const { return rn; }
  /* implicit */ operator Reg64() const {
    assert(isGP());
    return Reg64(rn);
  }
  /* implicit */ operator RegXMM() const {
    assert(isSIMD());
    return RegXMM(rn - X0);
  }
  /* implicit */ operator PhysReg() const { return physReg(); }
  bool isPhys() const {
    static_assert(G0 == 0, "");
    return rn < V0;
  }
  bool isGP() const { assert(!isVirt()); return rn < X0; }
  bool isSIMD() const { assert(!isVirt()); return rn >= X0 && rn < V0; }
  bool isVirt() const { return isValid() && rn >= V0; }
  bool isValid() const { return rn < 0xffffffff; }
  bool operator==(Vreg r) const { return rn == r.rn; }
  bool operator!=(Vreg r) const { return rn != r.rn; }
  PhysReg physReg() const {
    assert(isPhys());
    return isGP() ? PhysReg(/* implicit */operator Reg64()) :
                    PhysReg(/* implicit */operator RegXMM());
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
  unsigned rn{0xffffffff};
};

// instantiations of this wrap virtual register numbers in in a strongly
// typed wrapper that conveys physical constraints, similar to Reg64,
// Reg32, RegXMM, etc.
template<class Reg, VregKind k> struct Vr {
  static constexpr auto kind = k;
  explicit Vr(size_t rn) : rn(rn) {}
  /* implicit */ Vr(Vreg r) : rn(size_t(r)) {
    if (kind == VregKind::Simd) {
      assert(!r.isValid() || r.isVirt() || r.isSIMD());
    } else if (kind == VregKind::Gpr) {
      assert(!r.isValid() || r.isVirt() || r.isGP());
    }
  }
  /* implicit */ Vr(Reg r) : Vr{Vreg(r)} {}
  /* implicit */ Vr(PhysReg pr) : Vr{Vreg(pr)} {}
  Reg asReg() const {
    assert(isPhys());
    return isGP() ? Reg(rn) : Reg(rn-Vreg::X0);
  }
  /* implicit */ operator Reg() const { return asReg(); }
  /* implicit */ operator Vreg() const { return Vreg(rn); }
  /* implicit */ operator size_t() const { return rn; }
  bool isPhys() const {
    static_assert(Vreg::G0 == 0, "");
    return rn < Vreg::V0;
  }
  bool isGP() const { assert(!isVirt()); return rn < Vreg::X0; }
  bool isSIMD() const { assert(!isVirt());return rn>=Vreg::X0 && rn<Vreg::V0; }
  bool isVirt() const { return isValid() && rn >= Vreg::V0; }
  bool operator==(Vr<Reg,k> r) const { return rn == r.rn; }
  bool operator!=(Vr<Reg,k> r) const { return rn != r.rn; }
  bool operator==(PhysReg) const = delete;
  bool operator!=(PhysReg) const = delete;
  bool isValid() const { return rn < 0xffffffff; }
  Vptr operator[](int disp) const;
  Vptr operator[](ScaledIndex si) const;
  Vptr operator[](ScaledIndexDisp sid) const;
  Vptr operator[](Vptr) const;
  Vptr operator[](DispReg) const;
  Vptr operator*() const;
  Vptr operator+(size_t d) const;
  explicit operator PhysReg() const { return asReg(); }
private:
  unsigned rn;
};
typedef Vr<Reg64,VregKind::Gpr>   Vreg64;
typedef Vr<Reg32,VregKind::Gpr>   Vreg32;
typedef Vr<Reg16,VregKind::Gpr>   Vreg16;
typedef Vr<Reg8,VregKind::Gpr>    Vreg8;
typedef Vr<RegXMM,VregKind::Simd> VregXMM;

inline Reg64 r64(PhysReg r) { return r; }
inline Reg64 r64(Vreg64 r) { return r; }

// base + index*scale + disp.
// base is optional, implying baseless address
// index is optional
struct Vptr {
  enum Segment: uint8_t { DS, FS };
  template<class Base> Vptr(Base b, int d)
    : base(b), index(0xffffffff), scale(1), disp(d)
  {}
  template<class Base, class Index> Vptr(Base b, Index i, int s, int d)
    : base(b), index(i), scale(s), disp(d)
  {}
  /* implicit */ Vptr(MemoryRef m, Segment s = DS)
    : base(m.r.base), index(m.r.index), scale(m.r.scale)
    , seg(s), disp(m.r.disp)
  {}
  MemoryRef mr() const {
    if (index.isValid()) {
      return base.isValid() ? r64(base)[r64(index) * scale + disp] :
             *(IndexedDispReg{r64(index) * scale + disp});
    } else {
      return base.isValid() ? r64(base)[disp] :
             *(DispReg{disp});
    }
  }
  /* implicit */ operator MemoryRef() const {
    assert(seg == DS);
    return mr();
  }
public:
  Vreg64 base; // optional, for baseless mode
  Vreg64 index; // optional
  uint8_t scale; // 1,2,4,8
  Segment seg{DS}; // DS or FS
  int32_t disp;
};

struct Vscaled {
  Vreg64 index;
  int scale;
};

inline Vptr operator+(Vptr lhs, int32_t d) {
  return Vptr(lhs.base, lhs.index, lhs.scale, lhs.disp + d);
}

inline Vptr operator+(Vptr lhs, intptr_t d) {
  return Vptr(lhs.base, lhs.index, lhs.scale,
              safe_cast<int32_t>(lhs.disp + d));
}

inline Vptr operator+(Vreg64 base, int32_t d) {
  return Vptr(base, d);
}

// like PhysLoc: "location", either a single or pair of vregs.
// never a spill slot, since spilling is after code-gen.
// there is no information about GPR or SIMD here; that's
// a register allocator choice.
struct Vloc {
  enum Kind { kPair, kWide };
  Vloc() {}
  explicit Vloc(Vreg r) { m_regs[0] = r; }
  Vloc(Vreg r0, Vreg r1) { m_regs[0] = r0; m_regs[1] = r1; }
  /* implicit */ Vloc(PhysLoc loc) {
    assert(!loc.spilled());
    if (loc.isFullSIMD()) {
      m_kind = kWide;
      m_regs[0] = loc.reg(0);
    } else if (loc.numAllocated() == 1) {
      m_regs[0] = loc.reg(0);
    } else if (loc.numAllocated() == 2) {
      m_regs[0] = loc.reg(0);
      m_regs[1] = loc.reg(1);
    }
  }
  bool hasReg(int i = 0) const {
    return m_regs[i].isValid();
  }
  Vreg reg(int i = 0) const {
    return m_regs[i];
  }
  int numAllocated() const {
    return int(m_regs[0].isValid()) + int(m_regs[1].isValid());
  }
  int numWords() const {
    return m_kind == kWide ? 2 : numAllocated();
  }
  bool isFullSIMD() const {
    return m_kind == kWide;
  }

  bool operator==(Vloc other) const {
    return m_kind == other.m_kind &&
           m_regs[0] == other.m_regs[0] &&
           m_regs[1] == other.m_regs[1];
  }
  bool operator!=(Vloc other) const {
    return !(*this == other);
  }

private:
  Kind m_kind{kPair};
  Vreg m_regs[2];
};

inline Vscaled Vreg::operator*(int scale) const {
  return Vscaled{*this, scale};
}

inline Vptr Vreg::operator[](Vscaled si) const {
  return Vptr(*this, si.index, si.scale, 0);
}

inline Vptr Vreg::operator*() const { return Vptr(*this, 0); }
inline Vptr Vreg::operator[](int disp) const { return Vptr(*this, disp); }
inline Vptr Vreg::operator[](ScaledIndex si) const {
  return Vptr(*this, si.index, si.scale, 0);
}
inline Vptr Vreg::operator[](ScaledIndexDisp sid) const {
  return Vptr(*this, sid.si.index, sid.si.scale, sid.disp);
}
inline Vptr Vreg::operator[](Vptr p) const {
  return Vptr(*this, p.base, 1, p.disp);
}
inline Vptr Vreg::operator[](DispReg rd) const {
  return Vptr(*this, rd.base, 1, rd.disp);
}
inline Vptr Vreg::operator[](Vreg index) const {
  return Vptr(*this, index, 1, 0);
}

template<class Reg, VregKind k>
Vptr Vr<Reg,k>::operator*() const {
  return Vptr(*this, 0);
}

template<class Reg, VregKind k>
Vptr Vr<Reg,k>::operator[](int disp) const {
  return Vptr(*this, disp);
}

inline Vptr operator+(Vreg base, int32_t d) {
  return Vptr(base, safe_cast<int32_t>(d));
}

inline Vptr operator+(Vreg base, intptr_t d) {
  return Vptr(base, safe_cast<int32_t>(d));
}

inline Vptr Vreg::operator+(size_t d) const {
  return Vptr(*this, safe_cast<int32_t>(d));
}

inline Vptr operator+(Vreg64 base, intptr_t d) {
  return Vptr(base, safe_cast<int32_t>(d));
}

template<class Reg, VregKind k>
Vptr Vr<Reg,k>::operator[](ScaledIndex si) const {
  return Vptr(*this, si.index, si.scale, 0);
}

template<class Reg, VregKind k>
Vptr Vr<Reg,k>::operator[](ScaledIndexDisp sid) const {
  return Vptr(*this, sid.si.index, sid.si.scale, sid.disp);
}

template<class Reg, VregKind k>
Vptr Vr<Reg,k>::operator[](Vptr p) const {
  return Vptr(*this, p.base, 1, p.disp);
}

template<class Reg, VregKind k>
Vptr Vr<Reg,k>::operator[](DispReg rd) const {
  return Vptr(*this, rd.base, 1, rd.disp);
}

template<class Reg, VregKind k>
inline Vptr Vr<Reg,k>::operator+(size_t d) const {
  return Vptr(*this, safe_cast<int32_t>(d));
}

#define X64_OPCODES\
  /* intrinsics */\
  O(bindaddr, I(dest) I(sk), Un, Dn)\
  O(bindcall, I(sk) I(callee) I(argc), Un, Dn)\
  O(bindexit, I(cc) I(target), Un, Dn)\
  O(bindjcc1, I(cc) I(targets[0]) I(targets[1]), Un, Dn)\
  O(bindjcc2, I(cc) I(target), Un, Dn)\
  O(bindjmp, I(target) I(trflags), Un, Dn)\
  O(callstub, I(target) I(kills) I(fix), U(args), Dn)\
  O(contenter, Inone, U(fp) U(target), Dn)\
  O(copy, Inone, U(s), D(d))\
  O(copy2, Inone, U(s0) U(s1), D(d0) D(d1))\
  O(copyargs, Inone, U(s), D(d))\
  O(end, Inone, Un, Dn)\
  O(ldimm, I(s) I(saveflags), Un, D(d))\
  O(fallback, I(dest), Un, Dn)\
  O(fallbackcc, I(cc) I(dest), Un, Dn)\
  O(incstat, I(stat) I(n) I(force), Un, Dn)\
  O(kpcall, I(target) I(callee) I(prologIndex), Un, Dn)\
  O(ldpoint, I(s), Un, D(d))\
  O(load, Inone, U(s), D(d))\
  O(mccall, I(target), U(args), Dn)\
  O(mcprep, Inone, Un, D(d))\
  O(nop, Inone, Un, Dn)\
  O(nocatch, Inone, Un, Dn)\
  O(phidef, Inone, Un, D(defs))\
  O(phijmp, Inone, U(uses), Dn)\
  O(point, I(p), Un, Dn)\
  O(resume, Inone, Un, Dn)\
  O(retransopt, I(sk) I(id), Un, Dn)\
  O(store, Inone, U(s) U(d), Dn)\
  O(syncpoint, I(fix), Un, Dn)\
  O(unwind, Inone, Un, Dn)\
  /* x64 instructions */\
  O(andb, Inone, U(s0) U(s1), D(d)) \
  O(andbi, I(s0), U(s1), D(d)) \
  O(andbim, I(s), U(m), Dn) \
  O(andl, Inone, U(s0) U(s1), D(d)) \
  O(andli, I(s0), U(s1), D(d)) \
  O(andq, Inone, U(s0) U(s1), D(d)) \
  O(andqi, I(s0), U(s1), D(d)) \
  O(addlm, Inone, U(s0) U(m), Dn) \
  O(addq, Inone, U(s0) U(s1), D(d)) \
  O(addqi, I(s0), U(s1), D(d))\
  O(addsd, Inone, U(s0) U(s1), D(d))\
  O(call, I(target), U(args), Dn)\
  O(callm, Inone, U(target) U(args), Dn)\
  O(callr, Inone, U(target) U(args), Dn)\
  O(cloadq, I(cc), U(s), D(d))\
  O(cmovq, I(cc), U(f) U(t), D(d))\
  O(cmpb, Inone, U(s0) U(s1), Dn)\
  O(cmpbi, I(s0), U(s1), Dn)\
  O(cmpbim, I(s0), U(s1), Dn)\
  O(cmpl, Inone, U(s0) U(s1), Dn)\
  O(cmpli, I(s0), U(s1), Dn)\
  O(cmplim, I(s0), U(s1), Dn)\
  O(cmplm, Inone, U(s0) U(s1), Dn)\
  O(cmpq, Inone, U(s0) U(s1), Dn)\
  O(cmpqi, I(s0), U(s1), Dn)\
  O(cmpqim, I(s0), U(s1), Dn)\
  O(cmpqm, Inone, U(s0) U(s1), Dn)\
  O(cmpsd, I(pred), UA(s0) U(s1), D(d))\
  O(cqo, Inone, Un, Dn)\
  O(cvttsd2siq, Inone, U(s), D(d))\
  O(cvtsi2sd, Inone, U(s), D(d))\
  O(cvtsi2sdm, Inone, U(s), D(d))\
  O(decl, Inone, U(s), D(d))\
  O(declm, Inone, U(m), Dn)\
  O(decq, Inone, U(s), D(d))\
  O(decqm, Inone, U(m), Dn)\
  O(divsd, Inone, UA(s0) U(s1), D(d))\
  O(incwm, Inone, U(m), Dn)\
  O(idiv, Inone, U(s), Dn)\
  O(imul, Inone, U(s0) U(s1), D(d))\
  O(incl, Inone, U(s), D(d))\
  O(inclm, Inone, U(m), Dn)\
  O(incq, Inone, U(s), D(d))\
  O(incqm, Inone, U(m), Dn)\
  O(incqmlock, Inone, U(m), Dn)\
  O(jcc, I(cc), Un, Dn)\
  O(jmp, Inone, Un, Dn)\
  O(jmpr, Inone, U(target), Dn)\
  O(jmpm, Inone, U(target), Dn)\
  O(lea, Inone, U(s), D(d))\
  O(leap, I(s), Un, D(d))\
  O(loaddqu, Inone, U(s), D(d))\
  O(loadl, Inone, U(s), D(d))\
  O(loadq, Inone, U(s), D(d))\
  O(loadqp, I(s), Un, D(d))\
  O(loadsd, Inone, U(s), D(d))\
  O(loadzbl, Inone, U(s), D(d))\
  O(movb, Inone, U(s), D(d))\
  O(movbi, I(s), Un, D(d))\
  O(movdqa, Inone, U(s), D(d))\
  O(movl, Inone, U(s), D(d))\
  O(movq, Inone, U(s), D(d))\
  O(movqrx, Inone, U(s), D(d))\
  O(movqxr, Inone, U(s), D(d))\
  O(movsbl, Inone, U(s), D(d))\
  O(movzbl, Inone, U(s), D(d))\
  O(mulsd, Inone, U(s0) U(s1), D(d))\
  O(neg, Inone, U(s), D(d))\
  O(not, Inone, U(s), D(d))\
  O(orq, Inone, U(s0) U(s1), D(d))\
  O(orqi, I(s0), U(s1), D(d))\
  O(orqim, I(s0), U(m), Dn)\
  O(pop, Inone, Un, D(d))\
  O(popm, Inone, U(m), Dn)\
  O(psllq, I(s0), U(s1), D(d))\
  O(psrlq, I(s0), U(s1), D(d))\
  O(push, Inone, U(s), Dn)\
  O(pushl, Inone, U(s), Dn)\
  O(pushm, Inone, U(s), Dn)\
  O(ret, Inone, Un, Dn)\
  O(rorqi, I(s0), U(s1), D(d))\
  O(roundsd, I(dir), U(s), D(d))\
  O(sarq, Inone, U(s), D(d))\
  O(sarqi, I(s0), U(s1), D(d))\
  O(sbbl, Inone, UA(s0) U(s1), D(d))\
  O(setcc, I(cc), Un, D(d))\
  O(shlli, I(s0), U(s1), D(d))\
  O(shlq, Inone, U(s), D(d))\
  O(shlqi, I(s0), U(s1), D(d))\
  O(shrli, I(s0), U(s1), D(d))\
  O(shrqi, I(s0), U(s1), D(d))\
  O(sqrtsd, Inone, U(s), D(d))\
  O(storeb, Inone, U(s) U(m), Dn)\
  O(storebim, I(s), U(m), Dn)\
  O(storedqu, Inone, U(s) U(m), Dn)\
  O(storel, Inone, U(s) U(m), Dn)\
  O(storelim, I(s), U(m), Dn)\
  O(storeq, Inone, U(s) U(m), Dn)\
  O(storeqim, I(s), U(m), Dn)\
  O(storew, Inone, U(s) U(m), Dn)\
  O(storesd, Inone, U(s) U(m), Dn)\
  O(storewim, I(s), U(m), Dn)\
  O(subl, Inone, UA(s0) U(s1), D(d))\
  O(subli, I(s0), U(s1), D(d))\
  O(subq, Inone, UA(s0) U(s1), D(d))\
  O(subqi, I(s0), U(s1), D(d))\
  O(subsd, Inone, UA(s0) U(s1), D(d))\
  O(testb, Inone, U(s0) U(s1), Dn)\
  O(testbi, I(s0), U(s1), Dn)\
  O(testbim, I(s0), U(s1), Dn)\
  O(testl, Inone, U(s0) U(s1), Dn)\
  O(testli, I(s0), U(s1), Dn)\
  O(testlim, I(s0), U(s1), Dn)\
  O(testq, Inone, U(s0) U(s1), Dn)\
  O(testqm, Inone, U(s0) U(s1), Dn)\
  O(testqim, I(s0), U(s1), Dn)\
  O(ucomisd, Inone, U(s0) U(s1), Dn)\
  O(ud2, Inone, Un, Dn)\
  O(unpcklpd, Inone, UA(s0) U(s1), D(d))\
  O(xorb, Inone, U(s0) U(s1), D(d))\
  O(xorbi, I(s0), U(s1), D(d))\
  O(xorq, Inone, U(s0) U(s1), D(d))\
  O(xorqi, I(s0), U(s1), D(d))\

// intrinsics
struct bindaddr { TCA* dest; SrcKey sk; };
struct bindcall { SrcKey sk; const Func* callee; unsigned argc; };
struct bindexit { ConditionCode cc; SrcKey target; TransFlags trflags; };
struct bindjcc1 { ConditionCode cc; Offset targets[2]; };
struct bindjcc2 { ConditionCode cc; Offset target; };
struct bindjmp { SrcKey target; TransFlags trflags; };
struct callstub { CodeAddress target; RegSet args, kills; Fixup fix; };
struct contenter { Vreg64 fp, target; };
struct copy { Vreg s, d; };
struct copy2 { Vreg64 s0, s1, d0, d1; };
struct copyargs { Vtuple s, d; };
struct end {};
struct ldimm { Immed64 s; Vreg d; bool saveflags; };
struct fallback { SrcKey dest; TransFlags trflags; };
struct fallbackcc { ConditionCode cc; SrcKey dest; TransFlags trflags; };
struct incstat { Stats::StatCounter stat; int n; bool force; };
struct kpcall { CodeAddress target; const Func* callee; unsigned prologIndex; };
struct ldpoint { Vpoint s; Vreg64 d; };
struct load { Vptr s; Vreg d; };
struct mccall { CodeAddress target; RegSet args; };
struct mcprep { Vreg64 d; };
struct nop {};
struct nocatch {};
struct phidef { Vtuple defs; };
struct phijmp { Vlabel target; Vtuple uses; };
struct point { Vpoint p; };
struct resume {};
struct retransopt { SrcKey sk; TransID id; };
struct store { Vreg s; Vptr d; };
struct syncpoint { Fixup fix; };
struct unwind { Vlabel targets[2]; };

// ATT style operand order. for binary ops:
// op   s0 s1 d:  d = s1 op s0    =>   d=s1; d op= s0
// op   imm s1 d: d = s1 op imm   =>   d=s1; d op= imm
// cmp  s0 s1:    s1 cmp s0

// suffix conventions:
//  b   8-bit
//  w   16-bit
//  l   32-bit
//  q   64-bit
//  i   immediate
//  m   Vptr
//  p   RIPRelativeRef

// x64 instructions
struct andb  { Vreg8 s0, s1, d; };
struct andbi { Immed s0; Vreg8 s1, d; };
struct andbim { Immed s; Vptr m; };
struct andl  { Vreg32 s0, s1, d; };
struct andli { Immed s0; Vreg32 s1, d; };
struct andq  { Vreg64 s0, s1, d; };
struct andqi { Immed s0; Vreg64 s1, d; };
struct addlm { Vreg32 s0; Vptr m; };
struct addq  { Vreg64 s0, s1, d; };
struct addqi { Immed s0; Vreg64 s1, d; };
struct addsd  { VregXMM s0, s1, d; };
struct call { CodeAddress target; RegSet args; };
struct callm { Vptr target; RegSet args; };
struct callr { Vreg64 target; RegSet args; };
struct cloadq { ConditionCode cc; Vptr s; Vreg64 d; };
struct cmovq { ConditionCode cc; Vreg64 f, t, d; };
struct cmpb  { Vreg8  s0; Vreg8  s1; };
struct cmpbi { Immed  s0; Vreg8  s1; };
struct cmpbim { Immed s0; Vptr s1; };
struct cmpl  { Vreg32 s0; Vreg32 s1; };
struct cmpli { Immed  s0; Vreg32 s1; };
struct cmplim { Immed s0; Vptr s1; };
struct cmplm { Vreg32 s0; Vptr s1; };
struct cmpq  { Vreg64 s0; Vreg64 s1; };
struct cmpqi { Immed  s0; Vreg64 s1; };
struct cmpqim { Immed s0; Vptr s1; };
struct cmpqm { Vreg64 s0; Vptr s1; };
struct cmpsd { ComparisonPred pred; VregXMM s0, s1, d; };
struct cqo {};
struct cvttsd2siq { VregXMM s; Vreg64 d; };
struct cvtsi2sd { Vreg64 s; VregXMM d; };
struct cvtsi2sdm { Vptr s; VregXMM d; };
struct decl { Vreg32 s, d; };
struct declm { Vptr m; };
struct decq { Vreg64 s, d; };
struct incwm { Vptr m; };
struct decqm { Vptr m; };
struct divsd { VregXMM s0, s1, d; };
struct idiv { Vreg64 s; };
struct imul { Vreg64 s0, s1, d; };
struct incl { Vreg32 s, d; };
struct inclm { Vptr m; };
struct incq { Vreg64 s, d; };
struct incqm { Vptr m; };
struct incqmlock { Vptr m; };
struct jcc { ConditionCode cc; Vlabel targets[2]; };
struct jmp { Vlabel target; };
struct jmpr { Vreg64 target; };
struct jmpm { Vptr target; };
struct lea { Vptr s; Vreg64 d; };
struct leap { RIPRelativeRef s; Vreg64 d; };
struct loaddqu { Vptr s; VregXMM d; };
struct loadl { Vptr s; Vreg32 d; };
struct loadq { Vptr s; Vreg64 d; };
struct loadqp { RIPRelativeRef s; Vreg64 d; };
struct loadsd { Vptr s; VregXMM d; };
struct loadzbl { Vptr s; Vreg32 d; };
struct movb { Vreg8 s, d; };
struct movbi { Immed s; Vreg8 d; };
struct movdqa { VregXMM s, d; };
struct movl { Vreg32 s, d; };
struct movq { Vreg64 s, d; };
struct movqrx { Vreg64 s; VregXMM d; };
struct movqxr { VregXMM s; Vreg64 d; };
struct movsbl { Vreg8 s; Vreg32 d; };
struct movzbl { Vreg8 s; Vreg32 d; };
struct mulsd  { VregXMM s0, s1, d; };
struct neg { Vreg64 s, d; };
struct not { Vreg64 s, d; };
struct orq { Vreg64 s0, s1, d; };
struct orqi { Immed s0; Vreg64 s1, d; };
struct orqim { Immed s0; Vptr m; };
struct pop { Vreg64 d; };
struct popm { Vptr m; };
struct psllq { Immed s0; VregXMM s1, d; };
struct psrlq { Immed s0; VregXMM s1, d; };
struct push { Vreg64 s; };
struct pushl { Vreg32 s; };
struct pushm { Vptr s; };
struct ret {};
struct rorqi { Immed s0; Vreg64 s1, d; };
struct roundsd { RoundDirection dir; VregXMM s, d; };
struct sarq { Vreg64 s, d; }; // uses rcx
struct sarqi { Immed s0; Vreg64 s1, d; };
struct sbbl { Vreg32 s0, s1, d; };
struct setcc { ConditionCode cc; Vreg8 d; };
struct shlli { Immed s0; Vreg32 s1, d; };
struct shlq { Vreg64 s, d; }; // uses rcx
struct shlqi { Immed s0; Vreg64 s1, d; };
struct shrli { Immed s0; Vreg32 s1, d; };
struct shrqi { Immed s0; Vreg64 s1, d; };
struct sqrtsd { VregXMM s, d; };
struct storeb { Vreg8 s; Vptr m; };
struct storebim { Immed s; Vptr m; };
struct storedqu { VregXMM s; Vptr m; };
struct storel { Vreg32 s; Vptr m; };
struct storelim { Immed s; Vptr m; };
struct storeq { Vreg64 s; Vptr m; };
struct storeqim { Immed s; Vptr m; };
struct storesd { VregXMM s; Vptr m; };
struct storew { Vreg16 s; Vptr m; };
struct storewim { Immed s; Vptr m; };
struct subl { Vreg32 s0, s1, d; };
struct subli { Immed s0; Vreg32 s1, d; };
struct subq { Vreg64 s0, s1, d; };
struct subqi { Immed s0; Vreg64 s1, d; };
struct subsd { VregXMM s0, s1, d; };
struct testb { Vreg8 s0, s1; };
struct testbi { Immed s0; Vreg8 s1; };
struct testbim { Immed s0; Vptr s1; };
struct testl { Vreg32 s0, s1; };
struct testli { Immed s0; Vreg32 s1; };
struct testlim { Immed s0; Vptr s1; };
struct testq { Vreg64 s0, s1; };
struct testqm { Vreg64 s0; Vptr s1; };
struct testqim { Immed s0; Vptr s1; };
struct ucomisd { VregXMM s0, s1; };
struct ud2 {};
struct unpcklpd { VregXMM s0, s1, d; };
struct xorb { Vreg8 s0, s1, d; };
struct xorbi { Immed s0; Vreg8 s1, d; };
struct xorq { Vreg64 s0, s1, d; };
struct xorqi { Immed s0; Vreg64 s1, d; };

struct Vinstr {
#define O(name, imms, uses, defs) name,
  enum Opcode { X64_OPCODES };
#undef O

#define O(name, imms, uses, defs) \
  /* implicit */ Vinstr(x64::name i) : op(name), name##_(i) {}
  X64_OPCODES
#undef O

  Opcode op;
  unsigned pos;
  SrcKey sk;
#define O(name, imms, uses, defs) x64::name name##_;
  union { X64_OPCODES };
#undef O
};

enum class AreaIndex: unsigned { Main, Cold, Frozen, Max };

struct Vblock {
  explicit Vblock(AreaIndex area) : area(area) {}
  AreaIndex area;
  jit::vector<Vinstr> code;
};

typedef jit::vector<Vreg> VregList;

// all the assets that make up a vasm compilation unit
struct Vunit {
  Vlabel makeBlock(AreaIndex area);
  Vreg makeReg() { return Vreg{next_vr++}; }
  Vtuple makeTuple(const VregList& regs);
  Vreg makeConst(uint64_t);
  Vreg makeConst(double);
  Vreg makeConst(const void* p) { return makeConst(uint64_t(p)); }
  Vreg makeConst(int64_t v) { return makeConst(uint64_t(v)); }
  Vreg makeConst(int32_t v) { return makeConst(int64_t(v)); }
  Vreg makeConst(DataType t) { return makeConst(uint64_t(t)); }
  Vreg makeConst(Immed64 v) { return makeConst(uint64_t(v.q())); }
  bool hasVrs() const { return next_vr > Vreg::V0; }
  unsigned next_vr{Vreg::V0};
  jit::vector<Vblock> blocks;
  jit::vector<Vlabel> roots; // entry points
  jit::hash_map<uint64_t,Vreg> cpool;
  jit::vector<VregList> tuples;
};

// writer stream to add instructions to a block
struct Vout {
  Vout(Vmeta* m, Vunit& u, Vlabel b, AreaIndex area)
    : m_meta(m), m_unit(u), m_block(b), m_area(area)
  {}
  Vout(Vmeta* m, Vunit& u, Vlabel b, AreaIndex area, SrcKey sk)
    : m_meta(m), m_unit(u), m_block(b), m_area(area), m_sk(sk)
  {}
  Vout(const Vout& v)
    : m_meta(v.m_meta), m_unit(v.m_unit), m_block(v.m_block), m_area(v.m_area)
    , m_sk(v.m_sk)
  {}

  Vout& operator=(const Vout& v) {
    assert(&v.m_unit == &m_unit && v.m_area == m_area);
    m_block = v.m_block;
    m_sk = v.m_sk;
    return *this;
  }

  // implicit cast to label for initializing branch instructions
  /* implicit */ operator Vlabel() const;
  bool empty() const;
  bool closed() const;

  Vout makeBlock(); // create a stream connected to a new empty block
  Vout makeEntry(); // makeBlock() and add it to unit.roots

  // instruction emitter
  Vout& operator<<(Vinstr inst);

  Vpoint makePoint() { return m_meta->makePoint(); }
  Vmeta& meta() { return *m_meta; }
  Vunit& unit() { return m_unit; }
  template<class T> Vreg cns(T v) { return m_unit.makeConst(v); }
  void use(Vlabel b) { m_block = b; }
  void setSrcKey(SrcKey sk) { m_sk = sk; }
  Vreg makeReg() { return m_unit.makeReg(); }
  AreaIndex area() const { return m_area; }
  Vtuple makeTuple(const VregList& regs) const {
    return m_unit.makeTuple(regs);
  }

private:
  void add(Vinstr&);

private:
  Vmeta* const m_meta;
  Vunit& m_unit;
  Vlabel m_block;
  AreaIndex m_area;
  SrcKey m_sk;
};

// Similar to X64Assembler, but buffers instructions as they
// are written, then generates code all at once at the end.
// Areas represent the separate sections we generate code into;
struct Vasm {
  struct Area {
    Vout out;
    CodeBlock& code;
    CodeAddress start;
  };
  explicit Vasm(Vmeta* meta) : m_meta(meta) {
    m_areas.reserve(size_t(AreaIndex::Max));
  }
  void finish(const Abi&);

  // get an existing area
  Vout& main() { return area(AreaIndex::Main).out; }
  Vout& cold() { return area(AreaIndex::Cold).out; }
  Vout& frozen() { return area(AreaIndex::Frozen).out; }

  // create areas
  Vout& main(CodeBlock& cb) { return add(cb, AreaIndex::Main); }
  Vout& cold(CodeBlock& cb) { return add(cb, AreaIndex::Cold); }
  Vout& frozen(CodeBlock& cb) { return add(cb, AreaIndex::Frozen); }
  Vout& main(X64Assembler& a) { return main(a.code()); }
  Vout& cold(X64Assembler& a) { return cold(a.code()); }
  Vout& frozen(X64Assembler& a) { return frozen(a.code()); }
  Vunit& unit() { return m_unit; }

private:
  Vout& add(CodeBlock &cb, AreaIndex area);
  Area& area(AreaIndex i) {
    assert((unsigned)i < m_areas.size());
    return m_areas[(unsigned)i];
  }

private:
  Vmeta* const m_meta;
  Vunit m_unit;
protected:
  jit::vector<Area> m_areas; // indexed by AreaIndex
};

struct Vauto : Vasm {
  explicit Vauto(Vmeta* meta = nullptr) : Vasm{meta} {}
  ~Vauto();
  RegSet params;
};

template<class F> void visit(const Vunit&, Vreg v, F f) {
  f(v);
}
template<class F> void visit(const Vunit&, Vptr p, F f) {
  if (p.base.isValid()) f(p.base);
  if (p.index.isValid()) f(p.index);
}
template<class F> void visit(const Vunit& unit, Vtuple t, F f) {
  for (auto r : unit.tuples[t]) f(r);
}
template<class F> void visit(const Vunit& unit, RegSet regs, F f) {
  regs.forEach([&](Vreg r) { f(r); });
}

template<class Use>
void visitUses(const Vunit& unit, Vinstr& inst, Use use) {
  switch (inst.op) {
#define O(name, imms, uses, defs) \
    case Vinstr::name: { \
      auto& i = inst.name##_; (void)i; \
      uses \
      break; \
    }
#define U(f) visit(unit, i.f, use);
#define UA(f) visit(unit, i.f, use);
#define Un
    X64_OPCODES
#undef Un
#undef U
#undef UA
#undef O
  }
}

template<class Def>
void visitDefs(const Vunit& unit, Vinstr& inst, Def def) {
  switch (inst.op) {
#define O(name, imms, uses, defs) \
    case Vinstr::name: { \
      auto& i = inst.name##_; (void)i; \
      defs \
      break; \
    }
#define D(f) visit(unit, i.f, def);
#define Dn
    X64_OPCODES
#undef Dn
#undef D
#undef O
  }
}

template<class Visitor>
void visitOperands(Vinstr& inst, Visitor& visitor) {
  switch (inst.op) {
#define O(name, imms, uses, defs) \
    case Vinstr::name: { \
      auto& i = inst.name##_; (void)i; \
      imms \
      uses \
      defs \
      break; \
    }
#define I(f) visitor.imm(i.f);
#define U(f) visitor.use(i.f);
#define UA(f) visitor.across(i.f);
#define D(f) visitor.def(i.f);
#define Inone
#define Un
#define Dn
    X64_OPCODES
#undef Dn
#undef Un
#undef Inone
#undef D
#undef U
#undef UA
#undef I
#undef O
  }
}

// visit reachable blocks in postorder, calling fn on each one.
struct PostorderWalker {
  template<class Fn> void dfs(Vlabel b, Fn fn) {
    if (visited.test(b)) return;
    visited.set(b);
    for (auto s : succs(unit.blocks[b])) {
      dfs(s, fn);
    }
    fn(b);
  }
  template<class Fn> void dfs(Fn fn) {
    for (auto b : unit.roots) dfs(b, fn);
  }
  explicit PostorderWalker(Vunit& u)
    : unit(u)
    , visited(u.blocks.size())
  {}
  Vunit& unit;
  boost::dynamic_bitset<> visited;
};

extern const char* vinst_names[];
bool isBlockEnd(Vinstr& inst);
std::string format(Vreg);
bool check(Vunit&);

// search for the phidef in block b, then return its dest tuple
Vtuple findDefs(const Vunit& unit, Vlabel b);

typedef jit::vector<jit::vector<Vlabel>> PredVector;
PredVector computePreds(Vunit& unit);

}

}}
#endif
