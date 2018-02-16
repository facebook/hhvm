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
#ifndef incl_HPHP_UTIL_ASM_INTELXED_X64_H_
#define incl_HPHP_UTIL_ASM_INTELXED_X64_H_
#include <type_traits>

#include "hphp/util/atomic.h"
#include "hphp/util/data-block.h"
#include "hphp/util/immed.h"
#include "hphp/util/safe-cast.h"
#include "hphp/util/trace.h"

extern "C" {
    #include <xed-interface.h>
}
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

struct XedInit
{
    XedInit() {
        xed_tables_init();
    }
};

static XedInit xi;

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

  intptr_t disp;
};

// *(reg + x)
struct MemoryRef {
  /*
   * The default value of MemoryRef::segment should be XED_REG_DS, but Xed fails
   * to encode lea(r, m) if the value of m.segment is not XED_REG_INVALID.
   * XED_RED_INVALID is a safe default value for m.segment, it will produce
   * the same output as assigning it XED_REG_DS when emitting all other
   * instructions and it will also work for lea(r ,m).
   */
  explicit MemoryRef(DispReg dr) : r(dr), segment(XED_REG_INVALID) {}
  explicit MemoryRef(IndexedDispReg idr) : r(idr),
                                           segment(XED_REG_INVALID) {}
  IndexedDispReg r;
  xed_reg_enum_t segment;
  void fs() { segment = XED_REG_FS; }
  void gs() { segment = XED_REG_GS; }
};

// *(rip + x)
struct RIPRelativeRef {
  explicit RIPRelativeRef(DispRIP r) : r(r) {}
  DispRIP r;
};

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

#define SZ_TO_BITS(sz)                    (sz << 3)
#define BITS_TO_SZ(bits)                  (bits >> 3)

typedef bool(*immFitFunc)(int64_t, int);
#define IMMFITFUNC_SIGNED                 deltaFits
#define IMMFITFUNC_UNSIGNED               (immFitFunc) magFits

struct XedOperand
{
  xed_encoder_operand_t op;

  union xed_imm_value {
    int8_t    b;
    uint8_t   ub;
    int16_t   w;
    int32_t   l;
    int64_t   q;
    uint64_t uq;

    template<typename immtype>
    xed_imm_value(const immtype& imm, int immSize) {
      uq = 0;
      switch(immSize) {
        case sz::byte:
          b = imm.b(); break;
        case sz::word:
          w = imm.w(); break;
        case sz::dword:
          l = imm.l(); break;
        case sz::qword:
          q = imm.q();
      }
    }
  };

  inline xed_reg_enum_t xedFromReg (const Reg64& reg) {
    return xed_reg_enum_t(int(reg) + XED_REG_RAX);
  }

  inline xed_reg_enum_t xedFromReg (const Reg32& reg) {
    return xed_reg_enum_t(int(reg) + XED_REG_EAX);
  }

  inline xed_reg_enum_t xedFromReg (const Reg16& reg) {
    return xed_reg_enum_t(int(reg) + XED_REG_AX);
  }

  inline xed_reg_enum_t xedFromReg (const Reg8& reg) {
    int regid = int(reg);
    if((regid & 0x80) == 0) {
      return xed_reg_enum_t(regid + XED_REG_AL);
    }
    return xed_reg_enum_t((regid - 0x84) + XED_REG_AH);
  }

  inline xed_reg_enum_t xedFromReg (const RegXMM& reg) {
    return xed_reg_enum_t(int(reg) + XED_REG_XMM0);
  }

  xed_enc_displacement_t xedDispFromValue(intptr_t value) {
    if (value == 0) {
      return {0, 0};
    }
    if(deltaFits(value, sz::byte)) {
       return {(xed_uint64_t)safe_cast<int8_t>(value), SZ_TO_BITS(sz::byte)};
    }
    return {(xed_uint64_t)safe_cast<int32_t>(value), SZ_TO_BITS(sz::dword)};
  }

  xed_enc_displacement_t xedDispFromValue(intptr_t value, int64_t offset) {
    return xedDispFromValue(value - offset);
  }

  template<typename regtype>
  explicit XedOperand(const regtype& reg) {
    op = xed_reg(xedFromReg(reg));
  }

  explicit XedOperand(xed_reg_enum_t reg) {
    op = xed_reg(reg);
  }

  explicit XedOperand(const MemoryRef& m, int memSize) {
    xed_reg_enum_t base = (int(m.r.base) != -1 ?
                            xedFromReg(m.r.base) : XED_REG_INVALID);
    xed_reg_enum_t index = (int(m.r.index) != -1 ?
                            xedFromReg(m.r.index) : XED_REG_INVALID);
    op = xed_mem_gbisd(m.segment, base, index, m.r.scale,
                       xedDispFromValue(m.r.disp), SZ_TO_BITS(memSize));
  }

  explicit XedOperand(const RIPRelativeRef& r, int memSize, int64_t offset) {
    op = xed_mem_bd(XED_REG_RIP, xedDispFromValue(r.r.disp, offset),
                    SZ_TO_BITS(memSize));
  }

  template<typename immtype>
  explicit XedOperand(const immtype& immed, int immSize) {
    op = xed_imm0(xed_imm_value(immed, immSize).uq, SZ_TO_BITS(immSize));
  }

  explicit XedOperand(CodeAddress address, int size) {
    int64_t target = (int64_t)address;
    assert(deltaFits(target, size) &&
           "Relative address doesn't fit selected size");
    op = xed_relbr((int32_t)target, SZ_TO_BITS(size));
  }

  template<typename immtype>
  explicit XedOperand(const immtype& immed, int immSizes, immFitFunc func) {
    immSizes = reduceImmSize(immed.q(), immSizes, func);
    op = xed_imm0(xed_imm_value(immed, immSizes).uq, SZ_TO_BITS(immSizes));
  }

  inline int reduceImmSize(int64_t value, int allowedSizes, immFitFunc func) {
    for (int crtSize = sz::byte; crtSize < sz::qword; crtSize <<= 1) {
      if((allowedSizes & crtSize) && (*func)(value, crtSize))
        return crtSize;
      }
    assert((allowedSizes & sz::qword) &&
           "Could not find an optimal size for Immed");
    return sz::qword;
  }
};

#define XED_REG(reg)                      XedOperand(reg).op
#define XED_IMM(imm, size)                XedOperand(imm, size).op
#define XED_IMM_RED(imm, sizes, redfunc)  XedOperand(imm, sizes, redfunc).op
#define XED_MEMREF(m, size)               XedOperand(m, size).op
#define XED_MEMREF_RIP(m, size, offset)   XedOperand(m, size, offset).op
#define XED_BRREL(p, size)                XedOperand((CodeAddress)p, size).op
//////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

