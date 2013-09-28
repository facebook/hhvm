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
#ifndef incl_HPHP_VM_PHYSREG_H_
#define incl_HPHP_VM_PHYSREG_H_

#include "hphp/util/asm-x64.h"
#include "hphp/util/bitops.h"

namespace HPHP { namespace Transl {

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
  enum Type {
    GP,
    XMM,
    kNumTypes,  // keep last
  };
  explicit constexpr PhysReg(int n = -1) : n(n) {}
  constexpr /* implicit */ PhysReg(Reg64 r) : n(int(r)) {}
  constexpr /* implicit */ PhysReg(RegXMM r) : n(int(r) + kNumGPRegs) {}
  explicit constexpr PhysReg(Reg32 r) : n(int(RegNumber(r))) {}

  explicit constexpr PhysReg(RegNumber r) : n(int(r)) {}

  /* implicit */ operator Reg64() const {
    assert(isGP() || n == -1);
    return Reg64(n);
  }
  constexpr /* implicit */ operator RegNumber() const {
    return n < kNumGPRegs ? RegNumber(n) : RegNumber(n - kNumGPRegs);
  }
  /* implicit */ operator RegXMM() const {
    assert(isXMM() || n == -1);
    return RegXMM(n - kNumGPRegs);
  }

  Type type() const {
    assert(n >= 0 && n < kNumRegs);
    return n < kNumGPRegs ? GP : XMM;
  }
  bool isGP () const { return n >= 0 && n < kNumGPRegs; }
  bool isXMM() const { return n >= kNumGPRegs && n < kNumRegs; }
  explicit constexpr operator int() const { return n; }
  constexpr bool operator==(PhysReg r) const { return n == r.n; }
  constexpr bool operator!=(PhysReg r) const { return n != r.n; }
  constexpr bool operator==(Reg64 r) const { return Reg64(n) == r; }
  constexpr bool operator!=(Reg64 r) const { return Reg64(n) != r; }
  constexpr bool operator==(Reg32 r) const { return Reg32(n) == r; }
  constexpr bool operator!=(Reg32 r) const { return Reg32(n) != r; }

  MemoryRef operator[](intptr_t p) const {
    assert(type() == GP);
    return *(*this + p);
  }
  IndexedMemoryRef operator[](Reg64 i) const {
    assert(type() == GP);
    return *(*this + i);
  }
  IndexedMemoryRef operator[](ScaledIndex s) const {
    assert(type() == GP);
    return *(*this + s);
  }
  IndexedMemoryRef operator[](ScaledIndexDisp s) const {
    assert(type() == GP);
    return *(*this + s.si + s.disp);
  }
  IndexedMemoryRef operator[](DispReg dr) const {
    assert(type() == GP);
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
    if (pr != InvalidReg) {
      *this |= RegSet(pr);
    }
    return *this;
  }

  RegSet& remove(PhysReg pr) {
    if (pr != InvalidReg) {
      m_bits = m_bits & ~(1 << int(pr));
    }
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
    assert(!retval || (int(reg) >= 0 && int(reg) < 64));
    return retval;
  }

  bool findLast(PhysReg& reg) {
    uint64_t out;
    bool retval = fls64(m_bits, out);
    reg = PhysReg(out);
    assert(!retval || (int(reg) >= 0 && int(reg) < 64));
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

static_assert(boost::has_trivial_destructor<RegSet>::value,
              "RegSet must have a trivial destructor");

//////////////////////////////////////////////////////////////////////

struct PhysRegSaverParity {
  PhysRegSaverParity(int parity, X64Assembler& as, RegSet regs);
  ~PhysRegSaverParity();

  static void emitPops(X64Assembler& as, RegSet regs);

  PhysRegSaverParity(const PhysRegSaverParity&) = delete;
  PhysRegSaverParity(PhysRegSaverParity&&) = default;
  PhysRegSaverParity& operator=(const PhysRegSaverParity&) = delete;
  PhysRegSaverParity& operator=(PhysRegSaverParity&&) = default;

  int rspAdjustment() const;
  int rspTotalAdjustmentRegs() const;
  void bytesPushed(int64_t bytes);

private:
  X64Assembler& m_as;
  RegSet m_regs;
  int64_t m_adjust;
};

struct PhysRegSaverStub : public PhysRegSaverParity {
  PhysRegSaverStub(X64Assembler& as, RegSet regs)
      : PhysRegSaverParity(0, as, regs)
  {}
};

struct PhysRegSaver : public PhysRegSaverParity {
  PhysRegSaver(X64Assembler& as, RegSet regs)
      : PhysRegSaverParity(1, as, regs)
  {}
};

//////////////////////////////////////////////////////////////////////

}}

#endif
