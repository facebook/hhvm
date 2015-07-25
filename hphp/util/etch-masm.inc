; This performs the same thing as etch-helpers.h, except
; that this is for MASM instead.
if 0
  ; The Linux / Mac ABI
  ; This is here to make testing to ensure
  ; no errors were made in translation much
  ; easier.
  ARG1 equ rdi
  ARG2 equ rsi
  ARG2_4 equ esi
  ARG3 equ rdx
  ARG3_4 equ edx
  ARG3_1 equ dl
  ARG4 equ rcx
  ARG4_4 equ ecx
  ARG4_1 equ cl
  ARG5 equ r8
  ARG6 equ r9
else
  ; The Windows ABI
  ARG1 equ rcx
  ARG2 equ rdx
  ARG2_4 equ edx
  ARG3 equ r8
  ARG3_4 equ r8d
  ARG3_1 equ r8b
  ARG4 equ r9
  ARG4_4 equ r9d
  ARG4_1 equ r9b
  ARG5 equ r10
  ARG6 equ r11
endif
