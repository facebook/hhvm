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
#ifndef incl_HPHP_VM_PHYSREG_H_
#define incl_HPHP_VM_PHYSREG_H_

#include "hphp/util/asm-x64.h"
#include "hphp/util/bitops.h"
#include "hphp/vixl/a64/assembler-a64.h"

#include <folly/Optional.h>

namespace HPHP { namespace jit {
struct Vreg;
struct Vout;

//////////////////////////////////////////////////////////////////////

/*
 * PhysReg represents a physical machine register.
 *
 * To make it possible to use it with the assembler conveniently, it
 * can be implicitly converted to and from Reg64.  If you want to use
 * it as a 32 bit register call r32(physReg).
 */
struct PhysReg {
 private:
  static constexpr auto kMaxRegs = 64;
  static constexpr auto kGPOffset = 0;
  static constexpr auto kSIMDOffset = 33; // ARM has 33 GP registers.
  static constexpr auto kSFOffset = 63;
  static constexpr auto kNumSF = 1;

 public:
  enum Type {
    GP,
    SIMD,
    SF,
  };
  explicit constexpr PhysReg() : n(-1) {}
  constexpr /* implicit */ PhysReg(Reg64 r) : n(int(r)) {}
  constexpr /* implicit */ PhysReg(RegXMM r) : n(int(r) + kSIMDOffset) {}
  constexpr /* implicit */ PhysReg(RegSF r) : n(int(r) + kSFOffset) {}
  explicit constexpr PhysReg(Reg32 r) : n(int(r)) {}
  explicit constexpr PhysReg(Reg8 r) : n(int(r)) {}

  constexpr /* implicit */ PhysReg(vixl::Register r) : n(r.code()) {}
  constexpr /* implicit */ PhysReg(vixl::FPRegister r)
    : n(r.code() + kSIMDOffset) {}

  /* implicit */ operator Reg64() const {
    assertx(isGP() || n == -1);
    return Reg64(n);
  }
  /* implicit */ operator RegXMM() const {
    assertx(isSIMD() || n == -1);
    return RegXMM(n - kSIMDOffset);
  }
  /* implicit */ operator RegSF() const {
    assertx(isSF() || n == -1);
    return RegSF(n - kSFOffset);
  }

  /* implicit */ operator vixl::CPURegister() const {
    if (n == -1) {
      return vixl::NoCPUReg;
    } else {
      if (isGP()) {
        return vixl::CPURegister(n, vixl::kXRegSize,
                                 vixl::CPURegister::kRegister);
      } else if (isSIMD()) {
        return vixl::CPURegister(n - kSIMDOffset, vixl::kDRegSize,
                                 vixl::CPURegister::kFPRegister);
      } else {
        assertx(isSF());
        return vixl::NoCPUReg;
      }
    }
  }

  Type type() const {
    assertx(n >= 0 && n < kMaxRegs);
    return isGP() ? GP :
           isSIMD() ? SIMD :
           /* isSF() ? */ SF;
  }
  bool isGP() const { return n >= kGPOffset && n < kGPOffset+numGP(); }
  bool isSIMD() const { return n >= kSIMDOffset && n < kSIMDOffset+numSIMD(); }
  bool isSF() const { return n >= kSFOffset && n < kSFOffset+kNumSF; }
  constexpr bool operator==(PhysReg r) const { return n == r.n; }
  constexpr bool operator!=(PhysReg r) const { return n != r.n; }
  constexpr bool operator==(Reg64 r) const { return Reg64(n) == r; }
  constexpr bool operator!=(Reg64 r) const { return Reg64(n) != r; }
  constexpr bool operator==(Reg32 r) const { return Reg32(n) == r; }
  constexpr bool operator!=(Reg32 r) const { return Reg32(n) != r; }

  MemoryRef operator[](intptr_t p) const {
    assertx(type() == GP);
    return *(*this + p);
  }
  MemoryRef operator[](Reg64 i) const {
    assertx(type() == GP);
    return *(*this + i);
  }
  MemoryRef operator[](ScaledIndex s) const {
    assertx(type() == GP);
    return *(*this + s);
  }
  MemoryRef operator[](ScaledIndexDisp s) const {
    assertx(type() == GP);
    return *(*this + s.si + s.disp);
  }
  MemoryRef operator[](DispReg dr) const {
    assertx(type() == GP);
    return *(*this + ScaledIndex(dr.base, 0x1) + dr.disp);
  }

  static int getNumGP();
  static int getNumSIMD();
  static int numGP() {
    static int kNumGP = getNumGP();
    return kNumGP;
  }
  static int numSIMD() {
    static int kNumSIMD = getNumSIMD();
    return kNumSIMD;
  }

  /*
   * This struct can be used to efficiently represent a map from PhysReg to T.
   * Note that the semantics are that all keys are present at all times. There
   * is no such thing as adding to or removing from the map; all registers map
   * to a value -- initially, a default-constructed T.
   *
   * The purpose is to allow the use of PhysReg's convenient internal encoding
   * to be memory-efficient, without letting that abstraction leak.
   */
  template<typename T>
  struct Map {
    Map() : m_elms() {
    }

