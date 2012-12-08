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
#ifndef incl_UTIL_ASM_X64_H_
#define incl_UTIL_ASM_X64_H_

#include <type_traits>

#include "util/util.h"
#include "util/base.h"
#include "util/atomic.h"
#include "util/trace.h"

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

#ifndef __x86_64__
/*
 * Technically, you could use this to generate x86_64 instructions on some
 * other platform, e.g., in a cross-compiler.
 *
 * Most likely, you didn't mean to do this, though.
 */
#error Your architecture is unsupported.
#endif

/*
 * Some members cannot be const because their values aren't known in
 * an initialization list. Like the opposite of the "mutable" keyword.
 * This declares this property to readers.
 */
#define logical_const /* nothing */

namespace HPHP { namespace VM { namespace Transl {

#define TRACEMOD ::HPHP::Trace::asmx64

//////////////////////////////////////////////////////////////////////

struct MemoryRef;
struct IndexedMemoryRef;
struct RIPRelativeRef;
struct ScaledIndex;
struct ScaledIndexDisp;
struct DispReg;

/*
 * Type for register numbers, independent of the size we're going to
 * be using it as.  Also, the same register number may mean different
 * physical registers for different instructions (e.g. xmm0 and rax
 * are both 0).
 *
 * This type is mainly published for backward compatability with the
 * APIs that look like store_reg##_disp_reg##, which predate the
 * size-specific types.  (Some day it may become internal to this
 * module.)
 */
enum class RegNumber : int {};

struct Reg64 {
  explicit constexpr Reg64(int rn) : rn(rn) {}

  // Implicit conversion for backward compatability only.  This is
  // needed to keep the store_reg##_disp_reg## style apis working.
  constexpr operator RegNumber() const { return RegNumber(rn); }

  // Integer conversion is allowed but only explicitly.  (It's not
  // unusual to want to printf registers, etc.  Just cast it first.)
  explicit constexpr operator int() const { return rn; }

  MemoryRef operator[](intptr_t disp) const;
  IndexedMemoryRef operator[](Reg64) const;
  IndexedMemoryRef operator[](ScaledIndex) const;
  IndexedMemoryRef operator[](ScaledIndexDisp) const;
  IndexedMemoryRef operator[](DispReg) const;

  constexpr bool operator==(Reg64 o) const { return rn == o.rn; }
  constexpr bool operator!=(Reg64 o) const { return rn != o.rn; }

private:
  int rn;
};

#define SIMPLE_REGTYPE(What)                                        \
  struct What {                                                     \
    explicit constexpr What(int rn) : rn(rn) {}                     \
    explicit constexpr operator RegNumber() const {                 \
      return RegNumber(rn);                                         \
    }                                                               \
    explicit constexpr operator int() const { return rn; }          \
    constexpr bool operator==(What o) const { return rn == o.rn; }  \
    constexpr bool operator!=(What o) const { return rn != o.rn; }  \
  private:                                                          \
    int rn;                                                         \
  }

SIMPLE_REGTYPE(Reg32);
SIMPLE_REGTYPE(Reg8);
SIMPLE_REGTYPE(RegXMM);

#undef SIMPLE_REGTYPE

struct RegRIP {
  RIPRelativeRef operator[](intptr_t disp) const;
};

// Go from a RegNumber to the same physical register of a given
// size.

inline Reg8 rbyte(RegNumber r) { return Reg8(int(r)); }
inline Reg8 rbyte(Reg32 r)     { return Reg8(int(r)); }
inline Reg32 r32(RegNumber r)  { return Reg32(int(r)); }
inline Reg64 r64(RegNumber r)  { return Reg64(int(r)); }

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
    ASSERT((scale == 0x1 || scale == 0x2 || scale == 0x4 || scale == 0x8) &&
           "Invalid index register scaling (must be 1,2,4 or 8).");
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
  {}

  MemoryRef operator*() const;

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

  IndexedMemoryRef operator*() const;

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
  intptr_t disp;
};

// rip+x
struct DispRIP {
  explicit DispRIP(intptr_t disp) : disp(disp) {}

  RIPRelativeRef operator*() const;

  DispRIP operator+(intptr_t x) const {
    return DispRIP(disp + x);
  }

  DispRIP operator-(intptr_t x) const {
    return DispRIP(disp - x);
  }

  intptr_t disp;
};

// *(reg + reg*x + y)
struct IndexedMemoryRef {
  explicit IndexedMemoryRef(IndexedDispReg r) : r(r) {}
  IndexedDispReg r;
};

// *(reg + x)
struct MemoryRef {
  explicit MemoryRef(DispReg r) : r(r) {}
  DispReg r;
};

// *(rip + x)
struct RIPRelativeRef {
  explicit RIPRelativeRef(DispRIP r) : r(r) {}
  DispRIP r;
};

inline IndexedMemoryRef IndexedDispReg::operator*() const {
  return IndexedMemoryRef(*this);
}

inline MemoryRef DispReg::operator*() const {
  return MemoryRef(*this);
}

inline RIPRelativeRef DispRIP::operator*() const {
  return RIPRelativeRef(*this);
}

inline DispReg operator+(Reg64 r, intptr_t d) { return DispReg(r, d); }
inline DispReg operator-(Reg64 r, intptr_t d) { return DispReg(r, -d); }
inline DispRIP operator+(RegRIP r, intptr_t d) { return DispRIP(d); }
inline DispRIP operator-(RegRIP r, intptr_t d) { return DispRIP(d); }

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
inline DispRIP   operator*(RegRIP r) { return DispRIP(0); }

inline MemoryRef Reg64::operator[](intptr_t disp) const {
  return *(*this + disp);
}

inline IndexedMemoryRef Reg64::operator[](Reg64 idx) const {
  return *(*this + idx * 1);
}

inline IndexedMemoryRef Reg64::operator[](ScaledIndex si) const {
  return *(*this + si);
}

inline IndexedMemoryRef Reg64::operator[](DispReg dr) const {
  return *(*this + ScaledIndex(dr.base, 0x1) + dr.disp);
}

inline IndexedMemoryRef Reg64::operator[](ScaledIndexDisp sid) const {
  return *(*this + sid.si + sid.disp);
}

inline RIPRelativeRef RegRIP::operator[](intptr_t disp) const {
  return *(*this + disp);
}

//////////////////////////////////////////////////////////////////////

namespace reg {
  const RegNumber noreg = RegNumber(-1);
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

  /*
   * rScratch is a symbolic name for a register that is always free. The
   * ABI is silent about this register, other than to say that it is callee
   * saved.
   */
  constexpr Reg64 rScratch(r10);

  constexpr RegRIP rip;

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
#undef X

}

//////////////////////////////////////////////////////////////////////

static inline void
atomic_store64(volatile uint64_t* dest, uint64_t value) {
  // gcc on x64 will implement this with a 64-bit store, and
  // normal 64-bit stores don't tear across instruction boundaries
  // assuming all 8 bytes of dest are on the same cacheline.
  *dest = value;
}

/*
 * Note that CodeAddresses are not const; the whole point is that we intend
 * to mutate them. uint8_t is as good a type as any: instructions are
 * bytes, and pointer arithmetic works correctly for the architecture.
 */
typedef uint8_t* CodeAddress;
typedef uint8_t* Address;

namespace sz {
  static const int nosize = 0;
  static const int byte  = 1;
  static const int word  = 2;
  static const int dword = 4;
  static const int qword = 8;
}

Address allocSlab(size_t size);
void freeSlab(Address addr, size_t size);

/*
 * This needs to be a POD type (no user-declared constructors is the most
 * important characteristic) so that it can be made thread-local.
 */
struct DataBlock {
  logical_const Address base;
  Address               frontier;
  size_t                size;

  /*
   * mmap()s in the desired amount of memory. The size member must be set.
   */
  void init();

  /*
   * munmap()s the DataBlock's memory
   */
  void free();

  /*
   * Uses a preallocated slab of memory
   */
  void init(Address start, size_t size);

  /*
   * alloc --
   *
   *   Simple bump allocator.
   *
   * allocAt --
   *
   *   Some clients need to allocate with an externally maintained frontier.
   *   allocAt supports this.
   */
  void* allocAt(size_t &frontierOff, size_t sz, size_t align = 16) {
    align = Util::roundUpToPowerOfTwo(align);
    uint8_t* frontier = base + frontierOff;
    ASSERT(base && frontier);
    int slop = uintptr_t(frontier) & (align - 1);
    if (slop) {
      int leftInBlock = (align - slop);
      frontier += leftInBlock;
      frontierOff += leftInBlock;
    }
    ASSERT((uintptr_t(frontier) & (align - 1)) == 0);
    frontierOff += sz;
    ASSERT(frontierOff <= size);
    return frontier;
  }

  template<typename T> T* alloc(size_t align = 16, int n = 1) {
    size_t frontierOff = frontier - base;
    T* retval = (T*)allocAt(frontierOff, sizeof(T) * n, align);
    frontier = base + frontierOff;
    return retval;
  }

  bool canEmit(size_t nBytes) {
    ASSERT(frontier >= base);
    ASSERT(frontier <= base + size);
    return frontier + nBytes <= base + size;
  }

  bool isValidAddress(const CodeAddress tca) const {
    return tca >= base && tca < (base + size);
  }

  void byte(const uint8_t byte) {
    ASSERT(canEmit(sz::byte));
    TRACE(10, "%p b : %02x\n", frontier, byte);
    *frontier = byte;
    frontier += sz::byte;
  }
  void word(const uint16_t word) {
    ASSERT(canEmit(sz::word));
    *(uint16_t*)frontier = word;
    TRACE(10, "%p w : %04x\n", frontier, word);
    frontier += sz::word;
  }
  void dword(const uint32_t dword) {
    ASSERT(canEmit(sz::dword));
    TRACE(10, "%p d : %08x\n", frontier, dword);
    *(uint32_t*)frontier = dword;
    frontier += sz::dword;
  }
  void qword(const uint64_t qword) {
    ASSERT(canEmit(sz::qword));
    TRACE(10, "%p q : %016lx\n", frontier, qword);
    *(uint64_t*)frontier = qword;
    frontier += sz::qword;
  }