struct Label;

/**
 * Copyright (c) 2009, Andrew J. Paroski
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The names of the contributors may not be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL ANDREW J. PAROSKI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

struct X64Assembler {
private:
  friend struct Label;

  /*
   * Type for register numbers, independent of the size we're going to
   * be using it as. Also, the same register number may mean different
   * physical registers for different instructions (e.g. xmm0 and rax
   * are both 0). Only for internal use in X64Assembler.
   */
  enum class RegNumber : int {};
  static const RegNumber noreg = RegNumber(-1);

  xed_uint8_t m_xedInstrBuff[XED_MAX_INSTRUCTION_BYTES];
  xed_state_t m_xedState;
public:
  explicit X64Assembler(CodeBlock& cb) : codeBlock(cb)
  {
    m_xedState.stack_addr_width=XED_ADDRESS_WIDTH_64b;
    m_xedState.mmode=XED_MACHINE_MODE_LONG_64;
  }

  X64Assembler(const X64Assembler&) = delete;
  X64Assembler& operator=(const X64Assembler&) = delete;

  CodeBlock& code() const { return codeBlock; }

  CodeAddress base() const {
    return codeBlock.base();
  }

  CodeAddress frontier() const {
    return codeBlock.frontier();
  }

  CodeAddress toDestAddress(CodeAddress addr) const {
    return codeBlock.toDestAddress(addr);
  }

  void setFrontier(CodeAddress newFrontier) {
    codeBlock.setFrontier(newFrontier);
  }

  size_t capacity() const {
    return codeBlock.capacity();
  }

  size_t used() const {
    return codeBlock.used();
  }

  size_t available() const {
    return codeBlock.available();
  }

  bool contains(CodeAddress addr) const {
    return codeBlock.contains(addr);
  }

  bool empty() const {
    return codeBlock.empty();
  }

  void clear() {
    codeBlock.clear();
  }

  bool canEmit(size_t nBytes) const {
    assert(capacity() >= used());
    return nBytes < (capacity() - used());
  }

  /*
   * The following section defines the main interface for emitting
   * x64.
   *
   * Simple Examples:
   *
   *   a.  movq   (rax, rbx);       // order is AT&T: src, dest
   *   a.  loadq  (*rax, rbx);      // loads from *rax
   *   a.  loadq  (rax[0], rbx);    // also loads from *rax
   *   a.  storeq (rcx, rax[0xc]);  // store to rax + 0xc
   *   a.  addq   (0x1, rbx);       // increment rbx
   *
   * Addressing with index registers:
   *
   *   a.  movl   (index, ecx);
   *   a.  loadq  (*rax, rbx);
   *   a.  storeq (rbx, rbx[rcx*8]);
   *   a.  call   (rax);            // indirect call
   *
   */
#define XED_INVERSE(x, y)   y, x

#define BYTE_LOAD_OP(name, instr)                                     \
  void name##b(MemoryRef m, Reg8 r)     { xedInstrMR(instr, m, r); }

#define LOAD_OP(name, instr)                                          \
  void name##q(MemoryRef m, Reg64 r) { xedInstrMR(instr, m, r); }     \
  void name##l(MemoryRef m, Reg32 r) { xedInstrMR(instr, m, r); }     \
  void name##w(MemoryRef m, Reg16 r) { xedInstrMR(instr, m, r); }     \
  void name##q(RIPRelativeRef m, Reg64 r) { xedInstrMR(instr, m, r); }\
  BYTE_LOAD_OP(name, instr)

#define BYTE_STORE_OP(name, instr)                                    \
  void name##b(Reg8 r, MemoryRef m)  { xedInstrRM(instr, r, m); }     \
  void name##b(Immed i, MemoryRef m) { xedInstrIM(instr, i, m,        \
                                                  sz::byte); }

#define STORE_OP(name, instr)                                         \
  void name##w(Immed i, MemoryRef m) {                                \
    xedInstrIM(instr, i, m, IMMPROP(sz::word,                         \
                                    sz::word | sz::byte,              \
                                    IMMFITFUNC_SIGNED), sz::word);    \
  }                                                                   \
  void name##l(Immed i, MemoryRef m) {                                \
    xedInstrIM(instr, i, m, IMMPROP(sz::dword,                        \
                                    sz::dword | sz::byte,             \
                                    IMMFITFUNC_SIGNED), sz::dword);   \
  }                                                                   \
  void name##w(Reg16 r, MemoryRef m) { xedInstrRM(instr, r, m); }     \
  void name##l(Reg32 r, MemoryRef m) { xedInstrRM(instr, r, m); }     \
  void name##q(Reg64 r, MemoryRef m) { xedInstrRM(instr, r, m); }     \
  BYTE_STORE_OP(name, instr)

#define BYTE_REG_OP(name, instr)                                      \
  void name##b(Reg8 r1, Reg8 r2) { xedInstrRR(instr, r1, r2);}        \
  void name##b(Immed i, Reg8 r)  { xedInstrIR(instr, i, r); }

#define REG_OP(name, instr)                                           \
  void name##q(Reg64 r1, Reg64 r2)   { xedInstrRR(instr, r1, r2); }   \
  void name##l(Reg32 r1, Reg32 r2)   { xedInstrRR(instr, r1, r2); }   \
  void name##w(Reg16 r1, Reg16 r2)   { xedInstrRR(instr, r1, r2); }   \
  void name##l(Immed i, Reg32 r) {                                    \
    xedInstrIR(instr, i, r, IMMPROP(sz::dword,                        \
                                    sz::dword | sz::byte,             \
                                    IMMFITFUNC_SIGNED));              \
  }                                                                   \
  void name##w(Immed i, Reg16 r) {                                    \
    xedInstrIR(instr, i, r, IMMPROP(sz::word,                         \
                                    sz::word | sz::byte,              \
                                    IMMFITFUNC_SIGNED));              \
  }                                                                   \
  BYTE_REG_OP(name, instr)

#define IMM64_STORE_OP(name, instr)                                   \
  void name##q(Immed i, MemoryRef m) {                                \
    xedInstrIM(instr, i, m, IMMPROP(sz::dword,                        \
                                    sz::dword | sz::byte,             \
                                    IMMFITFUNC_SIGNED), sz::qword);   \
  }

#define IMM64R_OP(name, instr)                                        \
  void name##q(Immed imm, Reg64 r) {                                  \
    always_assert(imm.fits(sz::dword));                               \
    xedInstrIR(instr, imm, r, IMMPROP(sz::dword,                      \
                                      sz::dword | sz::byte,           \
                                      IMMFITFUNC_SIGNED));            \
  }