    T& operator[](const PhysReg& r) {
      assertx(r.n != -1);
      return m_elms[r.n];
    }

    const T& operator[](const PhysReg& r) const {
      assertx(r.n != -1);
      return m_elms[r.n];
    }

    bool operator==(const Map& other) const {
      for (auto reg : *this) {
        if ((*this)[reg] != other[reg]) return false;
      }
      return true;
    }
    bool operator!=(const Map& other) const { return !operator==(other); }

    struct iterator {
      PhysReg operator*() const {
        return PhysReg{idx};
      }

      bool operator!=(const iterator& other) const {
        return idx != other.idx;
      }

      iterator& operator++() {
        idx++;
        if (idx == kGPOffset + numGP() && idx < kSIMDOffset) {
          idx = kSIMDOffset;
        } else if (idx == kSIMDOffset + numSIMD() && idx < kSFOffset) {
          idx = kSFOffset;
        } else if (idx == kSFOffset + kNumSF && idx < kMaxRegs) {
          idx = kMaxRegs;
        }
        return *this;
      }

      const T* start;
      int idx;
    };

    iterator begin() const {
      return { m_elms, 0 };
    }

    iterator end() const {
      return { m_elms, sizeof(m_elms) / sizeof(m_elms[0]) };
    }

   private:
    T m_elms[kMaxRegs];
  };

private:
  friend struct RegSet;
  friend struct Vreg;
  explicit constexpr PhysReg(int n) : n(n) {}

  int8_t n;
};

inline Reg8 rbyte(PhysReg r) { return Reg8(int(Reg64(r))); }
inline Reg16 r16(PhysReg r) { return Reg16(int(Reg64(r))); }
inline Reg32 r32(PhysReg r) { return Reg32(int(Reg64(r))); }
inline Reg64 r64(PhysReg r) { return Reg64(r); }

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
  RegSet() : m_bits(0) {}
  explicit RegSet(PhysReg pr) : m_bits(uint64_t(1) << pr.n) {}

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

  RegSet& operator|=(PhysReg r) {
    return add(r);
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
    return __builtin_popcountll(m_bits);
  }

  RegSet& add(PhysReg pr) {
    if (pr != InvalidReg) {
      *this |= RegSet(pr);
    }
    return *this;
  }

  RegSet& remove(PhysReg pr) {
    if (pr != InvalidReg) {
      m_bits = m_bits & ~(uint64_t(1) << pr.n);
    }
    return *this;
  }

  bool contains(PhysReg pr) const {
    return bool(m_bits & (uint64_t(1) << pr.n));
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
    assertx(!retval || (reg.n >= 0 && reg.n < 64));
    return retval;
  }

  PhysReg findFirst() const {
    uint64_t out;
    if (ffs64(m_bits, out)) {
      return PhysReg(out);
    }
    return InvalidReg;
  }

  bool findLast(PhysReg& reg) {
    uint64_t out;
    bool retval = fls64(m_bits, out);
    reg = PhysReg(out);
    assertx(!retval || (reg.n >= 0 && reg.n < 64));
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
  static_assert(sizeof(decltype(m_bits)) * 8 >= PhysReg::kMaxRegs, "");
};

inline RegSet operator|(PhysReg r1, PhysReg r2) {
  return RegSet(r1).add(r2);
}

inline RegSet operator|(RegSet regs, PhysReg r) {
  return regs.add(r);
}

std::string show(RegSet regs);

static_assert(std::is_trivially_destructible<RegSet>::value,
              "RegSet must have a trivial destructor");

//////////////////////////////////////////////////////////////////////

struct PhysRegSaverParity {
  PhysRegSaverParity(int parity, X64Assembler& as, RegSet regs);
  PhysRegSaverParity(int parity, Vout& as, RegSet regs);
  ~PhysRegSaverParity();

  static void emitPops(X64Assembler& as, RegSet regs);
  static void emitPops(Vout&, RegSet regs);

  PhysRegSaverParity(const PhysRegSaverParity&) = delete;
  PhysRegSaverParity(PhysRegSaverParity&&) noexcept = default;
  PhysRegSaverParity& operator=(const PhysRegSaverParity&) = delete;
  PhysRegSaverParity& operator=(PhysRegSaverParity&&) = default;

  int rspAdjustment() const;
  int dwordsPushed() const;
  void bytesPushed(int bytes);

private:
  X64Assembler* m_as;
  Vout* m_v;
  RegSet m_regs;
  int m_adjust;
};

struct PhysRegSaverStub : public PhysRegSaverParity {
  PhysRegSaverStub(X64Assembler& as, RegSet regs)
      : PhysRegSaverParity(0, as, regs)
  {}
  PhysRegSaverStub(Vout& v, RegSet regs)
      : PhysRegSaverParity(0, v, regs)
  {}
};

struct PhysRegSaver : public PhysRegSaverParity {
  PhysRegSaver(X64Assembler& as, RegSet regs)
      : PhysRegSaverParity(1, as, regs)
  {}
  PhysRegSaver(Vout& v, RegSet regs)
      : PhysRegSaverParity(1, v, regs)
  {}
};

//////////////////////////////////////////////////////////////////////

}}

#endif
