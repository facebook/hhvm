/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
 * __enterTCHelper and other parts of translator-x64.cpp and the IR
 * translator.
 */

#ifndef incl_VM_RUNTIME_TRANSLATOR_ABI_X64_H_
#define incl_VM_RUNTIME_TRANSLATOR_ABI_X64_H_

#include "util/asm-x64.h"
#include "runtime/vm/translator/regalloc.h"

namespace HPHP { namespace VM { namespace Transl {

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
 * Target cache pointer.  Always points to the base of the target
 * cache block for the current request.
 */
constexpr PhysReg rVmTl      = reg::r12;

//////////////////////////////////////////////////////////////////////
/*
 * Registers used during a tracelet for program locations.
 *
 * These are partitioned into caller-saved and callee-saved regs
 * according to the x64 C abi.  These are all the registers that the
 * translator manages via its RegMap.
 */

const RegSet kCallerSaved = RegSet()
                          | RegSet(reg::rax)
                          | RegSet(reg::rcx)
                          | RegSet(reg::rdx)
                          | RegSet(reg::rsi)
                          | RegSet(reg::rdi)
                          | RegSet(reg::r8)
                          | RegSet(reg::r9)
                          // r10 is reserved by the assembler, and for
                          // various extremely-specific scratch uses.
                          | RegSet(reg::r11)
                          ;

const RegSet kCalleeSaved = RegSet()
                            // r12 is reserved for rVmTl
                          | RegSet(reg::r13)
                          | RegSet(reg::r14)
                          | RegSet(reg::r15);

const RegSet kAllRegs     = kCallerSaved | kCalleeSaved;

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
  | RegSet(rVmFp) | RegSet(rVmSp) | RegSet(rVmTl)
  ;

/*
 * Registers that can safely be used for scratch purposes in-between
 * traces.
 *
 * Note: there are portions of the func prologue code that will hit
 * assertions if you remove rax, rdx, or rcx from this set without
 * modifying them.
 */
const RegSet kScratchCrossTraceRegs = kAllRegs - kSpecialCrossTraceRegs;

//////////////////////////////////////////////////////////////////////
/*
 * Calling convention registers for service requests or calling C++.
 */

// x64 C argument registers.
const PhysReg argNumToRegName[] = {
  reg::rdi, reg::rsi, reg::rdx, reg::rcx, reg::r8, reg::r9
};
const int kNumRegisterArgs = sizeof(argNumToRegName) / sizeof(PhysReg);

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

#define SERVICE_REQUESTS \
  /*
   * Return from this nested VM invocation to the previous invocation.
   * (Ending the program if there is no previous invocation.)
   */ \
  REQ(EXIT) \
  \
  /*
   * BIND_* all are requests for the first time a call, jump, or
   * whatever is needed.  This generally involves translating new code
   * and then patching an address supplied as a service request
   * argument.
   */ \
  REQ(BIND_CALL)         \
  REQ(BIND_JMP)          \
  REQ(BIND_JCC)          \
  REQ(BIND_ADDR)         \
  REQ(BIND_SIDE_EXIT)    \
  REQ(BIND_JMPCC_FIRST)  \
  REQ(BIND_JMPCC_SECOND) \
  REQ(BIND_REQUIRE) \
  \
  /*
   * BIND_JMP_NO_IR is similar to BIND_JMP except that, if a new translation
   * needs to be generated, it'll force that HHIR is not used.
   * This is only used when HHIR is turned on.
   */ \
  REQ(BIND_JMP_NO_IR)  \
  \
  /*
   * When all translations don't support the incoming types, a
   * retranslate request is made.
   */ \
  REQ(RETRANSLATE) \
  \
  /*
   * If the max translations is reached for a SrcKey, the last
   * translation in the chain will jump to an interpret request stub.
   * This instructs enterTC to punt to the interpreter.
   */ \
  REQ(INTERPRET) \
  \
  /*
   * When the interpreter pushes an ActRec, the return address for
   * this ActRec will be set to a stub that raises POST_INTERP_RET,
   * since it doesn't have a TCA to return to.
   *
   * This request is raised in the case that translated machine code
   * executes the RetC for a frame that was pushed by the interpreter.
   */ \
  REQ(POST_INTERP_RET) \
  \
  /*
   * Raised when the execution stack overflowed.
   */ \
  REQ(STACK_OVERFLOW) \
  \
  /*
   * When HHIR is in use, this requests a retranslation that does not use HHIR.
   * This is only used when HHIR is turned on.
   *
   * Note that, when EvalJitUseIR is enabled, RETRANSLATE requests will attempt
   * to use HHIR.
   */ \
  REQ(RETRANSLATE_NO_IR) \
  \
  /*
   * Resume restarts execution at the current PC.  This is used after
   * an interpOne of an instruction that changes the PC, and in some
   * cases with FCall.
   */ \
  REQ(RESUME)

enum ServiceRequest {
#define REQ(nm) REQ_##nm,
  SERVICE_REQUESTS
#undef REQ
};

//////////////////////////////////////////////////////////////////////

// Set of all the x64 registers.
const RegSet kAllX64Regs = RegSet(kAllRegs).add(reg::r10)
                         | kSpecialCrossTraceRegs;
const int kNumX64Regs = 16;

/*
 * Some data structures are accessed often enough from translated code
 * that we have shortcuts for getting offsets into them.
 */
#define TVOFF(nm) offsetof(TypedValue, nm)
#define AROFF(nm) offsetof(ActRec, nm)
#define CONTOFF(nm) offsetof(c_Continuation, nm)

//////////////////////////////////////////////////////////////////////

/*
 * This much space (in bytes) at 8(%rsp) is allocated on entry to the
 * TC and made available for scratch purposes (right above the return
 * address).  It is used as spill locations by HHIR (see LinearScan),
 * and for MInstrState in translator-x64-vector.cpp.
 */
const size_t kReservedRSPScratchSpace = 0x80;

//////////////////////////////////////////////////////////////////////

}}}

#endif
