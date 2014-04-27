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

namespace HPHP { namespace JIT { namespace ARM {

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
  assert(index <= maxArgReg());
  return vixl::Register::XRegFromCode(index);
}

inline vixl::Register serviceReqArgReg(unsigned index) {
  // First arg holds the request number
  return argReg(index + 1);
}

inline vixl::Condition convertCC(JIT::ConditionCode cc) {
  if (cc == JIT::CC_P || cc == JIT::CC_NP) {
    // ARM has no parity flag
    always_assert(false);
  }
  assert(cc >= 0 && cc <= 0xF);

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
const vixl::Register rStashedAR(vixl::x21);
const vixl::Register rGContextReg(vixl::x24);
const vixl::Register rLinkReg(vixl::x30);
const vixl::Register rReturnReg(vixl::x0);
const vixl::Register rHostCallReg(vixl::x16);

const RegSet kGPCallerSaved = RegSet()
  | RegSet(vixl::x0)
  | RegSet(vixl::x1)
  | RegSet(vixl::x2)
  | RegSet(vixl::x3)
  | RegSet(vixl::x4)
  | RegSet(vixl::x5)
  | RegSet(vixl::x6)
  | RegSet(vixl::x7)
  | RegSet(vixl::x8)
  // x9  = rAsm
  // x10 = rAsm2
  | RegSet(vixl::x11)
  | RegSet(vixl::x12)
  | RegSet(vixl::x13)
  | RegSet(vixl::x14)
  | RegSet(vixl::x15)
  // x16 = rHostCallReg, used as ip0/tmp0 by MacroAssembler
  // x17 = used as ip1/tmp1 by MacroAssembler
  | RegSet(vixl::x18)
  ;

const RegSet kGPCalleeSaved = RegSet()
  // x19 = rVmSp
  // x20 = rVmTl
  // x21 = rStashedAR
  | RegSet(vixl::x22)
  | RegSet(vixl::x23)
  // x24 = rGContextReg
  | RegSet(vixl::x25)
  | RegSet(vixl::x26)
  | RegSet(vixl::x27)
  | RegSet(vixl::x28)
  ;

const RegSet kGPUnreserved = RegSet()
  | kGPCallerSaved
  | kGPCalleeSaved
  ;

const RegSet kGPReserved = RegSet()
  | RegSet(rAsm)
  | RegSet(rAsm2)
  | RegSet(rHostCallReg)
  | RegSet(vixl::x17)
  | RegSet(rVmSp)
  | RegSet(rVmTl)
  | RegSet(rStashedAR)
  | RegSet(rGContextReg)
  | RegSet(rVmFp)
  | RegSet(rLinkReg)
  // ARM machines really only have 32 GP regs. However, vixl has 33 separate
  // register codes, because it treats the zero register and stack pointer
  // (which are really both register 31) separately. Rather than lose this
  // distinction in vixl (it's really helpful for avoiding stupid mistakes), we
  // sacrifice the ability to represent all 32 SIMD regs, and pretend that are
  // 33 GP regs.
  | RegSet(vixl::xzr) // x31
  | RegSet(vixl::sp) // x31, but with special vixl code
  ;

const RegSet kSIMDCallerSaved = RegSet()
  | RegSet(vixl::d0)
  | RegSet(vixl::d1)
  | RegSet(vixl::d2)
  | RegSet(vixl::d3)
  | RegSet(vixl::d4)
  | RegSet(vixl::d5)
  | RegSet(vixl::d6)
  | RegSet(vixl::d7)
  // 8-15 are callee-saved
  | RegSet(vixl::d16)
  | RegSet(vixl::d17)
  | RegSet(vixl::d18)
  | RegSet(vixl::d19)
  | RegSet(vixl::d20)
  | RegSet(vixl::d21)
  | RegSet(vixl::d22)
  | RegSet(vixl::d23)
  | RegSet(vixl::d24)
  | RegSet(vixl::d25)
  | RegSet(vixl::d26)
  | RegSet(vixl::d27)
  | RegSet(vixl::d28)
  | RegSet(vixl::d29)
  | RegSet(vixl::d30)
  // d31 exists, but PhysReg can't represent it, so we don't use it.
  ;

const RegSet kSIMDCalleeSaved = RegSet()
  | RegSet(vixl::d8)
  | RegSet(vixl::d9)
  | RegSet(vixl::d10)
  | RegSet(vixl::d11)
  | RegSet(vixl::d12)
  | RegSet(vixl::d13)
  | RegSet(vixl::d14)
  | RegSet(vixl::d15)
  ;

const RegSet kSIMDUnreserved = RegSet()
  | kSIMDCallerSaved
  | kSIMDCalleeSaved
  ;

const RegSet kSIMDReserved = RegSet()
  ;

const RegSet kCalleeSaved = RegSet()
  | kGPCalleeSaved
  | kSIMDCalleeSaved
  ;

UNUSED const Abi abi {
  kGPUnreserved,   // gpUnreserved
  kGPReserved,     // gpReserved
  kSIMDUnreserved, // simdUnreserved
  kSIMDReserved,   // simdReserved
  kCalleeSaved     // calleeSaved
};

}}}

#endif
