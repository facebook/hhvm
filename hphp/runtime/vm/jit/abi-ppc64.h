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

// optional in function linkage/used in function prologues
constexpr PhysReg rfuncln()    { return ppc64_asm::reg::r0;  }
// optional in function linkage/function entry address at global entry point
constexpr PhysReg rfuncentry() { return ppc64_asm::reg::r12; }

/*
 * Thread pointer, used to access the thread local storage section.
 *
 * See ABI for more info, page 112, Chapter 3.7.2 "TLS Runtime Handling"
 * https://members.openpowerfoundation.org/document/dl/576
 */
constexpr PhysReg rthreadptr() { return ppc64_asm::reg::r13; }

/*
 * Return register 27, which has the value "1" (initialized in enterTCHelper).
 *
 * This is necessary for PPC64 since instructions like "inc" must update the CR
 * depending on the instruction result, and instructions like "addi" (with
 * immediate) do not set the CR.
 */
constexpr PhysReg rone()       { return ppc64_asm::reg::r27; }

inline RegSet vm_regs_no_sp()   { return rvmfp() | rvmtl(); }
inline RegSet vm_regs_with_sp() { return vm_regs_no_sp() | rvmsp(); }

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

/*
 * Scratch registers.
 *
 * rAsm: a general purpose temporary register
 * rFasm: a floating point temporary register
 */
constexpr Reg64 rAsm = ppc64_asm::reg::r11;
constexpr RegXMM rFasm = ppc64_asm::reg::f15;

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
