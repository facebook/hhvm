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
#include "hphp/util/asm-x64.h"
#include <gtest/gtest.h>

#include <vector>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "folly/Format.h"
#include "folly/String.h"

#include "hphp/util/disasm.h"

namespace HPHP { namespace JIT {

typedef X64Assembler Asm;
using namespace reg;

namespace {

struct TestDataBlock : public DataBlock {
  explicit TestDataBlock(size_t sz) {
    Address result = (Address)mmap(0, sz, PROT_READ | PROT_WRITE | PROT_EXEC,
                                   MAP_ANON | MAP_PRIVATE, -1, 0);
    always_assert(result != MAP_FAILED);
    m_base = m_frontier = result;
    m_size = sz;
  }

  ~TestDataBlock() {
    munmap(m_base, m_size);
    m_base = m_frontier = nullptr;
    m_size = 0;
  }
};

//////////////////////////////////////////////////////////////////////

/*
 * The following environment variables can be used to turn up the number of
 * combinations.  Off by default to keep the test running fast normally ...
 */
const bool testMore = getenv("ASM_TEST_MORE");
const bool testMax = getenv("ASM_TEST_MAX");

bool match_opcode_line(const std::string& line,
                       std::string& opName,
                       std::string& opArgs) {
  static boost::regex re{ R"(([^\s]*)\s+(.*))" };
  boost::smatch cm;
  if (!regex_match(line, cm, re)) return false;
  opName = cm[1];
  opArgs = cm[2];
  return true;
}

void compare(const char* expectedOpName,
             const std::vector<std::string>& actuals,
             const std::vector<std::string>& expecteds) {
  auto expectIt = expecteds.begin();

  std::string expect;
  std::string opName, opArgs;
  for (auto& real : actuals) {
    if (!match_opcode_line(real, opName, opArgs)) continue;

    // The xed library will add operand size suffixes on any opcode
    // that affects memory.  We could figure this out and check for
    // it, but it's good enough just to see that the opcode has the
    // prefix we expect.
    EXPECT_EQ(true, boost::starts_with(opName, expectedOpName))
      << "expected " << expectedOpName << ", got " << opName;

    if (expectIt == expecteds.end()) {
      EXPECT_EQ(1, 0) << "Incorrect number of assembler lines";
      break;
    }

    EXPECT_EQ(*expectIt, opArgs)
      << "in opcode: " << expectedOpName;
    ++expectIt;
  }

  EXPECT_EQ(expectIt, expecteds.end())
    << "More lines expected than read";
}

std::vector<std::string> dump_disasm(Asm& a) {
  Disasm dis(Disasm::Options()
               .addresses(false)
               .forceAttSyntax(true));
  std::ostringstream sstream;
  dis.disasm(sstream, a.base(), a.base() + a.used());

  std::vector<std::string> ret;
  folly::split('\n', sstream.str(), ret);
  return ret;
}

void expect_asm(Asm& a, const std::string& str) {
  auto const dump = dump_disasm(a);

  std::ostringstream out;
  out << '\n';

  std::string opName, opArgs;
  for (auto& line : dump) {
    if (match_opcode_line(line, opName, opArgs)) {
      out << opName << ' ' << opArgs << '\n';
    }
  }
  EXPECT_EQ(out.str(), str);
}

// Generate a bunch of operands of a given type.
template<class T> struct Gen;

template<> struct Gen<Reg64> {
  static const std::vector<Reg64>& gen() {
    if (testMax) {
      static const std::vector<Reg64> v = {
        rax, rbx, rcx, rdx, rsp, rbp, rsi, rdi,
        r8, r9, r10, r11, r12, r13, r14, r15
      };
      return v;
    }
    if (testMore) {
      static const std::vector<Reg64> v = {
        rax, rbx, rsp, rbp, r12, r13, r15
      };
      return v;
    }
    static const std::vector<Reg64> v = { rax, rbp, r15 };
    return v;
  }
};


template<> struct Gen<Reg32> {
  static const std::vector<Reg32>& gen() {
    if (testMax) {
      static const std::vector<Reg32> v = {
        eax, ecx, edx, ebx, esp, ebp, esi, edi,
        r8d, r9d, r10d, r11d, r12d, r13d, r14d, r15d
      };
      return v;
    }
    if (testMore) {
      static const std::vector<Reg32> v = {
        eax, ebx, esp, ebp, r12d, r13d, r14d
      };
      return v;
    }
    static const std::vector<Reg32> v = { eax, ebp, r15d };
    return v;
  }
};

template<> struct Gen<Reg8> {
  static const std::vector<Reg8>& gen() {
    // We can't really test the high-byte regs here because they can't be
    // used any time we have a REX byte.
    if (testMax) {
      static const std::vector<Reg8> v = {
        al, cl, dl, bl, spl, bpl, sil, dil, r8b, r9b, r10b,
        r11b, r12b, r13b, r14b, r15b
      };
      return v;
    }
    static const std::vector<Reg8> v = { al, r8b, sil };
    return v;
  }
};

template<> struct Gen<MemoryRef> {
  static const std::vector<MemoryRef>& gen() {
    static bool inited = false;
    static std::vector<MemoryRef> vec;
    if (inited) return vec;
    auto& regs = Gen<Reg64>::gen();
    const std::vector<int> disps = { -1024, 0, 12, 1024 };
    for (auto& r : regs) {
      for (auto& d : disps) {
        vec.push_back(r[d]);
      }
    }
    inited = true;
    return vec;
  }
};

template<> struct Gen<IndexedMemoryRef> {
  static const std::vector<IndexedMemoryRef>& gen() {
    static bool inited = false;
    static std::vector<IndexedMemoryRef> vec;
    if (inited) return vec;
    auto& indexes = Gen<Reg64>::gen();
    auto& mrs = Gen<MemoryRef>::gen();
    std::vector<int> scales = { 4 };

    for (auto& mr : mrs) {
      for (auto& idx : indexes) {
        if (idx == rsp) continue;
        for (auto& s : scales) {
          vec.push_back(*(IndexedDispReg(mr.r.base, idx * s) + mr.r.disp));
        }
      }
    }
    return vec;
  }
};

static bool doingByteOpcodes = false;

template<> struct Gen<Immed> {
  static const std::vector<Immed>& gen() {
    if (doingByteOpcodes) {
      // Don't use any immediates that don't fit in a byte.  (Normally we
      // want to though because they can be encoded different.)
      static const std::vector<Immed> vec { 1, 127 };
      return vec;
    }
    static const std::vector<Immed> vec { 1, 1 << 20 };
    return vec;
  }
};

const char* expected_str(Reg64 r) { return regname(r); }
const char* expected_str(Reg32 r) { return regname(r); }
const char* expected_str(Reg8 r)  { return regname(r); }
#undef X

void expected_disp_str(intptr_t disp, std::ostream& out) {
  if (disp < 0) {
    out << "-0x" << std::hex << -disp;
  } else {
    out << "0x" << std::hex << disp;
  }
}

std::string expected_str(MemoryRef mr) {
  std::ostringstream out;
  if (mr.r.disp != 0) {
    expected_disp_str(mr.r.disp, out);
  }
  out << '(' << expected_str(mr.r.base) << ')';
  return out.str();
}

std::string expected_str(IndexedMemoryRef imr) {
  std::ostringstream out;
  if (imr.r.disp != 0) {
    expected_disp_str(imr.r.disp, out);
  }
  out << '(' << expected_str(imr.r.base)
      << ',' << expected_str(imr.r.index)
      << ',' << imr.r.scale
      << ')';
  return out.str();
}

std::string expected_str(Immed i) {
  std::ostringstream out;
  out << "$0x" << std::hex << i.q();
  return out.str();
}

//////////////////////////////////////////////////////////////////////

template<class Arg>
void dotest(const char* opName, Asm& a, void (Asm::*memFn)(Arg)) {
  std::vector<std::string> expecteds;

  auto& args = Gen<Arg>::gen();
  for (auto& ar : args) {
    expecteds.push_back(expected_str(ar));
    (a.*memFn)(ar);
  }

  auto const dump = dump_disasm(a);
  compare(opName, dump, expecteds);
  a.clear();
}

template<class Arg1, class Arg2>
void dotest(const char* opName, Asm& a, void (Asm::*memFn)(Arg1, Arg2),
            const std::vector<Arg1>& args1, const std::vector<Arg2>& args2) {
  std::vector<std::string> expecteds;

  for (auto& ar1 : args1) {
    for (auto& ar2 : args2) {
      expecteds.push_back(
        folly::format("{}, {}", expected_str(ar1), expected_str(ar2)).str()
      );
      (a.*memFn)(ar1, ar2);
    }
  }

  auto const dump = dump_disasm(a);
  compare(opName, dump, expecteds);
  a.clear();
}

template<class Arg1, class Arg2>
void dotest(const char* opName, Asm& a, void (Asm::*memFn)(Arg1, Arg2)) {
  dotest(opName, a, memFn, Gen<Arg1>::gen(), Gen<Arg2>::gen());

}

//////////////////////////////////////////////////////////////////////

// Wrappers for generating test cases for various addressing modes.

typedef void (Asm::*OpR64)(Reg64);
typedef void (Asm::*OpR32)(Reg32);
typedef void (Asm::*OpR8)(Reg8);
typedef void (Asm::*OpRR64)(Reg64, Reg64);
typedef void (Asm::*OpRR32)(Reg32, Reg32);
typedef void (Asm::*OpRR8)(Reg8, Reg8);
typedef void (Asm::*OpR8R32)(Reg8, Reg32);
typedef void (Asm::*OpR8R64)(Reg8, Reg64);
typedef void (Asm::*OpMR64)(MemoryRef, Reg64);
typedef void (Asm::*OpMR32)(MemoryRef, Reg32);
typedef void (Asm::*OpMR8)(MemoryRef, Reg8);
typedef void (Asm::*OpSMR64)(IndexedMemoryRef, Reg64);
typedef void (Asm::*OpSMR32)(IndexedMemoryRef, Reg32);
typedef void (Asm::*OpSMR8)(IndexedMemoryRef, Reg8);
typedef void (Asm::*OpRM64)(Reg64, MemoryRef);
typedef void (Asm::*OpRM32)(Reg32, MemoryRef);
typedef void (Asm::*OpRM8)(Reg8, MemoryRef);
typedef void (Asm::*OpRSM64)(Reg64, IndexedMemoryRef);
typedef void (Asm::*OpRSM32)(Reg32, IndexedMemoryRef);
typedef void (Asm::*OpRSM8)(Reg8, IndexedMemoryRef);
typedef void (Asm::*OpIR64)(Immed, Reg64);
typedef void (Asm::*OpIR32)(Immed, Reg32);
typedef void (Asm::*OpIR8)(Immed, Reg8);
typedef void (Asm::*OpIM64)(Immed, MemoryRef);
typedef void (Asm::*OpIM32)(Immed, MemoryRef);
typedef void (Asm::*OpIM16)(Immed, MemoryRef);
typedef void (Asm::*OpISM64)(Immed, IndexedMemoryRef);
typedef void (Asm::*OpISM32)(Immed, IndexedMemoryRef);
typedef void (Asm::*OpISM16)(Immed, IndexedMemoryRef);

//////////////////////////////////////////////////////////////////////

}

TEST(Asm, General) {
  TestDataBlock db(10 << 24);
  Asm a { db };

  /*
   * Test is a little different, so we have this BASIC_OP stuff.
   *
   * Skips using a memory source operand---there's actually only a test
   * instruction with memory destination (even though our API allows
   * writing it the other way), so when we disassemble the args look like
   * the wrong order.
   */

#define BASIC_OP(op)                            \
  dotest(#op, a, OpRR32(&Asm::op##l));          \
  dotest(#op, a, OpRR64(&Asm::op##q));          \
  dotest(#op, a, OpRM32(&Asm::op##l));          \
  dotest(#op, a, OpRM64(&Asm::op##q));          \
  dotest(#op, a, OpRSM32(&Asm::op##l));         \
  dotest(#op, a, OpRSM64(&Asm::op##q));         \
  dotest(#op, a, OpIR64(&Asm::op##q));          \
  dotest(#op, a, OpIR32(&Asm::op##l));          \
  dotest(#op "q", a, OpIM64(&Asm::op##q));      \
  dotest(#op "l", a, OpIM32(&Asm::op##l));      \
  dotest(#op "q", a, OpISM64(&Asm::op##q));     \
  dotest(#op "l", a, OpISM32(&Asm::op##l));

#define FULL_OP(op)                             \
  BASIC_OP(op)                                  \
  dotest(#op, a, OpMR32(&Asm::op##l));          \
  dotest(#op, a, OpMR64(&Asm::op##q));          \
  dotest(#op, a, OpSMR32(&Asm::op##l));         \
  dotest(#op, a, OpSMR64(&Asm::op##q));


#define BASIC_BYTE_OP(op)                       \
  dotest(#op, a, OpIR8(&Asm::op##b));           \
  dotest(#op, a, OpRR8(&Asm::op##b));           \
  dotest(#op, a, OpRM8(&Asm::op##b));           \
  dotest(#op, a, OpRSM8(&Asm::op##b));          \
  dotest(#op, a, OpIR8(&Asm::op##b));

#define FULL_BYTE_OP(op)                        \
  BASIC_BYTE_OP(op)                             \
  dotest(#op, a, OpMR8(&Asm::op##b));           \
  dotest(#op, a, OpSMR8(&Asm::op##b));

#define UNARY_BYTE_OP(op)                       \
  dotest(#op, a, OpR8(&Asm::op##b));

  dotest("inc", a, OpR32(&Asm::incl));
  dotest("inc", a, OpR64(&Asm::incq));

  dotest("mov", a, OpRR8(&Asm::movb));
  dotest("mov", a, OpRR32(&Asm::movl));
  dotest("mov", a, OpRR64(&Asm::movq));
  dotest("mov", a, OpMR8(&Asm::loadb));
  dotest("mov", a, OpMR32(&Asm::loadl));
  dotest("mov", a, OpMR64(&Asm::loadq));
  dotest("mov", a, OpSMR8(&Asm::loadb));
  dotest("mov", a, OpSMR32(&Asm::loadl));
  dotest("mov", a, OpSMR64(&Asm::loadq));
  dotest("mov", a, OpRM8(&Asm::storeb));
  dotest("mov", a, OpRM32(&Asm::storel));
  dotest("mov", a, OpRM64(&Asm::storeq));
  dotest("movl",a, OpISM32(&Asm::storel));
  dotest("movq",a, OpISM64(&Asm::storeq));
  dotest("mov", a, OpRSM8(&Asm::storeb));
  dotest("mov", a, OpRSM32(&Asm::storel));
  dotest("mov", a, OpRSM64(&Asm::storeq));

  dotest("movzx", a, OpMR32(&Asm::loadzbl));
  dotest("movzx", a, OpSMR32(&Asm::loadzbl));
  dotest("movzx", a, OpR8R32(&Asm::movzbl));

  dotest("movsx", a, OpMR64(&Asm::loadsbq));
  dotest("movsx", a, OpSMR64(&Asm::loadsbq));
  dotest("movsx", a, OpR8R64(&Asm::movsbq));

  FULL_OP(add);
  FULL_OP(xor);
  FULL_OP(sub);
  FULL_OP(and);
  FULL_OP(or);
  FULL_OP(cmp);
  BASIC_OP(test);

  // Note: objdump disassembles xchg %rax,%rax as rex.W nop, so we're just
  // leaving it out.

  doingByteOpcodes = true;

  FULL_BYTE_OP(cmp);
  BASIC_BYTE_OP(test);
  UNARY_BYTE_OP(not);
  UNARY_BYTE_OP(neg);

  doingByteOpcodes = false;
}

TEST(Asm, WordSizeInstructions) {
  TestDataBlock db(10 << 24);
  Asm a { db };

  // single register operations
  a.    incw   (ax);
  // single memory operations
  a.    decw   (*r8);
  // register-register operations
  a.    addw   (ax, bx);
  a.    xorw   (r10w, r11w);
  a.    movw   (cx, si);
  // register-memory operations
  a.    storew (ax, *rbx);
  a.    testw  (r10w, rsi[0x10]);
  // memory-register operations
  a.    subw   (*rcx, ax);
  a.    orw    (r11[0x100], dx);
  // immediate-register operations
  a.    shlw   (0x3, di);
  a.    andw   (0x5555, r12w);
  // immediate-memory operations
  a.    storew (0x1, *r9);
  a.    storew (0x1, rax[0x100]);

  expect_asm(a, R"(
inc %ax
decw (%r8)
add %ax, %bx
xor %r10w, %r11w
mov %cx, %si
movw %ax, (%rbx)
testw %r10w, 0x10(%rsi)
subw (%rcx), %ax
orw 0x100(%r11), %dx
shl $0x3, %di
and $0x5555, %r12w
movw $0x1, (%r9)
movw $0x1, 0x100(%rax)
)");
}

TEST(Asm, RetImmediate) {
  TestDataBlock db(10 << 24);
  Asm a { db };

  a.ret(8);
  ASSERT_FALSE(a.base()[0] == kOpsizePrefix);
}

TEST(Asm, IncDecRegs) {
  TestDataBlock db(10 << 24);
  Asm a { db };

  // incq, incl, incw
  a.    incq(rax);
  a.    incl(eax);
  a.    incw(ax);
  a.    incq(r15);
  a.    incl(r15d);
  a.    incw(r15w);
  // decq, decl, decw
  a.    decq(rax);
  a.    decl(eax);
  a.    decw(ax);
  a.    decq(r15);
  a.    decl(r15d);
  a.    decw(r15w);

  expect_asm(a, R"(
inc %rax
inc %eax
inc %ax
inc %r15
inc %r15d
inc %r15w
dec %rax
dec %eax
dec %ax
dec %r15
dec %r15d
dec %r15w
)");
}

TEST(Asm, HighByteReg) {
  TestDataBlock db(10 << 24);
  Asm a { db };

  // Test movzbl with high byte regs, avoiding destination registers
  // that need a rex prefix
  std::vector<Reg8> hiregs = {ah, bh, ch, dh};
  std::vector<Reg32> reg32s = {eax, ecx, esi, ebp};
  dotest("movzx", a, OpR8R32(&Asm::movzbl), hiregs, reg32s);

  a.    movb   (al, ah);
  a.    testb  (0x1, ah);
  a.    cmpb   (ch, dh);

  expect_asm(a, R"(
mov %al, %ah
test $0x1, %ah
cmp %ch, %dh
)");
}

TEST(Asm, RandomJunk) {
  TestDataBlock db(10 << 24);
  Asm a { db };

  a.    push   (rbp);
  a.    movq   (rsp, rbp);
  a.    subq   (0x80, rsp);

  a.    movl   (0, eax);
  a.    incq   (rax);
  a.    storeq (rax, rsp[0x8]);
  a.    loadq  (rsp[0x8], rdi);

  a.    pop    (rbp);
  a.    ret    ();

  expect_asm(a, R"(
pushq %rbp
mov %rsp, %rbp
sub $0x80, %rsp
mov $0x0, %eax
inc %rax
movq %rax, 0x8(%rsp)
movq 0x8(%rsp), %rdi
popq %rbp
retq )" "\n"); // string concat to avoid space at end of line after retq
}

TEST(Asm, AluBytes) {
  TestDataBlock db(10 << 24);
  Asm a { db };

#define INSTRS \
 FROB(cmp)     \
 FROB(add)     \
 FROB(sub)     \
 FROB(and)     \
 FROB(or)      \
 FROB(xor)

#define FROB(instr) \
  a.   instr ## b(sil, al);          \
  a.   instr ## b(0xf, al);          \
  a.   instr ## b(sil, rcx[0x10]);   \
  a.   instr ## b(rsp[0x10], sil);   \
  a.   instr ## b(rcx[rsi * 8], al); \
  a.   instr ## b(al, rcx[rsi * 8]);

  INSTRS

#undef FROB

#define FROB(name)                              \
#name " %sil, %al\n"                            \
#name " $0xf, %al\n"                            \
#name "b %sil, 0x10(%rcx)\n"                    \
#name "b 0x10(%rsp), %sil\n"                    \
#name "b (%rcx,%rsi,8), %al\n"                  \
#name "b %al, (%rcx,%rsi,8)\n"

  expect_asm(a, "\n" INSTRS "");

#undef FROB
#undef INSTRS

  // test is asymmetric.
  a.clear();
  a.   testb(sil, al);
  a.   testb(0xf, al);
  a.   testb(sil, rcx[0x10]);
  a.   testb(sil, rcx[rsi * 8]);

  expect_asm(a, R"(
test %sil, %al
test $0xf, %al
testb %sil, 0x10(%rcx)
testb %sil, (%rcx,%rsi,8)
)");
}

TEST(Asm, CMov) {
  TestDataBlock db(10 << 24);
  Asm a { db };
  a.   testq  (rax, rax);
  a.   cload_reg64_disp_reg64(CC_Z, rax, 0, rax);
  a.   cload_reg64_disp_reg32(CC_Z, rax, 0, rax);
  expect_asm(a, R"(
test %rax, %rax
cmovzq (%rax), %rax
cmovzl (%rax), %eax
)");
}

TEST(Asm, SimpleLabelTest) {
  TestDataBlock db(10 << 24);
  Asm a { db };

  Label loop;

  auto loopCallee = [] (int* counter) { ++*counter; };

  // Function that calls loopCallee N times.
  auto function = reinterpret_cast<int (*)(int, int*)>(a.frontier());
  a.    push   (rbp);
  a.    movq   (rsp, rbp);
  a.    push   (r15);
  a.    push   (r12);
  a.    push   (r10);
  a.    push   (rbx);

  a.    movl   (edi, r12d);
  a.    movq   (rsi, r15);
  a.    movl   (0, ebx);

asm_label(a, loop);
  a.    movq   (r15, rdi);
  a.    movq   (CodeAddress(static_cast<void (*)(int*)>(loopCallee)), r10);
  a.    call   (r10);
  a.    incl   (ebx);
  a.    cmpl   (ebx, r12d);
  a.    jne    (loop);

  a.    pop    (rbx);
  a.    pop    (r10);
  a.    pop    (r12);
  a.    pop    (r15);
  a.    pop    (rbp);
  a.    ret    ();

  auto test_case = [&] (int n) {
    int counter = 0;
    function(n, &counter);
    EXPECT_EQ(n, counter);
  };
  for (int i = 1; i < 15; ++i) test_case(i);
  test_case(51);
  test_case(127);
}

TEST(Asm, ShiftingWithCl) {
  TestDataBlock db(10 << 24);
  Asm a { db };

  a.    shlq(rax);
  a.    shlq(rdx);
  a.    shlq(r8);
  a.    sarq(rbx);
  a.    sarq(rsi);
  a.    sarq(r8);
  expect_asm(a, R"(
shl %cl, %rax
shl %cl, %rdx
shl %cl, %r8
sar %cl, %rbx
sar %cl, %rsi
sar %cl, %r8
)");
}

TEST(Asm, FloatRounding) {
  if (folly::CpuId().sse41()) {
    TestDataBlock db(10 << 24);
    Asm a { db };

    a.    roundsd(RoundDirection::nearest,  xmm1, xmm2);
    a.    roundsd(RoundDirection::floor,    xmm2, xmm4);
    a.    roundsd(RoundDirection::ceil,     xmm8, xmm7);
    a.    roundsd(RoundDirection::truncate, xmm12, xmm9);

    expect_asm(a, R"(
roundsd $0x0, %xmm1, %xmm2
roundsd $0x1, %xmm2, %xmm4
roundsd $0x2, %xmm8, %xmm7
roundsd $0x3, %xmm12, %xmm9
)");
  }
}

TEST(Asm, SSEDivision) {
  TestDataBlock db(10 << 24);
  Asm a { db };
  a.    divsd(xmm0, xmm1);
  a.    divsd(xmm1, xmm2);
  a.    divsd(xmm2, xmm0);
  a.    divsd(xmm15, xmm0);
  a.    divsd(xmm12, xmm8);
  expect_asm(a, R"(
divsd %xmm0, %xmm1
divsd %xmm1, %xmm2
divsd %xmm2, %xmm0
divsd %xmm15, %xmm0
divsd %xmm12, %xmm8
)");
}

TEST(Asm, SSESqrt) {
  TestDataBlock db(10 << 24);
  Asm a { db };
  a.    sqrtsd(xmm0, xmm1);
  a.    sqrtsd(xmm1, xmm2);
  a.    sqrtsd(xmm2, xmm0);
  a.    sqrtsd(xmm15, xmm0);
  a.    sqrtsd(xmm12, xmm8);
  expect_asm(a, R"(
sqrtsd %xmm0, %xmm1
sqrtsd %xmm1, %xmm2
sqrtsd %xmm2, %xmm0
sqrtsd %xmm15, %xmm0
sqrtsd %xmm12, %xmm8
)");
}

TEST(Asm, DoubleToIntConv) {
  TestDataBlock db(10 << 24);
  Asm a { db };
  a.    cvttsd2siq(xmm0, rax);
  a.    cvttsd2siq(xmm1, rbx);
  a.    cvttsd2siq(xmm2, rcx);
  a.    cvttsd2siq(xmm15, rdx);
  a.    cvttsd2siq(xmm12, r12);
  expect_asm(a, R"(
cvttsd2si %xmm0, %rax
cvttsd2si %xmm1, %rbx
cvttsd2si %xmm2, %rcx
cvttsd2si %xmm15, %rdx
cvttsd2si %xmm12, %r12
)");
}

}}
