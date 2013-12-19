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
#ifndef incl_HPHP_JIT_ABI_ARM_H
#define incl_HPHP_JIT_ABI_ARM_H

#include <unordered_map>

#include "hphp/util/asm-x64.h"
#include "hphp/vixl/a64/assembler-a64.h"
#include "hphp/vixl/a64/constants-a64.h"

#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

namespace HPHP { namespace JIT { namespace ARM {

inline vixl::Register x2a(PhysReg x64reg) {
  return vixl::Register(vixl::CPURegister(x64reg));
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

const RegSet kCallerSaved = RegSet()
  | RegSet(vixl::x0)
  | RegSet(vixl::x1)
  | RegSet(vixl::x2)
  | RegSet(vixl::x3)
  | RegSet(vixl::x4)
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
  // x16 = rHostCallReg
  | RegSet(vixl::x17)
  | RegSet(vixl::x18)
  ;

const RegSet kCalleeSaved = RegSet()
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

}}}

#endif
