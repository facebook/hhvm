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
#ifndef incl_HPHP_UTIL_ASM_X64_H_
#define incl_HPHP_UTIL_ASM_X64_H_

#include <type_traits>

#include "hphp/util/atomic.h"
#include "hphp/util/data-block.h"
#include "hphp/util/immed.h"
#include "hphp/util/safe-cast.h"
#include "hphp/util/trace.h"

/*
 * An experimental macro assembler for x64, that strives for low coupling to
 * the runtime environment.
 *
 * There are more complete assemblers out there; if you use this one
 * yourself, expect not to find all the instructions you wanted to use. You
 * may have to go spelunking in the Intel manuals:
 *
 *   http://www.intel.com/products/processor/manuals/
 *
 * If you're looking for something more fully baked, here are some options
 * to consider:
 *
 *   1. Nanojit or llvm, both of which translate abstract virtual machine
 *      instructions to the native target architecture, or
 *   2. The embedded assemblers from v8, the Sun JVM, etc.
 */

/*
 * Some members cannot be const because their values aren't known in
 * an initialization list. Like the opposite of the "mutable" keyword.
 * This declares this property to readers.
 */
#define logical_const /* nothing */

namespace HPHP { namespace jit {

#define TRACEMOD ::HPHP::Trace::asmx64

//////////////////////////////////////////////////////////////////////

struct MemoryRef;
struct RIPRelativeRef;
struct ScaledIndex;
struct ScaledIndexDisp;
struct DispReg;

const uint8_t kOpsizePrefix = 0x66;

struct Reg64 {
  explicit constexpr Reg64(int rn) : rn(rn) {}

  // Integer conversion is allowed but only explicitly.  (It's not
  // unusual to want to printf registers, etc.  Just cast it first.)
  explicit constexpr operator int() const { return rn; }

  MemoryRef operator[](intptr_t disp) const;
  MemoryRef operator[](Reg64) const;
  MemoryRef operator[](ScaledIndex) const;
  MemoryRef operator[](ScaledIndexDisp) const;
  MemoryRef operator[](DispReg) const;

  constexpr bool operator==(Reg64 o) const { return rn == o.rn; }
  constexpr bool operator!=(Reg64 o) const { return rn != o.rn; }

private:
  int rn;
};

#define SIMPLE_REGTYPE(What)                                        \
  struct What {                                                     \
    explicit constexpr What(int rn) : rn(rn) {}                     \
    explicit constexpr operator int() const { return rn; }          \
    constexpr bool operator==(What o) const { return rn == o.rn; }  \
    constexpr bool operator!=(What o) const { return rn != o.rn; }  \
  private:                                                          \
    int rn;                                                         \
  }

SIMPLE_REGTYPE(Reg32);
SIMPLE_REGTYPE(Reg16);
SIMPLE_REGTYPE(Reg8);
SIMPLE_REGTYPE(RegXMM);
SIMPLE_REGTYPE(RegSF);

#undef SIMPLE_REGTYPE

struct RegRIP {
  RIPRelativeRef operator[](intptr_t disp) const;
};

// Convert between physical registers of different sizes
inline Reg8 rbyte(Reg32 r)     { return Reg8(int(r)); }
inline Reg8 rbyte(Reg64 r)     { return Reg8(int(r)); }
inline Reg16 r16(Reg8 r)       { return Reg16(int(r)); }
inline Reg32 r32(Reg8 r)       { return Reg32(int(r)); }
inline Reg32 r32(Reg16 r)      { return Reg32(int(r)); }
inline Reg32 r32(Reg32 r)      { return r; }
inline Reg32 r32(Reg64 r)      { return Reg32(int(r)); }
inline Reg64 r64(Reg8 r)       { return Reg64(int(r)); }
inline Reg64 r64(Reg16 r)      { return Reg64(int(r)); }
inline Reg64 r64(Reg32 r)      { return Reg64(int(r)); }
inline Reg64 r64(Reg64 r)      { return r; }

//////////////////////////////////////////////////////////////////////

/*
 * The following structures define intermediate types for various
 * addressing modes.  They overload some operators to allow using
 * registers to look somewhat like pointers.
 *
 * E.g. rax[rbx*2 + 3] or *(rax + rbx*2 + 3).
 *
 * These operators are not defined commutatively; the thought is it
 * mandates the order you normally write them in a .S, but it could be
 * changed if this proves undesirable.
 */

// reg*x
struct ScaledIndex {
  explicit ScaledIndex(Reg64 index, intptr_t scale)
    : index(index)
    , scale(scale)
  {
    assert((scale == 0x1 || scale == 0x2 || scale == 0x4 || scale == 0x8) &&
           "Invalid index register scaling (must be 1,2,4 or 8).");
    assert(int(index) != -1 && "invalid register");
  }

