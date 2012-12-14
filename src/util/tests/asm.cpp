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
#include "util/asm-x64.h"
#include <gtest/gtest.h>

#include <vector>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>

#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

namespace HPHP { namespace VM { namespace Transl {

typedef X64Assembler Asm;
using namespace reg;

namespace {

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
  static boost::regex re { R"([^\t]*\t[^\t]*\t([a-zA-Z]+)\s+(.*))" };
  boost::smatch cm;
  if (!regex_match(line, cm, re)) return false;
  opName = cm[1];
  opArgs = cm[2];
  return true;
}

void compare(const char* expectedOpName,
             std::istream& in,
             const std::vector<std::string>& expecteds) {
  auto expectIt = expecteds.begin();

  std::string expect, real;
  std::string opName, opArgs;
  while (std::getline(in, real)) {
    if (!match_opcode_line(real, opName, opArgs)) continue;

    EXPECT_EQ(expectedOpName, opName);

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

void dump_disasm(Asm& a, std::ifstream& dump) {
  char filename[32];
  char outfile[32];
  std::strcpy(filename, "/tmp/asmtmXXXXXX");
  std::strcpy(outfile, "/tmp/asmtmXXXXXX");
  close(mkstemp(filename));
  close(mkstemp(outfile));

  std::FILE* fp = std::fopen(filename, "w");
  std::fwrite(a.code.base, a.code.frontier - a.code.base, 1, fp);
  std::fclose(fp);

  system(str(boost::format(
    "objdump -D -b binary -mi386 -M x86-64 %s > %s")
      % filename
      % outfile
    ).c_str());

  dump.open(outfile);
  unlink(outfile);
  if (!dump.is_open()) std::abort();
}

void expect_asm(Asm& a, const std::string& str) {
  std::ifstream dump;
  dump_disasm(a, dump);

  std::ostringstream out;
  out << '\n';

  std::string line;
  std::string opName, opArgs;
  while (std::getline(dump, line)) {
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
  out << "0x" << std::hex << (uintptr_t)disp;
}

std::string expected_str(MemoryRef mr) {
  std::ostringstream out;
  /*
   * Operations with "rbp-like" registers still are encoded with a
   * displacement byte, even if the displacement is zero.  This wouldn't
   * matter, but objdump displays the zero displacements for these
   * registers (presumably because of this), so we have to add it to our
   * expected string in that case.
   */
  if (mr.r.base == rbp || mr.r.base == r13 || mr.r.disp != 0) {
    expected_disp_str(mr.r.disp, out);
  }
  out << '(' << expected_str(mr.r.base) << ')';
  return out.str();
}

std::string expected_str(IndexedMemoryRef imr) {
  std::ostringstream out;
  // See above about the rbp/r13 thing.
  if (imr.r.base == rbp || imr.r.base == r13 || imr.r.disp != 0) {
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

  std::ifstream dump;
  dump_disasm(a, dump);
  compare(opName, dump, expecteds);
  a.code.frontier = a.code.base;
}

template<class Arg1, class Arg2>
void dotest(const char* opName, Asm& a, void (Asm::*memFn)(Arg1, Arg2),
            const std::vector<Arg1>& args1, const std::vector<Arg2>& args2) {
  std::vector<std::string> expecteds;

  for (auto& ar1 : args1) {
    for (auto& ar2 : args2) {
      expecteds.push_back(str(
        boost::format("%s,%s") % expected_str(ar1)
                               % expected_str(ar2)
      ));
      (a.*memFn)(ar1, ar2);
    }
  }

  std::ifstream dump;
  dump_disasm(a, dump);
  compare(opName, dump, expecteds);
  a.code.frontier = a.code.base;
}

template<class Arg1, class Arg2>
void dotest(const char* opName, Asm& a, void (Asm::*memFn)(Arg1, Arg2)) {
  dotest(opName, a, memFn, Gen<Arg1>::gen(), Gen<Arg2>::gen());

}

//////////////////////////////////////////////////////////////////////

// Wrappers for generating test cases for various addressing modes.

typedef void (Asm::*OpR64)(Reg64);
typedef void (Asm::*OpR32)(Reg32);
typedef void (Asm::*OpRR64)(Reg64, Reg64);
typedef void (Asm::*OpRR32)(Reg32, Reg32);
typedef void (Asm::*OpRR8)(Reg8, Reg8);
typedef void (Asm::*OpR8R32)(Reg8, Reg32);
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
typedef void (Asm::*OpISM64)(Immed, IndexedMemoryRef);
typedef void (Asm::*OpISM32)(Immed, IndexedMemoryRef);

//////////////////////////////////////////////////////////////////////

}

TEST(Asm, General) {
  Asm a;
  a.init(10 << 24);

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

  dotest("movzbl", a, OpMR32(&Asm::loadzbl));
  dotest("movzbl", a, OpSMR32(&Asm::loadzbl));
  dotest("movzbl", a, OpR8R32(&Asm::movzbl));

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

  doingByteOpcodes = false;
}

TEST(Asm, HighByteReg) {
  Asm a;
  a.init(10 << 24);

  // Test movzbl with high byte regs, avoiding destination registers
  // that need a rex prefix
  std::vector<Reg8> hiregs = {ah, bh, ch, dh};
  std::vector<Reg32> reg32s = {eax, ecx, esi, ebp};
  dotest("movzbl", a, OpR8R32(&Asm::movzbl), hiregs, reg32s);

  a.    movb   (al, ah);
  a.    testb  (0x1, ah);
  a.    cmpb   (ch, dh);

  expect_asm(a, R"(
mov %al,%ah
test $0x1,%ah
cmp %ch,%dh
)");
}

TEST(Asm, RandomJunk) {
  Asm a;
  a.init(10 << 24);

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
push %rbp
mov %rsp,%rbp
sub $0x80,%rsp
mov $0x0,%eax
inc %rax
mov %rax,0x8(%rsp)
mov 0x8(%rsp),%rdi
pop %rbp
retq 
)");
}

}}}
