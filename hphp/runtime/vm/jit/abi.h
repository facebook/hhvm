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

#pragma once

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi-regs.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Return a suitable ABI for the targeted architecture and `kind'.
 */
const Abi& abi(CodeKind kind = CodeKind::Trace);


///////////////////////////////////////////////////////////////////////////////
// Principal reserved registers.
//
// These registers have special purposes both during and between traces.

/*
 * Frame pointer.
 *
 * When mid-trace, points to the ActRec for the function currently executing.
 */
PhysReg rvmfp();

/*
 * Stack pointer.
 *
 * When mid-trace, points to the top of the eval stack (lowest valid address)
 * at the start of the current tracelet.
 */
PhysReg rvmsp();

/*
 * RDS base pointer.
 *
 * Always points to the base of the RDS block for the current request.
 */
PhysReg rvmtl();

/*
 * Native stack pointer.
 */
PhysReg rsp();


///////////////////////////////////////////////////////////////////////////////
// Calling convention registers.

/*
 * PHP return value registers.
 */
PhysReg rret_data();
PhysReg rret_type();

/*
 * Native return value registers.
 */
PhysReg rret(size_t i = 0);
PhysReg rret_simd(size_t i);

/*
 * Native argument registers.
 */
PhysReg rarg(size_t i);
PhysReg rarg_simd(size_t i);
PhysReg rarg_ind_ret(size_t i);

/*
 * Number of available argument registers.
 */
size_t num_arg_regs();
size_t num_arg_regs_simd();
size_t num_arg_regs_ind_ret();

/*
 * RegSet for a call with `n' arguments.
 */
RegSet arg_regs(size_t n);
RegSet arg_regs_simd(size_t n);
RegSet arg_regs_ind_ret(size_t n);

/*
 * Service request argument registers.
 */
PhysReg r_svcreq_req();
PhysReg r_svcreq_spoff();
PhysReg r_svcreq_stub();
PhysReg r_svcreq_sf();
PhysReg r_svcreq_arg(size_t i);

/*
 * PHP call registers (used by Call IR).
 *
 * - r_php_call_flags: see struct CallFlags
 * - r_php_call_func: the func being called
 * - r_php_call_num_args: the number of arguments being passed
 * - r_php_call_ctx: the $this/static:class context (TCtx)
 */
inline PhysReg r_php_call_flags() { return rarg(0); }
inline PhysReg r_php_call_func() { return rarg(1); }
inline PhysReg r_php_call_num_args() { return rarg(2); }
inline PhysReg r_php_call_ctx() { return rarg(3); }

///////////////////////////////////////////////////////////////////////////////
// JIT and TC boundary ABI registers.
//
// These registers should not be used for scratch purposes between tracelets,
// and have to be specially handled if we are returning to the interpreter or
// invoking the translator.

/*
 * VM register sets.  The other sets are defined relative to these.
 */
RegSet vm_regs_with_sp();
RegSet vm_regs_no_sp();

/*
 * Registers that need to be preserved across enterTC. Should not
 * include rvmfp, or anything else that is "naturally" preserved.
 */
RegSet cross_jit_save();

/*
 * Registers that are live between tracelets, in two flavors, depending whether
 * we are between tracelets in a resumed function.
 */
inline RegSet cross_trace_regs()          { return vm_regs_no_sp(); }
inline RegSet cross_trace_regs_resumed()  { return vm_regs_with_sp(); }

/*
 * Registers that are live when we reenter the JIT from the TC (e.g., via
 * service requests).
 */
inline RegSet leave_trace_regs() { return vm_regs_no_sp(); }

/*
 * Registers that are live between the caller and the callee when making a PHP
 * function call.
 */
inline RegSet php_call_regs(bool withCtx) {
  auto regs =
    vm_regs_with_sp() |
    r_php_call_flags() |
    r_php_call_func() |
    r_php_call_num_args();
  if (withCtx) regs |= r_php_call_ctx();
  return regs;
}

/*
 * Registers that are live after a PHP function return.
 *
 * TODO(#2288359): We don't want this to include rvmsp() eventually.
 */
inline RegSet php_return_regs() {
  return vm_regs_with_sp() | rret_data() | rret_type();
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Return the status flags that must be set when testing 'cc'.
 */
Vflags required_flags(ConditionCode cc);

///////////////////////////////////////////////////////////////////////////////

}}