#define FULL_OP(name, instr)                                          \
  LOAD_OP(name, instr)                                                \
  STORE_OP(name, instr)                                               \
  REG_OP(name, instr)                                                 \
  IMM64_STORE_OP(name, instr)                                         \
  IMM64R_OP(name, instr)

  // We rename x64's mov to store and load for improved code
  // readability.
#define IMMPROP(size, allsizes, func) size
  LOAD_OP        (load, XED_ICLASS_MOV)
  STORE_OP       (store,XED_ICLASS_MOV)
  IMM64_STORE_OP (store,XED_ICLASS_MOV)
  REG_OP         (mov,  XED_ICLASS_MOV)
  FULL_OP        (test, XED_ICLASS_TEST)
#undef IMMPROP

#define IMMPROP(size, allsizes, func) allsizes, func
  FULL_OP(add, XED_ICLASS_ADD)
  FULL_OP(xor, XED_ICLASS_XOR)
  FULL_OP(sub, XED_ICLASS_SUB)
  FULL_OP(and, XED_ICLASS_AND)
  FULL_OP(or,  XED_ICLASS_OR)
  FULL_OP(cmp, XED_ICLASS_CMP)
  FULL_OP(sbb, XED_ICLASS_SBB)
#undef IMMPROP

#undef IMM64_OP
#undef IMM64R_OP
#undef FULL_OP
#undef REG_OP
#undef STORE_OP
#undef LOAD_OP
#undef BYTE_LOAD_OP
#undef BYTE_STORE_OP
#undef BYTE_REG_OP

  // 64-bit immediates work with mov to a register.
  void movq(Immed64 imm, Reg64 r) { xedInstrIR(XED_ICLASS_MOV, imm, r); }

  // movzbx is a special snowflake. We don't have movzbq because it behaves
  // exactly the same as movzbl but takes an extra byte.
  void loadzbl(MemoryRef m, Reg32 r)        { xedInstrMR(XED_ICLASS_MOVZX,
                                                         m, r, sz::byte); }
  void movzbl(Reg8 src, Reg32 dest)         { xedInstrRR(XED_ICLASS_MOVZX,
                                                         src, dest); }
  void movsbl(Reg8 src, Reg32 dest)         { xedInstrRR(XED_ICLASS_MOVSX,
                                                         src, dest); }
  void movzwl(Reg16 src, Reg32 dest)        { xedInstrRR(XED_ICLASS_MOVZX,
                                                         src, dest); }

  void loadsbq(MemoryRef m, Reg64 r)        { xedInstrMR(XED_ICLASS_MOVSX,
                                                         m, r); }
  void movsbq(Reg8 src, Reg64 dest)         { xedInstrRR(XED_ICLASS_MOVSX,
                                                         src, dest); }

  void lea(MemoryRef p, Reg64 reg)       { xedInstrMR(XED_ICLASS_LEA, p, reg); }
  void lea(RIPRelativeRef p, Reg64 reg)  { xedInstrMR(XED_ICLASS_LEA, p, reg); }

  void xchgq(Reg64 r1, Reg64 r2) { xedInstrRR(XED_ICLASS_XCHG, r1, r2); }
  void xchgl(Reg32 r1, Reg32 r2) { xedInstrRR(XED_ICLASS_XCHG, r1, r2); }
  void xchgb(Reg8 r1, Reg8 r2)   { xedInstrRR(XED_ICLASS_XCHG, r1, r2); }

  void imul(Reg64 r1, Reg64 r2)  { xedInstrRR(XED_ICLASS_IMUL, r1, r2); }

  void push(Reg64 r)  { xedInstrR(XED_ICLASS_PUSH,  r); }
  void pushl(Reg32 r) { xedInstrR(XED_ICLASS_PUSH,  r); }
  void pop (Reg64 r)  { xedInstrR(XED_ICLASS_POP,   r); }
  void idiv(Reg64 r)  { xedInstrR(XED_ICLASS_IDIV,  r); }
  void incq(Reg64 r)  { xedInstrR(XED_ICLASS_INC,   r); }
  void incl(Reg32 r)  { xedInstrR(XED_ICLASS_INC,   r); }
  void incw(Reg16 r)  { xedInstrR(XED_ICLASS_INC,   r); }
  void decq(Reg64 r)  { xedInstrR(XED_ICLASS_DEC,   r); }
  void decl(Reg32 r)  { xedInstrR(XED_ICLASS_DEC,   r); }
  void decw(Reg16 r)  { xedInstrR(XED_ICLASS_DEC,   r); }
  void notb(Reg8 r)   { xedInstrR(XED_ICLASS_NOT,   r); }
  void not(Reg64 r)   { xedInstrR(XED_ICLASS_NOT,   r); }
  void neg(Reg64 r)   { xedInstrR(XED_ICLASS_NEG,   r); }
  void negb(Reg8 r)   { xedInstrR(XED_ICLASS_NEG,   r); }
  void ret()          { xedInstr(XED_ICLASS_RET_NEAR); }
  void ret(Immed i)   { xedInstrI(XED_ICLASS_IRET, i,
                                                    sz::word); }
  void cqo()          { xedInstr(XED_ICLASS_CQO); }
  void nop()          { xedInstr(XED_ICLASS_NOP,    sz::byte); }
  void int3()         { xedInstr(XED_ICLASS_INT3,   sz::byte); }
  void ud2()          { xedInstr(XED_ICLASS_UD2,    sz::byte); }
  void pushf()        { xedInstr(XED_ICLASS_PUSHF,  sz::word); }
  void popf()         { xedInstr(XED_ICLASS_POPF,   sz::word); }
  void lock()         { assert(false); }

  void push(MemoryRef m)      { xedInstrM(XED_ICLASS_PUSH,m); }
  void pop (MemoryRef m)      { xedInstrM(XED_ICLASS_POP, m); }
  void incq(MemoryRef m)      { xedInstrM(XED_ICLASS_INC, m); }
  void incl(MemoryRef m)      { xedInstrM(XED_ICLASS_INC, m, sz::dword); }
  void incw(MemoryRef m)      { xedInstrM(XED_ICLASS_INC, m, sz::word); }
  void decqlock(MemoryRef m)  { xedInstrM(XED_ICLASS_DEC_LOCK, m); }
  void decq(MemoryRef m)      { xedInstrM(XED_ICLASS_DEC, m); }
  void decl(MemoryRef m)      { xedInstrM(XED_ICLASS_DEC, m, sz::dword); }
  void decw(MemoryRef m)      { xedInstrM(XED_ICLASS_DEC, m, sz::word); }

  //special case for push(imm)
  void push(Immed64 i) {
    XedOperand oper(i, sz::byte | sz::word | sz::dword, IMMFITFUNC_SIGNED);
    xedEmit1(XED_ICLASS_PUSH, oper.op, oper.op.width_bits < 32 ? 16 : 64);
  }


  void movups(RegXMM x, MemoryRef m)        { xedInstrRM(XED_ICLASS_MOVUPS,
                                                         x, m, sz::qword * 2); }
  void movups(MemoryRef m, RegXMM x)        { xedInstrMR(XED_ICLASS_MOVUPS,
                                                         m, x, sz::qword * 2); }
  void movdqu(RegXMM x, MemoryRef m)        { xedInstrRM(XED_ICLASS_MOVDQU,
                                                         x, m); }
  void movdqu(MemoryRef m, RegXMM x)        { xedInstrMR(XED_ICLASS_MOVDQU,
                                                         m, x); }
  void movdqa(RegXMM x, RegXMM y)           { xedInstrRR(XED_ICLASS_MOVDQA,
                                                         XED_INVERSE(x, y)); }
  void movdqa(RegXMM x, MemoryRef m)        { xedInstrRM(XED_ICLASS_MOVDQA,
                                                         x, m); }
  void movdqa(MemoryRef m, RegXMM x)        { xedInstrMR(XED_ICLASS_MOVDQA,
                                                         m, x); }
  void movsd (RegXMM x, RegXMM y)           { xedInstrRR(XED_ICLASS_MOVSD_XMM,
                                                         XED_INVERSE(x, y)); }
  void movsd (RegXMM x, MemoryRef m)        { xedInstrRM(XED_ICLASS_MOVSD_XMM,
                                                         x, m); }
  void movsd (MemoryRef m, RegXMM x)        { xedInstrMR(XED_ICLASS_MOVSD_XMM,
                                                         m, x); }
  void movsd (RIPRelativeRef m, RegXMM x)   { xedInstrMR(XED_ICLASS_MOVSD_XMM,
                                                         m, x); }
  void lddqu (MemoryRef m, RegXMM x)        { xedInstrMR(XED_ICLASS_LDDQU,
                                                         m, x); }
  void unpcklpd(RegXMM s, RegXMM d)         { xedInstrRR(XED_ICLASS_UNPCKLPD,
                                                         XED_INVERSE(s, d)); }

  void rorq  (Immed i, Reg64 r) { xedInstrIR(XED_ICLASS_ROR, i, r, sz::byte); }
  void shlq  (Immed i, Reg64 r) { xedInstrIR(XED_ICLASS_SHL, i, r, sz::byte); }
  void shrq  (Immed i, Reg64 r) { xedInstrIR(XED_ICLASS_SHR, i, r, sz::byte); }
  void sarq  (Immed i, Reg64 r) { xedInstrIR(XED_ICLASS_SAR, i, r, sz::byte); }
  void shll  (Immed i, Reg32 r) { xedInstrIR(XED_ICLASS_SHL, i, r, sz::byte); }
  void shrl  (Immed i, Reg32 r) { xedInstrIR(XED_ICLASS_SHR, i, r, sz::byte); }
  void shlw  (Immed i, Reg16 r) { xedInstrIR(XED_ICLASS_SHL, i, r, sz::byte); }
  void shrw  (Immed i, Reg16 r) { xedInstrIR(XED_ICLASS_SHR, i, r, sz::byte); }

  void shlq (Reg64 r) { xedInstrRR_CL(XED_ICLASS_SHL, r); }
  void sarq (Reg64 r) { xedInstrRR_CL(XED_ICLASS_SAR, r); }

  void roundsd (RoundDirection d, RegXMM src, RegXMM dst) {
    Immed i((int)d);
    xedInstrIRR(XED_ICLASS_ROUNDSD, dst, src, i, sz::byte);
  }

  void cmpsd(RegXMM src, RegXMM dst, ComparisonPred pred) {
    Immed i((int)pred);
    xedInstrIRR(XED_ICLASS_CMPSD_XMM, dst, src, i, sz::byte);
  }

  /*
   * Control-flow directives.  Primitive labeling/patching facilities
   * are available, as well as slightly higher-level ones via the
   * Label class.
   */

  bool jmpDeltaFits(CodeAddress dest) {
    int64_t delta = dest - (codeBlock.frontier() + 5);
    return deltaFits(delta, sz::dword);
  }

  void jmp(Reg64 r)            { xedInstrR(XED_ICLASS_JMP,        r); }
  void jmp(MemoryRef m)        { xedInstrM(XED_ICLASS_JMP,        m); }
  void jmp(RIPRelativeRef m)   { xedInstrM(XED_ICLASS_JMP,        m); }
  void call(Reg64 r)           { xedInstrR(XED_ICLASS_CALL_NEAR,  r); }
  void call(MemoryRef m)       { xedInstrM(XED_ICLASS_CALL_NEAR,  m); }
  void call(RIPRelativeRef m)  { xedInstrM(XED_ICLASS_CALL_NEAR,  m); }

  void jmp8(CodeAddress dest)  { xedInstrRelBr(XED_ICLASS_JMP,
                                               dest, sz::byte); }

  void jmp(CodeAddress dest) {
    xedInstrRelBr(XED_ICLASS_JMP, dest, sz::dword);
  }

  void call(CodeAddress dest) {
    xedInstrRelBr(XED_ICLASS_CALL_NEAR, dest, sz::dword);
  }

  void jcc(ConditionCode cond, CodeAddress dest) {
    xedInstrRelBr(ccToXedJump(cond), dest, sz::dword);
  }

  void jcc8(ConditionCode cond, CodeAddress dest) {
    xedInstrRelBr(ccToXedJump(cond), dest, sz::dword);
  }

  void jmpAuto(CodeAddress dest) {
    auto delta = dest - (codeBlock.frontier() + 2);
    if (deltaFits(delta, sz::byte)) {
      jmp8(dest);
    } else {
      jmp(dest);
    }
  }

  void jccAuto(ConditionCode cc, CodeAddress dest) {
    auto delta = dest - (codeBlock.frontier() + 2);
    if (deltaFits(delta, sz::byte)) {
      jcc8(cc, dest);
    } else {
      jcc(cc, dest);
    }
  }

  void call(Label&);
  void jmp(Label&);
  void jmp8(Label&);
  void jcc(ConditionCode, Label&);
  void jcc8(ConditionCode, Label&);

