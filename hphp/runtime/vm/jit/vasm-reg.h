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

#ifndef incl_HPHP_JIT_VASM_REG_H_
#define incl_HPHP_JIT_VASM_REG_H_

#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/util/asm-x64.h"

#include <vector>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

enum class Width : uint8_t;
struct Vptr;
struct Vscaled;
struct VscaledDisp;
template <Width w> struct Vp;

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
  /* implicit */ Vreg(Reg16 r)  : rn(int(r)) {}
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

/*
 * Vector of Vregs, for Vtuples and VcallArgs (see vasm-unit.h).
 */
using VregList = jit::vector<Vreg>;

///////////////////////////////////////////////////////////////////////////////

/*
 * A set of Vregs using a sorted vector.
 */
struct VregSet {
  /*
   * Constructors
   */

  VregSet() = default;
  template <typename InputIt>
  explicit VregSet(InputIt i1, InputIt i2) {
    while (i1 != i2) regs.emplace_back(*i1++);
    canonicalize();
  }
  explicit VregSet(std::initializer_list<Vreg> i) {
    regs.reserve(i.size());
    for (auto const r : i) regs.emplace_back(r);
    canonicalize();
  }
  explicit VregSet(const VregList& l) {
    regs.reserve(l.size());
    for (auto const r : l) regs.emplace_back(r);
    canonicalize();
  }
  explicit VregSet(const RegSet& s) {
    regs.reserve(s.size());
    s.forEach([&] (PhysReg r) { regs.emplace_back(r); });
    canonicalize();
  }
  explicit VregSet(Vreg r) { regs.emplace_back(r); }

  /*
   * Getters
   */

  bool operator[](Vreg r) const {
    return std::binary_search(regs.begin(), regs.end(), r);
  }

  bool any() const { return !regs.empty(); }
  bool none() const { return regs.empty(); }

  bool empty() const { return regs.empty(); }
  size_t size() const { return regs.size(); }

  /*
   * Call the provided callable on each vreg in the set. If the callable has a
   * bool return value, a false return will stop iteration early.
   */
  template<typename F>
  typename std::enable_if<
    std::is_same<
      typename std::result_of<F(Vreg)>::type,
      bool
    >::value,
    void
  >::type
  forEach(F&& f) const {
    for (auto const r : regs) { if (!f(r)) return; }
  }

  template<typename F>
  typename std::enable_if<
    !std::is_same<
      typename std::result_of<F(Vreg)>::type,
      bool
    >::value,
    void
  >::type
  forEach(F&& f) const {
    for (auto const r : regs) f(r);
  }

  bool operator==(const VregSet& o) const { return regs == o.regs; }
  bool operator!=(const VregSet& o) const { return regs != o.regs; }

  /*
   * Mutators
   */

  void add(Vreg r) {
    auto const it = std::lower_bound(regs.begin(), regs.end(), r);
    if (it != regs.end() && *it == r) return;
    regs.emplace(it, r);
  }
  void remove(Vreg r) {
    auto const it = std::lower_bound(regs.begin(), regs.end(), r);
    if (it == regs.end() || *it != r) return;
    regs.erase(it);
  }
  void reset() { regs.clear(); }

  /*
   * Combiners
   */

  VregSet& operator|=(const VregSet& o) {
    if (UNLIKELY(&o == this)) return *this;
    if (o.regs.empty()) return *this;
    if (regs.empty()) {
      *this = o;
      return *this;
    }

    // Union together the sets by doing a merge backwards (this lets us use the
    // regs storage in place) and then removing any duplicates. The merge
    // ensures that the result is still sorted.
    regs.resize(regs.size() + o.regs.size());

    auto it1 = regs.rbegin() + o.regs.size();
    auto it2 = o.regs.rbegin();
    auto const end1 = regs.rend();
    auto const end2 = o.regs.rend();
    auto insertIt = regs.rbegin();

    while (true) {
      if (it1 == end1) {
        std::copy(it2, end2, insertIt);
        break;
      }
      if (it2 == end2) {
        std::copy(it1, end1, insertIt);
        break;
      }
      if (*it2 > *it1) {
        *insertIt = *it2;
        ++it2;
      } else {
        *insertIt = *it1;
        ++it1;
      }
      ++insertIt;
    }

    regs.erase(std::unique(regs.begin(), regs.end()), regs.end());
    return *this;
  }

