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

#include "hphp/runtime/vm/jit/abi-arm.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"

namespace HPHP { namespace jit { namespace arm {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

const RegSet kGPCallerSaved =
  vixl::x0 | vixl::x1 | vixl::x2 | vixl::x3 |
  vixl::x4 | vixl::x5 | vixl::x6 | vixl::x7 |
  vixl::x8 |
  // x9  = rAsm
  vixl::x10 | vixl::x11 | vixl::x12 | vixl::x13 | vixl::x14 | vixl::x15 |
  // x16 = rHostCallReg, used as ip0/tmp0 by MacroAssembler
  // x17 = used as ip1/tmp1 by MacroAssembler
  vixl::x18;

const RegSet kGPCalleeSaved =
  // x19 = rvmsp()
  // x20 = rvmtl()
  vixl::x21 | vixl::x22 | vixl::x23 | vixl::x24 |
  vixl::x25 | vixl::x26 | vixl::x27 | vixl::x28;
  // x29 = rvmfp()
  // x30 = rLinkReg

const RegSet kGPUnreserved = kGPCallerSaved | kGPCalleeSaved;

const RegSet kGPReserved =
  rAsm | rHostCallReg | vixl::x17 | rvmsp() | rvmtl() | rvmfp() | rLinkReg |
  // ARM machines really only have 32 GP regs.  However, vixl has 33 separate
  // register codes, because it treats the zero register and stack pointer
  // (which are really both register 31) separately.  Rather than lose this
  // distinction in vixl (it's really helpful for avoiding stupid mistakes), we
  // sacrifice the ability to represent all 32 SIMD regs, and pretend there are
  // 33 GP regs.
  vixl::xzr | vixl::sp; // 31 is the encoded register number for zr and sp

const RegSet kSIMDCallerSaved =
  vixl::d0 | vixl::d1 | vixl::d2 | vixl::d3 |
  vixl::d4 | vixl::d5 | vixl::d6 | vixl::d7 |
  // d8-15 are callee-saved
  vixl::d16 | vixl::d17 | vixl::d18 | vixl::d19 |
  vixl::d20 | vixl::d21 | vixl::d22 | vixl::d23 |
  vixl::d24 | vixl::d25 | vixl::d26 | vixl::d27 |
  vixl::d28 | vixl::d29;
  // we don't use d30 and d31 because BitSet can't represent them

const RegSet kSIMDCalleeSaved =
  vixl::d8 | vixl::d9 | vixl::d10 | vixl::d11 |
  vixl::d12 | vixl::d13 | vixl::d14 | vixl::d15;

const RegSet kSIMDUnreserved = kSIMDCallerSaved | kSIMDCalleeSaved;
const RegSet kSIMDReserved;

const RegSet kCalleeSaved = kGPCalleeSaved | kSIMDCalleeSaved;

const RegSet kSF = RegSet(RegSF{0});

///////////////////////////////////////////////////////////////////////////////

const RegSet kScratchCrossTraceRegs =
  kSIMDCallerSaved | (kGPUnreserved - arm::vm_regs_with_sp());

///////////////////////////////////////////////////////////////////////////////

const Abi trace_abi {
  kGPUnreserved,
  kGPReserved,
  kSIMDUnreserved,
  kSIMDReserved,
  kCalleeSaved,
  kSF
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

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

const Abi& abi(CodeKind kind) {
  switch (kind) {
    case CodeKind::Trace:
      return trace_abi;
    case CodeKind::CrossTrace:
    case CodeKind::Helper:
      return cross_trace_abi;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

PhysReg rret(size_t i) {
  assertx(i < 2);
  return i == 0 ? vixl::x0 : vixl::x1;
}
PhysReg rret_simd(size_t i) {
  assertx(i == 0);
  return vixl::d0;
}

PhysReg rarg(size_t i) {
  assertx(i < num_arg_regs());
  return vixl::Register::XRegFromCode(i);
}
PhysReg rarg_simd(size_t i) {
  not_implemented();
}

RegSet arg_regs(size_t n) {
  return jit::arg_regs(n);
}
RegSet arg_regs_simd(size_t n) {
  return jit::arg_regs_simd(n);
}

PhysReg r_svcreq_req() { return rarg(0); }
PhysReg r_svcreq_stub() { return rarg(1); }
PhysReg r_svcreq_sf() { return abi().sf.choose(); }
PhysReg r_svcreq_arg(size_t i) { return rarg(i + 2); }

///////////////////////////////////////////////////////////////////////////////

}}}
