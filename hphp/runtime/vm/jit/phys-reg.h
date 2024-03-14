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

#pragma once

#include "hphp/util/asm-x64.h"
#include "hphp/util/bitops.h"
#include "hphp/vixl/a64/assembler-a64.h"

#include <type_traits>

namespace HPHP::jit {

struct Vreg;
struct Vout;

///////////////////////////////////////////////////////////////////////////////

/*
 * PhysReg represents a physical machine register.
 *
 * To make it possible to use it with the assembler conveniently, it can be
 * implicitly converted to and from Reg64.  To use a PhysReg `r' as a 32-bit
 * register, call r32(r).
 */
struct PhysReg {
private:
  static constexpr auto kGPOffset = 0;
  static constexpr auto kNumGP = 48;

  static constexpr auto kSIMDOffset = kNumGP;
  static constexpr auto kNumSIMD = 64;

  static constexpr auto kSFOffset = kNumGP + kNumSIMD;
  static constexpr auto kNumSF = 1;

public:
  /*
   * 48 GP regs + 64 SIMD regs + 1 SF reg + 15 empty == 128 regs.
   *
   * We can toggle these values, but they need sum to 128 so that RegSet can
   * fit into two registers.
   */
  static constexpr auto kMaxRegs = 128;

  enum Type {
    GP,
    SIMD,
    SF,
  };
  explicit constexpr PhysReg() : n(0xff) {}
  constexpr /* implicit */ PhysReg(Reg64 r) : n(int(r)) {}
  constexpr /* implicit */ PhysReg(RegXMM r) : n(int(r) + kSIMDOffset) {}
  constexpr /* implicit */ PhysReg(RegSF r) : n(int(r) + kSFOffset) {}
  explicit constexpr PhysReg(Reg32 r) : n(int(r)) {}
  explicit constexpr PhysReg(Reg16 r) : n(int(r)) {}
  explicit constexpr PhysReg(Reg8 r) : n(int(r)) {}

  constexpr /* implicit */ PhysReg(vixl::Register r) : n(r.code()) {}
  constexpr /* implicit */ PhysReg(vixl::FPRegister r)
    : n(r.code() + kSIMDOffset) {}

  /* implicit */ operator Reg64() const {
    assertx(isGP() || n == 0xff);
    return Reg64(n);
  }
  /* implicit */ operator RegXMM() const {
    assertx(isSIMD() || n == 0xff);
    return RegXMM(n - kSIMDOffset);
  }
  /* implicit */ operator RegSF() const {
    assertx(isSF() || n == 0xff);
    return RegSF(n - kSFOffset);
  }

