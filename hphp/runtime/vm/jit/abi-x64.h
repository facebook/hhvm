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
#include "hphp/runtime/vm/jit/reserved-stack.h"

namespace HPHP { namespace JIT { namespace X64 {

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

/*
 * Reserved for CodeGenerator.
 */
constexpr Reg64 rCgGP        = reg::r11;
constexpr RegXMM rCgXMM0     = reg::xmm6;
constexpr RegXMM rCgXMM1     = reg::xmm7;
constexpr RegXMM rCgXMM2     = reg::xmm5;

//////////////////////////////////////////////////////////////////////
/*
 * Registers used during a tracelet for program locations.
 *
 * These are partitioned into caller-saved and callee-saved regs
 * according to the x64 C abi.  These are all the registers that the
 * translator manages via its RegMap.
 */

const RegSet kGPCallerSaved = RegSet()
  | RegSet(reg::rax)
  | RegSet(reg::rcx)
  | RegSet(reg::rdx)
  | RegSet(reg::rsi)
  | RegSet(reg::rdi)
  | RegSet(reg::r8)
  | RegSet(reg::r9)
  ;

const RegSet kGPCalleeSaved = RegSet()
  | RegSet(reg::r13)
  | RegSet(reg::r14)
  | RegSet(reg::r15)
  ;

const RegSet kGPUnreserved = RegSet()
  | kGPCallerSaved
  | kGPCalleeSaved
  ;

const RegSet kGPReserved = RegSet()
  | RegSet(rVmSp)
  | RegSet(reg::rsp)
  | RegSet(rVmFp)
  | RegSet(rVmTl)
  | RegSet(rCgGP)
  | RegSet(rAsm)
  ;

const RegSet kGPRegs = RegSet()
  | kGPUnreserved
  | kGPReserved
  ;

const RegSet kXMMCallerSaved = RegSet()
  | RegSet(reg::xmm0)
  | RegSet(reg::xmm1)
  | RegSet(reg::xmm2)
  | RegSet(reg::xmm3)
  | RegSet(reg::xmm4)
  //| RegSet(reg::xmm5) rCgXmm2
  //| RegSet(reg::xmm6) rCgXMM0
  //| RegSet(reg::xmm7) rCgXMM1
  | RegSet(reg::xmm8)
  | RegSet(reg::xmm9)
  | RegSet(reg::xmm10)
  | RegSet(reg::xmm11)
  | RegSet(reg::xmm12)
  | RegSet(reg::xmm13)
  | RegSet(reg::xmm14)
  | RegSet(reg::xmm15)
  ;

const RegSet kXMMCalleeSaved = RegSet()
  ;

const RegSet kXMMUnreserved = RegSet()
  | kXMMCallerSaved
  | kXMMCalleeSaved
  ;

const RegSet kXMMReserved = RegSet()
  | RegSet(rCgXMM0)
  | RegSet(rCgXMM1)
  | RegSet(rCgXMM2)
  ;

const RegSet kCallerSaved = RegSet()
  | kGPCallerSaved
  | kXMMCallerSaved
  ;

const RegSet kCalleeSaved = RegSet()
  | kGPCalleeSaved
  | kXMMCalleeSaved
  ;

const RegSet kXMMRegs = RegSet()
  | kXMMUnreserved
  | kXMMReserved
  ;

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
 * A set of all special cross-tracelet registers.
 */
const RegSet kSpecialCrossTraceRegs
  = RegSet()
  | RegSet(rStashedAR)
  // These registers go through various states between tracelets, but
  // should all be considered special.
  | RegSet(rVmFp)
  | RegSet(rVmSp)
  | RegSet(rVmTl)
  ;

/*
 * Registers that can safely be used for scratch purposes in-between
 * traces.
 *
 * Note: there are portions of the func prologue code that will hit
 * assertions if you remove rax, rdx, or rcx from this set without
 * modifying them.
 */
const RegSet kScratchCrossTraceRegs
  = RegSet()
  | (kGPUnreserved - kSpecialCrossTraceRegs)
  | kXMMCallerSaved
  ;

//////////////////////////////////////////////////////////////////////
/*
 * Calling convention registers for service requests or calling C++.
 */

// x64 INTEGER class argument registers.
const PhysReg argNumToRegName[] = {
  reg::rdi, reg::rsi, reg::rdx, reg::rcx, reg::r8, reg::r9
};
const int kNumRegisterArgs = sizeof(argNumToRegName) / sizeof(PhysReg);

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
const PhysReg serviceReqArgRegs[] = {
  // rdi: contains request number
  reg::rsi, reg::rdx, reg::rcx, reg::r8, reg::r9
};
const int kNumServiceReqArgRegs =
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
  kCalleeSaved    // calleeSaved
};

//////////////////////////////////////////////////////////////////////

}}}

#endif
