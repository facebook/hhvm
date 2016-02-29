/*
 * enterTCHelper
 *
 * This helper routine is written in assembly to take care of the details
 * when transferring control between jitted code and the translator.
 *
 * Note that MSVC uses translator-asm-helpers.asm instead of this.
 *
 * The columns are registers of Linux and Mac ABI / Windows ABI / ARM ABI.
 *   rdi / rcx   / x0:  Cell* vm_sp
 *   rsi / rdx   / x1:  Cell* vm_fp
 *   rdx / r8    / x2:  unsigned char* start
 *   rcx / r9    / x4:  ActRec* firstAR
 *   r8  / stack / x5:  uint8_t* targetCacheBase
 *   r9  / stack / x6:  ActRec* calleeAR
 *
 * Note that on Windows, ETCH_GET_ARG5/6 borrow r10/r11 respectively
 */

#include "hphp/util/etch-helpers.h"

///////////////////////////////////////////////////////////////////////////////
#if defined(__x86_64__)
  .byte 0
  ETCH_ALIGN16
  ETCH_SECTION(enterTCHelper)
  .globl ETCH_NAME(enterTCHelper)
ETCH_NAME(enterTCHelper):
  // Prologue
  CFI(startproc)             // amongst other things, cfa reg is now rsp, and offset is 8

  // On Windows, get the 5th and 6th arguments from the stack.
  ETCH_GET_ARG5
  ETCH_GET_ARG6

  push ETCH_VMFP
  CFI2(adjust_cfa_offset, 8) // cfa is now 8 bytes further from rsp than it was before
  CFI3C(offset, rbp, -16)    // Where to find previous value of rbp, relative to cfa

  // Set firstAR->m_sfp to point to this frame.
  mov %rsp, (ETCH_ARG4)

  // Set up special registers used for translated code.
  mov ETCH_ARG1, ETCH_VMSP
  mov ETCH_ARG5, ETCH_VMTL
  mov ETCH_ARG2, ETCH_VMFP

  sub $8, %rsp // align native stack
  CFI2(adjust_cfa_offset, 8)

  /*
   * If we're entering the TC at a function prologue, make it look like we got
   * there via a callphp{} by pushing return addresses, setting the callee
   * frame pointer, then jumping to the prologue. We leave the TC with a ret
   * instruction, so if we enter it with a jmp, that will unbalance the RSB and
   * cause tons of branch mispredictions in the frames above us. To avoid this,
   * we get to the prologue by calling a stub that pops the return address
   * pushed by the call and jumps to the prologue. This pushes a bogus address
   * on the RSB but the ret to callToExit always mispredicts anyway, and this
   * keeps the RSB balanced.
   */
  test ETCH_ARG6, ETCH_ARG6
  jz ETCH_LABEL(enterTCHelper$callTC)
  push ETCH_NAME_REL(enterTCExit)
  push 0x8(ETCH_ARG6)
  mov ETCH_ARG6, ETCH_VMFP
  call ETCH_LABEL(enterTCHelper$prologue)

  /*
   * The translated code we are about to enter does not follow the
   * standard prologue of pushing rbp at entry, so we are purposely 8
   * bytes short of 16-byte alignment before this call instruction so
   * that the return address being pushed will make the native stack
   * 16-byte aligned.
   */
ETCH_LABEL(enterTCHelper$callTC):
  call *ETCH_ARG3

  /*
   * enterTCExit is never called directly; this exists to give the jit
   * access to the address of the expected return address while in the TC.
   */
  .globl ETCH_NAME(enterTCExit)
ETCH_NAME(enterTCExit):
  /*
   * Eager vm-reg save. Must match values in rds-header.h
   */
  mov ETCH_VMSP, 0x10(ETCH_VMTL)
  mov ETCH_VMFP, 0x20(ETCH_VMTL)
  add $8, %rsp
  CFI2(adjust_cfa_offset, -8)

  // Epilogue
  pop ETCH_VMFP
  CFI2(restore, rbp)
  CFI2(adjust_cfa_offset, -8)
  ret

ETCH_LABEL(enterTCHelper$prologue):
  pop %rax
  jmp *ETCH_ARG3

  CFI(endproc)
  ETCH_SIZE(enterTCHelper)

/*
 * handleSRHelper
 *
 * Translated code will jump to this stub to perform all service requests. It
 * calls out to C++ to handle the request, then jumps to the returned address
 * (which may be the callToExit stub).
 *
 * The columns are registers of:
 * X64 ABI
 *   rdi:  ServiceRequest req (r_svcreq_req)
 *   r10:  TCA stub (r_svcreq_stub)
 *   rsi:  args[0]
 *   rdx:  args[1]
 *   rcx:  args[2]
 *   r8 :  args[3]
 */
  ETCH_ALIGN16
  ETCH_SECTION(handleSRHelper)
  .globl ETCH_NAME(handleSRHelper)