  VregSet& operator&=(const VregSet& o) {
    if (UNLIKELY(&o == this)) return *this;
    if (regs.empty()) return *this;
    if (o.regs.empty()) {
      regs.clear();
      return *this;
    }

    auto it1 = regs.begin();
    auto it2 = o.regs.begin();
    auto const end1 = regs.end();
    auto const end2 = o.regs.end();
    auto insertIt = it1;

    while (it1 < end1 && it2 < end2) {
      if (*it1 == *it2) {
        if (insertIt != it1) *insertIt = *it1;
        ++insertIt;
        ++it1;
        ++it2;
      } else if (*it1 < *it2) {
        ++it1;
      } else {
        ++it2;
      }
    }

    regs.erase(insertIt, regs.end());
    return *this;
  }

  VregSet& operator-=(const VregSet& o) {
    if (UNLIKELY(&o == this)) {
      regs.clear();
      return *this;
    }
    if (regs.empty() || o.regs.empty()) return *this;

    auto it1 = regs.begin();
    auto it2 = o.regs.begin();
    auto const end1 = regs.end();
    auto const end2 = o.regs.end();
    auto insertIt = it1;

    while (it1 != end1) {
      if (it2 == end2) {
        insertIt = (it1 == insertIt)
          ? end1
          : std::copy(it1, end1, insertIt);
        break;
      }

      if (*it1 < *it2) {
        if (insertIt != it1) *insertIt = *it1;
        ++insertIt;
        ++it1;
      } else {
        if (*it1 == *it2) ++it1;
        ++it2;
      }
    }

    regs.erase(insertIt, regs.end());
    return *this;
  }

private:
  // The canonical form of the vector is for the Vregs to be sorted with no
  // duplicates. This restores that property after a bulk insert.
  void canonicalize() {
    std::sort(regs.begin(), regs.end());
    regs.erase(std::unique(regs.begin(), regs.end()), regs.end());
  }

  jit::vector<Vreg> regs;

  friend VregSet operator|(const VregSet&, const VregSet&);
  friend VregSet operator-(const VregSet&, const VregSet&);
  friend VregSet operator&(const VregSet&, const VregSet&);
};

/* VregSet friend operators */
inline VregSet operator|(const VregSet& s1, const VregSet& s2) {
  if (s1.regs.empty()) return s2;
  if (s2.regs.empty()) return s1;
  VregSet o;
  o.regs.reserve(s1.size() + s2.size());
  std::set_union(
    s1.regs.begin(), s1.regs.end(),
    s2.regs.begin(), s2.regs.end(),
    std::back_inserter(o.regs)
  );
  return o;
}
inline VregSet operator&(const VregSet& s1, const VregSet& s2) {
  if (s1.regs.empty()) return s1;
  if (s2.regs.empty()) return s2;
  VregSet o;
  o.regs.reserve(std::min(s1.size(), s2.size()));
  std::set_intersection(
    s1.regs.begin(), s1.regs.end(),
    s2.regs.begin(), s2.regs.end(),
    std::back_inserter(o.regs)
  );
  return o;
}
inline VregSet operator-(const VregSet& s1, const VregSet& s2) {
  if (s1.regs.empty() || s2.regs.empty()) return s1;
  VregSet o;
  o.regs.reserve(s1.size());
  std::set_difference(
    s1.regs.begin(), s1.regs.end(),
    s2.regs.begin(), s2.regs.end(),
    std::back_inserter(o.regs)
  );
  return o;
}

inline VregSet operator|(VregSet&& s1, const VregSet& s2) {
  s1 |= s2;
  return std::move(s1);
}
inline VregSet operator&(VregSet&& s1, const VregSet& s2) {
  s1 &= s2;
  return std::move(s1);
}
inline VregSet operator-(VregSet&& s1, const VregSet& s2) {
  s1 -= s2;
  return std::move(s1);
}

inline VregSet operator|(VregSet&& s1, VregSet&& s2) {
  s1 |= s2;
  return std::move(s1);
}
inline VregSet operator&(VregSet&& s1, VregSet&& s2) {
  s1 &= s2;
  return std::move(s1);
}
inline VregSet operator-(VregSet&& s1, VregSet&& s2) {
  s1 -= s2;
  return std::move(s1);
}

inline VregSet operator|(const VregSet& s1, VregSet&& s2) {
  s2 |= s1;
  return std::move(s2);
}
inline VregSet operator&(const VregSet& s1, VregSet&& s2) {
  s2 &= s1;
  return std::move(s2);
}

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

///////////////////////////////////////////////////////////////////////////////

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

/*
 * Vreg width constraint (or flags).
 *
 * Guaranteed to be a bitfield, which users of Width can do with as they
 * please.
 */
