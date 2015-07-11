/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

/*
 * Enumerations and constants defining the binary interface between
 * tracelets.
 *
 * Most changes here will likely require corresponding changes in
 * __enterTCHelper and other parts of mc-generator.cpp and the IR
 * translator.
 */

#ifndef incl_HPHP_VM_RUNTIME_TRANSLATOR_ABI_X64_H_
#define incl_HPHP_VM_RUNTIME_TRANSLATOR_ABI_X64_H_

#include "hphp/util/asm-x64.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

namespace HPHP { namespace jit { namespace x64 {

//////////////////////////////////////////////////////////////////////
/*
 * Principal reserved registers.
 *
 * These registers have special purposes both during and between
 * traces.
 */

/*
 * Frame pointer.  When mid-trace, points to the ActRec for the
 * function currently executing.
 */
constexpr PhysReg rVmFp      = reg::rbp;

/*
 * Stack pointer.  When mid-trace, points to the top of the eval stack
 * (lowest valid address) at the start of the current tracelet.
 */
constexpr PhysReg rVmSp      = reg::rbx;

/*
 * RDS base pointer.  Always points to the base of the RDS block for
 * the current request.
 */
constexpr PhysReg rVmTl      = reg::r12;

/*
 * scratch register
 */
constexpr Reg64 rAsm         = reg::r10;

//////////////////////////////////////////////////////////////////////
/*
 * Registers used during a tracelet for program locations.
 *
 * These are partitioned into caller-saved and callee-saved regs
 * according to the x64 C abi.  These are all the registers that the
 * translator manages via its RegMap.
 */

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
  reg::rbx | reg::r13 | reg::r14 | reg::r15;
#endif

const RegSet kGPUnreserved = kGPCallerSaved | kGPCalleeSaved;

const RegSet kGPReserved = reg::rsp | rVmFp | rVmTl;

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

//////////////////////////////////////////////////////////////////////
/*
 * Registers reserved for cross-tracelet ABI purposes.
 *
 * These registers should not be used for scratch purposes between
 * tracelets, and have to be specially handled if we are returning to
 * the interpreter.
 */

/*
 * Registers that are live between tracelets, in two flavors, depending whether
 * we are between tracelets in a resumed function.
 */
const RegSet kCrossTraceRegs        = rVmFp | rVmTl;
const RegSet kCrossTraceRegsResumed = kCrossTraceRegs | rVmSp;

/*
 * Registers live on entry to the fcallArrayHelper.
 *
 * TODO(#2288359): we don't want this to include rVmSp eventually.
 */
const RegSet kCrossTraceRegsFCallArray = kCrossTraceRegs | rVmSp;

/*
 * Registers live on entry to an interpOneCFHelper.
 */
const RegSet kCrossTraceRegsInterpOneCF = kCrossTraceRegs | rVmSp | rAsm;

/*
 * Registers that are live after a PHP function return.
 *
 * TODO(#2288359): we don't want this to include rVmSp eventually.
 */
const RegSet kCrossTraceRegsReturn = kCrossTraceRegs | rVmSp;

/*
 * Registers that are live during a PHP function call, between the caller and
 * the callee.
 */
const RegSet kCrossCallRegs = kCrossTraceRegs;

/*
 * Registers that can safely be used for scratch purposes in-between
 * traces.
 *
 * Note: there are portions of the func prologue code that will hit
 * assertions if you remove rax, rdx, or rcx from this set without
 * modifying them.
 */
const RegSet kScratchCrossTraceRegs = kXMMCallerSaved |
  (kGPUnreserved - (kCrossTraceRegs | kCrossTraceRegsResumed));

//////////////////////////////////////////////////////////////////////
/*
 * Calling convention registers for service requests or calling C++.
 */

// x64 INTEGER class argument registers.
#if defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER)
const PhysReg argNumToRegName[] = {
  reg::rcx, reg::rdx, reg::r8, reg::r9
};
#else
const PhysReg argNumToRegName[] = {
  reg::rdi, reg::rsi, reg::rdx, reg::rcx, reg::r8, reg::r9
};
#endif
const int kNumRegisterArgs = sizeof(argNumToRegName) / sizeof(PhysReg);

inline RegSet argSet(int n) {
  RegSet regs;
  for (int i = 0; i < n; i++) {
    regs.add(argNumToRegName[i]);
  }
  return regs;
}

// x64 SSE class argument registers.
#if defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER)
const PhysReg argNumToSIMDRegName[] = {
  reg::xmm0, reg::xmm1, reg::xmm2, reg::xmm3,
};
#else
const PhysReg argNumToSIMDRegName[] = {
  reg::xmm0, reg::xmm1, reg::xmm2, reg::xmm3,
  reg::xmm4, reg::xmm5, reg::xmm6, reg::xmm7,
};
#endif
const int kNumSIMDRegisterArgs = sizeof(argNumToSIMDRegName) / sizeof(PhysReg);

/*
 * JIT'd code "reverse calls" the enterTC routine by returning to it,
 * with a service request number and arguments.
 */
constexpr PhysReg serviceReqArgRegs[] = {
  // rdi: contains request number
  reg::rsi, reg::rdx, reg::rcx, reg::r8
};
constexpr int kNumServiceReqArgRegs =
  sizeof(serviceReqArgRegs) / sizeof(PhysReg);

/*
 * Some data structures are accessed often enough from translated code
 * that we have shortcuts for getting offsets into them.
 */
#define TVOFF(nm) int(offsetof(TypedValue, nm))
#define AROFF(nm) int(offsetof(ActRec, nm))
#define AFWHOFF(nm) int(offsetof(c_AsyncFunctionWaitHandle, nm))
#define GENDATAOFF(nm) int(offsetof(Generator, nm))

UNUSED const Abi abi {
  .gpUnreserved   = kGPUnreserved,
  .gpReserved     = kGPReserved,
  .simdUnreserved = kXMMUnreserved,
  .simdReserved   = kXMMReserved,
  .calleeSaved    = kCalleeSaved,
  .sf             = kSF,
  .canSpill       = true,
};

UNUSED const Abi cross_trace_abi {
  .gpUnreserved   = abi.gp() & kScratchCrossTraceRegs,
  .gpReserved     = abi.gp() - kScratchCrossTraceRegs,
  .simdUnreserved = abi.simd() & kScratchCrossTraceRegs,
  .simdReserved   = abi.simd() - kScratchCrossTraceRegs,
  .calleeSaved    = abi.calleeSaved & kScratchCrossTraceRegs,
  .sf             = abi.sf,
  .canSpill       = false
};

//////////////////////////////////////////////////////////////////////

}}}

#endif
