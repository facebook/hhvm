/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/abi-regs.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

#include "hphp/util/asm-x64.h"
#include "hphp/vixl/a64/assembler-a64.h"
#include "hphp/vixl/a64/constants-a64.h"

#include <unordered_map>

namespace HPHP { namespace jit {

struct Abi;

namespace arm {

///////////////////////////////////////////////////////////////////////////////

/*
 * Mirrors the API of abi.h.
 */

const Abi& abi(CodeKind kind = CodeKind::Trace);

inline PhysReg rvmfp() { return vixl::x29; }
inline PhysReg rvmsp() { return vixl::x28; }
inline PhysReg rvmtl() { return vixl::x27; }
inline PhysReg rsp()   { return vixl::sp; }

inline RegSet vm_regs_no_sp()   { return rvmfp() | rvmtl(); }
inline RegSet vm_regs_with_sp() { return vm_regs_no_sp() | rvmsp(); }

inline PhysReg rret_data() { return vixl::x0; }
inline PhysReg rret_type() { return vixl::x1; }

PhysReg rret(size_t i = 0);
PhysReg rret_simd(size_t i);
inline PhysReg rret_indirect() { return vixl::x8; }

PhysReg rarg(size_t i);
PhysReg rarg_simd(size_t i);

inline PhysReg rfp() { return vixl::x29; }
inline PhysReg rlr() { return vixl::x30; }

constexpr size_t num_arg_regs() { return 8; }
constexpr size_t num_arg_regs_simd() { return 8; }

RegSet arg_regs(size_t n);
RegSet arg_regs_simd(size_t n);

PhysReg r_svcreq_req();
PhysReg r_svcreq_stub();
PhysReg r_svcreq_sf();
PhysReg r_svcreq_arg(size_t i);

///////////////////////////////////////////////////////////////////////////////

inline vixl::Register x2a(PhysReg x64reg) {
  always_assert(!x64reg.isSIMD());
  return vixl::Register(vixl::CPURegister(x64reg));
}

inline vixl::FPRegister x2f(PhysReg x64reg) {
  always_assert(x64reg.isSIMD());
  return vixl::FPRegister(vixl::CPURegister(x64reg));
}

inline vixl::VRegister x2v(PhysReg x64reg) {
  always_assert(x64reg.isSIMD());
  return vixl::VRegister(vixl::CPURegister(x64reg));
}

/*
 * ARM    int meaning                   fp meaning                         JIT mapping        JIT meaning
 * ------------------------------------------------------------------------------------------------------------------
 * eq     equal                         equal                              CC_E/CC_Z          equal
 * ne     not equal                     not equal or unordered             CC_NE/CC_NZ        not equal
 * cs/hs  carry set                     greater than, equal, or unordered  CC_AE/CC_NB/CC_NC  unsigned higher or same
 * cc/lo  carry clear                   less than                          CC_B/CC_NAE        unsigned lower
 * mi     minus, negative               less than                          CC_S               minus (sign set)
 * pl     plus, positive, or zero       greater than, equal, or unordered  CC_NS              plus (sign clear)
 * vs     overflow                      unordered                          CC_O               overflow set
 * vc     no overflow                   ordered                            CC_NO              overflow clear
 * hi     unsigned higher               greater than, or unordered         CC_A/CC_NBE        unsigned higher
 * ls     unsigned lower or same        less than or equal                 CC_BE/CC_NA        unsigned lower or same
 * ge     signed greater than or equal  greater than or equal              CC_GE/CC_NL        signed greater or equal
 * lt     signed less than              less than, or unordered            CC_L/CC_NGE        signed less than
 * gt     signed greater than           greater than                       CC_G/CC_NLE        signed greater than
 * le     signed less than or equal     less than, equal, or unordered     CC_LE/CC_NG        signed less or equal
 * al     always (not used)             always (not used)                  CC_P               parity
 * nv     not valid                                                        CC_NP              no parity
 */

inline vixl::Condition convertCC(jit::ConditionCode cc) {
  assertx(cc >= 0 && cc <= 0xF);

  using namespace vixl;

  // We'll index into this array by the x64 condition code.
  constexpr vixl::Condition mapping[] = {
    vs,     // CC_O
    vc,     // CC_NO
    lo,     // CC_B, CC_NAE
    hs,     // CC_AE, CC_NB, CC_NC
    eq,     // CC_E, CC_Z
    ne,     // CC_NE, CC_NZ
    ls,     // CC_BE, CC_NA
    hi,     // CC_A, CC_NBE
    mi,     // CC_S
    pl,     // CC_NS
    nv, nv, // CC_P, CC_NP (invalid)
    lt,     // CC_L, CC_NGE
    ge,     // CC_GE, CC_NL
    le,     // CC_LE, CC_NG
    gt,     // CC_G, CC_NLE
  };

  return mapping[cc];
}

inline jit::ConditionCode convertCC(vixl::Condition cc) {
  if (cc == vixl::nv || cc == vixl::al) {
    // nv and al are invalid on ARM
    always_assert(false);
  }
  assertx(cc >= 0 && cc <= 0xF);

  using namespace vixl;

  // We'll index into this array by the arm64 condition code.
  constexpr jit::ConditionCode mapping[] = {
    CC_E,        // eq
    CC_NE,       // ne
    CC_AE,       // hs
    CC_B,        // lo
    CC_S,        // mi
    CC_NS,       // pl
    CC_O,        // vs
    CC_NO,       // vc
    CC_A,        // hi
    CC_BE,       // ls
    CC_GE,       // ge
    CC_L,        // lt
    CC_G,        // gt
    CC_LE,       // le
    CC_P, CC_NP, // nv, al (invalid)
  };

  return mapping[cc];
}

///////////////////////////////////////////////////////////////////////////////

inline vixl::Register svcReqArgReg(unsigned index) {
  // First arg holds the request number
  return x2a(rarg(index + 1));
}

// x18 is used as assembler temporary
const vixl::Register rAsm(vixl::x18);

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