enum class Width : uint8_t {
  None  = 0,
  Byte  = 1,
  Word  = 1 << 1,
  Long  = 1 << 2,
  Quad  = 1 << 3,
  Octa  = 1 << 4,
  Flags = 1 << 5,
  // X-or-narrower widths.
  WordN = Byte | Word,
  LongN = Byte | Word | Long,
  QuadN = Byte | Word | Long | Quad,
  // Any non-flags register.
  AnyNF = Byte | Word | Long | Quad | Octa,
  Any   = Byte | Word | Long | Quad | Octa | Flags,
};

inline Width operator&(Width w1, Width w2) {
  return static_cast<Width>(
    static_cast<uint8_t>(w1) & static_cast<uint8_t>(w2)
  );
}
inline Width& operator&=(Width& w1, Width w2) {
  return w1 = w1 & w2;
}

inline Width width(Vreg)    { return Width::AnyNF; }
inline Width width(Vreg8)   { return Width::Byte; }
inline Width width(Vreg16)  { return Width::Word; }
inline Width width(Vreg32)  { return Width::Long; }
inline Width width(Vreg64)  { return Width::Quad; }
inline Width width(Vreg128) { return Width::Octa; }
inline Width width(VregDbl) { return Width::Quad; }
inline Width width(VregSF)  { return Width::Flags; }

std::string show(Width w);

///////////////////////////////////////////////////////////////////////////////

struct Vscaled {
  Vreg64 index;
  int scale;
  Width width;
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
  Vptr(Width w = Width::None)
    : base(Vreg{})
    , index(Vreg{})
    , disp(0)
    , width(w)
  {}

  Vptr(Vreg b, uint32_t d, Width w = Width::None)
    : base(b)
    , index(Vreg{})
    , disp(d)
    , scale(1)
    , width(w)
  {}

  Vptr(Vreg b, Vreg i, uint8_t s, uint32_t d, Width w = Width::None)
    : base(b)
    , index(i)
    , disp(d)
    , scale(s)
    , width(w)
  {
    validate();
  }

  /* implicit */ Vptr(MemoryRef m, Segment s = DS)
    : base(m.r.base)
    , index(m.r.index)
    , disp(m.r.disp)
    , scale(m.r.scale)
    , seg(s)
  {
    validate();
  }

  Vptr(MemoryRef m, Width w, Segment s = DS)
    : base(m.r.base)
    , index(m.r.index)
    , disp(m.r.disp)
    , scale(m.r.scale)
    , seg(s)
    , width(w)
  {
    validate();
  }

  Vptr(const Vptr& o) = default;
  Vptr& operator=(const Vptr& o) = default;

  MemoryRef mr() const;
  /* implicit */ operator MemoryRef() const;

  bool operator==(const Vptr&) const;
  bool operator!=(const Vptr&) const;

  void validate() {
    assertx((scale == 0x1 || scale == 0x2 || scale == 0x4 || scale == 0x8) &&
           "Invalid index register scaling (must be 1,2,4 or 8).");
  }

  Vreg64 base;      // optional, for baseless mode
  Vreg64 index;     // optional
  int32_t disp;
  uint8_t scale;    // 1,2,4,8
  Segment seg{DS};  // DS, FS or GS
  Width width{Width::None};
};

template <Width w>
struct Vp : Vptr {
  Vp(Vreg b, uint32_t d) : Vptr(b, d, w) {}
  /* implicit */ Vp(MemoryRef m, Segment s = DS) : Vptr(m, w, s) {}
  Vp(Vreg b, Vreg i, uint8_t s, int32_t d) : Vptr(b, i, d, s, w) {}
  /* implicit */ Vp(const Vptr& m) : Vptr(m) { width = w; }
};

using Vptr8 = Vp<Width::Byte>;
using Vptr16 = Vp<Width::Word>;
using Vptr32 = Vp<Width::Long>;
using Vptr64 = Vp<Width::Quad>;
using Vptr128 = Vp<Width::Octa>;

Vptr operator+(Vptr lhs, int32_t d);
Vptr operator+(Vptr lhs, intptr_t d);
Vptr operator+(Vptr lhr, size_t d);

Vptr baseless(VscaledDisp);

inline Width width(Vptr8)   { return Width::Byte; }
inline Width width(Vptr16)  { return Width::Word; }
inline Width width(Vptr32)  { return Width::Long; }
inline Width width(Vptr64)  { return Width::Quad; }
inline Width width(Vptr128) { return Width::Octa; }
inline Width width(Vptr m)  { return m.width; }

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
  VregList regs() const;
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

namespace std {
  template<> struct hash<HPHP::jit::Vreg> {
    size_t operator()(HPHP::jit::Vreg r) const { return (size_t)r; }
  };
}

///////////////////////////////////////////////////////////////////////////////

#include "hphp/runtime/vm/jit/vasm-reg-inl.h"

#endif