  /* implicit */ operator vixl::CPURegister() const {
    if (n == 0xff) {
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
    assertx(n < kMaxRegs);
    return isGP() ? GP :
           isSIMD() ? SIMD :
           /* isSF() ? */ SF;
  }
  bool isGP() const {
    static_assert(kGPOffset == 0, "kGPOffset is expected to be zero.");
    return n < kGPOffset+numGP();
  }
  bool isSIMD() const { return n >= kSIMDOffset && n < kSIMDOffset+numSIMD(); }
  bool isSF() const { return n >= kSFOffset && n < kSFOffset+kNumSF; }
  constexpr bool operator==(PhysReg r) const { return n == r.n; }
  constexpr bool operator!=(PhysReg r) const { return n != r.n; }
  constexpr bool operator==(Reg64 r) const { return Reg64(n) == r; }
  constexpr bool operator!=(Reg64 r) const { return Reg64(n) != r; }
  constexpr bool operator==(Reg32 r) const { return Reg32(n) == r; }
  constexpr bool operator!=(Reg32 r) const { return Reg32(n) != r; }
  constexpr bool operator==(RegXMM r) const { return n == PhysReg(r).n; }
  constexpr bool operator!=(RegXMM r) const { return n != PhysReg(r).n; }

  size_t hash() const { return n; }

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
    static int kNumGP_2 = getNumGP();
    return kNumGP_2;
  }
  static int numSIMD() {
    static int kNumSIMD_2 = getNumSIMD();
    return kNumSIMD_2;
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
    Map() {
      // Workaround for a potential GCC 5 bug, value initializing m_elms seems
      // to use zero-initialization instead of default initialization.
      for (auto& elm : m_elms) {
        elm = T();
      }
    }

    T& operator[](const PhysReg& r) {
      assertx(r.n != 0xff);
      return m_elms[r.n];
    }

    const T& operator[](const PhysReg& r) const {
      assertx(r.n != 0xff);
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
  friend struct VregSet;
  explicit constexpr PhysReg(int n) : n(n) {}

  uint8_t n;
};

inline Reg8 rbyte(PhysReg r) { return Reg8(int(Reg64(r))); }
inline Reg16 r16(PhysReg r) { return Reg16(int(Reg64(r))); }
inline Reg32 r32(PhysReg r) { return Reg32(int(Reg64(r))); }
inline Reg64 r64(PhysReg r) { return Reg64(r); }

constexpr PhysReg InvalidReg;

std::string show(PhysReg r);

///////////////////////////////////////////////////////////////////////////////

/*
 * A set of registers.
 *
 * This type is guaranteed to be a standard layout class with a trivial
 * destructor.  (This makes it usable in classes that are arena-allocated.)
 *
 * Zero-initializing this class is guaranteed to produce an empty set.
 */
struct RegSet {
  constexpr RegSet() : m_lo(0), m_hi(0) {}

  explicit RegSet(PhysReg r)
    : m_lo(r.n < 64 ? uint64_t{1} << r.n : 0)
    , m_hi(r.n >= 64 && r.n < 128 ? uint64_t{1} << (r.n - 64) : 0)
  {
    static_assert(std::is_unsigned<decltype(r.n)>::value, "");
  }

private:
  explicit RegSet(uint64_t lo, uint64_t hi) : m_lo(lo), m_hi(hi) {}

  /////////////////////////////////////////////////////////////////////////////
  // Operators.

public:
  /*
   * Equality.
   */
  bool operator==(const RegSet& rhs) const {
    return m_lo == rhs.m_lo &&
           m_hi == rhs.m_hi;
  }
  bool operator!=(const RegSet& rhs) const {
    return !(*this == rhs);
  }

  /*
   * Union.
   */
  RegSet& operator|=(const RegSet& rhs) {
    m_lo |= rhs.m_lo;
    m_hi |= rhs.m_hi;
    return *this;
  }
  RegSet& operator|=(PhysReg r) {
    *this |= RegSet(r);
    return *this;
  }

  RegSet operator|(const RegSet& rhs) const {
    auto copy = *this;
    return copy |= rhs;
  }
  RegSet operator|(PhysReg r) const {
    auto copy = *this;
    return copy |= r;
  }

  /*
   * Intersection.
   */
  RegSet& operator&=(const RegSet& rhs) {
    m_lo &= rhs.m_lo;
    m_hi &= rhs.m_hi;
    return *this;
  }
  RegSet operator&(const RegSet& rhs) const {
    auto copy = *this;
    return copy &= rhs;
  }

  /*
   * Difference.
   */
  RegSet& operator-=(const RegSet& rhs) {
    m_lo &= ~rhs.m_lo;
    m_hi &= ~rhs.m_hi;
    return *this;
  }
  RegSet& operator-=(PhysReg r) {
    *this -= RegSet(r);
    return *this;
  }

  RegSet operator-(const RegSet& rhs) const {
    auto copy = *this;
    return copy -= rhs;
  }
  RegSet operator-(PhysReg r) const {
    auto copy = *this;
    return copy -= r;
  }

  /////////////////////////////////////////////////////////////////////////////

  void clear() { m_lo = m_hi = 0; }

  int size() const {
    return __builtin_popcountll(m_lo) +
           __builtin_popcountll(m_hi);
  }
  bool empty() const { return m_lo == 0 && m_hi == 0; }

  bool contains(PhysReg r) const {
    return !(*this & RegSet(r)).empty();
  }

  /*
   * Iterate through the registers in the set.
   *
   * The only ordering guarantee is that forEachR() iterates in the reverse
   * order that forEach() iterates in.
   */
  template<class Fun>
  void forEach(Fun f) const {
    uint64_t out;

    auto const go = [&] (uint64_t& bits, off_t off) {
      while (ffs64(bits, out)) {
        assertx(0 <= out && out < 64);
        bits &= ~(uint64_t{1} << out);
        f(PhysReg(out + off));
      }
    };

    // Low to high.
    auto copy = *this;
    go(copy.m_lo, 0);
    go(copy.m_hi, 64);
  }

  template<class Fun>
  void forEachPair(Fun f) const {
    uint64_t out;
    uint8_t r[2];
    uint8_t i = 0;

    auto const go = [&] (uint64_t& bits, off_t off) {
      while (ffs64(bits, out)) {
        assertx(0 <= out && out < 64);
        bits &= ~(uint64_t{1} << out);
        r[i++] = out + off;
        if (i > 1) {
          f(PhysReg(r[0]), PhysReg(r[1]));
          i = 0;
        }
      }
    };

    // Low to high.
    auto copy = *this;
    go(copy.m_lo, 0);
    go(copy.m_hi, 64);
    if (i > 0) f(PhysReg(r[0]), InvalidReg);
  }

  template<class Fun>
  void forEachR(Fun f) const {
    uint64_t out;

    auto const go = [&] (uint64_t& bits, off_t off) {
      while (fls64(bits, out)) {
        assertx(0 <= out && out < 64);
        bits &= ~(uint64_t{1} << out);
        f(PhysReg(out + off));
      }
    };

    // High to low.
    auto copy = *this;
    go(copy.m_hi, 64);
    go(copy.m_lo, 0);
  }

  template<class Fun>
  void forEachPairR(Fun f) const {
    uint64_t out;
    uint8_t r[2];
    uint8_t i = 0;
    bool adjust;

    auto const go = [&] (uint64_t& bits, off_t off) {
      while (fls64(bits, out)) {
        assertx(0 <= out && out < 64);
        bits &= ~(uint64_t{1} << out);
        r[i++] = out + off;
        if (i > 1) {
          f(PhysReg(r[0]), PhysReg(r[1]));
          i = 0;
        }
        if (adjust) {
          assertx(i == 1);
          f(InvalidReg, PhysReg(r[0]));
          adjust = false;
          i = 0;
        }
      }
    };

    // High to low.
    auto copy = *this;
    adjust = (folly::popcount(copy.m_hi) +
              folly::popcount(copy.m_lo)) & 0x1;
    go(copy.m_hi, 64);
    go(copy.m_lo, 0);
  }

  /*
   * Return a register in the set, or InvalidReg if the set is empty.
   *
   * As implemented, this just chooses the highest-numbered PhysReg, since it's
   * only ever used to choose an SF register.
   */
  PhysReg choose() const {
    uint64_t out;
    if (fls64(m_hi, out)) return PhysReg(out + 64);
    if (fls64(m_lo, out)) return PhysReg(out);
    return InvalidReg;
  }

  /////////////////////////////////////////////////////////////////////////////

private:
  uint64_t m_lo;
  uint64_t m_hi;
};

inline RegSet operator|(PhysReg r1, PhysReg r2) {
  return RegSet(r1) | r2;
}

std::string show(RegSet regs);

static_assert(std::is_trivially_destructible<RegSet>::value,
              "RegSet must have a trivial destructor");

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

namespace std {
  template<> struct hash<HPHP::jit::PhysReg> {
    size_t operator()(HPHP::jit::PhysReg r) const { return r.hash(); }
  };
}

///////////////////////////////////////////////////////////////////////////////
