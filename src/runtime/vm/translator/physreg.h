/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#ifndef incl_HPHP_VM_PHYSREG_H_
#define incl_HPHP_VM_PHYSREG_H_

#include "util/asm-x64.h"
#include "util/bitops.h"

namespace HPHP { namespace VM { namespace Transl {

//////////////////////////////////////////////////////////////////////

/*
 * PhysReg represents a physical machine register.  (Currently it only
 * knows how to do GPRs.)
 *
 * To make it possible to use it with the assembler conveniently, it
 * can be implicitly converted to and from Reg64.  If you want to use
 * it as a 32 bit register call r32(physReg).
 *
 * The implicit conversion to RegNumber is historical: it exists
 * for backward-compatability with the old-style asm-x64.h api
 * (e.g. store_reg##_disp_reg##).
 */
struct PhysReg {
  explicit constexpr PhysReg(int n = -1) : n(n) {}
  /* implicit */ constexpr PhysReg(Reg64 r) : n(int(r)) {}
  explicit constexpr PhysReg(Reg32 r) : n(int(RegNumber(r))) {}

  explicit constexpr PhysReg(RegNumber r) : n(int(r)) {}

  constexpr operator Reg64() const { return Reg64(n); }
  constexpr operator RegNumber() const { return RegNumber(n); }

  explicit constexpr operator int() const { return n; }
  constexpr bool operator==(PhysReg r) const { return n == r.n; }
  constexpr bool operator!=(PhysReg r) const { return n != r.n; }
  constexpr bool operator==(Reg64 r) const { return Reg64(n) == r; }
  constexpr bool operator!=(Reg64 r) const { return Reg64(n) != r; }
  constexpr bool operator==(Reg32 r) const { return Reg32(n) == r; }
  constexpr bool operator!=(Reg32 r) const { return Reg32(n) != r; }

  MemoryRef operator[](intptr_t p) const { return *(*this + p); }
  IndexedMemoryRef operator[](Reg64 i) const { return *(*this + i); }
  IndexedMemoryRef operator[](ScaledIndex s) const { return *(*this + s); }
  IndexedMemoryRef operator[](ScaledIndexDisp s) const {
    return *(*this + s.si + s.disp);
  }
  IndexedMemoryRef operator[](DispReg dr) const {
    return *(*this + ScaledIndex(dr.base, 0x1) + dr.disp);
  }

private:
  int n;
};

constexpr PhysReg InvalidReg;

/*
 * A set of registers.
 *
 * This type is guaranteed to be a standard layout class with a
 * trivial destructor.  (This makes it usable in classes that are
 * arena-allocated.)
 *
 * Zero-initializing this class is guaranteed to produce an empty set.
 */
struct RegSet {
  explicit RegSet() : m_bits(0) {}
  explicit RegSet(PhysReg pr) : m_bits(1 << int(pr)) {}

  // Union
  RegSet operator|(const RegSet& rhs) const {
    RegSet retval;
    retval.m_bits = m_bits | rhs.m_bits;
    return retval;
  }

  RegSet& operator|=(const RegSet& rhs) {
    m_bits |= rhs.m_bits;
    return *this;
  }

  // Intersection
  RegSet operator&(const RegSet& rhs) const {
    RegSet retval;
    retval.m_bits = m_bits & rhs.m_bits;
    return retval;
  }

  RegSet& operator&=(const RegSet& rhs) {
    m_bits &= rhs.m_bits;
    return *this;
  }

  // Equality
  bool operator==(const RegSet& rhs) const {
    return m_bits == rhs.m_bits;
  }

  bool operator!=(const RegSet& rhs) const {
    return !(*this == rhs);
  }

  // Difference
  RegSet operator-(const RegSet& rhs) const {
    RegSet retval;
    retval.m_bits = m_bits & ~rhs.m_bits;
    return retval;
  }

  RegSet& operator-=(const RegSet& rhs) {
    *this = *this - rhs;
    return *this;
  }

  void clear() {
    m_bits = 0;
  }

  int size() const {
    return __builtin_popcount(m_bits);
  }

  RegSet& add(PhysReg pr) {
    *this |= RegSet(pr);
    return *this;
  }

  RegSet& remove(PhysReg pr) {
    m_bits = m_bits & ~(1 << int(pr));
    return *this;
  }

  bool contains(PhysReg pr) const {
    return bool(m_bits & (1 << int(pr)));
  }

  bool empty() const {
    return m_bits == 0;
  }

  /*
   * Can be used for iterating over present registers, in forward or
   * backward order:
   *
   *   while (regSet.findFirst(reg)) {
   *     regSet.remove(reg);
   *     foo(reg);
   *   }
   *
   *   while (regSet.findLast(reg)) {
   *     regSet.remove(reg);
   *     foo(reg);
   *   }
   *
   * Consider forEach if you don't want to mutate your register set.
   */
  bool findFirst(PhysReg& reg) {
    uint64_t out;
    bool retval = ffs64(m_bits, out);
    reg = PhysReg(out);
    ASSERT(!retval || (int(reg) >= 0 && int(reg) < 64));
    return retval;
  }

  bool findLast(PhysReg& reg) {
    uint64_t out;
    bool retval = fls64(m_bits, out);
    reg = PhysReg(out);
    ASSERT(!retval || (int(reg) >= 0 && int(reg) < 64));
    return retval;
  }

  /*
   * Do something for each register in the set, in either forward or
   * reverse order.
   */

  template<class Fun>
  void forEach(Fun f) const {
    RegSet cpy = *this;
    PhysReg r;
    while (cpy.findFirst(r)) {
      cpy.remove(r);
      f(r);
    }
  }

  template<class Fun>
  void forEachR(Fun f) const {
    RegSet cpy = *this;
    PhysReg r;
    while (cpy.findLast(r)) {
      cpy.remove(r);
      f(r);
    }
  }

private:
  uint64_t m_bits;
};

static_assert(std::has_trivial_destructor<RegSet>::value,
              "RegSet must have a trivial destructor");

//////////////////////////////////////////////////////////////////////

template<int StackParity>
struct PhysRegSaverParity : private boost::noncopyable {
  PhysRegSaverParity(X64Assembler& a_, RegSet s_) : a(a_), s(s_) {
    s.forEach([&] (PhysReg pr) {
      a.    push   (pr);
    });
    if ((s.size() & 1) == StackParity) {
      // Maintain stack evenness for SIMD compatibility.
      a.    subq   (8, reg::rsp);
    }
  }

  ~PhysRegSaverParity() {
    if ((s.size() & 1) == StackParity) {
      // See above; stack parity.
      a.    addq   (8, reg::rsp);
    }
    s.forEachR([&] (PhysReg pr) {
      a.    pop    (pr);
    });
  }

  int rspAdjustment() const {
    return s.size() + ((s.size() & 1) == StackParity);
  }

private:
  X64Assembler& a;
  RegSet s;
};

//////////////////////////////////////////////////////////////////////

}}}

#endif

