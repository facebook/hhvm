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

///////////////////////////////////////////////////////////////////////////////
#if defined(__x86_64__)
  .byte 0
  .align 16
  .section .text
  .globl enterTCHelper
enterTCHelper:
  // Prologue
  .cfi_startproc
  push %rbp
  .cfi_adjust_cfa_offset 8  // offset to previous frame relative to %rsp
  .cfi_offset rbp, -16      // Where to find previous value of rbp

  // Set firstAR->m_savedRbp to point to this frame.
  mov %rsp, (%r8)

  // Save infoPtr
  push %rcx
  .cfi_adjust_cfa_offset 8

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

  sub $0x100, %rsp // kReservedRSPScratchSpace

  /*
   * If returning from a BIND_CALL request, push the return IP saved
   * in the ActRec pointed to by r15.  The 0x1 in the cmp instruction
   * must be kept in sync with REQ_BIND_CALL in abi-x64.h.
   */
  cmp $0x1, 0x0(%rcx)
  jne .LenterTCHelper$jumpToTC
  lea .LenterTCHelper$serviceReqLabel(%rip), %rax
  push %rax
  push 0x8(%r15)
  jmp *%rdx
.LenterTCHelper$jumpToTC:
  // May need cfi_adjust_cfa_offset annotations: Task #1747813
  call *%rdx
.LenterTCHelper$serviceReqLabel:

  add $0x100, %rsp // kReservedRSPScratchSpace
  // Restore infoPtr into %rbx
  pop %rbx
  .cfi_adjust_cfa_offset -8

  // Copy the values passed from jitted code into *infoPtr
  mov %rdi, 0x0(%rbx)
  test %rdi,%rdi
  jnz .LenterTCHelper$copyReqArgs
  .cfi_remember_state
  pop %rbp
  .cfi_restore rbp
  .cfi_adjust_cfa_offset -8
  ret

.LenterTCHelper$copyReqArgs:
  .cfi_restore_state
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
  .cfi_restore rbp
  .cfi_adjust_cfa_offset -8
  ret
  .cfi_endproc
///////////////////////////////////////////////////////////////////////////////
#elif defined(__AARCH64_EL__)
  .globl enterTCHelper
enterTCHelper:
  brk 0

#endif