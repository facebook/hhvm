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

#include "hphp/runtime/vm/jit/abi.h"

#include "hphp/util/arch.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/abi-x64.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

const Abi& abi(CodeKind kind) { return ARCH_SWITCH_CALL(abi, kind); }

PhysReg rvmfp() { return ARCH_SWITCH_CALL(rvmfp); }
PhysReg rvmsp() { return ARCH_SWITCH_CALL(rvmsp); }
PhysReg rvmtl() { return ARCH_SWITCH_CALL(rvmtl); }
PhysReg rsp() { return ARCH_SWITCH_CALL(rsp); }

RegSet vm_regs_with_sp() { return ARCH_SWITCH_CALL(vm_regs_with_sp); }
RegSet vm_regs_no_sp() { return ARCH_SWITCH_CALL(vm_regs_no_sp); }
RegSet cross_jit_save() { return ARCH_SWITCH_CALL(cross_jit_save); }

PhysReg rret_data() { return ARCH_SWITCH_CALL(rret_data); }
PhysReg rret_type() { return ARCH_SWITCH_CALL(rret_type); }

PhysReg rret(size_t i) { return ARCH_SWITCH_CALL(rret, i); }
PhysReg rret_simd(size_t i) { return ARCH_SWITCH_CALL(rret_simd, i); }

PhysReg rarg(size_t i) { return ARCH_SWITCH_CALL(rarg, i); }
PhysReg rarg_simd(size_t i) { return ARCH_SWITCH_CALL(rarg_simd, i); }
PhysReg rarg_ind_ret(size_t i) { return ARCH_SWITCH_CALL(rarg_ind_ret, i); }

size_t num_arg_regs() { return ARCH_SWITCH_CALL(num_arg_regs); }
size_t num_arg_regs_simd() { return ARCH_SWITCH_CALL(num_arg_regs_simd); }
size_t num_arg_regs_ind_ret() { return ARCH_SWITCH_CALL(num_arg_regs_ind_ret); }

PhysReg r_svcreq_req() { return ARCH_SWITCH_CALL(r_svcreq_req); }
PhysReg r_svcreq_spoff() { return ARCH_SWITCH_CALL(r_svcreq_spoff); }
PhysReg r_svcreq_stub() { return ARCH_SWITCH_CALL(r_svcreq_stub); }
PhysReg r_svcreq_sf() { return ARCH_SWITCH_CALL(r_svcreq_sf); }
PhysReg r_svcreq_arg(size_t i) { return ARCH_SWITCH_CALL(r_svcreq_arg, i); }

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

RegSet arg_regs_ind_ret(size_t n) {
  RegSet regs;
  for (auto i = 0; i < n; i++) regs |= rarg_ind_ret(i);
  return regs;
}

///////////////////////////////////////////////////////////////////////////////

Vflags required_flags(jit::ConditionCode cc) {
  return ARCH_SWITCH_CALL(required_flags, cc);
}

///////////////////////////////////////////////////////////////////////////////

}}
