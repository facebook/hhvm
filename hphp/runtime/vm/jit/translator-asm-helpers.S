/*
 * enterTCHelper
 *
 * This helper routine is written in assembly to take care of the details
 * when transferring control between jitted code and the translator.
 *   rdi / x0:  Cell* vm_sp
 *   rsi / x1:  Cell* vm_fp
 *   rdx / x2:  unsigned char* start
 *   rcx / x3:  TReqInfo* infoPtr
 *   r8  / x4:  ActRec* firstAR
 *   r9  / x5:  uint8_t* targetCacheBase
 */

#include "hphp/runtime/vm/jit/reserved-stack.h"
#include "hphp/runtime/vm/jit/etch-helpers.h"

///////////////////////////////////////////////////////////////////////////////
#if defined(__x86_64__)
  .byte 0
  ETCH_ALIGN16
  ETCH_SECTION(enterTCHelper)
  .globl ETCH_NAME(enterTCHelper)
ETCH_NAME(enterTCHelper):
  // Prologue
  CFI(startproc)             // amongst other things, cfa reg is now rsp, and offset is 8
  push %rbp
  CFI2(adjust_cfa_offset, 8) // cfa is now 8 bytes further from rsp than it was before
  CFI3C(offset, rbp, -16)    // Where to find previous value of rbp, relative to cfa

  // Set firstAR->m_sfp to point to this frame.
  mov %rsp, (%r8)

  // Save infoPtr
  push %rcx
  CFI2(adjust_cfa_offset, 8)

  // Set up special registers used for translated code.
  mov %rdi, %rbx          // rVmSp
  mov %r9, %r12           // rVmTl
  mov %rsi, %rbp          // rVmFp
  mov 0x30(%rcx), %r15    // rStashedAR saved across service requests

  /*
   * The translated code we are about to enter does not follow the
   * standard prologue of pushing rbp at entry, so we are purposely 8
   * bytes short of 16-byte alignment before this call instruction so
   * that the return address being pushed will make the native stack
   * 16-byte aligned.
   */

  sub $RESERVED_STACK_TOTAL_SPACE, %rsp
  CFI2(adjust_cfa_offset, RESERVED_STACK_TOTAL_SPACE)

  /*
   * If returning from a BIND_CALL request, push the return IP saved
   * in the ActRec pointed to by r15.  The 0x1 in the cmp instruction
   * must be kept in sync with REQ_BIND_CALL in abi-x64.h.
   */
  cmpl $0x1, 0x0(%rcx)
  jne ETCH_LABEL(enterTCHelper$jumpToTC)
  lea ETCH_LABEL(enterTCHelper$serviceReqLabel(%rip)), %rax
  push %rax
  push 0x8(%r15)
  jmp *%rdx
ETCH_LABEL(enterTCHelper$jumpToTC):
  call *%rdx
ETCH_LABEL(enterTCHelper$serviceReqLabel):

  /*
   * enterTCServiceReq is never called directly; this exists to give the jit
   * access to the address of the expected return address while in the TC.
   */
  .globl ETCH_NAME(enterTCServiceReq)
  ETCH_NAME(enterTCServiceReq):
  /*
   * Eager vm-reg save. Must match values in rds-header.h
   */
  mov %rbx, 16(%r12)
  mov %rbp, 32(%r12)
  add $RESERVED_STACK_TOTAL_SPACE, %rsp
  CFI2(adjust_cfa_offset, -RESERVED_STACK_TOTAL_SPACE)
  // Restore infoPtr into %rbx
  pop %rbx
  CFI2(adjust_cfa_offset, -8)

  // Copy the values passed from jitted code into *infoPtr
  mov %rdi, 0x0(%rbx)
  test %rdi,%rdi
  jnz ETCH_LABEL(enterTCHelper$copyReqArgs)
  CFI(remember_state)
  pop %rbp
  CFI2(restore, rbp)
  CFI2(adjust_cfa_offset, -8)

  /*
   * Like enterTCServiceReq, this is never called directly, but we need
   * access to its address.
   */
  .globl ETCH_NAME(enterTCReturn)
  ETCH_NAME(enterTCReturn):
  ret

ETCH_LABEL(enterTCHelper$copyReqArgs):
  CFI(restore_state)
  mov %rsi, 0x8(%rbx)
  mov %rdx, 0x10(%rbx)
  mov %rcx, 0x18(%rbx)
  mov %r8,  0x20(%rbx)
  mov %r9,  0x28(%rbx)

  // Service request "callee-saved".  (Returnee-saved?)
  mov %r15, 0x30(%rbx)

  // copy stub address into infoPtr->stubAddr
  mov %r10, 0x38(%rbx)

  // Epilogue
  pop %rbp
  CFI2(restore, rbp)
  CFI2(adjust_cfa_offset, -8)
  ret
  CFI(endproc)
  ETCH_SIZE(enterTCHelper)
///////////////////////////////////////////////////////////////////////////////
#elif defined(__AARCH64EL__)
  .globl enterTCHelper
enterTCHelper:
  .globl enterTCServiceReq
enterTCServiceReq:
  brk 0

#endif