  Reg64 index;
  intptr_t scale;
};

// reg*x + disp
struct ScaledIndexDisp {
  explicit ScaledIndexDisp(ScaledIndex si, intptr_t disp)
    : si(si)
    , disp(disp)
  {}

  ScaledIndexDisp operator+(intptr_t x) const {
    return ScaledIndexDisp(si, disp + x);
  }

  ScaledIndexDisp operator-(intptr_t x) const {
    return ScaledIndexDisp(si, disp - x);
  }

  ScaledIndex si;
  intptr_t disp;
};

// reg+x
struct DispReg {
  explicit DispReg(Reg64 base, intptr_t disp = 0)
    : base(base)
    , disp(disp)
  {
    assert(int(base) != -1 && "invalid register");
  }

  // Constructor for baseless().
  explicit DispReg(intptr_t disp)
    : base(-1)
    , disp(disp)
  {}

  MemoryRef operator*() const;
  MemoryRef operator[](intptr_t) const;

  DispReg operator+(intptr_t x) const {
    return DispReg(base, disp + x);
  }

  DispReg operator-(intptr_t x) const {
    return DispReg(base, disp - x);
  }

  Reg64 base;
  intptr_t disp;
};

// reg + reg*x + y
struct IndexedDispReg {
  explicit IndexedDispReg(Reg64 base, ScaledIndex sr)
    : base(base)
    , index(sr.index)
    , scale(sr.scale)
    , disp(0)
  {}

  explicit IndexedDispReg(DispReg r)
    : base(r.base)
    , index(-1)
    , scale(1)
    , disp(r.disp)
  {}

  // Constructor for baseless()
  explicit IndexedDispReg(ScaledIndexDisp sid)
    : base(-1)
    , index(sid.si.index)
    , scale(sid.si.scale)
    , disp(sid.disp)
  {}

  MemoryRef operator*() const;
  MemoryRef operator[](intptr_t disp) const;

  IndexedDispReg operator+(intptr_t disp) const {
    auto ret = *this;
    ret.disp += disp;
    return ret;
  }

  IndexedDispReg operator-(intptr_t disp) const {
    auto ret = *this;
    ret.disp -= disp;
    return ret;
  }

  Reg64 base;
  Reg64 index;
  int scale;
  intptr_t disp; // TODO #4613274: should be int32_t
};

// rip+x
struct DispRIP {
  explicit DispRIP(intptr_t disp) : disp(disp) {}

  RIPRelativeRef operator*() const;
  RIPRelativeRef operator[](intptr_t x) const;

  DispRIP operator+(intptr_t x) const {
    return DispRIP(disp + x);
  }

  DispRIP operator-(intptr_t x) const {
    return DispRIP(disp - x);
  }

  bool operator==(DispRIP o) const { return disp == o.disp; }
  bool operator!=(DispRIP o) const { return disp != o.disp; }

  intptr_t disp;
};

// *(rip + x)
struct RIPRelativeRef {
  explicit RIPRelativeRef(DispRIP r) : r(r) {}
  DispRIP r;

  bool operator==(const RIPRelativeRef& o) const { return r == o.r; }
  bool operator!=(const RIPRelativeRef& o) const { return r != o.r; }
};

//////////////////////////////////////////////////////////////////////

namespace reg {
  constexpr Reg64 rax(0);
  constexpr Reg64 rcx(1);
  constexpr Reg64 rdx(2);
  constexpr Reg64 rbx(3);
  constexpr Reg64 rsp(4);
  constexpr Reg64 rbp(5);
  constexpr Reg64 rsi(6);
  constexpr Reg64 rdi(7);

  constexpr Reg64 r8 (8);
  constexpr Reg64 r9 (9);
  constexpr Reg64 r10(10);
  constexpr Reg64 r11(11);
  constexpr Reg64 r12(12);
  constexpr Reg64 r13(13);
  constexpr Reg64 r14(14);
  constexpr Reg64 r15(15);

  constexpr RegRIP rip = RegRIP();