#define CCS \
  CC(o,   CC_O)         \
  CC(no,  CC_NO)        \
  CC(nae, CC_NAE)       \
  CC(ae,  CC_AE)        \
  CC(nb,  CC_NB)        \
  CC(e,   CC_E)         \
  CC(z,   CC_Z)         \
  CC(ne,  CC_NE)        \
  CC(nz,  CC_NZ)        \
  CC(b,   CC_B)         \
  CC(be,  CC_BE)        \
  CC(nbe, CC_NBE)       \
  CC(s,   CC_S)         \
  CC(ns,  CC_NS)        \
  CC(p,   CC_P)         \
  CC(np,  CC_NP)        \
  CC(nge, CC_NGE)       \
  CC(g,   CC_G)         \
  CC(l,   CC_L)         \
  CC(ge,  CC_GE)        \
  CC(nl,  CC_NL)        \
  CC(ng,  CC_NG)        \
  CC(le,  CC_LE)        \
  CC(nle, CC_NLE)

#define CC(_nm, _code)                                        \
  void j ## _nm(CodeAddress dest)      { jcc(_code, dest); }  \
  void j ## _nm ## 8(CodeAddress dest) { jcc8(_code, dest); } \
  void j ## _nm(Label&);                                      \
  void j ## _nm ## 8(Label&);
  CCS
