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

#include "hphp/runtime/vm/jit/abi-ppc64.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"

namespace HPHP { namespace jit { namespace ppc64 {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

namespace reg = ppc64_asm::reg;

const RegSet kGPRegs =
  reg::r0  | reg::r1  | reg::r2  | reg::r3 |
  reg::r4  | reg::r5  | reg::r6  | reg::r7  |
  reg::r8  | reg::r9  | reg::r10 | reg::r11 |
  reg::r12 | reg::r13 | reg::r14 | reg::r15 |
  reg::r16 | reg::r17 | reg::r18 | reg::r19 |
  reg::r20 | reg::r21 | reg::r22 | reg::r23 |
  reg::r24 | reg::r25 | reg::r26 | reg::r27 |
  reg::r28 | reg::r29 | reg::r30 | reg::r31;

const RegSet kGPCallerSaved =
  reg::r3 | reg::r4 | reg::r5 | reg::r6 |
  reg::r7 | reg::r8 | reg::r9 | reg::r10;
  // r0 is used in function linkage as rfuncln
  // r11 is used as a scratch register (rAsm)
  // r12 is used in function linkage

const RegSet kGPCalleeSaved =
  reg::r14 | reg::r15 | reg::r16 | reg::r17 | reg::r18 | reg::r19 | reg::r20 |
  reg::r21 | reg::r22 | reg::r23 | reg::r24 | reg::r25;

  // r27 is used as rsp
  // r29 is used as rvmsp
  // r26 is used as rvmtl
  // r30 is reserved as a PIC register
  // r31 is used as rvmfp
  // r28 is used as rone

const RegSet kGPReserved =
  rtoc() | rsp() | rvmfp() | rvmtl() | rvmsp() | rAsm | rsfp() | rfuncln() |
  rfuncentry() | rthreadptr() | rone() | r_svcreq_stub() | reg::r30;
const RegSet kGPUnreserved = kGPRegs - kGPReserved;

const RegSet kXMMRegs =
  reg::f0  | reg::f1  | reg::f2  | reg::f3  | reg::f4  | reg::f5  |
  reg::f6  | reg::f7  | reg::f8  | reg::f9  | reg::f10 | reg::f11 |
  reg::f12 | reg::f13 | reg::v16 | reg::v17 | reg::v18 | reg::v19 |
  reg::f14 | reg::f15 | reg::v20 | reg::v21 | reg::v22 |
  reg::v23 | reg::v24 | reg::v25 | reg::v26 | reg::v27 |
  reg::v28 | reg::v30 | reg::v31 | reg::v29;

const RegSet kXMMCallerSaved =
  reg::f0  | reg::f1  | reg::f2  | reg::f3  | reg::f4  | reg::f5  |
  reg::f6  | reg::f7  | reg::f8  | reg::f9  | reg::f10 | reg::f11 |
  reg::f12 | reg::f13 | reg::v16 | reg::v17 | reg::v18 | reg::v19;

const RegSet kXMMCalleeSaved =
  reg::f14 | reg::f15 | reg::v20 | reg::v21 | reg::v22 |
  reg::v23 | reg::v24 | reg::v25 | reg::v26 | reg::v27 |
  reg::v28 | reg::v30 | reg::v31;
  // v29 reserved for Vxls::m_tmp

const RegSet kXMMReserved = RegSet(reg::v29);
const RegSet kXMMUnreserved = kXMMRegs - kXMMReserved;

const RegSet kCallerSaved = kGPCallerSaved | kXMMCallerSaved;
const RegSet kCalleeSaved = kGPCalleeSaved | kXMMCalleeSaved;

const RegSet kSF = RegSet(RegSF{0});

///////////////////////////////////////////////////////////////////////////////

/*
 * Registers that can safely be used for scratch purposes in-between traces.
 */
const RegSet kScratchCrossTraceRegs =
  kXMMCallerSaved | (kGPUnreserved - vm_regs_with_sp());

/*
 * Helper code ABI registers.
 */
const RegSet kGPHelperRegs = ppc64::rAsm | reg::r10; //TODO
const RegSet kXMMHelperRegs; // TODO

///////////////////////////////////////////////////////////////////////////////

const Abi trace_abi {
  kGPUnreserved,
  kGPReserved,
  kXMMUnreserved,
  kXMMReserved,
  kCalleeSaved,
  kSF,
  true,
};

const Abi cross_trace_abi {
  trace_abi.gp() & kScratchCrossTraceRegs,
  trace_abi.gp() - kScratchCrossTraceRegs,
  trace_abi.simd() & kScratchCrossTraceRegs,
  trace_abi.simd() - kScratchCrossTraceRegs,
  trace_abi.calleeSaved & kScratchCrossTraceRegs,
  trace_abi.sf,
  false
};

const Abi helper_abi {
  kGPHelperRegs,
  trace_abi.gp() - kGPHelperRegs,
  kXMMHelperRegs,
  trace_abi.simd() - kXMMHelperRegs,
  trace_abi.calleeSaved,
  trace_abi.sf,
  false

};

constexpr PhysReg gp_args[] = {
  reg::r3, reg::r4, reg::r5, reg::r6, reg::r7, reg::r8, reg::r9, reg::r10
};

constexpr PhysReg simd_args[] = {
  reg::f1, reg::f2, reg::f3, reg::f4, reg::f5, reg::f6, reg::f7, reg::f8,
  reg::f9, reg::f10, reg::f11, reg::f12, reg::f13
};

constexpr PhysReg svcreq_args[] = {
  reg::r4, reg::r5, reg::r6, reg::r7
};

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

const Abi& abi(CodeKind kind) {
  switch (kind) {
    case CodeKind::Trace:
      return trace_abi;
    case CodeKind::CrossTrace:
      return cross_trace_abi;
    case CodeKind::Helper:
      return helper_abi;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

PhysReg rret(size_t i) {
  assertx(i < 2);
  return i == 0 ? reg::r3 : reg::r4;
}
PhysReg rret_simd(size_t i) {
  assertx(i == 0);
  return reg::f1;
}

PhysReg rarg(size_t i) {
  assertx(i < num_arg_regs());
  return gp_args[i];
}
PhysReg rarg_simd(size_t i) {
  assertx(i < num_arg_regs_simd());
  return simd_args[i];
}
PhysReg rarg_ind_ret(size_t i) {
  assertx(i < num_arg_regs_ind_ret());
  return PhysReg();
}

size_t num_arg_regs() {
  return sizeof(gp_args) / sizeof(PhysReg);
}
size_t num_arg_regs_simd() {
  return sizeof(simd_args) / sizeof(PhysReg);
}
size_t num_arg_regs_ind_ret() {
  return 0;
}


PhysReg r_svcreq_sf() {
  return abi().sf.choose();
}
PhysReg r_svcreq_arg(size_t i) {
  return svcreq_args[i];
}

///////////////////////////////////////////////////////////////////////////////

}}}
