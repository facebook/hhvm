/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2015-2016                              |
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

constexpr PhysReg rvmfp()      { return ppc64_asm::reg::r31; }
constexpr PhysReg rvmsp()      { return ppc64_asm::reg::r29; }
constexpr PhysReg rvmtl()      { return ppc64_asm::reg::r26; }
constexpr PhysReg rsp()        { return ppc64_asm::reg::r27; }

// optional in function linkage/used in function prologues
constexpr PhysReg rfuncln()    { return ppc64_asm::reg::r0;  }
// optional in function linkage/function entry address at global entry point
constexpr PhysReg rfuncentry() { return ppc64_asm::reg::r12; }

/*
 * TOC ("Table of Contents")
 * Section that combines the functions of the GOT and the small data section.
 *
 * GOT ("Global Offset Table")
 * Section used to hold addresses for position independent code.
 *
 * The TOC section is accessed via the dedicated TOC pointer register, r2.
 */
constexpr PhysReg rtoc()       { return ppc64_asm::reg::r2;  }

/*
 * The native stack frame pointer, which has to be handled according to ABI.
 *
 * As PPC64 has no stack pointer nor push/pop operations, it can't be replaced
 * by rsp() register as it should be pointing to the current frame.
 */
constexpr PhysReg rsfp()       { return ppc64_asm::reg::r1;  }

/*
 * Thread pointer, used to access the thread local storage section.
 *
 * See ABI for more info, page 112, Chapter 3.7.2 "TLS Runtime Handling"
 * https://members.openpowerfoundation.org/document/dl/576
 */
constexpr PhysReg rthreadptr() { return ppc64_asm::reg::r13; }

/*
 * Return register 28, which has the value "1" (initialized in enterTCHelper).
 *
 * This is necessary for PPC64 since instructions like "inc" must update the CR
 * depending on the instruction result, and instructions like "addi" (with
 * immediate) do not set the CR.
 */
constexpr PhysReg rone()       { return ppc64_asm::reg::r28; }

inline RegSet vm_regs_no_sp()   { return rvmfp() | rvmtl(); }
inline RegSet vm_regs_with_sp() { return vm_regs_no_sp() | rvmsp(); }

constexpr PhysReg rret_data() { return ppc64_asm::reg::r3; }
constexpr PhysReg rret_type() { return ppc64_asm::reg::r4; }

PhysReg rret(size_t i = 0);
PhysReg rret_simd(size_t i);

PhysReg rarg(size_t i);
PhysReg rarg_simd(size_t i);
PhysReg rarg_ind_ret(size_t i);

size_t num_arg_regs();
size_t num_arg_regs_simd();
size_t num_arg_regs_ind_ret();

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

inline Vflags required_flags(jit::ConditionCode /*cc*/) {
  return 0xff;
}

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
