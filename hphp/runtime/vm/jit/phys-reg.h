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

#include "hphp/vixl/a64/assembler-a64.h"

#include "hphp/runtime/vm/jit/arch.h"

namespace HPHP { namespace JIT {

namespace X64 {
constexpr auto kNumGPRegs   = 16;
constexpr auto kNumSIMDRegs = 16;
constexpr auto kNumRegs     = kNumGPRegs + kNumSIMDRegs;
}

namespace ARM {
// ARM machines really only have 32 GP regs. However, vixl has 33 separate
// register codes, because it treats the zero register and stack pointer (which
// are really both register 31) separately. Rather than lose this distinction in
// vixl (it's really helpful for avoiding stupid mistakes), we sacrifice the
// ability to represent all 32 SIMD regs, and pretend that are 33 GP regs.
constexpr auto kNumGPRegs   = 33;
constexpr auto kNumSIMDRegs = 31;
constexpr auto kNumRegs     = kNumGPRegs + kNumSIMDRegs;
}

//////////////////////////////////////////////////////////////////////

/*
 * PhysReg represents a physical machine register.
 *
 * To make it possible to use it with the assembler conveniently, it
 * can be implicitly converted to and from Reg64.  If you want to use
 * it as a 32 bit register call r32(physReg).
 *
 * The implicit conversion to RegNumber is historical: it exists
 * for backward-compatibility with the old-style asm-x64.h api
 * (e.g. store_reg##_disp_reg##).
 */
struct PhysReg {
 private:
  static constexpr auto kMaxRegs = 64;
  static constexpr auto kSIMDOffset = 33;
  static_assert(kSIMDOffset >= X64::kNumGPRegs, "");
  static_assert(kSIMDOffset >= ARM::kNumGPRegs, "");
  static_assert(kMaxRegs - kSIMDOffset >= X64::kNumSIMDRegs, "");
  static_assert(kMaxRegs - kSIMDOffset >= ARM::kNumSIMDRegs, "");
  static_assert(kMaxRegs >= X64::kNumRegs, "");
  static_assert(kMaxRegs >= ARM::kNumRegs, "");

  // These are populated in Map's constructor, because they depend on a
  // RuntimeOption.
  static int kNumGP;
  static int kNumSIMD;

 public:
  enum Type {
    GP,
    SIMD,
    kNumTypes,  // keep last
  };
  explicit constexpr PhysReg() : n(-1) {}
  constexpr /* implicit */ PhysReg(Reg64 r) : n(int(r)) {}
  constexpr /* implicit */ PhysReg(RegXMM r) : n(int(r) + kSIMDOffset) {}
  explicit constexpr PhysReg(Reg32 r) : n(int(RegNumber(r))) {}

  constexpr /* implicit */ PhysReg(vixl::Register r) : n(r.code()) {}

  explicit constexpr PhysReg(RegNumber r) : n(int(r)) {}

  /* implicit */ operator Reg64() const {
    assert(isGP() || n == -1);
    return Reg64(n);
  }
  constexpr /* implicit */ operator RegNumber() const {
    return n < kSIMDOffset ? RegNumber(n) : RegNumber(n - kSIMDOffset);
  }
  /* implicit */ operator RegXMM() const {
    assert(isSIMD() || n == -1);
    return RegXMM(n - kSIMDOffset);
  }

  /* implicit */ operator vixl::CPURegister() const {
    if (n == -1) {
      return vixl::NoCPUReg;
    } else {
      if (isGP()) {
        return vixl::CPURegister(n, vixl::kXRegSize,
                                 vixl::CPURegister::kRegister);
      } else {
        return vixl::CPURegister(n - kSIMDOffset, vixl::kDRegSize,
                                 vixl::CPURegister::kFPRegister);
      }
    }
  }

  Type type() const {
    assert(n >= 0 && n < kMaxRegs);
    return n < kSIMDOffset ? GP : SIMD;
  }
  bool isGP () const { return n >= 0 && n < kSIMDOffset; }
  bool isSIMD() const { return n >= kSIMDOffset && n < kMaxRegs; }
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
      // These are used in operator++ to determine how to iterate. They're
      // initialized here because they depend on a RuntimeOption so they can't
      // be inited at static init time.
      if (kNumGP == 0 || kNumSIMD == 0) {
        if (JIT::arch() == JIT::Arch::X64) {
          kNumGP = X64::kNumGPRegs;
          kNumSIMD = X64::kNumSIMDRegs;
        } else if (JIT::arch() == JIT::Arch::ARM) {
          kNumGP = ARM::kNumGPRegs;
          kNumSIMD = ARM::kNumSIMDRegs;
        } else {
          not_implemented();
        }
      }
    }

    T& operator[](const PhysReg& r) {
      assert(r.n != -1);
      return m_elms[r.n];
    }

    const T& operator[](const PhysReg& r) const {
      assert(r.n != -1);
      return m_elms[r.n];
    }

    struct iterator {
      PhysReg operator*() const {
        return PhysReg{idx};
      }

      bool operator!=(const iterator& other) {
        return idx != other.idx;
      }

      iterator& operator++() {
        idx++;
        if (idx == kNumGP) {
          idx = kSIMDOffset;
        } else if (idx == kSIMDOffset + kNumSIMD) {
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
  explicit constexpr PhysReg(int n) : n(n) {}

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
    assert(!retval || (reg.n >= 0 && reg.n < 64));
    return retval;
  }

  bool findLast(PhysReg& reg) {
    uint64_t out;
    bool retval = fls64(m_bits, out);
    reg = PhysReg(out);
    assert(!retval || (reg.n >= 0 && reg.n < 64));
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
  static_assert(sizeof(m_bits) * 8 >= PhysReg::kMaxRegs, "");
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
