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

PhysReg rarg(size_t i);
PhysReg rarg_simd(size_t i);
PhysReg rarg_ind_ret(size_t i);

inline PhysReg rfp() { return vixl::x29; }
inline PhysReg rlr() { return vixl::x30; }

constexpr size_t num_arg_regs() { return 8; }
constexpr size_t num_arg_regs_simd() { return 8; }
constexpr size_t num_arg_regs_ind_ret() { return 1; }

RegSet arg_regs(size_t n);
RegSet arg_regs_simd(size_t n);
RegSet arg_regs_ind_ret(size_t n);

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
 * ARM    int meaning                fp meaning
 *        bit tests                  JIT mapping        JIT meaning
 * ----------------------------------------------------------------------------
 * eq     equal                      equal
 *        Z == 1                     CC_E/CC_Z          equal
 * ne     not equal                  not equal or unordered
 *        Z == 0                     CC_NE/CC_NZ        not equal
 * cs/hs  carry set                  greater than, equal, or unordered
 *        C == 1                     CC_AE/CC_NB/CC_NC  unsigned higher or same
 * cc/lo  carry clear                less than
 *        C == 0                     CC_B/CC_NAE        unsigned lower
 * mi     minus, negative            less than
 *        N == 1                     CC_S               minus (sign set)
 * pl     plus, nonnegative          greater than, equal, or unordered
 *        N == 0                     CC_NS              plus (sign clear)
 * vs     overflow                   unordered
 *        V == 1                     CC_O               overflow set
 * vc     no overflow                ordered
 *        V == 0                     CC_NO              overflow clear
 * hi     unsigned higher            greater than, or unordered
 *        C == 1 && Z == 0           CC_A/CC_NBE        unsigned higher
 * ls     unsigned lower or same     less than or equal
 *        !(C == 1 && Z == 0)        CC_BE/CC_NA        unsigned lower or same
 * ge     signed greater or equal    greater than or equal
 *        N == V                     CC_GE/CC_NL        signed greater or equal
 * lt     signed less than           less than, or unordered
 *        N != V                     CC_L/CC_NGE        signed less than
 * gt     signed greater than        greater than
 *        Z == 0 && N == V           CC_G/CC_NLE        signed greater than
 * le     signed less than or equal  less than, equal, or unordered
 *        !(Z == 0 && N == V)        CC_LE/CC_NG        signed less or equal
 * al     always (not used)          always (not used)
 *        any                        CC_P               parity
 * nv     not valid
 *        any                        CC_NP              no parity
 */

inline vixl::Condition convertCC(jit::ConditionCode cc) {
  assertx(cc >= 0 && cc <= 0xF);

  using namespace vixl;

  // We'll index into this array by the x64 condition code.
  constexpr vixl::Condition mapping[] = {
    vs,  // overflow set
    vc,  // overflow clear
    lo,  // unsigned lower
    hs,  // unsigned higher or same
    eq,  // equal
    ne,  // not equal
    ls,  // unsigned lower or same
    hi,  // unsigned higher
    mi,  // minus (N set)
    pl,  // plus (N clear)
    vs,  // parity = 1, valid only for float conversion and comparison
    vc,  // parity = 0
    lt,  // signed less than
    ge,  // signed greater or equal
    le,  // signed less or equal
    gt,  // signed greater than
  };

  return mapping[cc];
}

inline jit::ConditionCode convertCC(vixl::Condition cc) {
  using namespace vixl;

  // We'll index into this array by the arm64 condition code.
  constexpr jit::ConditionCode mapping[] = {
    jit::CC_E,   // equal
    jit::CC_NE,  // not equal
    jit::CC_AE,  // unsigned higher or same
    jit::CC_B,   // unsigned lower
    jit::CC_S,   // minus (N set)
    jit::CC_NS,  // plus (N clear)
    jit::CC_O,   // overflow set
    jit::CC_NO,  // overflow clear
    jit::CC_A,   // unsigned higher
    jit::CC_NA,  // unsigned lower or same
    jit::CC_GE,  // signed greater or equal
    jit::CC_L,   // signed less than
    jit::CC_G,   // signed greater than
    jit::CC_LE,  // signed less or equal
    jit::CC_P, jit::CC_NP, // invalid. These are the parity flags.
  };

  return mapping[cc];
}

enum class StatusFlags : uint8_t {
  None = 0,
  V    = 1,
  Z    = 1 << 1,
  C    = 1 << 2,
  N    = 1 << 3,
  CZ   = C | Z,
  NV = N | V,
  NZV = N | Z | V,
  NotC = N | Z | V,
  NotV = N | C | Z,
  All  = N | C | Z | V,
};

inline Vflags required_flags(jit::ConditionCode cc) {
  assertx(cc >= 0 && cc <= 0xF);

  // A jit cc is first converted to an ARM cc before using this mapping
  constexpr StatusFlags mapping[] = {
    StatusFlags::Z,         // eq
    StatusFlags::Z,         // ne
    StatusFlags::C,         // hs
    StatusFlags::C,         // lo
    StatusFlags::N,         // mi
    StatusFlags::N,         // pl
    StatusFlags::V,         // vs
    StatusFlags::V,         // vc
    StatusFlags::CZ,        // hi
    StatusFlags::CZ,        // ls
    StatusFlags::NV,        // ge
    StatusFlags::NV,        // lt
    StatusFlags::NZV,       // gt
    StatusFlags::NZV,       // le
    StatusFlags::None,      // nv (invalid)
    StatusFlags::None,      // al (invalid)
  };

  return static_cast<Vflags>(mapping[convertCC(cc)]);
}

///////////////////////////////////////////////////////////////////////////////

inline vixl::Register svcReqArgReg(unsigned index) {
  // First arg holds the request number
  return x2a(rarg(index + 1));
}

// x16 and x17 are vixl macroassembler temporaries
const vixl::Register rVixlScratch0(vixl::x16);
const vixl::Register rVixlScratch1(vixl::x17);

// w18/x18 is used as assembler temporary. The 32-bit w18 is used
// primarily to hold 32-bit branch targets.
const vixl::Register rAsm(vixl::x18);
const vixl::Register rAsm_w(vixl::w18);

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