  constexpr Reg32 eax (0);
  constexpr Reg32 ecx (1);
  constexpr Reg32 edx (2);
  constexpr Reg32 ebx (3);
  constexpr Reg32 esp (4);
  constexpr Reg32 ebp (5);
  constexpr Reg32 esi (6);
  constexpr Reg32 edi (7);
  constexpr Reg32 r8d (8);
  constexpr Reg32 r9d (9);
  constexpr Reg32 r10d(10);
  constexpr Reg32 r11d(11);
  constexpr Reg32 r12d(12);
  constexpr Reg32 r13d(13);
  constexpr Reg32 r14d(14);
  constexpr Reg32 r15d(15);

  constexpr Reg16 ax  (0);
  constexpr Reg16 cx  (1);
  constexpr Reg16 dx  (2);
  constexpr Reg16 bx  (3);
  constexpr Reg16 sp  (4);
  constexpr Reg16 bp  (5);
  constexpr Reg16 si  (6);
  constexpr Reg16 di  (7);
  constexpr Reg16 r8w (8);
  constexpr Reg16 r9w (9);
  constexpr Reg16 r10w(10);
  constexpr Reg16 r11w(11);
  constexpr Reg16 r12w(12);
  constexpr Reg16 r13w(13);
  constexpr Reg16 r14w(14);
  constexpr Reg16 r15w(15);

  constexpr Reg8 al  (0);
  constexpr Reg8 cl  (1);
  constexpr Reg8 dl  (2);
  constexpr Reg8 bl  (3);
  constexpr Reg8 spl (4);
  constexpr Reg8 bpl (5);
  constexpr Reg8 sil (6);
  constexpr Reg8 dil (7);
  constexpr Reg8 r8b (8);
  constexpr Reg8 r9b (9);
  constexpr Reg8 r10b(10);
  constexpr Reg8 r11b(11);
  constexpr Reg8 r12b(12);
  constexpr Reg8 r13b(13);
  constexpr Reg8 r14b(14);
  constexpr Reg8 r15b(15);

  // Reminder: these registers may not be mixed in any instruction
  // using a REX prefix (i.e. anything using r8-r15, spl, bpl, sil,
  // dil, etc).
  constexpr Reg8 ah(0x80 | 4);
  constexpr Reg8 ch(0x80 | 5);
  constexpr Reg8 dh(0x80 | 6);
  constexpr Reg8 bh(0x80 | 7);

  constexpr RegXMM xmm0(0);
  constexpr RegXMM xmm1(1);
  constexpr RegXMM xmm2(2);
  constexpr RegXMM xmm3(3);
  constexpr RegXMM xmm4(4);
  constexpr RegXMM xmm5(5);
  constexpr RegXMM xmm6(6);
  constexpr RegXMM xmm7(7);
  constexpr RegXMM xmm8(8);
  constexpr RegXMM xmm9(9);
  constexpr RegXMM xmm10(10);
  constexpr RegXMM xmm11(11);
  constexpr RegXMM xmm12(12);
  constexpr RegXMM xmm13(13);
  constexpr RegXMM xmm14(14);
  constexpr RegXMM xmm15(15);

#define X(x) if (r == x) return "%"#x
  inline const char* regname(Reg64 r) {
    X(rax); X(rbx); X(rcx); X(rdx); X(rsp); X(rbp); X(rsi); X(rdi);
    X(r8); X(r9); X(r10); X(r11); X(r12); X(r13); X(r14); X(r15);
    return nullptr;
  }
  inline const char* regname(Reg32 r) {
    X(eax); X(ecx); X(edx); X(ebx); X(esp); X(ebp); X(esi); X(edi);
    X(r8d); X(r9d); X(r10d); X(r11d); X(r12d); X(r13d); X(r14d); X(r15d);
    return nullptr;
  }
  inline const char* regname(Reg16 r) {
    X(ax); X(cx); X(dx); X(bx); X(sp); X(bp); X(si); X(di);
    X(r8w); X(r9w); X(r10w); X(r11w); X(r12w); X(r13w); X(r14w); X(r15w);
    return nullptr;
  }
  inline const char* regname(Reg8 r) {
    X(al); X(cl); X(dl); X(bl); X(spl); X(bpl); X(sil); X(dil);
    X(r8b); X(r9b); X(r10b); X(r11b); X(r12b); X(r13b); X(r14b); X(r15b);
    X(ah); X(ch); X(dh); X(bh);
    return nullptr;
  }
  inline const char* regname(RegXMM r) {
    X(xmm0); X(xmm1); X(xmm2); X(xmm3); X(xmm4); X(xmm5); X(xmm6);
    X(xmm7); X(xmm8); X(xmm9); X(xmm10); X(xmm11); X(xmm12); X(xmm13);
    X(xmm14); X(xmm15);
    return nullptr;
  }
  inline const char* regname(RegSF /*r*/) {
    return "%flags";
  }
#undef X

}

enum class RoundDirection : ssize_t {
  nearest  = 0,
  floor    = 1,
  ceil     = 2,
  truncate = 3,
};

const char* show(RoundDirection);

enum class ComparisonPred : uint8_t {
  // True if...
  eq_ord = 0,    // ...operands are ordered AND equal
  ne_unord = 4,  // ...operands are unordered OR unequal
};

enum ConditionCode {
  CC_None = -1,
  CC_O    = 0x00,
  CC_NO   = 0x01,