  void bytes(size_t n, const uint8_t *bs) {
    ASSERT(canEmit(n));
    TRACE(10, "%p [%ld b] : [%p]\n", frontier, n, bs);
    if (n <= 8) {
      // If it is a modest number of bytes, try executing in one machine
      // store. This allows control-flow edges, including nop, to be
      // appear idempotent on other CPUs.
      union {
        uint64_t qword;
        uint8_t bytes[8];
      } u;
      u.qword = *(uint64_t*)frontier;
      for (size_t i = 0; i < n; ++i) {
        u.bytes[i] = bs[i];
      }
      atomic_store64((uint64_t*)frontier, u.qword);
    } else {
      memcpy(frontier, bs, n);
    }
    frontier += n;
  }

protected:
  void makeExecable();

  void *rawBytes(size_t n) {
    void* retval = (void*) frontier;
    frontier += n;
    return retval;
  }
};

/*
 * This is sugar on top of DataBlock, providing a constructor (see
 * DataBlock's comment for why it can't provide constructors itself) and
 * making the allocated memory executable.
 *
 * We seqeuntially pour code into a codeblock from beginning to end.
 * Managing entry points, ensuring the block is big enough, keeping track
 * of cross-codeblock references in code and data, etc., is beyond the
 * scope of this module.
 */
struct CodeBlock : public DataBlock {

  CodeBlock() {};

  /*
   * Allocate executable memory of the specified size, anywhere in
   * the address space.
   */
  void initCodeBlock(size_t sz);

  /*
   * User has pre-allocated the memory. This constructor might change
   * virtual memory permissions to make this block "+rwx".
   */
  void initCodeBlock(CodeAddress start, size_t len);
};

enum instrFlags {
  IF_REVERSE    = 0x0001, // The operand encoding for some instructions are
                          // "backwards" in x64; these instructions are
                          // called "reverse" instructions. There are a few
                          // details about emitting "reverse" instructions:
                          // (1) for the R_M address mode, we use the MR
                          // opcode, (2) for M_R and R address modes, we use
                          // the RM opcode, and (3) for the R_R address mode,
                          // we still use MR opcode, but we have to swap the
                          // first argument and the second argument.

  IF_TWOBYTEOP  = 0x0002, // Some instructions have two byte opcodes. For
                          // these instructions, an additional byte (0x0F) is
                          // emitted before the standard opcode byte.

