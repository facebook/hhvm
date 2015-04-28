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

const RegSet kGPCallerSaved =
  reg::rax | reg::rcx | reg::rdx | reg::rsi | reg::rdi | reg::r8 | reg::r9;

const RegSet kGPCalleeSaved =
  reg::rbx | reg::r13 | reg::r14 | reg::r15;

const RegSet kGPUnreserved = kGPCallerSaved | kGPCalleeSaved;

const RegSet kGPReserved =
  reg::rsp | rVmFp | rVmTl | reg::r11 | rAsm;

const RegSet kGPRegs = kGPUnreserved | kGPReserved;

const RegSet kXMMCallerSaved =
  reg::xmm0 | reg::xmm1 | reg::xmm2 | reg::xmm3 |
  reg::xmm4 | // reg::xmm5 | reg::xmm6 | reg::xmm7 // for vasm
  reg::xmm8 | reg::xmm9 | reg::xmm10 | reg::xmm11 |
  reg::xmm12 | reg::xmm13 | reg::xmm14; // | reg::xmm15 // for vasm

const RegSet kXMMCalleeSaved;

const RegSet kXMMUnreserved = kXMMCallerSaved | kXMMCalleeSaved;

const RegSet kXMMReserved =
  reg::xmm5 | reg::xmm6 | reg::xmm7 | reg::xmm15; // for vasm

const RegSet kCallerSaved = kGPCallerSaved | kXMMCallerSaved;

const RegSet kCalleeSaved = kGPCalleeSaved | kXMMCalleeSaved;

const RegSet kSF = RegSet(RegSF{0});

const RegSet kXMMRegs = kXMMUnreserved | kXMMReserved;

//////////////////////////////////////////////////////////////////////
/*
 * Registers reserved for cross-tracelet ABI purposes.
 *
 * These registers should not be used for scratch purposes between
 * tracelets, and have to be specially handled if we are returning to
 * the interpreter.
 */

/*
 * When preparing to call a function prologue, the callee's frame
 * pointer (the new ActRec) is placed into this register.  rVmFp still
 * points to the caller's ActRec when the prologue is entered.
 */
constexpr PhysReg rStashedAR = reg::r15;

/*
 * Registers that are live between all tracelets.
 */
const RegSet kCrossTraceRegs =
  rVmFp | rVmSp | rVmTl;

/*
 * Registers that are live during a PHP function call, between the caller and
 * the callee.
 */
const RegSet kCrossCallRegs =
  kCrossTraceRegs | rStashedAR;

/*
 * Registers that can safely be used for scratch purposes in-between
 * traces.
 *
 * Note: there are portions of the func prologue code that will hit
 * assertions if you remove rax, rdx, or rcx from this set without
 * modifying them.
 */
const RegSet kScratchCrossTraceRegs = kXMMCallerSaved |
  (kGPUnreserved - kCrossCallRegs);

//////////////////////////////////////////////////////////////////////
/*
 * Calling convention registers for service requests or calling C++.
 */

// x64 INTEGER class argument registers.
const PhysReg argNumToRegName[] = {
  reg::rdi, reg::rsi, reg::rdx, reg::rcx, reg::r8, reg::r9
};
const int kNumRegisterArgs = sizeof(argNumToRegName) / sizeof(PhysReg);

inline RegSet argSet(int n) {
  RegSet regs;
  for (int i = 0; i < n; i++) {
    regs.add(argNumToRegName[i]);
  }
  return regs;
}

// x64 SSE class argument registers.
const PhysReg argNumToSIMDRegName[] = {
  reg::xmm0, reg::xmm1, reg::xmm2, reg::xmm3,
  reg::xmm4, reg::xmm5, reg::xmm6, reg::xmm7,
};
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
#define CONTOFF(nm) int(offsetof(c_Generator, nm))

UNUSED const Abi abi {
  kGPUnreserved,  // gpUnreserved
  kGPReserved,    // gpReserved
  kXMMUnreserved, // simdUnreserved
  kXMMReserved,   // simdReserved
  kCalleeSaved,   // calleeSaved
  kSF,            // sf
  true,           // canSpill
};

//////////////////////////////////////////////////////////////////////

}}}

#endif
