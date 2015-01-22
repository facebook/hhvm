/*
 * enterTCHelper
 *
 * This helper routine is written in assembly to take care of the details
 * when transferring control between jitted code and the translator.
 *   rdi / x0:  Cell* vm_sp
 *   rsi / x1:  Cell* vm_fp
 *   rdx / x2:  unsigned char* start
 *   rcx / x4:  ActRec* firstAR
 *   r8  / x5:  uint8_t* targetCacheBase
 *   r9  / x6:  ActRec* stashedAR
 */

#include "hphp/runtime/vm/jit/reserved-stack.h"
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
  push %rbp
  CFI2(adjust_cfa_offset, 8) // cfa is now 8 bytes further from rsp than it was before
  CFI3C(offset, rbp, -16)    // Where to find previous value of rbp, relative to cfa

  // Set firstAR->m_sfp to point to this frame.
  mov %rsp, (%rcx)

  // Set up special registers used for translated code.
  mov %rdi, %rbx          // rVmSp
  mov %r8,  %r12          // rVmTl
  mov %rsi, %rbp          // rVmFp

  sub $RESERVED_STACK_TOTAL_SPACE, %rsp
  CFI2(adjust_cfa_offset, RESERVED_STACK_TOTAL_SPACE)

  /*
   * If we're entering the TC at a function prologue, make it look like we got
   * there via a bindcall by pushing return addresses, setting rStashedAR, then
   * jumping to the prologue.
   */
  test %r9, %r9
  jz ETCH_LABEL(enterTCHelper$callTC)
  push $enterTCExit
  push 0x8(%r9)
  mov %r9, %r15
  jmp *%rdx

  /*
   * The translated code we are about to enter does not follow the
   * standard prologue of pushing rbp at entry, so we are purposely 8
   * bytes short of 16-byte alignment before this call instruction so
   * that the return address being pushed will make the native stack
   * 16-byte aligned.
   */
ETCH_LABEL(enterTCHelper$callTC):
  call *%rdx

  /*
   * enterTCExit is never called directly; this exists to give the jit
   * access to the address of the expected return address while in the TC.
   */
  .globl ETCH_NAME(enterTCExit)
ETCH_NAME(enterTCExit):
  /*
   * Eager vm-reg save. Must match values in rds-header.h
   */
  mov %rbx, 0x10(%r12)
  mov %rbp, 0x20(%r12)
  add $RESERVED_STACK_TOTAL_SPACE, %rsp
  CFI2(adjust_cfa_offset, -RESERVED_STACK_TOTAL_SPACE)

  // Epilogue
  pop %rbp
  CFI2(restore, rbp)
  CFI2(adjust_cfa_offset, -8)
  ret
  CFI(endproc)
  ETCH_SIZE(enterTCHelper)

  // handleSRHelper: Translated code will jump to this stub to perform all
  // service requests. It calls out to C++ to handle the request, then jumps
  // to the returned address (which may be the callToExit stub).
  ETCH_ALIGN16
  ETCH_SECTION(handleSRHelper)
  .globl ETCH_NAME(handleSRHelper)
ETCH_NAME(handleSRHelper):
  CFI(startproc)

  // Sync vmsp & vmfp
  mov %rbx, 0x10(%r12)
  mov %rbp, 0x20(%r12)

  // Push a ServiceReqInfo struct onto the stack and call handleServiceRequest.
  push %rax // padding
  push %r8
  push %rcx
  push %rdx
  push %rsi
  push %r15
  push %r10
  push %rdi
  CFI2(adjust_cfa_offset, 0x40)

  // call mcg->handleServiceRequest(%rsp)
  mov mcg(%rip), %rdi
  mov %rsp, %rsi
  call MCGenerator_handleServiceRequest

  // Load the service request we were processing from the ServiceReqInfo,
  // then pop it off the stack.
  mov (%rsp), %edi
  add $0x40, %rsp
  CFI2(adjust_cfa_offset, -0x40)

  // rVmTl and rStashedAR were preserved by the callee but vmsp and vmfp
  // might've changed if we interpreted anything. Reload them.
  mov 0x10(%r12), %rbx
  mov 0x20(%r12), %rbp

  // If we're coming back from a REQ_BIND_CALL, we have to push m_savedRip
  // from rStashedAR for the func prologue we're about to jump to.
  cmpl $0, %edi
  jne ETCH_LABEL(handleSRHelper$jumpToTC)
  push 0x8(%r15)
ETCH_LABEL(handleSRHelper$jumpToTC):
  jmp *%rax
  CFI(endproc)
  ETCH_SIZE(handleSRHelper)

///////////////////////////////////////////////////////////////////////////////
#elif defined(__AARCH64EL__)
  .globl enterTCHelper
enterTCHelper:
  .globl enterTCServiceReq
enterTCServiceReq:
  brk 0

#endif