#undef CC
  void setcc(int cc, Reg8 byteReg) {
    xedInstrR(ccToXedSetCC(cc), byteReg);
  }

#define CC(_nm, _cond)                          \
  void set ## _nm(Reg8 byteReg) {               \
    setcc(_cond, byteReg);                      \
  }
  CCS
#undef CC

  void psllq(Immed i, RegXMM r) { xedInstrIR(XED_ICLASS_PSLLQ, i, r,
                                             sz::byte); }
  void psrlq(Immed i, RegXMM r) { xedInstrIR(XED_ICLASS_PSRLQ, i, r,
                                             sz::byte); }

  void movq_rx(Reg64 rSrc, RegXMM rdest) {
    xedInstrRR(XED_ICLASS_MOVQ, XED_INVERSE(rdest, rSrc));
  }
  void movq_xr(RegXMM rSrc, Reg64 rdest) {
    xedInstrRR(XED_ICLASS_MOVQ, rSrc, rdest);
  }

  void addsd(RegXMM src, RegXMM srcdest) {
    xedInstrRR(XED_ICLASS_ADDSD, srcdest, src);
  }
  void mulsd(RegXMM src, RegXMM srcdest) {
    xedInstrRR(XED_ICLASS_MULSD, srcdest, src);
  }
  void subsd(RegXMM src, RegXMM srcdest) {
    xedInstrRR(XED_ICLASS_SUBSD, srcdest, src);
  }
  void pxor(RegXMM src, RegXMM srcdest) {
    xedInstrRR(XED_ICLASS_PXOR, srcdest, src);
  }
  void cvtsi2sd(Reg64 src, RegXMM dest) {
    xedInstrRR(XED_ICLASS_CVTSI2SD, XED_INVERSE(dest, src));
  }
  void cvtsi2sd(MemoryRef m, RegXMM dest) {
    xedInstrMR(XED_ICLASS_CVTSI2SD, m, dest);
  }
  void ucomisd(RegXMM l, RegXMM r) {
    xedInstrRR(XED_ICLASS_UCOMISD, l, r);
  }
  void sqrtsd(RegXMM src, RegXMM dest) {
    xedInstrRR(XED_ICLASS_SQRTSD, dest, src);
  }

  void divsd(RegXMM src, RegXMM srcdest) {
    xedInstrRR(XED_ICLASS_DIVSD, srcdest, src);
  }
  void cvttsd2siq(RegXMM src, Reg64 dest) {
    xedInstrRR(XED_ICLASS_CVTTSD2SI, XED_INVERSE(dest, src));
  }

  /*
   * The following utility functions do more than emit specific code.
   * (E.g. combine common idioms or patterns, smash code, etc.)
   */

  void emitImmReg(Immed64 imm, Reg64 dest) {
    if (imm.q() == 0) {
      // Zeros the top bits also.
      xorl  (r32(dest), r32(dest));
      return;
    }
    if (LIKELY(imm.q() > 0 && imm.fits(sz::dword))) {
      // This will zero out the high-order bits.
      movl (imm.l(), r32(dest));
      return;
    }
    movq (imm.q(), dest);
  }

  static void patchJcc(CodeAddress jmp, CodeAddress from, CodeAddress dest) {
    assert(jmp[0] == 0x0F && (jmp[1] & 0xF0) == 0x80);
    ssize_t diff = dest - (from + 6);
    *(int32_t*)(jmp + 2) = safe_cast<int32_t>(diff);
  }

  static void patchJcc8(CodeAddress jmp, CodeAddress from, CodeAddress dest) {
    assert((jmp[0] & 0xF0) == 0x70);
    ssize_t diff = dest - (from + 2);  // one for opcode, one for offset
    *(int8_t*)(jmp + 1) = safe_cast<int8_t>(diff);
  }

  static void patchJmp(CodeAddress jmp, CodeAddress from, CodeAddress dest) {
    assert(jmp[0] == 0xE9);
    ssize_t diff = dest - (from + 5);
    *(int32_t*)(jmp + 1) = safe_cast<int32_t>(diff);
  }

  static void patchJmp8(CodeAddress jmp, CodeAddress from, CodeAddress dest) {
    assert(jmp[0] == 0xEB);
    ssize_t diff = dest - (from + 2);  // one for opcode, one for offset
    *(int8_t*)(jmp + 1) = safe_cast<int8_t>(diff);
  }

  static void patchCall(CodeAddress call, CodeAddress from, CodeAddress dest) {
    assert(call[0] == 0xE8);
    ssize_t diff = dest - (from + 5);
    *(int32_t*)(call + 1) = safe_cast<int32_t>(diff);
  }

