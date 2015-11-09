/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2015                                   |
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

#ifndef incl_HPHP_JIT_ABI_PPC64_H_
#define incl_HPHP_JIT_ABI_PPC64_H_

#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/bytecode.h"

#include "hphp/ppc64-asm/asm-ppc64.h"

namespace HPHP { namespace jit {

struct Abi;

namespace ppc64 {

///////////////////////////////////////////////////////////////////////////////
/*
 * Mirrors the API of abi.h.
 */

const Abi& abi(CodeKind kind = CodeKind::Trace);

/* VM registers must match etch-helpers.h definitions! */
constexpr PhysReg rvmfp()      { return ppc64_asm::reg::r28; }
constexpr PhysReg rvmsp()      { return ppc64_asm::reg::r29; }
constexpr PhysReg rvmtl()      { return ppc64_asm::reg::r30; }
constexpr PhysReg rsp()        { return ppc64_asm::reg::r1;  }
constexpr PhysReg rfuncln()    { return ppc64_asm::reg::r0;  }
constexpr PhysReg rthreadptr() { return ppc64_asm::reg::r13; }
constexpr PhysReg rfuncentry() { return ppc64_asm::reg::r12; }

// rone() returns register 27, which has the value "1" (Initiated in
// translator-asm-helpers.S).
// This is necessary for PPC64 since instructions like "inc" must updates the
// CR depending the instruction result and instructions like "addi" (using
// immediate) does not set the CR.
constexpr PhysReg rone()       { return ppc64_asm::reg::r27; }
constexpr PhysReg rbackchain() { return ppc64_asm::reg::r26; }

namespace detail {
  const RegSet kVMRegs      = rvmfp() | rvmtl() | rvmsp();
  const RegSet kVMRegsNoSP  = rvmfp() | rvmtl();
}

inline RegSet vm_regs_with_sp() { return detail::kVMRegs; }
inline RegSet vm_regs_no_sp()   { return detail::kVMRegsNoSP; }

PhysReg rret(size_t i = 0);
PhysReg rret_simd(size_t i);

PhysReg rarg(size_t i);
PhysReg rarg_simd(size_t i);

size_t num_arg_regs();
size_t num_arg_regs_simd();

constexpr PhysReg r_svcreq_req()  { return ppc64_asm::reg::r3; }
constexpr PhysReg r_svcreq_stub() { return ppc64_asm::reg::r8; }
PhysReg r_svcreq_sf();
PhysReg r_svcreq_arg(size_t i);

///////////////////////////////////////////////////////////////////////////////

/* Used on vasm for defining a minimal callstack on call/ret */
constexpr int min_callstack_size          = AROFF(_dummyB);   // next union
constexpr int lr_position_on_callstack    = AROFF(m_savedRip);
constexpr int rvmfp_position_on_callstack = 8;  // CR save area not in use

/* Parameters for push/pop and keep stack aligned */
constexpr int push_pop_position           = 8;

/*
 * Scratch register.
 */
constexpr Reg64 rAsm = ppc64_asm::reg::r11;
constexpr RegXMM rFasm = ppc64_asm::reg::f29;

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
