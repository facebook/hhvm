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

#include "hphp/util/safe-cast.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////
// Vreg casting.

inline Vreg::Vreg(PhysReg r) {
  rn = (r == InvalidReg)  ? kInvalidReg :
       r.isGP()           ? G0+int(Reg64(r)) :
       r.isSIMD()         ? X0+int(RegXMM(r)) :
       /* r.isSF() ? */     S0+int(RegSF(r));
}

inline Vreg::operator Reg64() const {
  assertx(isGP());
  return Reg64(rn - G0);
}
inline Vreg::operator RegXMM() const {
  assertx(isSIMD());
  return RegXMM(rn - X0);
}
inline Vreg::operator RegSF() const {
  assertx(isSF());
  return RegSF(rn - S0);
}

inline PhysReg Vreg::physReg() const {
  assertx(!isValid() || isPhys());
  return !isValid() ? InvalidReg :
         isGP() ? PhysReg(/* implicit */operator Reg64()) :
         isSIMD() ? PhysReg(/* implicit */operator RegXMM()) :
         /* isSF() ? */ PhysReg(/* implicit */operator RegSF());
}

///////////////////////////////////////////////////////////////////////////////
// Vreg addressing.

inline Vptr Vreg::operator[](int disp) const {
  return Vptr(*this, disp);
}
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
inline Vptr Vreg::operator[](Vscaled si) const {
  return Vptr(*this, si.index, si.scale, 0);
}
inline Vptr Vreg::operator[](VscaledDisp vd) const {
  return Vptr(*this, vd.vs.index, vd.vs.scale, vd.disp);
}
inline Vptr Vreg::operator[](Vreg index) const {
  return Vptr(*this, index, 1, 0);
}

inline Vptr Vreg::operator*() const {
  return Vptr(*this, 0);
}
inline Vscaled Vreg::operator*(int scale) const {
  return Vscaled{*this, scale};
}

inline Vptr Vreg::operator+(size_t d) const {
  return Vptr(*this, safe_cast<int32_t>(d));
}
inline Vptr Vreg::operator+(int32_t d) const {
  return Vptr(*this, safe_cast<int32_t>(d));
}
inline Vptr Vreg::operator+(intptr_t d) const {
  return Vptr(*this, safe_cast<int32_t>(d));
}

///////////////////////////////////////////////////////////////////////////////
// Vr casting.

inline bool allowable(Vreg64*, Vreg r) { return r.isVirt() || r.isGP(); }
inline bool allowable(Vreg32*, Vreg r) { return r.isVirt() || r.isGP(); }
inline bool allowable(Vreg16*, Vreg r) { return r.isVirt() || r.isGP(); }
inline bool allowable(Vreg8*, Vreg r) { return r.isVirt() || r.isGP(); }
inline bool allowable(VregSF*, Vreg r) { return r.isVirt() || r.isSF(); }
inline bool allowable(Vr<RegXMM>*, Vreg r) { return r.isVirt() || r.isSIMD(); }

template<class Reg>
/* implicit */ Vr<Reg>::Vr(Vreg r)
  : rn(size_t(r)) {
  assertx(allowable(this, r) || !r.isValid());
}

template<class Reg>
inline Reg Vr<Reg>::asReg() const {
  assertx(isPhys());
  return isGP()         ? Reg(rn) :
         isSIMD()       ? Reg(rn-Vreg::X0) :
         /* isSF() ? */   Reg(rn-Vreg::S0);
}

///////////////////////////////////////////////////////////////////////////////
// Vr addressing.

template<class Reg>
Vptr Vr<Reg>::operator[](int disp) const {
  return Vptr(*this, disp);
}
template<class Reg>
Vptr Vr<Reg>::operator[](ScaledIndex si) const {
  return Vptr(*this, si.index, si.scale, 0);
}
template<class Reg>
Vptr Vr<Reg>::operator[](ScaledIndexDisp sid) const {
  return Vptr(*this, sid.si.index, sid.si.scale, sid.disp);
}
template<class Reg>
Vptr Vr<Reg>::operator[](Vptr p) const {
  return Vptr(*this, p.base, 1, p.disp);
}
template<class Reg>
Vptr Vr<Reg>::operator[](DispReg rd) const {
  return Vptr(*this, rd.base, 1, rd.disp);
}

template<class Reg>
Vptr Vr<Reg>::operator*() const {
  return Vptr(*this, 0);
}

template<class Reg>
inline Vptr Vr<Reg>::operator+(size_t d) const {
  return Vptr(*this, safe_cast<int32_t>(d));
}
template<class Reg>
inline Vptr Vr<Reg>::operator+(intptr_t d) const {
  return Vptr(*this, safe_cast<int32_t>(d));
}

///////////////////////////////////////////////////////////////////////////////
// Vscaled/VscaledDisp

inline VscaledDisp operator+(Vscaled vs, int32_t disp) {
  return {vs, disp};
}

///////////////////////////////////////////////////////////////////////////////
// Vptr.

inline MemoryRef Vptr::mr() const {
  if (index.isValid()) {
    return base.isValid() ? r64(base)[r64(index) * scale + disp] :
           *(IndexedDispReg{r64(index) * scale + disp});
  } else {
    return base.isValid() ? r64(base)[disp] :
           *(DispReg{disp});
  }
}

inline Vptr::operator MemoryRef() const {
  assertx(seg == DS);
  return mr();
}

inline bool Vptr::operator==(const Vptr& other) const {
  return base == other.base &&
         index == other.index &&
         (!index.isValid() || scale == other.scale) &&
         seg == other.seg &&
         disp == other.disp;
}

inline bool Vptr::operator!=(const Vptr& other) const {
  return !(*this == other);
}

inline Vptr operator+(Vptr lhs, int32_t d) {
  return Vptr(lhs.base, lhs.index, lhs.scale, lhs.disp + d);
}

inline Vptr operator+(Vptr lhs, intptr_t d) {
  return Vptr(lhs.base, lhs.index, lhs.scale,
              safe_cast<int32_t>(lhs.disp + d));
}

///////////////////////////////////////////////////////////////////////////////
// Vloc.

inline bool Vloc::hasReg(int i) const {
  return m_regs[i].isValid();
}

inline Vreg Vloc::reg(int i) const {
  return m_regs[i];
}

inline int Vloc::numAllocated() const {
  return int(m_regs[0].isValid()) + int(m_regs[1].isValid());
}

inline int Vloc::numWords() const {
  return m_kind == kWide ? 2 : numAllocated();
}

inline bool Vloc::isFullSIMD() const {
  return m_kind == kWide;
}

inline bool Vloc::operator==(Vloc other) const {
  return m_kind == other.m_kind &&
         m_regs[0] == other.m_regs[0] &&
         m_regs[1] == other.m_regs[1];
}

inline bool Vloc::operator!=(Vloc other) const {
  return !(*this == other);
}

///////////////////////////////////////////////////////////////////////////////
}}