private:
  //Xed conversion funcs
  #define CC_TO_XED_ARRAY(xed_instr) {                            \
      XED_ICLASS_##xed_instr##O,    /*CC_O                  */    \
      XED_ICLASS_##xed_instr##NO,   /*CC_NO                 */    \
      XED_ICLASS_##xed_instr##B,    /*CC_B, CC_NAE          */    \
      XED_ICLASS_##xed_instr##NB,   /*CC_AE, CC_NB, CC_NC   */    \
      XED_ICLASS_##xed_instr##Z,    /*CC_E, CC_Z            */    \
      XED_ICLASS_##xed_instr##NZ,   /*CC_NE, CC_NZ          */    \
      XED_ICLASS_##xed_instr##BE,   /*CC_BE, CC_NA          */    \
      XED_ICLASS_##xed_instr##NBE,  /*CC_A, CC_NBE          */    \
      XED_ICLASS_##xed_instr##S,    /*CC_S                  */    \
      XED_ICLASS_##xed_instr##NS,   /*CC_NS                 */    \
      XED_ICLASS_##xed_instr##P,    /*CC_P                  */    \
      XED_ICLASS_##xed_instr##NP,   /*CC_NP                 */    \
      XED_ICLASS_##xed_instr##L,    /*CC_L, CC_NGE          */    \
      XED_ICLASS_##xed_instr##NL,   /*CC_GE, CC_NL          */    \
      XED_ICLASS_##xed_instr##LE,   /*CC_LE, CC_NG          */    \
      XED_ICLASS_##xed_instr##NLE   /*CC_G, CC_NLE          */    \
    }

  ALWAYS_INLINE
  xed_iclass_enum_t ccToXedJump(ConditionCode c) {
    assert(c != CC_None);
    static xed_iclass_enum_t jumps[] = CC_TO_XED_ARRAY(J);
    return jumps[(int)c];
  }

  ALWAYS_INLINE
  xed_iclass_enum_t ccToXedSetCC(int c) {
    assert(c != -1);
    static xed_iclass_enum_t setccs[] = CC_TO_XED_ARRAY(SET);
    return setccs[c];
  }

  ALWAYS_INLINE
  xed_iclass_enum_t ccToXedCMov(ConditionCode c) {
    assert(c != CC_None);
    static xed_iclass_enum_t cmovs[] = CC_TO_XED_ARRAY(CMOV);
    return cmovs[(int)c];
  }

  //Xed Emit funcs
  #define DECLARE_UNUSED(type, name)  type name; static_cast<void>(name)

  #define XED_EMIT_PREP_BEGIN                                                 \
    xed_encoder_instruction_t instruction;                                    \
    xed_encoder_request_t request;                                            \
    uint32_t encodedSize = 0;                                                 \
    DECLARE_UNUSED(xed_error_enum_t, xedError);                               \
    DECLARE_UNUSED(xed_bool_t, convert_ok);

  #define XED_EMIT_PREP_ENCODE                                                \
    xed_encoder_request_zero_set_mode(&request, &m_xedState);                 \
    convert_ok = xed_convert_to_encoder_request(&request, &instruction);      \
    assert(convert_ok);                                                       \
    xedError = xed_encode(&request, m_xedInstrBuff, XED_MAX_INSTRUCTION_BYTES,\
                          &encodedSize);

  #define XED_ASSERT_ERR(args)                                                \
    assert_flog(xedError == XED_ERROR_NONE,                                   \
                "XED: Error when encoding {}(" args ")"                       \
                "with effOpSize({}): {}",                                     \
                xed_iclass_enum_t2str(instr),                                 \
                effOperandSizeBits,                                           \
                xed_error_enum_t2str(xedError));

  ALWAYS_INLINE
  uint32_t xedEmit0Prep(xed_iclass_enum_t instr,
                        xed_uint_t effOperandSizeBits = 0) {
    XED_EMIT_PREP_BEGIN
    xed_inst0(&instruction, m_xedState, instr, effOperandSizeBits);
    XED_EMIT_PREP_ENCODE
    XED_ASSERT_ERR("")
    return encodedSize;
  }

  ALWAYS_INLINE
  void xedEmit0(xed_iclass_enum_t instr, xed_uint_t effOperandSizeBits = 0) {
    bytes(xedEmit0Prep(instr, effOperandSizeBits), m_xedInstrBuff);
  }

  ALWAYS_INLINE
  uint32_t xedEmit1Prep(xed_iclass_enum_t instr,
                        const xed_encoder_operand_t& op,
                        xed_uint_t effOperandSizeBits = 0) {
    XED_EMIT_PREP_BEGIN
    xed_inst1(&instruction, m_xedState, instr, effOperandSizeBits, op);
    XED_EMIT_PREP_ENCODE
    XED_ASSERT_ERR("arg")
    return encodedSize;
  }

  ALWAYS_INLINE
  void xedEmit1(xed_iclass_enum_t instr, const xed_encoder_operand_t& op,
                    xed_uint_t effOperandSizeBits = 0) {
    bytes(xedEmit1Prep(instr, op, effOperandSizeBits), m_xedInstrBuff);
  }

  ALWAYS_INLINE
  uint32_t xedEmit2Prep(xed_iclass_enum_t instr,
                        const xed_encoder_operand_t& op_1,
                        const xed_encoder_operand_t& op_2,
                        xed_uint_t effOperandSizeBits = 0) {
    XED_EMIT_PREP_BEGIN
    xed_inst2(&instruction, m_xedState, instr, effOperandSizeBits, op_1, op_2);
    XED_EMIT_PREP_ENCODE
    XED_ASSERT_ERR("arg, arg")
    return encodedSize;
  }

  ALWAYS_INLINE
  void xedEmit2(xed_iclass_enum_t instr, const xed_encoder_operand_t& op_1,
                const xed_encoder_operand_t& op_2,
                xed_uint_t effOperandSizeBits = 0) {
    bytes(xedEmit2Prep(instr, op_1, op_2, effOperandSizeBits), m_xedInstrBuff);
  }

  ALWAYS_INLINE
  uint32_t xedEmit3Prep(xed_iclass_enum_t instr,
                        const xed_encoder_operand_t& op_1,
                        const xed_encoder_operand_t& op_2,
                        const xed_encoder_operand_t& op_3,
                        xed_uint_t effOperandSizeBits = 0) {
    XED_EMIT_PREP_BEGIN
    xed_inst3(&instruction, m_xedState, instr, effOperandSizeBits,
              op_1, op_2, op_3);
    XED_EMIT_PREP_ENCODE
    XED_ASSERT_ERR("arg, arg, arg")
    return encodedSize;
  }

  ALWAYS_INLINE
  void xedEmit3(xed_iclass_enum_t instr,
                const xed_encoder_operand_t& op_1,
                const xed_encoder_operand_t& op_2,
                const xed_encoder_operand_t& op_3,
                xed_uint_t effOperandSizeBits = 0) {
    bytes(xedEmit3Prep(instr, op_1, op_2, op_3, effOperandSizeBits),
          m_xedInstrBuff);
  }