  CC_B    = 0x02,
  CC_NAE  = 0x02,
  CC_AE   = 0x03,
  CC_NB   = 0x03,
  CC_NC   = 0x03,

  CC_E    = 0x04,
  CC_Z    = 0x04,
  CC_NE   = 0x05,
  CC_NZ   = 0x05,

  CC_BE   = 0x06,
  CC_NA   = 0x06,
  CC_A    = 0x07,
  CC_NBE  = 0x07,

  CC_S    = 0x08,
  CC_NS   = 0x09,

  CC_P    = 0x0A,
  CC_NP   = 0x0B,

  CC_L    = 0x0C,
  CC_NGE  = 0x0C,
  CC_GE   = 0x0D,
  CC_NL   = 0x0D,

  CC_LE   = 0x0E,
  CC_NG   = 0x0E,
  CC_G    = 0x0F,
  CC_NLE  = 0x0F,
};

// names of condition codes, indexable by the ConditionCode enum value.
extern const char* cc_names[];

inline ConditionCode ccNegate(ConditionCode c) {
  return ConditionCode(int(c) ^ 1); // And you thought x86 was irregular!
}
}};

#ifdef HAVE_LIBXED
  #include "hphp/util/asm-x64-intelxed.h"
#else
  #include "hphp/util/asm-x64-legacy.h"
#endif

namespace HPHP { namespace jit {
inline MemoryRef IndexedDispReg::operator*() const {
  return MemoryRef(*this);
}

inline MemoryRef IndexedDispReg::operator[](intptr_t x) const {
  return *(*this + x);
}

inline MemoryRef DispReg::operator*() const {
  return MemoryRef(*this);
}

inline MemoryRef DispReg::operator[](intptr_t x) const {
  return *(*this + x);
}

inline RIPRelativeRef DispRIP::operator*() const {
  return RIPRelativeRef(*this);
}

inline RIPRelativeRef DispRIP::operator[](intptr_t x) const {
  return *(*this + x);
}

inline DispReg operator+(Reg64 r, intptr_t d) { return DispReg(r, d); }
inline DispReg operator-(Reg64 r, intptr_t d) { return DispReg(r, -d); }
inline DispRIP operator+(RegRIP /*r*/, intptr_t d) {
  return DispRIP(d);
}
inline DispRIP operator-(RegRIP /*r*/, intptr_t d) {
  return DispRIP(d);
}

inline ScaledIndex operator*(Reg64 r, int scale) {
  return ScaledIndex(r, scale);
}
inline IndexedDispReg operator+(Reg64 base, ScaledIndex sr) {
  return IndexedDispReg(base, sr);
}
inline ScaledIndexDisp operator+(ScaledIndex si, intptr_t disp) {
  return ScaledIndexDisp(si, disp);
}
inline IndexedDispReg operator+(Reg64 b, Reg64 i) {
  return b + ScaledIndex(i, 0x1);
}

inline MemoryRef operator*(Reg64 r)  { return MemoryRef(DispReg(r)); }
inline DispRIP operator*(RegRIP /*r*/) {
  return DispRIP(0);
}

inline MemoryRef Reg64::operator[](intptr_t disp) const {
  return *(*this + disp);
}

inline MemoryRef Reg64::operator[](Reg64 idx) const {
  return *(*this + idx * 1);
}

inline MemoryRef Reg64::operator[](ScaledIndex si) const {
  return *(*this + si);
}

inline MemoryRef Reg64::operator[](DispReg dr) const {
  return *(*this + ScaledIndex(dr.base, 0x1) + dr.disp);
}

inline MemoryRef Reg64::operator[](ScaledIndexDisp sid) const {
  return *(*this + sid.si + sid.disp);
}

inline RIPRelativeRef RegRIP::operator[](intptr_t disp) const {
  return *(*this + disp);
}

/*
 * Used for the x64 addressing mode where there is a displacement,
 * possibly with a scaled index, but no base register.
 */
inline MemoryRef baseless(intptr_t disp) { return *(DispReg { disp }); }
inline MemoryRef baseless(ScaledIndexDisp sid) {
  return *(IndexedDispReg { sid });
}

//////////////////////////////////////////////////////////////////////

struct Label {
  explicit Label()
    : m_a(nullptr)
    , m_address(nullptr)
  {}