ETCH_NAME(handleSRHelper):
  CFI(startproc)

  // Sync vmsp & vmfp
  mov ETCH_VMSP, 0x10(ETCH_VMTL)
  mov ETCH_VMFP, 0x20(ETCH_VMTL)

  // Push a ReqInfo struct onto the stack and call handleServiceRequest.
  push ETCH_SVCREQ_ARG4
  push ETCH_SVCREQ_ARG3
  push ETCH_SVCREQ_ARG2
  push ETCH_SVCREQ_ARG1
  push ETCH_SVCREQ_STUB
  push ETCH_SVCREQ_REQ
  CFI2(adjust_cfa_offset, 0x30)

  // call mcg->handleServiceRequest(%rsp)
  mov ETCH_NAME(mcg)(%rip), ETCH_ARG1
  mov %rsp, ETCH_ARG2
  call MCGenerator_handleServiceRequest

  // Pop the ReqInfo struct off the stack.
  add $0x30, %rsp
  CFI2(adjust_cfa_offset, -0x30)

  // rVmTl was preserved by the callee, but vmsp and vmfp might've changed if
  // we interpreted anything. Reload them.
  mov 0x10(ETCH_VMTL), ETCH_VMSP
  mov 0x20(ETCH_VMTL), ETCH_VMFP

  jmp *ETCH_RET1

  CFI(endproc)
  ETCH_SIZE(handleSRHelper)

///////////////////////////////////////////////////////////////////////////////
#elif defined(__aarch64__)
  .globl enterTCHelper
enterTCHelper:
  // Prologue
  stp x29, x30, [sp, -16]!

  // Set firstAR->m_sfp to point to this frame
  mov x9, sp
  str x9, [x3]

  // Set up special registers used for translated code
  mov x28, x0
  mov x27, x4
  mov x29, x1

  // Enter TC
  tst x5, x5
  beq enterTCHelper$callTC
  adr x9, enterTCExit
  ldr x10, [x5, #8]
  stp x10, x9, [sp, -16]!
  mov x29, x5
  ldr x30, [sp]
  br x2

enterTCHelper$callTC:
  // The translated does not follow the standard prologue of pushing frame
  // pointer and return address (x29, x30). So, push them first and branch
  adr x18, enterTCExit
  stp x29, x18, [sp, -16]!
  blr x2

  .globl enterTCExit
enterTCExit:
  // VM Reg save
  str x28, [x27, 16]
  str x29, [x27, 32]

  // Epilogue
  ldp x29, x30, [sp], 16
  ret

  .globl handleSRHelper
handleSRHelper:
  // Sync VMSP and VMFP
  str x28, [x27, 16]
  str x29, [x27, 32]

  // Push a ReqInfo struct onto the stack (arg4, arg3, arg2, arg1, stub, req)
  stp x4, x5, [sp, -16]!
  stp x2, x3, [sp, -16]!
  stp x0, x1, [sp, -16]!

  // call handleServiceRequest
  adrp x0, mcg
  add x0, x0, :lo12:mcg
  ldr x0, [x0]
  mov x1, sp
  adrp x18, MCGenerator_handleServiceRequest
  add x18, x18, :lo12:MCGenerator_handleServiceRequest
  blr x18

  // Pop the ReqInfo struct off the stack
  add sp, sp, 0x30

  // Load the VM regs
  ldr x28, [x27, 16]
  ldr x29, [x27, 32]

  // Jump back
  br x0

#elif defined(__powerpc64__)
ETCH_SECTION(handleSRHelper)
.globl ETCH_NAME(handleSRHelper)
ETCH_NAME(handleSRHelper):
  blr
ETCH_SIZE(handleSRHelper)

ETCH_SECTION(handleSRResumeTC)
.globl ETCH_NAME(handleSRResumeTC)
ETCH_NAME(handleSRResumeTC):
  blr
ETCH_SIZE(handleSRResumeTC)

ETCH_SECTION(enterTCServiceReq)
.globl ETCH_NAME(enterTCServiceReq)
ETCH_NAME(enterTCServiceReq):
  blr
ETCH_SIZE(enterTCServiceReq)

ETCH_SECTION(enterTCExit)
.globl ETCH_NAME(enterTCExit)
ETCH_NAME(enterTCExit):
  blr
ETCH_SIZE(enterTCExit)
#endif