public:
  void emitInt3s(int n) {
    if (n == 0) return;
    DECLARE_UNUSED(uint32_t, int3size);
    int3size = xedEmit0Prep(XED_ICLASS_INT3, sz::byte);
    assert(int3size == 1);
    for (auto i = 0; i < n; ++i) {
      byte(m_xedInstrBuff[0]);
    }
  }

  void emitNop(int n) {
    if (n == 0) return;
    static const xed_iclass_enum_t nops[] = {
      XED_ICLASS_INVALID,
      XED_ICLASS_NOP,
      XED_ICLASS_NOP2,
      XED_ICLASS_NOP3,
      XED_ICLASS_NOP4,
      XED_ICLASS_NOP5,
      XED_ICLASS_NOP6,
      XED_ICLASS_NOP7,
      XED_ICLASS_NOP8,
      XED_ICLASS_NOP9,
    };
    // While n >= 9, emit 9 byte NOPs
    while (n >= 9) {
      xedInstr(XED_ICLASS_NOP9, 0);
      n -= 9;
    }
    // Emit remaining NOPs (if any)
    if(n) {
      xedInstr(nops[n], 0);
    }
  }

  /*
   * Low-level emitter functions.
   *
   * These functions are the core of the assembler, and can also be
   * used directly.
   */

  void byte(uint8_t b) {
    codeBlock.byte(b);
  }
  void word(uint16_t w) {
    codeBlock.word(w);
  }
  void dword(uint32_t dw) {
    codeBlock.dword(dw);
  }
  void qword(uint64_t qw) {
    codeBlock.qword(qw);
  }
  void bytes(size_t n, const uint8_t* bs) {
    codeBlock.bytes(n, bs);
  }

public:
  /*
   * The following functions use a naming convention for an older API
   * to the assembler; conditional loads and moves haven't yet been
   * ported.
   */

  // CMOVcc [rbase + off], rdest
  inline void cload_reg64_disp_reg64(ConditionCode cc, Reg64 rbase,
                                     int off, Reg64 rdest) {
    MemoryRef m(DispReg(rbase, off));
    xedInstrMR(ccToXedCMov(cc), m, rdest);
  }
  inline void cload_reg64_disp_reg32(ConditionCode cc, Reg64 rbase,
                                     int off, Reg32 rdest) {
    MemoryRef m(DispReg(rbase, off));
    xedInstrMR(ccToXedCMov(cc), m, rdest);
  }
  inline void cmov_reg64_reg64(ConditionCode cc, Reg64 rsrc, Reg64 rdest) {
    xedInstrRR(ccToXedCMov(cc), rsrc, rdest);
  }

private:
  RegNumber rn(Reg8 r)   { return RegNumber(int(r)); }
  RegNumber rn(Reg16 r)  { return RegNumber(int(r)); }
  RegNumber rn(Reg32 r)  { return RegNumber(int(r)); }
  RegNumber rn(Reg64 r)  { return RegNumber(int(r)); }
  RegNumber rn(RegXMM r) { return RegNumber(int(r)); }

  // Wraps a bunch of the emit* functions to make using them with the
  // typed wrappers more terse. We should have these replace
  // the emit functions eventually.

#define XED_WRAP_IMPL() \
  XED_WRAP_X(64)        \
  XED_WRAP_X(32)        \
  XED_WRAP_X(16)        \
  XED_WRAP_X(8)

  // instr(reg)

#define XED_INSTR_WRAPPER_IMPL(bitsize)                             \
  ALWAYS_INLINE                                                     \
  void xedInstrR(xed_iclass_enum_t instr, const Reg##bitsize& r) {  \
    xedEmit1(instr, XED_REG(r), bitsize);                           \
  }

#define XED_WRAP_X XED_INSTR_WRAPPER_IMPL
  XED_WRAP_IMPL()
#undef XED_WRAP_X

  // instr(imm, reg)

#define XED_INSTIR_WRAPPER_IMPL(bitsize)                            \
  ALWAYS_INLINE                                                     \
  void xedInstrIR(xed_iclass_enum_t instr, const Immed& i,          \
                  const Reg##bitsize& r,                            \
                  int immSize = BITS_TO_SZ(bitsize)) {              \
    xedEmit2(instr, XED_REG(r), XED_IMM(i, immSize), bitsize);      \
  }                                                                 \
  ALWAYS_INLINE                                                     \
  void xedInstrIR(xed_iclass_enum_t instr, const Immed& i,          \
                  const Reg##bitsize& r,                            \
                  int immSize, immFitFunc fitFunc) {                \
    xedEmit2(instr, XED_REG(r),                                     \
             XED_IMM_RED(i, immSize, fitFunc), bitsize);            \
  }

#define XED_WRAP_X XED_INSTIR_WRAPPER_IMPL
  XED_WRAP_IMPL()
#undef XED_WRAP_X

  ALWAYS_INLINE
  void xedInstrIR(xed_iclass_enum_t instr, const Immed64& i, const Reg64& r) {
    xedEmit2(instr, XED_REG(r), XED_IMM(i, sz::qword), SZ_TO_BITS(sz::qword));
  }

  ALWAYS_INLINE
  void xedInstrIR(xed_iclass_enum_t instr, const Immed& i,
                  const RegXMM& r, int immSize) {
    xedEmit2(instr, XED_REG(r), XED_IMM(i, immSize));
  }

  // instr(reg, reg)

#define XED_INSTRR_WRAPPER_IMPL(bitsize)                            \
  ALWAYS_INLINE                                                     \
  void xedInstrRR(xed_iclass_enum_t instr, const Reg##bitsize& r1,  \
                  const Reg##bitsize& r2) {                         \
    xedEmit2(instr, XED_REG(r2), XED_REG(r1), bitsize);             \
  }

#define XED_WRAP_X XED_INSTRR_WRAPPER_IMPL
  XED_WRAP_IMPL()