  ~Label() {
    if (!m_toPatch.empty()) {
      assert(m_a && m_address && "Label had jumps but was never set");
    }
    for (auto& ji : m_toPatch) {
      auto realSrc = ji.a->toDestAddress(ji.addr);
      switch (ji.type) {
      case Branch::Jmp:   ji.a->patchJmp(realSrc, ji.addr, m_address);  break;
      case Branch::Jmp8:  ji.a->patchJmp8(realSrc, ji.addr, m_address); break;
      case Branch::Jcc:   ji.a->patchJcc(realSrc, ji.addr, m_address);  break;
      case Branch::Jcc8:  ji.a->patchJcc8(realSrc, ji.addr, m_address); break;
      case Branch::Call:  ji.a->patchCall(realSrc, ji.addr, m_address); break;
      }
    }
  }

  Label(const Label&) = delete;
  Label& operator=(const Label&) = delete;

  void jmp(X64Assembler& a) {
    addJump(&a, Branch::Jmp);
    a.jmp(m_address ? m_address : a.frontier());
  }

  void jmp8(X64Assembler& a) {
    addJump(&a, Branch::Jmp8);
    a.jmp8(m_address ? m_address : a.frontier());
  }

  void jcc(X64Assembler& a, ConditionCode cc) {
    addJump(&a, Branch::Jcc);
    a.jcc(cc, m_address ? m_address : a.frontier());
  }

  void jcc8(X64Assembler& a, ConditionCode cc) {
    addJump(&a, Branch::Jcc8);
    a.jcc8(cc, m_address ? m_address : a.frontier());
  }

  void call(X64Assembler& a) {
    addJump(&a, Branch::Call);
    a.call(m_address ? m_address : a.frontier());
  }

  void jmpAuto(X64Assembler& a) {
    assert(m_address);
    auto delta = m_address - (a.frontier() + 2);
    if (deltaFits(delta, sz::byte)) {
      jmp8(a);
    } else {
      jmp(a);
    }
  }

  void jccAuto(X64Assembler& a, ConditionCode cc) {
    assert(m_address);
    auto delta = m_address - (a.frontier() + 2);
    if (deltaFits(delta, sz::byte)) {
      jcc8(a, cc);
    } else {
      jcc(a, cc);
    }
  }

  friend void asm_label(X64Assembler& a, Label& l) {
    assert(!l.m_address && !l.m_a && "Label was already set");
    l.m_a = &a;
    l.m_address = a.frontier();
  }

private:
  enum class Branch {
    Jcc,
    Jcc8,
    Jmp,
    Jmp8,
    Call
  };

  struct JumpInfo {
    Branch type;
    X64Assembler* a;
    CodeAddress addr;
  };

private:
  void addJump(X64Assembler* a, Branch type) {
    if (m_address) return;
    JumpInfo info;
    info.type = type;
    info.a = a;
    info.addr = a->codeBlock.frontier();
    m_toPatch.push_back(info);
  }

private:
  X64Assembler* m_a;
  CodeAddress m_address;
  std::vector<JumpInfo> m_toPatch;
};

inline void X64Assembler::jmp(Label& l) { l.jmp(*this); }
inline void X64Assembler::jmp8(Label& l) { l.jmp8(*this); }
inline void X64Assembler::jcc(ConditionCode c, Label& l) { l.jcc(*this, c); }
inline void X64Assembler::jcc8(ConditionCode c, Label& l) { l.jcc8(*this, c); }
inline void X64Assembler::call(Label& l) { l.call(*this); }

#define CC(nm, code)                                                    \
  inline void X64Assembler::j##nm(Label& l) { l.jcc(*this, code); }     \
  inline void X64Assembler::j##nm##8(Label& l) { l.jcc8(*this, code); }
  CCS
#undef CC

//////////////////////////////////////////////////////////////////////

/*
 * Select the assembler which contains a given address.
 *
 * E.g.:
 *
 *   Asm& a = codeBlockChoose(toPatch, a, acold);
 *   a.patchJmp(...);
 */
inline CodeBlock& codeBlockChoose(CodeAddress addr) {
  always_assert_flog(false,
                     "address {} was not part of any known code block", addr);
}
template<class... Blocks>
CodeBlock& codeBlockChoose(CodeAddress addr, CodeBlock& a, Blocks&... as) {
  if (a.contains(addr)) return a;
  return codeBlockChoose(addr, as...);
}

//////////////////////////////////////////////////////////////////////

namespace x64 {

struct DecodedInstruction {
  DecodedInstruction(uint8_t* ip, uint8_t* base)
    : m_base(base)
  { decode(ip); }

