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

#include "hphp/runtime/vm/jit/abi-x64.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"

namespace HPHP { namespace jit { namespace x64 {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

#if defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER)
const RegSet kGPCallerSaved =
  reg::rax | reg::rcx | reg::rdx |
  reg::r8  | reg::r9  | reg::r10 | reg::r11;

const RegSet kGPCalleeSaved =
  reg::rbx | reg::rsi | reg::rdi | reg::r13 | reg::r14 | reg::r15;
#else
const RegSet kGPCallerSaved =
  reg::rax | reg::rcx | reg::rdx | reg::rsi | reg::rdi |
  reg::r8  | reg::r9  | reg::r10 | reg::r11;

const RegSet kGPCalleeSaved =
  reg::rbx | reg::r12 | reg::r13 | reg::r14 | reg::r15;
#endif

const RegSet kGPUnreserved = kGPCallerSaved | kGPCalleeSaved;
const RegSet kGPReserved = x64::rsp() | x64::rvmfp() | x64::rvmtl();
const RegSet kGPRegs = kGPUnreserved | kGPReserved;

const RegSet kXMMCallerSaved =
  reg::xmm0  | reg::xmm1  | reg::xmm2  | reg::xmm3 |
  reg::xmm4  | reg::xmm5  | reg::xmm6  | reg::xmm7 |
  reg::xmm8  | reg::xmm9  | reg::xmm10 | reg::xmm11 |
  reg::xmm12 | reg::xmm13 | reg::xmm14 | reg::xmm15;

const RegSet kXMMCalleeSaved;

const RegSet kXMMUnreserved = kXMMCallerSaved | kXMMCalleeSaved;
const RegSet kXMMReserved;
const RegSet kXMMRegs = kXMMUnreserved | kXMMReserved;

const RegSet kCallerSaved = kGPCallerSaved | kXMMCallerSaved;
const RegSet kCalleeSaved = kGPCalleeSaved | kXMMCalleeSaved;

const RegSet kSF = RegSet(RegSF{0});

///////////////////////////////////////////////////////////////////////////////

/*
 * Registers that can safely be used for scratch purposes in-between traces.
 */
const RegSet kScratchCrossTraceRegs =
  kXMMCallerSaved | (kGPUnreserved - x64::vm_regs_with_sp());

/*
 * Helper code ABI registers.
 */
const RegSet kGPHelperRegs = x64::rAsm | reg::r11;
const RegSet kXMMHelperRegs = reg::xmm5 | reg::xmm6 | reg::xmm7;

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

///////////////////////////////////////////////////////////////////////////////

// x64 INTEGER class argument registers.
constexpr PhysReg gp_args[] = {
#if defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER)
  reg::rcx, reg::rdx, reg::r8, reg::r9
#else
  reg::rdi, reg::rsi, reg::rdx, reg::rcx, reg::r8, reg::r9
#endif
};

// x64 SSE class argument registers.
constexpr PhysReg simd_args[] = {
#if defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER)
  reg::xmm0, reg::xmm1, reg::xmm2, reg::xmm3,
#else
  reg::xmm0, reg::xmm1, reg::xmm2, reg::xmm3,
  reg::xmm4, reg::xmm5, reg::xmm6, reg::xmm7,
#endif
};

constexpr PhysReg svcreq_args[] = {
  reg::rsi, reg::rdx, reg::rcx, reg::r8
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
  return i == 0 ? reg::rax : reg::rdx;
}
PhysReg rret_simd(size_t i) {
  assertx(i == 0);
  return reg::xmm0;
}

PhysReg rarg(size_t i) {
  assertx(i < num_arg_regs());
  return gp_args[i];
}
PhysReg rarg_simd(size_t i) {
  assertx(i < num_arg_regs_simd());
  return simd_args[i];
}

size_t num_arg_regs() {
  return sizeof(gp_args) / sizeof(PhysReg);
}
size_t num_arg_regs_simd() {
  return sizeof(simd_args) / sizeof(PhysReg);
}

RegSet arg_regs(size_t n) {
  return jit::arg_regs(n);
}
RegSet arg_regs_simd(size_t n) {
  return jit::arg_regs_simd(n);
}

PhysReg r_svcreq_sf() {
  return abi().sf.choose();
}
PhysReg r_svcreq_arg(size_t i) {
  return svcreq_args[i];
}

///////////////////////////////////////////////////////////////////////////////

}}}
