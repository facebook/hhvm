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

#ifndef incl_HPHP_JIT_ABI_X64_H_
#define incl_HPHP_JIT_ABI_X64_H_

#include "hphp/runtime/vm/jit/abi-regs.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

#include "hphp/util/asm-x64.h"

namespace HPHP { namespace jit {

struct Abi;

namespace x64 {

///////////////////////////////////////////////////////////////////////////////

/*
 * Mirrors the API of abi.h.
 */

const Abi& abi(CodeKind kind = CodeKind::Trace);

/* VM registers must match etch-helpers.h definitions! */
constexpr PhysReg rvmfp() { return reg::rbp; }
constexpr PhysReg rvmsp() { return reg::rbx; }
constexpr PhysReg rvmtl() { return reg::r12; }
constexpr PhysReg rsp()   { return reg::rsp; }

inline RegSet vm_regs_no_sp()   { return rvmfp() | rvmtl(); }
inline RegSet vm_regs_with_sp() { return vm_regs_no_sp() | rvmsp(); }

PhysReg rret(size_t i = 0);
PhysReg rret_simd(size_t i);

PhysReg rarg(size_t i);
PhysReg rarg_simd(size_t i);

size_t num_arg_regs();
size_t num_arg_regs_simd();

RegSet arg_regs(size_t n);
RegSet arg_regs_simd(size_t n);

constexpr PhysReg r_svcreq_req()  { return reg::rdi; }
constexpr PhysReg r_svcreq_stub() { return reg::r10; }
PhysReg r_svcreq_sf();
PhysReg r_svcreq_arg(size_t i);

///////////////////////////////////////////////////////////////////////////////

/*
 * Scratch register.
 */
constexpr Reg64 rAsm = reg::r10;

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