#undef XED_WRAP_X

  ALWAYS_INLINE
  void xedInstrRR_CL(xed_iclass_enum_t instr, const Reg64& r) {
    xedEmit2(instr, XED_REG(r), XED_REG(XED_REG_CL), SZ_TO_BITS(sz::qword));
  }

  ALWAYS_INLINE
  void xedInstrRR(xed_iclass_enum_t instr, const Reg8& r1, const Reg32& r2) {
    xedEmit2(instr, XED_REG(r2), XED_REG(r1), SZ_TO_BITS(sz::byte));
  }

  ALWAYS_INLINE
  void xedInstrRR(xed_iclass_enum_t instr, const Reg16& r1, const Reg32& r2) {
    xedEmit2(instr, XED_REG(r2), XED_REG(r1), SZ_TO_BITS(sz::word));
  }

  ALWAYS_INLINE
  void xedInstrRR(xed_iclass_enum_t instr, const Reg8& r1, const Reg64& r2) {
    xedEmit2(instr, XED_REG(r2), XED_REG(r1), SZ_TO_BITS(sz::byte));
  }

  ALWAYS_INLINE
  void xedInstrRR(xed_iclass_enum_t instr, const Reg64& r1, const RegXMM& r2) {
    xedEmit2(instr, XED_REG(r2), XED_REG(r1));
  }

  ALWAYS_INLINE
  void xedInstrRR(xed_iclass_enum_t instr, const RegXMM& r1, const Reg64& r2) {
    xedEmit2(instr, XED_REG(r2), XED_REG(r1));
  }


  // most instr(xmm_1, xmm_2) instructions take operands in reverse order
  // compared to instr(reg_1, reg_2): source and destination are swapped
  ALWAYS_INLINE
  void xedInstrRR(xed_iclass_enum_t instr, const RegXMM& r1, const RegXMM& r2) {
    xedEmit2(instr, XED_REG(r1), XED_REG(r2));
  }

  // instr(imm)

  ALWAYS_INLINE
  void xedInstrI(xed_iclass_enum_t instr, const Immed& i, int immSize) {
      xedEmit1(instr, XED_IMM(i, immSize), SZ_TO_BITS(immSize));
  }

  // instr(mem)

  ALWAYS_INLINE
  void xedInstrM(xed_iclass_enum_t instr, const MemoryRef& m,
                 int size = sz::qword) {
      xedEmit1(instr, XED_MEMREF(m, size), SZ_TO_BITS(size));
  }

  ALWAYS_INLINE
  void xedInstrM(xed_iclass_enum_t instr, const RIPRelativeRef& m,
                 int size = sz::qword) {
    uint32_t instrLen = xedEmit1Prep(instr, XED_MEMREF_RIP(m, size, 0),
                                     SZ_TO_BITS(size));
    xedEmit1(instr, XED_MEMREF_RIP(m, size,
             (int64_t)frontier() + (int64_t)instrLen), SZ_TO_BITS(size));
  }


  // instr(imm, mem)

  ALWAYS_INLINE
  void xedInstrIM(xed_iclass_enum_t instr, const Immed& i, const MemoryRef& m,
                  int size = sz::qword) {
      xedEmit2(instr,  XED_MEMREF(m, size), XED_IMM(i, size),
               SZ_TO_BITS(size));
  }

  ALWAYS_INLINE
  void xedInstrIM(xed_iclass_enum_t instr, const Immed& i, const MemoryRef& m,
                  int immSize, int memSize) {
      xedEmit2(instr,  XED_MEMREF(m, memSize), XED_IMM(i, immSize),
               SZ_TO_BITS(memSize));
  }

  ALWAYS_INLINE
  void xedInstrIM(xed_iclass_enum_t instr, const Immed& i, const MemoryRef& m,
                  int immSize, immFitFunc fitFunc, int memSize) {
      xedEmit2(instr, XED_MEMREF(m, memSize), XED_IMM_RED(i, immSize, fitFunc),
               SZ_TO_BITS(memSize));
  }

  // instr(mem, reg)

#define XED_INSTMR_WRAPPER_IMPL(bitsize)                              \
  ALWAYS_INLINE                                                       \
  void xedInstrMR(xed_iclass_enum_t instr, const MemoryRef& m,        \
                  const Reg##bitsize& r,                              \
                  int memSize = BITS_TO_SZ(bitsize)) {                \
    xedEmit2(instr, XED_REG(r), XED_MEMREF(m, memSize), bitsize);     \
  }                                                                   \
  ALWAYS_INLINE                                                       \
  void xedInstrMR(xed_iclass_enum_t instr, const RIPRelativeRef& m,   \
                  const Reg##bitsize& r) {                            \
    uint32_t instrLen = xedEmit2Prep(instr, XED_REG(r),               \
                                     XED_MEMREF_RIP(m,                \
                                     BITS_TO_SZ(bitsize), 0),         \
                                     bitsize);                        \
    xedEmit2(instr, XED_REG(r),                                       \
             XED_MEMREF_RIP(m, BITS_TO_SZ(bitsize),                   \
             (int64_t)frontier() + (int64_t)instrLen), bitsize);      \
  }

#define XED_WRAP_X XED_INSTMR_WRAPPER_IMPL
  XED_WRAP_IMPL()
#undef XED_WRAP_X

  ALWAYS_INLINE
  void xedInstrMR(xed_iclass_enum_t instr, const MemoryRef& m,
                  const RegXMM& r, int memSize = sz::qword) {
    xedEmit2(instr, XED_REG(r), XED_MEMREF(m, memSize));
  }

  ALWAYS_INLINE                                                       \
  void xedInstrMR(xed_iclass_enum_t instr, const RIPRelativeRef& m,   \
                  const RegXMM& r, int memSize = sz::qword) {         \
    uint32_t instrLen = xedEmit2Prep(instr, XED_REG(r),               \
                                     XED_MEMREF_RIP(m, memSize, 0));  \
    xedEmit2(instr, XED_REG(r),                                       \
             XED_MEMREF_RIP(m, memSize, (int64_t)frontier() +         \
             (int64_t)instrLen));                                     \
  }

// instr(reg, mem)

#define XED_INSTRM_WRAPPER_IMPL(bitsize)                            \
  ALWAYS_INLINE                                                     \
  void xedInstrRM(xed_iclass_enum_t instr, const Reg##bitsize& r,   \
                  const MemoryRef& m) {                             \
    xedEmit2(instr, XED_MEMREF(m, BITS_TO_SZ(bitsize)), XED_REG(r), \
             bitsize);                                              \
  }

#define XED_WRAP_X XED_INSTRM_WRAPPER_IMPL
  XED_WRAP_IMPL()
#undef XED_WRAP_X

  ALWAYS_INLINE
  void xedInstrRM(xed_iclass_enum_t instr, const RegXMM& r,
                  const MemoryRef& m, int memSize = sz::qword) {
    xedEmit2(instr, XED_MEMREF(m, memSize), XED_REG(r));
  }

// instr(xmm, xmm, imm)

  ALWAYS_INLINE
  void xedInstrIRR(xed_iclass_enum_t instr, const RegXMM& r1, const RegXMM& r2,
                   const Immed& i, int immSize) {
    xedEmit3(instr, XED_REG(r1), XED_REG(r2), XED_IMM(i, immSize));
  }

// instr(relbr)

  void xedInstrRelBr(xed_iclass_enum_t instr, CodeAddress dest, int size)
  {
    auto target = dest - (codeBlock.frontier() +
                  xedEmit1Prep(instr,
                  XED_BRREL((CodeAddress)0, size)));
    xedEmit1(instr, XED_BRREL(target, size));
  }

// instr()

  ALWAYS_INLINE
  void xedInstr(xed_iclass_enum_t instr, int size = sz::qword) {
    xedEmit0(instr, SZ_TO_BITS(size));
  }

  CodeBlock& codeBlock;
};

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