  IF_JCC        = 0x0004, // instruction is jcc
  IF_IMUL       = 0x0008, // instruction is imul
  IF_HAS_IMM8   = 0x0010, // instruction has an encoding that takes an 8-bit
                          // immediate
  IF_SHIFT      = 0x0020, // instruction is rol, ror, rcl, rcr, shl, shr, sar
  IF_RET        = 0x0040, // instruction is ret
  IF_SHIFTD     = 0x0080, // instruction is shld, shrd
  IF_NO_REXW    = 0x0100, // rexW prefix is not needed
  IF_MOV        = 0x0200, // instruction is mov
  IF_COMPACTR   = 0x0400, // instruction supports compact-R encoding
  IF_RAX        = 0x0800, // instruction supports special rax encoding
  IF_XCHG       = 0x1000, // instruction is xchg (not xchgb)
  IF_BYTEREG    = 0x2000, // instruction is movzbq, movsbq
  IF_66PREFIXED = 0x4000, // instruction requires a manditory 0x66 prefix
  IF_F3PREFIXED = 0x8000, // instruction requires a manditory 0xf3 prefix
  IF_F2PREFIXED = 0x10000, // instruction requires a manditory 0xf2 prefix
};

/*
  Address mode to table index map:
      Table index 0 <- R_R / M_R(n) / R_M(r) / R(n)
      Table index 1 <- R_M(n) / M_R(r) / R(r)
      Table index 2 <- I / R_I / M_I / R_R_I / M_R_I / R_M_I
      Table index 3 <- "/digit" value used by the above address modes
      Table index 4 <- special R_I (for rax)
      Table index 5 <- compact-R / none

  (n) - for normal instructions only (IF_REVERSE flag is not set)
  (r) - for reverse instructions only (IF_REVERSE flag is set)

  0xF1 is used to indicate invalid opcodes.
*/

struct X64Instr {
  unsigned char table[6];
  unsigned long flags;
};

//                                    0    1    2    3    4    5     flags
const X64Instr instr_movdqa =  { { 0x6F,0x7F,0xF1,0x00,0xF1,0xF1 }, 0x4103 };
const X64Instr instr_movdqu =  { { 0x6F,0x7F,0xF1,0x00,0xF1,0xF1 }, 0x8103 };
const X64Instr instr_gpr2xmm = { { 0x6e,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x4002 };
const X64Instr instr_xmm2gpr = { { 0x7e,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x4002 };
const X64Instr instr_xmmsub =  { { 0x5c,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x10102 };
const X64Instr instr_xmmadd =  { { 0x58,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x10102 };
const X64Instr instr_xmmmul =  { { 0x59,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x10102 };
const X64Instr instr_ucomisd = { { 0x2e,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x4002 };
const X64Instr instr_pxor=     { { 0xef,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x4002 };
const X64Instr instr_cvtsi2sd= { { 0x2a,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x10002 };
const X64Instr instr_lddqu =   { { 0xF0,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x10103 };
const X64Instr instr_jmp =     { { 0xFF,0xF1,0xE9,0x04,0xE9,0xF1 }, 0x0910 };
const X64Instr instr_call =    { { 0xFF,0xF1,0xE8,0x02,0xE8,0xF1 }, 0x0900 };
const X64Instr instr_push =    { { 0xFF,0xF1,0x68,0x06,0xF1,0x50 }, 0x0510 };
const X64Instr instr_pop =     { { 0x8F,0xF1,0xF1,0x00,0xF1,0x58 }, 0x0500 };
const X64Instr instr_inc =     { { 0xFF,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_dec =     { { 0xFF,0xF1,0xF1,0x01,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_not =     { { 0xF7,0xF1,0xF1,0x02,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_neg =     { { 0xF7,0xF1,0xF1,0x03,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_add =     { { 0x01,0x03,0x81,0x00,0x05,0xF1 }, 0x0810 };
const X64Instr instr_sub =     { { 0x29,0x2B,0x81,0x05,0x2D,0xF1 }, 0x0810 };
const X64Instr instr_and =     { { 0x21,0x23,0x81,0x04,0x25,0xF1 }, 0x0810 };
const X64Instr instr_or  =     { { 0x09,0x0B,0x81,0x01,0x0D,0xF1 }, 0x0810 };
const X64Instr instr_xor =     { { 0x31,0x33,0x81,0x06,0x35,0xF1 }, 0x0810 };
const X64Instr instr_movb =    { { 0x88,0x8A,0xC6,0x00,0xF1,0xB0 }, 0x0610 };
const X64Instr instr_mov =     { { 0x89,0x8B,0xC7,0x00,0xF1,0xB8 }, 0x0600 };
const X64Instr instr_testb =   { { 0x84,0x84,0xF6,0x00,0xA8,0xF1 }, 0x0810 };
const X64Instr instr_test =    { { 0x85,0x85,0xF7,0x00,0xA9,0xF1 }, 0x0800 };
const X64Instr instr_cmpb =    { { 0x38,0x3A,0x80,0x07,0x3C,0xF1 }, 0x0810 };
const X64Instr instr_cmp =     { { 0x39,0x3B,0x81,0x07,0x3D,0xF1 }, 0x0810 };
const X64Instr instr_sbb =     { { 0x19,0x1B,0x81,0x03,0x1D,0xF1 }, 0x0810 };
const X64Instr instr_adc =     { { 0x11,0x13,0x81,0x02,0x15,0xF1 }, 0x0810 };
const X64Instr instr_lea =     { { 0xF1,0x8D,0xF1,0x00,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_xchgb =   { { 0x86,0x86,0xF1,0x00,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_xchg =    { { 0x87,0x87,0xF1,0x00,0xF1,0x90 }, 0x1000 };
const X64Instr instr_imul =    { { 0xAF,0xF7,0x69,0x05,0xF1,0xF1 }, 0x0019 };
const X64Instr instr_mul =     { { 0xF7,0xF1,0xF1,0x04,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_div =     { { 0xF7,0xF1,0xF1,0x06,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_idiv =    { { 0xF7,0xF1,0xF1,0x07,0xF1,0xF1 }, 0x0000 };
const X64Instr instr_cdq =     { { 0xF1,0xF1,0xF1,0x00,0xF1,0x99 }, 0x0400 };
const X64Instr instr_ret =     { { 0xF1,0xF1,0xC2,0x00,0xF1,0xC3 }, 0x0540 };
const X64Instr instr_jcc =     { { 0xF1,0xF1,0x80,0x00,0xF1,0xF1 }, 0x0114 };
const X64Instr instr_cmovcc =  { { 0x40,0x40,0xF1,0x00,0xF1,0xF1 }, 0x0003 };
const X64Instr instr_setcc =   { { 0x90,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0102 };
const X64Instr instr_movswx =  { { 0xBF,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0003 };
const X64Instr instr_movsbx =  { { 0xBE,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x2003 };
const X64Instr instr_movzwx =  { { 0xB7,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0003 };
const X64Instr instr_movzbx =  { { 0xB6,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x2003 };
const X64Instr instr_cwde =    { { 0xF1,0xF1,0xF1,0x00,0xF1,0x98 }, 0x0400 };
const X64Instr instr_rol =     { { 0xD3,0xF1,0xC1,0x00,0xF1,0xF1 }, 0x0020 };
const X64Instr instr_ror =     { { 0xD3,0xF1,0xC1,0x01,0xF1,0xF1 }, 0x0020 };
const X64Instr instr_rcl =     { { 0xD3,0xF1,0xC1,0x02,0xF1,0xF1 }, 0x0020 };
const X64Instr instr_rcr =     { { 0xD3,0xF1,0xC1,0x03,0xF1,0xF1 }, 0x0020 };
const X64Instr instr_shl =     { { 0xD3,0xF1,0xC1,0x04,0xF1,0xF1 }, 0x0020 };
const X64Instr instr_shr =     { { 0xD3,0xF1,0xC1,0x05,0xF1,0xF1 }, 0x0020 };
const X64Instr instr_sar =     { { 0xD3,0xF1,0xC1,0x07,0xF1,0xF1 }, 0x0020 };
const X64Instr instr_xadd =    { { 0xC1,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0002 };
const X64Instr instr_cmpxchg = { { 0xB1,0xF1,0xF1,0x00,0xF1,0xF1 }, 0x0002 };
const X64Instr instr_nop =     { { 0xF1,0xF1,0xF1,0x00,0xF1,0x90 }, 0x0500 };
const X64Instr instr_shld =    { { 0xA5,0xF1,0xA4,0x00,0xF1,0xF1 }, 0x0082 };
const X64Instr instr_shrd =    { { 0xAD,0xF1,0xAC,0x00,0xF1,0xF1 }, 0x0082 };
const X64Instr instr_int3 =    { { 0xF1,0xF1,0xF1,0x00,0xF1,0xCC }, 0x0500 };

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

inline ConditionCode ccNegate(ConditionCode c) {
  return ConditionCode(int(c) ^ 1); // And you thought x86 was irregular!
}

/*
 * When selecting encodings, we often need to assess a two's complement
 * distance to see if it fits in a shorter encoding.
 */
inline bool deltaFits(int64_t delta, int s) {
  // sz::qword is always true
  ASSERT(s == sz::byte ||
         s == sz::word ||
         s == sz::dword);
  int64_t bits = s * 8;
  return delta < (1ll << (bits-1)) && delta >= -(1ll << (bits-1));
}

// The unsigned equivalent of deltaFits
inline bool magFits(uint64_t val, int s) {
  // sz::qword is always true
  ASSERT(s == sz::byte ||
         s == sz::word ||
         s == sz::dword);
  uint64_t bits = s * 8;
  return (val & ((1ull << bits) - 1)) == val;
}

/*
 * Immediate wrapper for the assembler.
 *
 * This wrapper picks up whether the immediate argument was an integer
 * or a pointer type, so we don't have to cast pointers at callsites.
 *
 * Immediates are always treated as sign-extended values, but it's
 * often convenient to use unsigned types, so we allow it with an
 * implicit implementation-defined conversion.
 */
struct Immed {
  template<class T>
  /* implicit */ Immed(T i,
                       typename std::enable_if<
                         std::is_integral<T>::value ||
                         std::is_enum<T>::value
                       >::type* = 0)
    : m_int(i)
  {}

  template<class T>
  /* implicit */ Immed(T* p)
    : m_int(reinterpret_cast<uintptr_t>(p))
  {}

  int64_t q() const { return m_int; }
  int32_t l() const { return safe_cast<int32_t>(m_int); }
  int16_t w() const { return safe_cast<int16_t>(m_int); }
  int8_t  b() const { return safe_cast<int8_t>(m_int); }

  bool fits(int sz) const { return deltaFits(m_int, sz); }

private:
  intptr_t m_int;
};

///////////////////////////////////////////////////////////////////////////////
// License for Andrew J. Paroski's x86 machine code emitter

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
  CodeBlock code;

  // must use init() later
  X64Assembler() {}

  void init(size_t sz);
  void init(CodeAddress start, size_t sz);

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

#define LOAD_OP(name, instr)                                          \
  void name##q(MemoryRef m, Reg64 r) { instrMR(instr, m, r); }        \
  void name##l(MemoryRef m, Reg32 r) { instrMR(instr, m, r); }        \
  void name##q(IndexedMemoryRef m, Reg64 r) { instrMR(instr, m, r); } \
  void name##l(IndexedMemoryRef m, Reg32 r) { instrMR(instr, m, r); }

#define STORE_OP(name, instr)                                           \
  void name##l(Immed i, MemoryRef m) { instrIM32(instr, i, m); }        \
  void name##l(Reg32 r, MemoryRef m) { instrRM(instr, r, m); }          \
  void name##q(Reg64 r, MemoryRef m) { instrRM(instr, r, m); }          \
  void name##l(Immed i, IndexedMemoryRef m) { instrIM32(instr, i, m); } \
  void name##l(Reg32 r, IndexedMemoryRef m) { instrRM(instr, r, m); }   \
  void name##q(Reg64 r, IndexedMemoryRef m) { instrRM(instr, r, m); }

#define REG_OP(name, instr)                                       \
  void name##q(Reg64 r1, Reg64 r2)   { instrRR(instr, r1, r2); }  \
  void name##l(Reg32 r1, Reg32 r2)   { instrRR(instr, r1, r2); }  \
  void name##l(Immed i, Reg32 r)     { instrIR(instr, i, r); }

  /*
   * For when we a have a memory operand and the operand size is
   * 64-bits, only a 32-bit (sign-extended) immediate is supported.
   * If the immediate is too big, we'll move it into rScratch first.
   */
#define IMM64_STORE_OP(name, instr)             \
  void name##q(Immed i, MemoryRef m) {          \
    if (i.fits(sz::dword)) {                    \
      return instrIM(instr, i, m);              \
    }                                           \
    movq   (i, reg::rScratch);                  \
    name##q(reg::rScratch, m);                  \
  }                                             \
                                                \
  void name##q(Immed i, IndexedMemoryRef m) {   \
    if (i.fits(sz::dword)) {                    \
      return instrIM(instr, i, m);              \
    }                                           \
    movq   (i, reg::rScratch);                  \
    name##q(reg::rScratch, m);                  \
  }

  /*
   * For instructions other than movq, even when the operand size is
   * 64 bits only a 32-bit (sign-extended) immediate is supported.  We
   * provide foo##q instructions that may emit multiple x64
   * instructions (smashing rScratch) if the immediate does not
   * actually fit in a long.
   */
#define IMM64R_OP(name, instr)                  \
  void name##q(Immed imm, Reg64 r) {            \
    if (imm.fits(sz::dword)) {                  \
      return instrIR(instr, imm, r);            \
    }                                           \
    movq   (imm, reg::rScratch);                \
    name##q(reg::rScratch, r);                  \
  }

#define FULL_OP(name, instr)                    \
  LOAD_OP(name, instr)                          \
  STORE_OP(name, instr)                         \
  REG_OP(name, instr)                           \
  IMM64_STORE_OP(name, instr)                   \
  IMM64R_OP(name, instr)

#define BYTE_REG_OP(name, instr)                              \
  void name##b(Reg8 r1, Reg8 r2) { instrRR(instr, r1, r2); }  \
  void name##b(Immed i, Reg8 r)  { instrIR(instr, i, r); }

#define BYTE_LOAD_OP(name, instr)                                     \
  void name##b(MemoryRef m, Reg8 r)        { instrMR(instr, m, r); }  \
  void name##b(IndexedMemoryRef m, Reg8 r) { instrMR(instr, m, r); }

#define BYTE_STORE_OP(name, instr)                                      \
  void name##b(Reg8 r, MemoryRef m)         { instrRM(instr, r, m); }   \
  void name##b(Immed i, MemoryRef r)        { instrIM8(instr, i, r); }  \
  void name##b(Reg8 r, IndexedMemoryRef m)  { instrRM(instr, r, m); }   \
  void name##b(Immed i, IndexedMemoryRef r) { instrIM8(instr, i, r); }

#define BYTE_OP(name, instr)                    \
  BYTE_REG_OP(name, instr)                      \
  BYTE_LOAD_OP(name, instr)                     \
  BYTE_STORE_OP(name, instr)

  // We rename x64's mov to store and load for improved code
  // readability.
  LOAD_OP        (load,  instr_mov)
  STORE_OP       (store, instr_mov)
  IMM64_STORE_OP (store, instr_mov)
  REG_OP         (mov,   instr_mov)

  FULL_OP(add, instr_add)
  FULL_OP(xor, instr_xor)
  FULL_OP(sub, instr_sub)
  FULL_OP(and, instr_and)
  FULL_OP(or,  instr_or)
  FULL_OP(test,instr_test)
  FULL_OP(cmp, instr_cmp)

  BYTE_REG_OP  (mov,   instr_movb)
  BYTE_LOAD_OP (load,  instr_movb)
  BYTE_STORE_OP(store, instr_movb)

  BYTE_OP(test, instr_testb)
  BYTE_OP(cmp,  instr_cmpb)

#undef IMM64_OP
#undef IMM64R_OP
#undef FULL_OP
#undef STORE_OP
#undef LOAD_OP
#undef BYTE_OP
#undef BYTE_LOAD_OP
#undef BYTE_STORE_OP

  // 64-bit immediates work with mov to a register.
  void movq(Immed imm, Reg64 r) { instrIR(instr_mov, imm, r); }

  // movzbx is a special snowflake. We don't have movzbq because it behaves
  // exactly the same as movzbl but takes an extra byte.
  void loadzbl(MemoryRef m, Reg32 r)        { instrMR(instr_movzbx,
                                                      m, rbyte(r)); }
  void loadzbl(IndexedMemoryRef m, Reg32 r) { instrMR(instr_movzbx,
                                                      m, rbyte(r)); }
  void movzbl(Reg8 src, Reg32 dest)         { emitRR32(instr_movzbx,
                                                       rn(src), rn(dest)); }

  void lea(IndexedMemoryRef p, Reg64 reg) { instrMR(instr_lea, p, reg); }
  void lea(MemoryRef p, Reg64 reg)        { instrMR(instr_lea, p, reg); }
  void lea(RIPRelativeRef p, Reg64 reg)   { instrMR(instr_lea, p, reg); }

  void xchgq(Reg64 r1, Reg64 r2) { instrRR(instr_xchg, r1, r2); }
  void xchgl(Reg32 r1, Reg32 r2) { instrRR(instr_xchg, r1, r2); }
  void xchgb(Reg8 r1, Reg8 r2)   { instrRR(instr_xchgb, r1, r2); }
  void imul(Reg64 r1, Reg64 r2)  { instrRR(instr_imul, r1, r2); }

  void push(Reg64 r)  { instrR(instr_push, r); }
  void pop (Reg64 r)  { instrR(instr_pop,  r); }
  void idiv(Reg64 r)  { instrR(instr_idiv, r); }
  void incq(Reg64 r)  { instrR(instr_inc,  r); }
  void incl(Reg32 r)  { instrR(instr_inc,  r); }
  void decq(Reg64 r)  { instrR(instr_dec,  r); }
  void decl(Reg32 r)  { instrR(instr_dec,  r); }
  void not(Reg64 r)   { instrR(instr_not,  r); }
  void neg(Reg64 r)   { instrR(instr_neg,  r); }
  void ret()          { emit(instr_ret); }
  void ret(Immed i)   { emitI(instr_ret, i.w(), sz::word); }
  void nop()          { emit(instr_nop); }
  void int3()         { emit(instr_int3); }
  void ud2()          { byte(0x0f); byte(0x0b); }
  void pushf()        { byte(0x9c); }
  void popf()         { byte(0x9d); }
  void lock()         { byte(0xF0); }

  void push(MemoryRef m) { instrM(instr_push, m); }
  void pop (MemoryRef m) { instrM(instr_pop,  m); }
  void incq(MemoryRef m) { instrM(instr_inc,  m); }
  void incl(MemoryRef m) { instrM32(instr_inc, m); }
  void decq(MemoryRef m) { instrM(instr_dec,  m); }
  void decl(MemoryRef m) { instrM32(instr_inc, m); }

  void movdqu(RegXMM x, MemoryRef m)        { instrRM(instr_movdqu, x, m); }
  void movdqu(RegXMM x, IndexedMemoryRef m) { instrRM(instr_movdqu, x, m); }
  void movdqu(MemoryRef m, RegXMM x)        { instrMR(instr_movdqu, m, x); }
  void movdqu(IndexedMemoryRef m, RegXMM x) { instrMR(instr_movdqu, m, x); }
  void movdqa(RegXMM x, MemoryRef m)        { instrRM(instr_movdqa, x, m); }
  void movdqa(RegXMM x, IndexedMemoryRef m) { instrRM(instr_movdqa, x, m); }
  void movdqa(MemoryRef m, RegXMM x)        { instrMR(instr_movdqa, m, x); }
  void movdqa(IndexedMemoryRef m, RegXMM x) { instrMR(instr_movdqa, m, x); }
  void lddqu (MemoryRef m, RegXMM x)        { instrMR(instr_lddqu, m, x); }
  void lddqu (IndexedMemoryRef m, RegXMM x) { instrMR(instr_lddqu, m, x); }

  void shlq  (Immed i, Reg64 r) { instrIR(instr_shl, i.b(), r); }
  void shrq  (Immed i, Reg64 r) { instrIR(instr_shr, i.b(), r); }
  void shll  (Immed i, Reg32 r) { instrIR(instr_shl, i.b(), r); }
  void shrl  (Immed i, Reg32 r) { instrIR(instr_shr, i.b(), r); }

  /*
   * Control-flow directives. The labeling/patching facilities
   * available are primitive.
   */

  bool jmpDeltaFits(CodeAddress dest) {
    int64_t delta = dest - (code.frontier + 5);
    return deltaFits(delta, sz::dword);
  }

  void jmp(Reg64 r)            { instrR(instr_jmp, r); }
  void jmp(MemoryRef m)        { instrM(instr_jmp, m); }
  void jmp(IndexedMemoryRef m) { instrM(instr_jmp, m); }
  void call(Reg64 r)           { instrR(instr_call, r); }
  void call(MemoryRef m)       { instrM(instr_call, m); }
  void call(IndexedMemoryRef m){ instrM(instr_call, m); }

  void jmp8(CodeAddress dest)  { emitJ8(instr_jmp, ssize_t(dest)); }

  // May smash rScratch.
  void jmp(CodeAddress dest) {
    if (!jmpDeltaFits(dest)) {
      movq (dest, reg::rScratch);
      jmp  (reg::rScratch);
      return;
    }
    emitJ32(instr_jmp, ssize_t(dest));
  }

  // May smash rScratch.
  void call(CodeAddress dest) {
    if (!jmpDeltaFits(dest)) {
      movq (dest, reg::rScratch);
      call (reg::rScratch);
      return;
    }
    emitJ32(instr_call, ssize_t(dest));
  }

  void jcc(ConditionCode cond, CodeAddress dest) {
    emitCJ32(instr_jcc, cond, (ssize_t)dest);
  }

  void jcc8(ConditionCode cond, CodeAddress dest) {
    emitCJ8(instr_jcc, cond, (ssize_t)dest);
  }

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

#define CC(_nm, _code) \
  void j ## _nm(CodeAddress dest)      { jcc(_code, dest); } \
  void j ## _nm ## 8(CodeAddress dest) { jcc8(_code, dest); }
  CCS
#undef CC

  void setcc(int cc, Reg8 byteReg) {
    emitCR(instr_setcc, cc, rn(byteReg), sz::byte);
  }

#define CC(_nm, _cond)                          \
  void set ## _nm(Reg8 byteReg) {               \
    setcc(_cond, byteReg);                      \
  }
  CCS
#undef CC

  /*
   * The following utility functions do more than emit specific code.
   * (E.g. combine common idioms or patterns, smash code, etc.)
   */

  void emitImmReg(Immed imm, Reg64 dest) {
    if (imm.q() == 0) {
      // Zeros the top bits also.
      xorl  (r32(rn(dest)), r32(rn(dest)));
      return;
    }
    if (LIKELY(imm.q() > 0 && deltaFits(imm.q(), sz::dword))) {
      // This will zero out the high-order bits.
      movl (imm.l(), r32(rn(dest)));
      return;
    }
    movq (imm.q(), dest);
  }

  void patchJcc(CodeAddress jmp, CodeAddress dest) {
    ASSERT(jmp[0] == 0x0F && (jmp[1] & 0xF0) == 0x80);
    ssize_t diff = dest - (jmp + 6);
    *(int32_t*)(jmp + 2) = safe_cast<int32_t>(diff);
  }

  void patchJcc8(CodeAddress jmp, CodeAddress dest) {
    ASSERT((jmp[0] & 0xF0) == 0x70);
    ssize_t diff = dest - (jmp + 2);  // one for opcode, one for offset
    *(int8_t*)(jmp + 1) = safe_cast<int8_t>(diff);
  }

  void patchJmp(CodeAddress jmp, CodeAddress dest) {
    ASSERT(jmp[0] == 0xE9);
    ssize_t diff = dest - (jmp + 5);
    *(int32_t*)(jmp + 1) = safe_cast<int32_t>(diff);
  }

  void patchJmp8(CodeAddress jmp, CodeAddress dest) {
    ASSERT(jmp[0] == 0xEB);
    ssize_t diff = dest - (jmp + 2);  // one for opcode, one for offset
    *(int8_t*)(jmp + 1) = safe_cast<int8_t>(diff);
  }

  void patchCall(CodeAddress call, CodeAddress dest) {
    ASSERT(call[0] == 0xE8);
    ssize_t diff = dest - (call + 5);
    *(int32_t*)(call + 1) = safe_cast<int32_t>(diff);
  }

  void emitInt3s(int n) {
    memset(code.frontier, 0xcc, n);
    code.frontier += n;
  }

  void emitNop(int n) {
    if (n == 0) return;
    static const uint8_t nops[][9] = {
      { },
      { 0x90 },
      { 0x66, 0x90 },
      { 0x0f, 0x1f, 0x00 },
      { 0x0f, 0x1f, 0x40, 0x00 },
      { 0x0f, 0x1f, 0x44, 0x00, 0x00 },
      { 0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00 },
      { 0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00 },
      { 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 },
      { 0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 },
    };
    // While n >= 9, emit 9 byte NOPs
    while (n >= 9) {
      bytes(9, nops[9]);
      n -= 9;
    }
    bytes(n, nops[n]);
  }

  /*
   * Low-level emitter functions.
   *
   * These functions are the core of the assembler, and can also be
   * used directly.
   */

  void byte(uint8_t b) {
    code.byte(b);
  }
  void word(uint16_t w) {
    code.word(w);
  }
  void dword(uint32_t dw) {
    code.dword(dw);
  }
  void qword(uint64_t qw) {
    code.qword(qw);
  }
  void bytes(size_t n, const uint8_t* bs) {
    code.bytes(n, bs);
  }

  // op %r
  // ------
  // Restrictions:
  //     r cannot be set to 'none'
  void emitCR(X64Instr op, int jcond,
              RegNumber regN,
              int opSz = sz::qword) ALWAYS_INLINE {
    ASSERT(regN != reg::noreg);
    int r = int(regN);

    // REX
    unsigned char rex = 0;
    bool highByteReg = false;
    if (opSz == sz::byte) {
      if (byteRegNeedsRex(r)) {
        rex |= 0x40;
      }
      r = byteRegEncodeNumber(r, highByteReg);
    }
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
    if (r & 8) rex |= 1;
    if (rex) {
      byte(0x40 | rex);
      if (highByteReg) byteRegMisuse();
    }
    // If the instruction supports compact-R mode, use that
    if (op.flags & IF_COMPACTR) {
      byte(op.table[5] | (r & 7));
      return;
    }
    char opcode = (op.flags & IF_REVERSE) ? op.table[1] : op.table[0];
    char rval = op.table[3];
    // Handle two byte opcodes
    if (op.flags & IF_TWOBYTEOP) byte(0x0F);
    byte(opcode | jcond);
    emitModrm(3, rval, r);
  }

  void emitR(X64Instr op, RegNumber r, int opSz = sz::qword)
      ALWAYS_INLINE {
    emitCR(op, 0, r, opSz);
  }

  void emitR32(X64Instr op, RegNumber r) ALWAYS_INLINE {
    emitCR(op, 0, r, sz::dword);
  }

  // op %r2, %r1
  // -----------
  // Restrictions:
  //     r1 cannot be set to 'reg::noreg'
  //     r2 cannot be set to 'reg::noreg'
  void emitCRR(X64Instr op, int jcond, RegNumber rn1,
               RegNumber rn2, int opSz = sz::qword)
      ALWAYS_INLINE {
    ASSERT(rn1 != reg::noreg && rn2 != reg::noreg);
    int r1 = int(rn1);
    int r2 = int(rn2);
    bool reverse = ((op.flags & IF_REVERSE) != 0);
    prefixBytes(op.flags);
    // The xchg instruction is special; we have compact encodings for
    // exchanging with rax or eax.
    if (op.flags & IF_XCHG) {
      if (r1 == int(reg::rax)) {
        // REX
        unsigned char rex = 0;
        if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
        ASSERT(!(op.flags & IF_BYTEREG));
        if (r2 & 8) rex |= (reverse ? 4 : 1);
        if (rex) byte(0x40 | rex);
        // If the second register is rax, emit opcode with the first
        // register id embedded
        byte(op.table[5] | (r2 & 7));
        return;
      } else if (r2 == int(reg::rax)) {
        reverse = !reverse;
        // REX
        unsigned char rex = 0;
        if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) {
          rex |= 8;
        }
        if (r1 & 8) rex |= (reverse ? 1 : 4);
        if (rex) byte(0x40 | rex);
        // If the first register is rax, emit opcode with the second
        // register id embedded
        byte(op.table[5] | (r1 & 7));
        return;
      }
    }
    // REX
    unsigned char rex = 0;
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
    bool highByteReg = false;
    // movzbx's first operand is a bytereg regardless of operand size
    if (opSz == sz::byte || (op.flags & IF_BYTEREG)) {
      if (byteRegNeedsRex(r1) ||
          (!(op.flags & IF_BYTEREG) && byteRegNeedsRex(r2))) {
        rex |= 0x40;
      }
      r1 = byteRegEncodeNumber(r1, highByteReg);
      r2 = byteRegEncodeNumber(r2, highByteReg);
    }
    if (r1 & 8) rex |= (reverse ? 1 : 4);
    if (r2 & 8) rex |= (reverse ? 4 : 1);
    if (rex) {
      byte(0x40 | rex);
      if (highByteReg) byteRegMisuse();
    }
    // For two byte opcodes
    if ((op.flags & (IF_TWOBYTEOP | IF_IMUL)) != 0) byte(0x0F);
    byte(op.table[0] | jcond);
    if (reverse) {
      emitModrm(3, r2, r1);
    } else {
      emitModrm(3, r1, r2);
    }
  }

  void emitCRR32(X64Instr op, int jcond, RegNumber r1,
                 RegNumber r2)
      ALWAYS_INLINE {
    emitCRR(op, jcond, r1, r2, sz::dword);
  }

  void emitRR(X64Instr op, RegNumber r1, RegNumber r2,
              int opSz = sz::qword)
      ALWAYS_INLINE {
    emitCRR(op, 0, r1, r2, opSz);
  }

  void emitRR32(X64Instr op, RegNumber r1,
                RegNumber r2) ALWAYS_INLINE {
    emitCRR(op, 0, r1, r2, sz::dword);
  }

  void emitRR8(X64Instr op, RegNumber r1, RegNumber r2) {
    emitCRR(op, 0, r1, r2, sz::byte);
  }

  // op $imm, %r
  // -----------
  // Restrictions:
  //     r cannot be set to 'reg::noreg'
  void emitIR(X64Instr op, RegNumber rname, ssize_t imm,
              int opSz = sz::qword)
      ALWAYS_INLINE {
    ASSERT(rname != reg::noreg);
    int r = int(rname);
    // Determine the size of the immediate.  This might change opSz so
    // do it first.
    int immSize;
    if ((op.flags & IF_MOV) && opSz == sz::qword) {
      immSize = computeImmediateSizeForMovRI64(op, imm, opSz);
    } else {
      immSize = computeImmediateSize(op, imm);
    }
    // REX
    unsigned char rex = 0;
    bool highByteReg = false;
    if (opSz == sz::byte) {
      if (byteRegNeedsRex(r)) {
        rex |= 0x40;
      }
      r = byteRegEncodeNumber(r, highByteReg);
    }
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
    if (r & 8) rex |= 1;
    if (rex) {
      byte(0x40 | rex);
      if (highByteReg) byteRegMisuse();
    }
    // Use the special rax encoding if the instruction supports it
    if (r == int(reg::rax) && immSize == sz::dword &&
        (op.flags & IF_RAX)) {
      byte(op.table[4]);
      emitImmediate(op, imm, immSize);
      return;
    }
    // Use the compact-R encoding if the operand size and the immediate
    // size are the same
    if ((op.flags & IF_COMPACTR) && immSize == opSz) {
      byte(op.table[5] | (r & 7));
      emitImmediate(op, imm, immSize);
      return;
    }
    int rval = op.table[3];
    // shift/rotate instructions have special opcode when
    // immediate is 1
    if ((op.flags & IF_SHIFT) != 0 && imm == 1) {
      byte(0xd1);
      emitModrm(3, rval, r);
      // don't emit immediate
      return;
    }
    int opcode = (immSize == sz::byte && opSz != sz::byte) ?
      (op.table[2] | 2) : op.table[2];
    byte(opcode);
    emitModrm(3, rval, r);
    emitImmediate(op, imm, immSize);
  }

  void emitIR32(X64Instr op, RegNumber r, ssize_t imm)
      ALWAYS_INLINE {
    emitIR(op, r, imm, sz::dword);
  }

  void emitIR8(X64Instr op, RegNumber r, ssize_t imm) {
    emitIR(op, r, safe_cast<int8_t>(imm), sz::byte);
  }

  // op $imm, %r2, %r1
  // -----------------
  // Restrictions:
  //     r1 cannot be set to 'reg::noreg'
  //     r2 cannot be set to 'reg::noreg'
  void emitIRR(X64Instr op, RegNumber rn1, RegNumber rn2,
               ssize_t imm, int opSz = sz::qword)
      ALWAYS_INLINE {
    ASSERT(rn1 != reg::noreg && rn2 != reg::noreg);
    int r1 = int(rn1);
    int r2 = int(rn2);
    bool reverse = ((op.flags & IF_REVERSE) != 0);
    // REX
    unsigned char rex = 0;
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;
    if (r1 & 8) rex |= (reverse ? 1 : 4);
    if (r2 & 8) rex |= (reverse ? 4 : 1);
    if (rex) byte(0x40 | rex);
    // Determine the size of the immediate
    int immSize = computeImmediateSize(op, imm);
    // Use 2-byte opcode for cmovcc, setcc, movsx, movzx, movsx8, movzx8
    // instructions
    if ((op.flags & IF_TWOBYTEOP) != 0) byte(0x0F);
    int opcode = (immSize == sz::byte && opSz != sz::byte) ?
      (op.table[2] | 2) : op.table[2];
    byte(opcode);
    if (reverse) {
      emitModrm(3, r2, r1);
    } else {
      emitModrm(3, r1, r2);
    }
    emitImmediate(op, imm, immSize);
  }

  void emitCI(X64Instr op, int jcond, ssize_t imm, int opSz = sz::qword)
      ALWAYS_INLINE {
    // REX
    if ((op.flags & IF_NO_REXW) == 0) {
      byte(0x48);
    }
    // Determine the size of the immediate
    int immSize = computeImmediateSize(op, imm);
    // Emit opcode
    if ((op.flags & IF_JCC) != 0) {
      // jcc is weird so we handle it separately
      if (immSize != sz::byte) {
        byte(0x0F);
        byte(jcond | 0x80);
      } else {
        byte(jcond | 0x70);
      }
    } else {
      int opcode = (immSize == sz::byte && opSz != sz::byte) ?
        (op.table[2] | 2) : op.table[2];
      byte(jcond | opcode);
    }
    emitImmediate(op, imm, immSize);
  }

  void emitI(X64Instr op, ssize_t imm, int opSz = sz::qword)
      ALWAYS_INLINE {
    emitCI(op, 0, imm, opSz);
  }

  void emitJ8(X64Instr op, ssize_t imm)
    ALWAYS_INLINE {
    ASSERT((op.flags & IF_JCC) == 0);
    ssize_t delta = imm - ((ssize_t)code.frontier + 2);
    // Emit opcode and 8-bit immediate
    byte(0xEB);
    byte(safe_cast<int8_t>(delta));
  }

  void emitCJ8(X64Instr op, int jcond, ssize_t imm)
    ALWAYS_INLINE {
    // this is for jcc only
    ASSERT(op.flags & IF_JCC);
    ssize_t delta = imm - ((ssize_t)code.frontier + 2);
    // Emit opcode
    byte(jcond | 0x70);
    // Emit 8-bit offset
    byte(safe_cast<int8_t>(delta));
  }

  void emitJ32(X64Instr op, ssize_t imm) ALWAYS_INLINE {
    // call and jmp are supported, jcc is not supported
    ASSERT((op.flags & IF_JCC) == 0);
    int32_t delta = safe_cast<int32_t>(imm - ((ssize_t)code.frontier + 5));
    uint8_t *bdelta = (uint8_t*)&delta;
    uint8_t instr[] = { op.table[2],
      bdelta[0], bdelta[1], bdelta[2], bdelta[3] };
    bytes(5, instr);
  }

  void emitCJ32(X64Instr op, int jcond, ssize_t imm)
      ALWAYS_INLINE {
    // jcc is supported, call and jmp are not supported
    ASSERT(op.flags & IF_JCC);
    int32_t delta = safe_cast<int32_t>(imm - ((ssize_t)code.frontier + 6));
    uint8_t* bdelta = (uint8_t*)&delta;
    uint8_t instr[6] = { 0x0f, uint8_t(0x80 | jcond),
      bdelta[0], bdelta[1], bdelta[2], bdelta[3] };
    bytes(6, instr);
  }

  // op disp(%br,%ir,s)
  //   (for reverse == false, hasImmediate == false, r == reg::noreg)
  // op $imm, disp(%br,%ir,s)
  //   (for reverse == false, hasImmediate == true,  r == reg::noreg)
  // op %r, disp(%br,%ir,s)
  //   (for reverse == false, hasImmediate == false, r != reg::noreg)
  // op $imm, %r, disp(%br,%ir,s)
  //   (for reverse == false, hasImmediate == true,  r != reg::noreg)
  // op disp(%br,%ir,s), %r
  //   (for reverse == true,  hasImmediate == false, r != reg::noreg)
  // op $imm, disp(%br,%ir,s), %r
  //   (for reverse == true,  hasImmediate == true,  r != reg::noreg)
  // -----------------------------------------------------------------
  // Restrictions:
  //     ir cannot be set to 'sp'
  void emitCMX(X64Instr op, int jcond, RegNumber brName,
               RegNumber irName, int s, int disp,
               RegNumber rName, bool reverse, ssize_t imm,
               bool hasImmediate, int opSz = sz::qword,
               bool ripRelative = false) ALWAYS_INLINE {
    ASSERT(irName != reg::rsp);

    int ir = int(irName);
    int r = int(rName);
    int br = int(brName);

    // When an instruction has a manditory prefix, it goes before the
    // REX byte if we end up needing one.
    prefixBytes(op.flags);

    // Determine immSize from the 'hasImmediate' flag
    int immSize = sz::nosize;
    if (hasImmediate) {
      immSize = computeImmediateSize(op, imm);
    }
    if ((op.flags & IF_REVERSE) != 0) reverse = !reverse;
    // Determine if we need to use a two byte opcode;
    // imul is weird so we have a special case for it
    bool twoByteOpcode = ((op.flags & IF_TWOBYTEOP) != 0) ||
      ((op.flags & IF_IMUL) != 0 && rName != reg::noreg &&
      immSize == sz::nosize);
    // Again, imul is weird
    if ((op.flags & IF_IMUL) != 0 && rName != reg::noreg) {
      reverse = !reverse;
    }
    // The wily rex byte, a multipurpose extension to the opcode space for x64
    unsigned char rex = 0;
    if ((op.flags & IF_NO_REXW) == 0 && opSz == sz::qword) rex |= 8;

    bool highByteReg = false;
    // XXX: This IF_BYTEREG check is a special case for movzbl: we currently
    // encode it using an opSz of sz::byte but it doesn't actually have a
    // byte-sized operand like other instructions can.
    if (!(op.flags & IF_BYTEREG) && opSz == sz::byte && rName != reg::noreg) {
      if (byteRegNeedsRex(r)) {
        rex |= 0x40;
      }
      r = byteRegEncodeNumber(r, highByteReg);
    }

    if (rName != reg::noreg && (r & 8)) rex |= 4;
    if (irName != reg::noreg && (ir & 8)) rex |= 2;
    if (brName != reg::noreg && (br & 8)) rex |= 1;
    if (rex) {
      byte(0x40 | rex);
      if (highByteReg) byteRegMisuse();
    }
    // Emit the opcode
    if (immSize != sz::nosize) {
      if (twoByteOpcode) byte(0x0F);
      if (immSize == sz::byte && opSz != sz::byte) {
        byte(op.table[2] | 2 | jcond);
      } else {
        byte(op.table[2] | jcond);
      }
    } else {
      if (twoByteOpcode) byte(0x0F);
      int opcode;
      if ((op.flags & IF_IMUL) != 0) {
        opcode = (rName == reg::noreg) ? op.table[1] : op.table[0];
      } else {
        opcode = reverse ? op.table[1] : op.table[0];
      }
      byte(opcode | jcond);
    }
    // SIB byte if:
    //   1. We're using an index register.
    //   2. The base register is rsp-like.
    //   3. We're doing a baseless disp access and it is not rip-relative.
    bool sibIsNeeded =
      ir != int(reg::noreg) ||                      /* 1 */
      br == int(reg::rsp) || br == int(reg::r12) || /* 2 */
      (br == int(reg::noreg) && !ripRelative);
    // If there is no register and no immediate, use the /r value
    if (r == int(reg::noreg)) r = op.table[3];
    // If 'reg::noreg' was specified for 'ir', we use
    // the encoding for the sp register
    if (ir == int(reg::noreg)) ir = 4;
    int dispSize = sz::nosize;
    if (disp != 0) {
      if (disp <= 127 && disp >= -128) {
        dispSize = sz::byte;
      } else {
        dispSize = sz::dword;
      }
    }
    // Set 'mod' based on the size of the displacement
    int mod;
    switch (dispSize) {
      case sz::nosize: mod = 0; break;
      case sz::byte: mod = 1; break;
      default: mod = 2; break;
    }
    // Handle special cases for 'br'
    if (br == int(reg::noreg)) {
      // If 'reg::noreg' was specified for 'br', we use the encoding
      // for the rbp register (or rip, if we're emitting a
      // rip-relative instruction), and we must set mod=0 and
      // "upgrade" to a DWORD-sized displacement
      br = 5;
      mod = 0;
      dispSize = sz::dword;
    } else if ((br & 7) == 5 && dispSize == sz::nosize) {
      // If br == rbp and no displacement was specified, we
      // must "upgrade" to using a 1-byte displacement value
      dispSize = sz::byte;
      mod = 1;
    }
    // Emit modr/m and the sib
    if (sibIsNeeded) {
      // s:                               0  1  2   3  4   5   6   7  8
      static const int scaleLookup[] = { -1, 0, 1, -1, 2, -1, -1, -1, 3 };
      ASSERT(s > 0 && s <= 8);
      int scale = scaleLookup[s];
      ASSERT(scale != -1);
      emitModrm(mod, r, 4);
      byte((scale << 6) | ((ir & 7) << 3) | (br & 7));
    } else {
      emitModrm(mod, r, br);
    }
    // Emit displacement if needed
    if (dispSize == sz::dword) {
      dword(disp);
    } else if (dispSize == sz::byte) {
      byte(disp & 0xff);
    }
    // Emit immediate if needed
    if (immSize != sz::nosize) {
      emitImmediate(op, imm, immSize);
    }
  }

  void emitIM(X64Instr op, RegNumber br, RegNumber ir,
              int s, int disp, ssize_t imm, int opSz = sz::qword)
        ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, reg::noreg, false, imm, true, opSz);
  }

  void emitIM8(X64Instr op, RegNumber br, RegNumber ir, int s,
               int disp, ssize_t imm) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, reg::noreg, false, imm, true,
            sz::byte);
  }

  void emitIM32(X64Instr op, RegNumber br, RegNumber ir, int s,
                int disp, ssize_t imm) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, reg::noreg, false, imm, true,
            sz::dword);
  }

  void emitRM(X64Instr op, RegNumber br, RegNumber ir, int s,
              int disp, RegNumber r, int opSz = sz::qword)
        ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, r, false, 0, false, opSz);
  }

  void emitRM32(X64Instr op, RegNumber br, RegNumber ir, int s,
                int disp, RegNumber r) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, r, false, 0, false, sz::dword);
  }

  void emitRM8(X64Instr op, RegNumber br, RegNumber ir, int s,
               int disp, RegNumber r) {
    emitCMX(op, 0, br, ir, s, disp, r, false, 0, false, sz::byte);
  }

  void emitCMR(X64Instr op, int jcond, RegNumber br,
               RegNumber ir, int s, int disp, RegNumber r,
               int opSz = sz::qword) ALWAYS_INLINE {
    emitCMX(op, jcond, br, ir, s, disp, r, true, 0, false, opSz);
  }

  void emitMR(X64Instr op, RegNumber br, RegNumber ir, int s,
              int disp, RegNumber r, int opSz = sz::qword,
              bool ripRelative = false) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, r, true, 0, false, opSz, ripRelative);
  }

  void emitMR32(X64Instr op, RegNumber br, RegNumber ir,
                int s, int disp, RegNumber r) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, r, true, 0, false, sz::dword);
  }

  void emitMR8(X64Instr op, RegNumber br, RegNumber ir,
               int s, int disp, RegNumber r) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, r, true, 0, false, sz::byte);
  }

  void emitIRM(X64Instr op, RegNumber br, RegNumber ir,
               int s, int disp, RegNumber r, ssize_t imm,
               int opSz = sz::qword) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, r, false, imm, true, opSz);
  }

  void emitIMR(X64Instr op, RegNumber br, RegNumber ir,
               int s, int disp, RegNumber r, ssize_t imm,
               int opSz = sz::qword) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, r, true, imm, true, opSz);
  }

  void emitM(X64Instr op, RegNumber br, RegNumber ir,
             int s, int disp, int opSz = sz::qword) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, reg::noreg, false, 0, false, opSz);
  }

  void emitM32(X64Instr op, RegNumber br, RegNumber ir,
               int s, int disp) ALWAYS_INLINE {
    emitCMX(op, 0, br, ir, s, disp, reg::noreg, false, 0, false,
            sz::dword);
  }

  void emitCM(X64Instr op, int jcond, RegNumber br,
              RegNumber ir, int s, int disp, int opSz = sz::qword)
        ALWAYS_INLINE {
    emitCMX(op, jcond, br, ir, s, disp, reg::noreg, false, 0, false, opSz);
  }

  // emit (with no arguments)
  void emit(X64Instr op) ALWAYS_INLINE {
    if ((op.flags & IF_NO_REXW) == 0) {
      byte(0x48);
    }
    byte(op.table[5]);
  }

  // Segment register prefixes.
  void fs()           { byte(0x64); }
  void gs()           { byte(0x65); }

public:
  /*
   * The following functions are an older API to the assembler, which
   * still partially exist here for backward compatability.
   *
   * Our ordering convention follows the gas standard of "destination
   * last": <op>_<src1>_<src2>_<dest>. Be warned that Intel manuals go the
   * other way; in practice it's more important to be consistent with the
   * tools (gdb, gas, inline asm, etc.) than the manuals, since you look at
   * the former an order of magnitude more.
   */

  inline void mov_imm64_reg(int64_t imm, RegNumber rn) {
    emitIR(instr_mov, rn, imm);
  }

  inline void mov_imm32_reg32(int32_t imm, RegNumber rn) {
    emitIR(instr_mov, rn, imm, sz::dword);
  }

  inline void store_reg32_disp_reg64(RegNumber rsrc,
                                     int off,
                                     RegNumber rdest) {
    emitRM32(instr_mov, rdest, reg::noreg, sz::byte, off, rsrc);
  }

  inline void load_reg64_disp_reg8(RegNumber rsrc, int off,
                                   RegNumber rdest) {
    emitMR8(instr_movb, rsrc, reg::noreg, sz::byte, off, rdest);
  }

  inline void load_reg64_disp_reg32(RegNumber rsrc, int off,
                                    RegNumber rdest) {
    emitMR32(instr_mov, rsrc, reg::noreg, sz::byte, off, rdest);
  }

  inline void load_disp32_reg64(int disp, RegNumber rdest) {
    emitMR(instr_mov, reg::noreg, reg::noreg, sz::byte, disp, rdest);
  }

  inline void load_reg64_disp_index_reg32(RegNumber rsrc,
                                          int disp,
                                          RegNumber rindex,
                                          RegNumber rdest) {
    emitMR32(instr_mov, rsrc, rindex, sz::byte, disp, rdest);
  }

  inline void load_reg64_index_scale_disp_reg64(RegNumber rbase,
                                                RegNumber rindex,
                                                int scale, int disp,
                                                RegNumber rdest) {
    emitMR(instr_mov, rbase, rindex, scale, disp, rdest);
  }

  inline void load_reg64_index_scale_disp_reg32(RegNumber rbase,
                                                RegNumber rindex,
                                                int scale, int disp,
                                                RegNumber rdest) {
    emitMR32(instr_mov, rbase, rindex, scale, disp, rdest);
  }

  inline void store_imm8_disp_reg(int imm, int off, RegNumber rdest) {
    emitIM8(instr_movb, rdest, reg::noreg, sz::byte, off, imm);
  }

  inline void store_imm32_disp_reg(int imm, int off, RegNumber rdest) {
    emitIM32(instr_mov, rdest, reg::noreg, sz::byte, off, imm);
  }

  inline void mov_reg64_reg64(RegNumber rsrc, RegNumber rdest) {
    emitRR(instr_mov, rsrc, rdest);
  }

  inline void mov_reg32_reg32(RegNumber rsrc, RegNumber rdest) {
    emitRR32(instr_mov, rsrc, rdest);
  }

  inline void store_imm64_disp_reg64(int64_t imm, int off,
                                     RegNumber rdest) {
    if (deltaFits(imm, sz::dword)) {
      emitIM(instr_mov, rdest, reg::noreg, sz::byte, off, imm);
    } else {
      mov_imm64_reg(imm, reg::rScratch);
      emitRM(instr_mov, rdest, reg::noreg, sz::byte, off, reg::rScratch);
    }
  }
  // mov %rsrc, disp(%rdest)
  inline void store_reg64_disp_reg64(RegNumber rsrc, int off,
                                     RegNumber rdest) {
    emitRM(instr_mov, rdest, reg::noreg, sz::byte, off, rsrc);
  }

  // mov disp(%rsrc), %rdest
  inline void load_reg64_disp_reg64(RegNumber rsrc, int off,
                                    RegNumber rdest) {
    emitMR(instr_mov, rsrc, reg::noreg, sz::byte, off, rdest);
  }

  // mov disp(%rsrc) + S*%rindex, %rdest
  inline void load_reg64_disp_index_reg64(RegNumber rsrc,
                                          int off,
                                          RegNumber rindex,
                                          RegNumber rdest) {
    emitMR(instr_mov, rsrc, rindex, sz::qword, off, rdest);
  }

  // CMOVcc [rbase + off], rdest
  inline void cload_reg64_disp_reg64(ConditionCode cc, RegNumber rbase,
                                     int off, RegNumber rdest) {
    emitCMX(instr_cmovcc, cc, rbase, reg::noreg, sz::byte, off,
            rdest, false, 0, false);
  }
  inline void cmov_reg64_reg64(ConditionCode cc, RegNumber rsrc,
                               RegNumber rdest) {
    emitCRR(instr_cmovcc, cc, rsrc, rdest);
  }

  // lea disp(%rsrc), %rdest
  inline void lea_reg64_disp_reg64(RegNumber rsrc, int off,
                                   RegNumber rdest) {
    emitMR(instr_lea, rsrc, reg::noreg, sz::byte, off, rdest);
  }

  /*
   * Escaped opcodes for setcc family of instructions; always preceded with
   * lock prefix/opcode escape byte 0x0f for these meanings. Generally if
   * setX tests for condition foo, setX ^ 1 test for condition !foo.
   *
   * Some are aliases: "nge" is the same as "l", and "equal" and "zero" are
   * treated the same on x86.
   */

#define SIMPLE_OP(name)                                                 \
  /* op rsrc, rdest */                                                  \
  inline void name ## _reg64_reg64(RegNumber rsrc,                \
                                   RegNumber rdest) {             \
    emitRR(instr_ ## name, rsrc, rdest);                                \
  }                                                                     \
  /* op esrc, edest */                                                  \
  inline void name ## _reg32_reg32(RegNumber rsrc,                \
                                   RegNumber rdest) {             \
    emitRR32(instr_ ## name, rsrc, rdest);                              \
  }                                                                     \
  /* op imm32, rdest */                                                 \
  inline void name ## _imm32_reg64(int64_t imm, RegNumber rdest) { \
    emitIR(instr_ ## name, rdest, safe_cast<int32_t>(imm));             \
  }                                                                     \
  /* op imm32, edest */                                                 \
  inline void name ## _imm32_reg32(int64_t imm, RegNumber rdest) { \
    emitIR32(instr_ ## name, rdest, safe_cast<int32_t>(imm));           \
  }                                                                     \
  /* opl imm, disp(rdest) */                                            \
  inline void name ## _imm32_disp_reg32(int64_t imm, int disp,          \
                                        RegNumber rdest) {        \
    emitIM32(instr_ ## name, rdest, reg::noreg,                         \
             sz::byte, disp, safe_cast<int32_t>(imm));                  \
  }                                                                     \
  /* opq imm, disp(rdest) */                                            \
  inline void name ## _imm64_disp_reg64(int64_t imm, int disp,          \
                                        RegNumber rdest) {        \
    emitIM(instr_ ## name, rdest, reg::noreg, sz::byte,                 \
           disp, imm);                                                  \
  }                                                                     \
  /* op imm64, rdest */                                                 \
  /* NOTE: This will emit multiple x64 instructions and use the */      \
  /* scratch register if the immediate does not fit in 32 bits. */      \
  inline void name ## _imm64_reg64(int64_t imm, RegNumber rdest) { \
    if (deltaFits(imm, sz::dword)) {                                    \
      name ## _imm32_reg64(imm, rdest);                                 \
      return;                                                           \
    }                                                                   \
    mov_imm64_reg(imm, reg::rScratch);                                  \
    name ## _reg64_reg64(reg::rScratch, rdest);                         \
  }                                                                     \
  /* opq rsrc, disp(rdest) */                                           \
  inline void name ## _reg64_disp_reg64(RegNumber rsrc, int disp, \
                                        RegNumber rdest) {        \
    emitRM(instr_ ## name, rdest, reg::noreg,                           \
           sz::byte, disp, rsrc);                                       \
  }                                                                     \
  /* opl esrc, disp(rdest) */                                           \
  inline void name ## _reg32_disp_reg64(RegNumber rsrc, int disp, \
                                        RegNumber rdest) {        \
    emitRM32(instr_ ## name, rdest, reg::noreg, sz::byte, disp, rsrc);  \
  }                                                                     \
  /* opq disp(rsrc), rdest */                                           \
  inline void name ## _disp_reg64_reg64(int disp, RegNumber rsrc, \
                                        RegNumber rdest) {        \
    emitMR(instr_ ## name, rsrc, reg::noreg, sz::byte, disp, rdest);    \
  }                                                                     \
  /* opl disp(esrc), edest */                                           \
  inline void name ## _disp_reg64_reg32(int disp, RegNumber rsrc, \
                                        RegNumber rdest) {        \
    emitMR32(instr_ ## name, rsrc, reg::noreg, sz::byte, disp, rdest);  \
  }

#define SCALED_OP(name)                                                 \
  SIMPLE_OP(name)                                                       \
  JUST_SCALED_OP(name)
#define JUST_SCALED_OP(name) \
  /* opl rsrc, disp(rbase, rindex, scale), rdest */                     \
  inline void name ## _reg64_reg64_index_scale_disp(RegNumber rsrc, \
                      RegNumber rbase, RegNumber rindex,    \
                      int scale, int disp) {                            \
    emitRM(instr_ ## name, rbase, rindex, scale, disp, rsrc);           \
  }                                                                     \
  /* opl disp(rbase, rindex, scale), rdest */                           \
  inline void name ## _reg64_index_scale_disp_reg64(RegNumber rbase, \
                      RegNumber rindex, int scale, int disp,      \
                      RegNumber rdest) {                          \
    emitMR(instr_ ## name, rbase, rindex, scale, disp, rdest);          \
  }                                                                     \
  /* opq imm, disp(rdest, rindex, scale) */                             \
  inline void name ## _imm64_index_scale_disp_reg64(                    \
    int64 imm, RegNumber rindex, int scale, int disp,             \
    RegNumber rdest) {                                            \
    emitIM(instr_ ## name, rdest, rindex, scale, disp, imm);             \
  }

  SCALED_OP(add)
  SCALED_OP(xor)
  SCALED_OP(sub)
  SCALED_OP(and)
  SCALED_OP(or)
  SCALED_OP(test)
  SCALED_OP(cmp)
  JUST_SCALED_OP(lea)
#undef SCALED_OP
#undef SIMPLE_OP
#undef JUST_SCALED_OP

  // imul rsrc, rdest
  inline void imul_reg64_reg64(RegNumber rsrc, RegNumber rdest) {
    emitRR(instr_imul, rsrc, rdest);
  }

  // imul imm, rdest
  inline void imul_imm64_reg64(int64_t imm, RegNumber rdest) {
    mov_imm64_reg(imm, reg::rScratch);
    imul_reg64_reg64(reg::rScratch, rdest);
  }

  inline void xchg_reg64_reg64(RegNumber rsrc, RegNumber rdest) {
    emitRR(instr_xchg, rsrc, rdest);
  }
  inline void xchg_reg32_reg32(RegNumber rsrc, RegNumber rdest) {
    emitRR32(instr_xchg, rsrc, rdest);
  }

  inline void mov_reg64_mmx(RegNumber rnsrc, RegNumber rndest) {
    int rsrc = (int)rnsrc;
    int rdst = (int)rndest;
    // REX
    unsigned char rex = 0x48;
    if (rsrc & 8) rex |= 1;
    byte(rex);
    // two-byte opcode
    byte(0x0F);
    byte(0x6E);
    emitModrm(3, rdst, rsrc);
  }
  inline void mov_mmx_reg64(RegNumber rnsrc, RegNumber rndest) {
    int rsrc = (int)rnsrc;
    int rdst = (int)rndest;
    // REX
    unsigned char rex = 0x48;
    if (rdst & 8) rex |= 1;
    byte(rex);
    // two-byte opcode
    byte(0x0F);
    byte(0x7E);
    emitModrm(3, rsrc, rdst);
  }

  void mov_reg64_xmm(RegNumber rSrc, RegXMM rdest) {
    emitRR(instr_gpr2xmm, rn(rdest), rSrc);
  }
  void mov_xmm_reg64(RegXMM rSrc, RegNumber rdest) {
    emitRR(instr_xmm2gpr, rn(rSrc), rdest);
  }

  void addsd_xmm_xmm(RegXMM src, RegXMM srcdest) {
    emitRR(instr_xmmadd, rn(srcdest), rn(src));
  }
  void mulsd_xmm_xmm(RegXMM src, RegXMM srcdest) {
    emitRR(instr_xmmmul, rn(srcdest), rn(src));
  }
  void subsd_xmm_xmm(RegXMM src, RegXMM srcdest) {
    emitRR(instr_xmmsub, rn(srcdest), rn(src));
  }
  void pxor_xmm_xmm(RegXMM src, RegXMM srcdest) {
    emitRR(instr_pxor, rn(srcdest), rn(src));
  }
  void cvtsi2sd_reg64_xmm(RegNumber src, RegXMM dest) {
    emitRR(instr_cvtsi2sd, rn(dest), src);
  }
  void ucomisd_xmm_xmm(RegXMM l, RegXMM r) {
    emitRR(instr_ucomisd, rn(l), rn(r));
  }

private:
  bool byteRegNeedsRex(int rn) const {
    // Without a rex, 4 through 7 mean the high 8-bit byte registers.
    return rn >= 4 && rn <= 7;
  }
  int byteRegEncodeNumber(int rn, bool& seenHigh) const {
    // We flag a bit in ah, ch, dh, bh so byteRegNeedsRex doesn't
    // trigger.
    if (rn & 0x80) seenHigh = true;
    return rn & ~0x80;
  }
  // In 64-bit mode, you can't mix accesses to high byte registers
  // with low byte registers other than al,cl,bl,dl.  We assert this.
  void byteRegMisuse() const {
    ASSERT(!"High byte registers can't be used with new x64 registers, or"
            " anything requiring a REX prefix");
  }

  int computeImmediateSize(X64Instr op, ssize_t imm) {
    // Most instructions take a 32-bit immediate, except
    // for ret which takes a 16-bit immediate
    int immSize = sz::dword;
    if (op.flags & IF_RET) {
      immSize = sz::word;
    }
    // Use an 8-bit immediate if the instruction supports it and if
    // the immediate value fits in a byte
    if (deltaFits(imm, sz::byte) && (op.flags & IF_HAS_IMM8) != 0) {
      immSize = sz::byte;
    }
    return immSize;
  }

  void emitModrm(int x, int y, int z) {
    byte((x << 6) | ((y & 7) << 3) | (z & 7));
  }

  /*
   * The mov instruction supports an 8 byte immediate for the RI
   * address mode when opSz is qword.  It also supports a 4-byte
   * immediate with opSz qword (the immediate is sign-extended).
   *
   * On the other hand, if it fits in 32-bits as an unsigned, we can
   * change opSz to dword, which will zero the top 4 bytes instead of
   * sign-extending.
   */
  int computeImmediateSizeForMovRI64(X64Instr op, ssize_t imm, int& opSz) {
    ASSERT(opSz == sz::qword);
    if (deltaFits(imm, sz::dword)) {
      return computeImmediateSize(op, imm);
    }
    if (magFits(imm, sz::dword)) {
      opSz = sz::dword;
      return sz::dword;
    }
    return sz::qword;
  }

  void emitImmediate(X64Instr op, ssize_t imm, int immSize) {
    if (immSize == sz::nosize) {
      return;
    }
    if ((op.flags & (IF_SHIFT | IF_SHIFTD)) == 0) {
      if (immSize == sz::dword) {
        dword(imm);
      } else if (immSize == sz::byte) {
        byte(imm);
      } else if (immSize == sz::word) {
        word(imm);
      } else {
        qword(imm);
      }
    } else {
      // we always use a byte-sized immediate for shift instructions
      byte(imm);
    }
  }

  void prefixBytes(unsigned long flags) {
    if (flags & IF_66PREFIXED) byte(0x66);
    if (flags & IF_F2PREFIXED) byte(0xF2);
    if (flags & IF_F3PREFIXED) byte(0xF3);
  }

private:
  RegNumber rn(Reg8 r)   { return RegNumber(r); }
  RegNumber rn(Reg32 r)  { return RegNumber(r); }
  RegNumber rn(Reg64 r)  { return RegNumber(r); }
  RegNumber rn(RegXMM x) { return RegNumber(x); }

  // Wraps a bunch of the emit* functions to make using them with the
  // typed wrappers more terse.  TODO: we should have these replace
  // the emit functions eventually.

#define UMR(m)  rn(m.r.base), reg::noreg, sz::byte, m.r.disp
#define UIMR(m) rn(m.r.base), rn(m.r.index), m.r.scale, m.r.disp
#define URIP(m) reg::noreg, reg::noreg, sz::byte, m.r.disp

  void instrR(X64Instr op, Reg64 r)           { emitR(op, rn(r)); }
  void instrR(X64Instr op, Reg32 r)           { emitR32(op, rn(r)); }
  void instrRR(X64Instr op, Reg64 x, Reg64 y) { emitRR(op, rn(x), rn(y)); }
  void instrRR(X64Instr op, Reg32 x, Reg32 y) { emitRR32(op, rn(x), rn(y)); }
  void instrRR(X64Instr op, Reg8 x, Reg8 y)   { emitRR8(op, rn(x), rn(y)); }
  void instrM(X64Instr op, MemoryRef m)       { emitM(op, UMR(m)); }
  void instrM(X64Instr op, IndexedMemoryRef m){ emitM(op, UIMR(m)); }
  void instrM32(X64Instr op, MemoryRef m)     { emitM32(op, UMR(m)); }

  void instrRM(X64Instr op,
               Reg64 r,
               MemoryRef m)        { emitRM(op, UMR(m), rn(r)); }
  void instrRM(X64Instr op,
               Reg32 r,
               MemoryRef m)        { emitRM32(op, UMR(m), rn(r)); }
  void instrRM(X64Instr op,
               Reg8 r,
               MemoryRef m)        { emitRM8(op, UMR(m), rn(r)); }
  void instrRM(X64Instr op,
               Reg64 r,
               IndexedMemoryRef m) { emitRM(op, UIMR(m), rn(r)); }
  void instrRM(X64Instr op,
               Reg32 r,
               IndexedMemoryRef m) { emitRM32(op, UIMR(m), rn(r)); }
  void instrRM(X64Instr op,
               Reg8 r,
               IndexedMemoryRef m) { emitRM8(op, UIMR(m), rn(r)); }
  void instrRM(X64Instr op,
               RegXMM x,
               MemoryRef m)        { emitRM(op, UMR(m), rn(x)); }
  void instrRM(X64Instr op,
               RegXMM x,
               IndexedMemoryRef m) { emitRM(op, UIMR(m), rn(x)); }

  void instrMR(X64Instr op,
               MemoryRef m,
               Reg64 r)            { emitMR(op, UMR(m), rn(r)); }
  void instrMR(X64Instr op,
               MemoryRef m,
               Reg32 r)            { emitMR32(op, UMR(m), rn(r)); }
  void instrMR(X64Instr op,
               MemoryRef m,
               Reg8 r)             { emitMR8(op, UMR(m), rn(r)); }
  void instrMR(X64Instr op,
               IndexedMemoryRef m,
               Reg64 r)            { emitMR(op, UIMR(m), rn(r)); }
  void instrMR(X64Instr op,
               IndexedMemoryRef m,
               Reg32 r)            { emitMR32(op, UIMR(m), rn(r)); }
  void instrMR(X64Instr op,
               IndexedMemoryRef m,
               Reg8 r)             { emitMR8(op, UIMR(m), rn(r)); }
  void instrMR(X64Instr op,
               MemoryRef m,
               RegXMM x)           { emitMR(op, UMR(m), rn(x)); }
  void instrMR(X64Instr op,
               IndexedMemoryRef m,
               RegXMM x)           { emitMR(op, UIMR(m), rn(x)); }
  void instrMR(X64Instr op,
               RIPRelativeRef m,
               Reg64 r)            { emitMR(op, URIP(m), rn(r),
                                            sz::qword, true); }

  void instrIR(X64Instr op, Immed i, Reg64 r) {
    emitIR(op, rn(r), i.q());
  }
  void instrIR(X64Instr op, Immed i, Reg32 r) {
    emitIR32(op, rn(r), i.l());
  }
  void instrIR(X64Instr op, Immed i, Reg8 r) {
    emitIR8(op, rn(r), i.b());
  }

  void instrIM(X64Instr op, Immed i, MemoryRef m) {
    emitIM(op, UMR(m), i.q());
  }
  void instrIM32(X64Instr op, Immed i, MemoryRef m) {
    emitIM32(op, UMR(m), i.l());
  }
  void instrIM8(X64Instr op, Immed i, MemoryRef m) {
    emitIM8(op, UMR(m), i.b());
  }

  void instrIM(X64Instr op, Immed i, IndexedMemoryRef m) {
    emitIM(op, UIMR(m), i.q());
  }
  void instrIM32(X64Instr op, Immed i, IndexedMemoryRef m) {
    emitIM32(op, UIMR(m), i.l());
  }
  void instrIM8(X64Instr op, Immed i, IndexedMemoryRef m) {
    emitIM8(op, UIMR(m), i.b());
  }

#undef UMR
#undef UIMR
#undef URIP
};

//////////////////////////////////////////////////////////////////////

inline void emitImmReg(X64Assembler& a, Immed imm, Reg64 dest) {
  a.emitImmReg(imm, dest);
}

class StoreImmPatcher {
 public:
  StoreImmPatcher(X64Assembler& as, uint64_t initial, RegNumber reg,
                  int32_t offset, RegNumber base);
  void patch(uint64_t actual);
 private:
  CodeAddress m_addr;
  bool m_is32;
};

/*
 * Select the assembler which contains a given address.
 *
 * E.g.:
 *
 *   Asm& a = asmChoose(toPatch, a, astubs);
 *   a.patchJmp(...);
 */
inline X64Assembler& asmChoose(CodeAddress addr) {
  ASSERT(false && "addr was not part of any known assembler");
  NOT_REACHED();
  return *static_cast<X64Assembler*>(nullptr);
}
template<class... Asm>
X64Assembler& asmChoose(CodeAddress addr, X64Assembler& a, Asm&... as) {
  if (a.code.isValidAddress(addr)) return a;
  return asmChoose(addr, as...);
}

//////////////////////////////////////////////////////////////////////

#undef TRACEMOD
#undef logical_const

}}}

#endif
