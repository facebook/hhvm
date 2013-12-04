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

namespace std {
template<>
struct hash<HPHP::Transl::Reg64> {
  size_t operator()(const HPHP::Transl::Reg64& r) const {
    return std::hash<int>()(static_cast<int>(r));
  }
};
}

namespace HPHP { namespace JIT { namespace ARM {

inline vixl::Register x2a(const Transl::Reg64& x64reg) {
  static const std::unordered_map<Transl::Reg64, vixl::Register> s_x2aRegMap = {
    // Special VM registers.
    { Transl::reg::rbp, vixl::x29 },  // x29 is ARM's designated frame pointer.
    { Transl::reg::rbx, vixl::x19 },  //
    { Transl::reg::r12, vixl::x20 },  // General callee-saved.
    { Transl::reg::r15, vixl::x21 },  //
    { Transl::reg::r10, vixl::x9  },  // General caller-saved.

    // Argument registers.
    { Transl::reg::rdi, vixl::x0 },
    { Transl::reg::rsi, vixl::x1 },
    { Transl::reg::rdx, vixl::x2 },
    { Transl::reg::rcx, vixl::x3 },
    { Transl::reg::r8,  vixl::x4 },
    { Transl::reg::r9,  vixl::x5 },

    // General caller-saved.
    { Transl::reg::r11, vixl::x10 },

    // General callee-saved.
    { Transl::reg::r13, vixl::x22 },
    { Transl::reg::r14, vixl::x23 },

    // VERY SKETCHY! On ARM, x0 is both the first arg register and the return
    // value register. Anywhere our x64 code implicitly assumes that these two
    // regs don't alias each other may break.
    { Transl::reg::rax, vixl::x0 },

    { Transl::reg::rsp, vixl::sp },
  };

  if (Transl::reg::noreg == x64reg) {
    return vixl::Register();
  }

  auto it = s_x2aRegMap.find(x64reg);
  assert(it != s_x2aRegMap.end());
  return it->second;
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

inline vixl::Condition convertCC(Transl::ConditionCode cc) {
  if (cc == Transl::CC_P || cc == Transl::CC_NP) {
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

}}}

#endif
