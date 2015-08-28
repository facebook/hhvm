/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/abi.h"

#include "hphp/runtime/base/arch.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/abi-x64.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

const Abi& abi(CodeKind kind) { ARCH_SWITCH_CALL(abi, kind); }

PhysReg rvmfp() { ARCH_SWITCH_CALL(rvmfp); }
PhysReg rvmsp() { ARCH_SWITCH_CALL(rvmsp); }
PhysReg rvmtl() { ARCH_SWITCH_CALL(rvmtl); }
PhysReg rsp() { ARCH_SWITCH_CALL(rsp); }

RegSet vm_regs_with_sp() { ARCH_SWITCH_CALL(vm_regs_with_sp); }
RegSet vm_regs_no_sp() { ARCH_SWITCH_CALL(vm_regs_no_sp); }

RegSet interp_one_cf_regs() { ARCH_SWITCH_CALL(interp_one_cf_regs); }

PhysReg rret() { ARCH_SWITCH_CALL(rret); }
PhysReg rarg(size_t i) { ARCH_SWITCH_CALL(rarg, i); }
PhysReg rarg_simd(size_t i) { ARCH_SWITCH_CALL(rarg_simd, i); }

size_t num_arg_regs() { ARCH_SWITCH_CALL(num_arg_regs); }
size_t num_arg_regs_simd() { ARCH_SWITCH_CALL(num_arg_regs_simd); }

PhysReg r_svcreq_req() { ARCH_SWITCH_CALL(r_svcreq_req); }
PhysReg r_svcreq_stub() { ARCH_SWITCH_CALL(r_svcreq_stub); }
PhysReg r_svcreq_sf() { ARCH_SWITCH_CALL(r_svcreq_sf); }
PhysReg r_svcreq_arg(size_t i) { ARCH_SWITCH_CALL(r_svcreq_arg, i); }

///////////////////////////////////////////////////////////////////////////////

RegSet arg_regs(size_t n) {
  RegSet regs;
  for (auto i = 0; i < n; i++) regs |= rarg(i);
  return regs;
}

RegSet arg_regs_simd(size_t n) {
  RegSet regs;
  for (auto i = 0; i < n; i++) regs |= rarg_simd(i);
  return regs;
}

///////////////////////////////////////////////////////////////////////////////

}}
