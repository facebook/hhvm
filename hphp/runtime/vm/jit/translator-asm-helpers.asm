;
; enterTCHelper - MSVC Edition
;
; This helper routine is written in assembly to take care of the details
; when transferring control between jitted code and the translator.
;
; Note that this is only used when compiling under MSVC. The .S version
; of this file is used by everything else.
;
; The columns are registers of Linux and Mac ABI / Windows ABI / ARM ABI.
;   rdi / rcx   / x0:  Cell* vm_sp
;   rsi / rdx   / x1:  Cell* vm_fp
;   rdx / r8    / x2:  unsigned char* start
;   rcx / r9    / x4:  ActRec* firstAR
;   r8  / stack / x5:  uint8_t* targetCacheBase
;   r9  / stack / x6:  ActRec* calleeAR
;
; Note that on Windows, ETCH_GET_ARG5/6 borrow r10/r11 respectively

.CODE
enterTCHelper PROC FRAME
  mov r10, [rsp + 28h]
  mov r11, [rsp + 30h]
  push rbp
  .pushframe
  .endprolog

  ; Set firstAR->m_sfp to point to this frame.
  mov [r9], rsp

  ; Set up special registers used for translated code.
  mov rbx, rcx ; rVmSp
  mov r12, r10 ; rVmTl
  mov rbp, rdx ; rVmFp

  sub rsp, 8

  ; If we're entering the TC at a function prologue, make it look like we got
  ; there via a callphp{} by pushing return addresses, setting the callee frame
  ; pointer, then jumping to the prologue. We leave the TC with a ret
  ; instruction, so if we enter it with a jmp, that will unbalance the RSB and
  ; cause tons of branch mispredictions in the frames above us. To avoid this,
  ; we get to the prologue by calling a stub that pops the return address
  ; pushed by the call and jumps to the prologue. This pushes a bogus address
  ; on the RSB but the ret to callToExit always mispredicts anyway, and this
  ; keeps the RSB balanced.
  test r11, r11
  jz enterTCHelper$callTC
  push IMAGEREL enterTCExit
  push QWORD PTR [r11 + 8]
  mov rbp, r11
  call enterTCHelper$prologue

enterTCHelper$callTC:
  ; The translated code we are about to enter does not follow the
  ; standard prologue of pushing rbp at entry, so we are purposely 8
  ; bytes short of 16-byte alignment before this call instruction so
  ; that the return address being pushed will make the native stack
  ; 16-byte aligned.
  call r8

; enterTCExit is never called directly; this exists to give the jit
; access to the address of the expected return address while in the TC.
public enterTCExit
enterTCExit LABEL PTR
  ; Eager vm-reg save. Must match values in rds-header.h
  mov [r12 + 10h], rbx
  mov [r12 + 20h], rbp
  add rsp, 8

  ; Epilogue
  pop rbp
  ret

enterTCHelper$prologue:
  pop rax
  jmp r8

enterTCHelper ENDP

; This is the mangled name of MCGenerator::handleServiceRequest, as
; we can't explicitly set the name of a member's mangle in MSVC.
?handleServiceRequest@MCGenerator@jit@HPHP@@QEAAPEAEAEAUReqInfo@svcreq@23@@Z PROTO

.DATA?
EXTERN mcg : QWORD

.CODE
; handleSRHelper: Translated code will jump to this stub to perform all
; service requests. It calls out to C++ to handle the request, then jumps
; to the returned address (which may be the callToExit stub).
handleSRHelper PROC
  ; Sync vmsp & vmfp
  mov [r12 + 10h], rbx
  mov [r12 + 20h], rbp

  ; Push a ServiceReqInfo struct onto the stack and call handleServiceRequest.
  push r8
  push rcx
  push rdx
  push rsi
  push r10
  push rdi

  ; call mcg->handleServiceRequest(%rsp)
  mov rcx, mcg
  mov rdx, rsp
  call ?handleServiceRequest@MCGenerator@jit@HPHP@@QEAAPEAEAEAUReqInfo@svcreq@23@@Z

  ; Pop the ServiceReqInfo off the stack.
  add rsp, 30h

  ; rVmTl was preserved by the callee, but vmsp and vmfp might've changed if
  ; we interpreted anything. Reload them.
  mov rbx, [r12 + 10h]
  mov rbp, [r12 + 20h]

  jmp rax
handleSRHelper ENDP
END
