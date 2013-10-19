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

#ifndef __APPLE__
#define CFI(x)         .cfi_##x
#define CFI2(x, y)     .cfi_##x y
#define CFI3C(x, y, z) .cfi_##x y##, z
#define ETCH_ALIGNMENT .align 16
#define ETCH_SECTION   .section .text
#define ETCH_NAME(x)   x
#define ETCH_LABEL(x)  .L##x
#else
#define CFI(x)         /* not used on OSX */
#define CFI2(x, y)     /* not used on OSX */
#define CFI3C(x, y, z) /* not used on OSX */
#define ETCH_ALIGNMENT .align 4 // on OSX this is 2^value
#define ETCH_SECTION   .section .text,
#define ETCH_NAME(x)   _##x
#define ETCH_LABEL(x)  .L##_##x
#endif

///////////////////////////////////////////////////////////////////////////////
#if defined(__x86_64__)
  .byte 0
  ETCH_ALIGNMENT
  ETCH_SECTION
  .globl ETCH_NAME(enterTCHelper)
ETCH_NAME(enterTCHelper:)
  // Prologue
  CFI(startproc)             // amongst other things, cfa reg is now rsp, and offset is 8
  push %rbp
  CFI2(adjust_cfa_offset, 8) // cfa is now 8 bytes further from rsp than it was before
  CFI3C(offset, rbp, -16)    // Where to find previous value of rbp, relative to cfa

  // Set firstAR->m_savedRbp to point to this frame.
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
ETCH_LABEL(enterTCHelper$jumpToTC:)
  call *%rdx
ETCH_LABEL(enterTCHelper$serviceReqLabel:)

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
  ret

ETCH_LABEL(enterTCHelper$copyReqArgs:)
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
///////////////////////////////////////////////////////////////////////////////
#elif defined(__AARCH64EL__)
  .globl enterTCHelper
enterTCHelper:
  brk 0

#endif
