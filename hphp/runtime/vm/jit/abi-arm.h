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
#ifndef incl_HPHP_JIT_ABI_ARM_H
#define incl_HPHP_JIT_ABI_ARM_H

#include <unordered_map>

#include "hphp/util/asm-x64.h"
#include "hphp/vixl/a64/assembler-a64.h"
#include "hphp/vixl/a64/constants-a64.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

namespace HPHP { namespace jit { namespace arm {

inline vixl::Register x2a(PhysReg x64reg) {
  always_assert(!x64reg.isSIMD());
  return vixl::Register(vixl::CPURegister(x64reg));
}

inline vixl::FPRegister x2simd(PhysReg x64reg) {
  always_assert(x64reg.isSIMD());
  return vixl::FPRegister(vixl::CPURegister(x64reg));
}

inline constexpr unsigned maxArgReg() { return 7; }

inline vixl::Register argReg(unsigned index) {
  assertx(index <= maxArgReg());
  return vixl::Register::XRegFromCode(index);
}

inline RegSet argSet(int n) {
  RegSet regs;
  for (int i = 0; i < n; i++) {
    regs.add(PhysReg(argReg(i)));
  }
  return regs;
}

inline vixl::Register serviceReqArgReg(unsigned index) {
  // First arg holds the request number
  return argReg(index + 1);
}

inline vixl::Condition convertCC(jit::ConditionCode cc) {
  if (cc == jit::CC_P || cc == jit::CC_NP) {
    // ARM has no parity flag
    always_assert(false);
  }
  assertx(cc >= 0 && cc <= 0xF);

  using namespace vixl;

  // We'll index into this array by the x64 condition code. The order matches
  // the enum above.
  constexpr vixl::Condition mapping[] = {
    vs,  // overflow set
    vc,  // overflow clear
    lo,  // unsigned lower
    hs,  // unsigned higher or same
    eq,  // equal
    ne,  // not equal
    ls,  // unsigned lower or same
    hi,  // unsigned higher
    pl,  // plus (sign set)
    mi,  // minus (sign clear)
    nv, nv,  // invalid. These are the parity flags.
    lt,  // signed less than
    ge,  // signed greater or equal
    le,  // signed less or equal
    gt,  // signed greater than
  };

  return mapping[cc];
}

const vixl::Register rVmFp(vixl::x29);
const vixl::Register rVmSp(vixl::x19);
const vixl::Register rVmTl(vixl::x20);
const vixl::Register rAsm(vixl::x9);
const vixl::Register rAsm2(vixl::x10);
const vixl::Register rGContextReg(vixl::x24);
const vixl::Register rLinkReg(vixl::x30);
const vixl::Register rReturnReg(vixl::x0);
const vixl::Register rHostCallReg(vixl::x16);

const RegSet kGPCallerSaved =
  vixl::x0 | vixl::x1 | vixl::x2 | vixl::x3 |
  vixl::x4 | vixl::x5 | vixl::x6 | vixl::x7 |
  vixl::x8 |
  // x9  = rAsm
  // x10 = rAsm2
  vixl::x11 | vixl::x12 | vixl::x13 | vixl::x14 | vixl::x15 |
  // x16 = rHostCallReg, used as ip0/tmp0 by MacroAssembler
  // x17 = used as ip1/tmp1 by MacroAssembler
  vixl::x18;

const RegSet kGPCalleeSaved =
  // x19 = rVmSp
  // x20 = rVmTl
  vixl::x22 | vixl::x23 |
  // x24 = rGContextReg
  vixl::x25 | vixl::x26 | vixl::x27 | vixl::x28;
  // x29 = rVmFp
  // x30 = rLinkReg

const RegSet kGPUnreserved = kGPCallerSaved | kGPCalleeSaved;

const RegSet kGPReserved =
  rAsm | rAsm2 | rHostCallReg | vixl::x17 |
  rVmSp | rVmTl | rGContextReg | rVmFp | rLinkReg |
  // ARM machines really only have 32 GP regs. However, vixl has 33 separate
  // register codes, because it treats the zero register and stack pointer
  // (which are really both register 31) separately. Rather than lose this
  // distinction in vixl (it's really helpful for avoiding stupid mistakes), we
  // sacrifice the ability to represent all 32 SIMD regs, and pretend that are
  // 33 GP regs.
  vixl::xzr | // x31
  vixl::sp; // x31, but with special vixl code

const RegSet kSIMDCallerSaved =
  vixl::d0 | vixl::d1 | vixl::d2 | vixl::d3 |
  vixl::d4 | vixl::d5 | vixl::d6 | vixl::d7 |
  // 8-15 are callee-saved
  vixl::d16 | vixl::d17 | vixl::d18 | vixl::d19 |
  vixl::d20 | vixl::d21 | vixl::d22 | vixl::d23 |
  vixl::d24 | vixl::d25 | vixl::d26 | vixl::d27 |
  vixl::d28 | vixl::d29;
  // d30 exists, but PhysReg can't represent it, so we don't use it.
  // d31 exists, but PhysReg can't represent it, so we don't use it.

const RegSet kSIMDCalleeSaved =
  vixl::d8 | vixl::d9 | vixl::d10 | vixl::d11 |
  vixl::d12 | vixl::d13 | vixl::d14 | vixl::d15;

const RegSet kSIMDUnreserved = kSIMDCallerSaved | kSIMDCalleeSaved;

const RegSet kSIMDReserved;

const RegSet kCalleeSaved = kGPCalleeSaved | kSIMDCalleeSaved;

const RegSet kSF = RegSet(RegSF{0});

UNUSED const Abi abi {
  kGPUnreserved,   // gpUnreserved
  kGPReserved,     // gpReserved
  kSIMDUnreserved, // simdUnreserved
  kSIMDReserved,   // simdReserved
  kCalleeSaved,    // calleeSaved
  kSF              // sf
};

}}}

#endif