  explicit DecodedInstruction(uint8_t* ip) : DecodedInstruction(ip, ip) {}

  std::string toString();
  size_t size() { return m_size; }

  bool hasPicOffset() const { return m_flags.picOff; }
  uint8_t* picAddress() const;
  bool setPicAddress(uint8_t* target);

  bool hasOffset() const { return m_offSz != 0; }
  int32_t offset() const;

  bool hasImmediate() const { return m_immSz; }
  int64_t immediate() const;
  bool setImmediate(int64_t value);
  bool isNop() const;
  enum BranchType {
    Conditional = 1,
    Unconditional = 1 << 1,
  };
  bool isBranch(BranchType branchType = BranchType(Conditional |
                                                   Unconditional)) const;
  bool isCall() const;
  bool isJmp() const;
  bool isLea() const;
  bool isFuseable(const DecodedInstruction& next) const;
  ConditionCode jccCondCode() const;
  bool shrinkBranch();
  void widenBranch();
  uint8_t getModRm() const;
private:
  void decode(uint8_t* ip);
  bool decodePrefix(uint8_t* ip);
  int decodeRexVexXop(uint8_t* ip);
  int decodeOpcode(uint8_t* ip);
  void determineOperandsMap0(uint8_t* ip);
  void determineOperandsMap1(uint8_t* ip);
  void determineOperandsMap2(uint8_t* ip);
  void determineOperandsMap3(uint8_t* ip);
  int decodeModRm(uint8_t* ip);
  int decodeImm(uint8_t* ip);

  // We may wish to decode an instruction whose address is m_ip, but treat all
  // PIC references as relative to m_base.
  uint8_t* m_base;

  uint8_t*   m_ip;
  uint32_t   m_size;

  union {
    uint32_t m_flagsVal;
    struct {
      uint32_t lock      : 1;
      uint32_t repNE     : 1;
      uint32_t rep       : 1;

      uint32_t cs        : 1;
      uint32_t ss        : 1;
      uint32_t ds        : 1;
      uint32_t es        : 1;
      uint32_t fs        : 1;
      uint32_t gs        : 1;
      uint32_t bTaken    : 1;
      uint32_t bNotTaken : 1;

      uint32_t opndSzOvr : 1;
      uint32_t addrSzOvr : 1;

      uint32_t rex       : 1;
      uint32_t vex       : 1;
      uint32_t xop       : 1;

      uint32_t w         : 1;
      uint32_t r         : 1;
      uint32_t x         : 1;
      uint32_t b         : 1;
      uint32_t l         : 1;

      uint32_t def64     : 1;
      uint32_t immIsAddr : 1;
      uint32_t picOff    : 1;
      uint32_t hasModRm  : 1;
      uint32_t hasSib    : 1;
    } m_flags;
  };

  uint8_t       m_map_select;
  uint8_t       m_xtra_op;
  uint8_t       m_opcode;
  uint8_t       m_immSz;
  uint8_t       m_offSz;
};

constexpr DecodedInstruction::BranchType operator|(
    DecodedInstruction::BranchType a,
    DecodedInstruction::BranchType b
  ) {
  return DecodedInstruction::BranchType((int)a | (int)b);
}

inline DecodedInstruction::BranchType& operator|=(
    DecodedInstruction::BranchType& a,
    const DecodedInstruction::BranchType& b
  ) {
  return (a = DecodedInstruction::BranchType((int)a | (int)b));
}

#undef TRACEMOD
#undef logical_const
#undef CCS

}}}

#endif
